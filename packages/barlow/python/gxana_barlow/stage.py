"""`gxana run barlow`: the Barlow cut-variation check, driven by
analyses/<channel>/config/barlow.yaml. Legacy chain in brackets:

  trees   gxana_barlow_trees           [GetVariationTreesUML.C, variation trees]
  check   gxana_barlow_trees --check   [GetVariationTreesUML.C, yield fits; opt-in]
  bin     gxana_xsec_bin variation     [SplitVariationTrees.C]
  tables  gxana_xsec_tables            [GetXSecFilesUML.C, fit JohnsonMCShapeSyst]
  weight  gxana_xsection.weighted_average per variation  [GetWeightedXSecFiles.py]
  plot    gxana_barlow_plot            [PlotXSecBarlow*.C]

`trees` writes <output_dir>/variations.json once every trees command has succeeded; every
later step takes its variation ids from it and stops if the config changed since (a missing
file is written from the config with a note).
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana import config
from gxana.bins import energy_args, energy_bins, flatten_t_bins
from gxana.stages import xsection as xs
from gxana.stages.runner import Command, Env, Runner, check_steps, executable, num, python_module, run_steps
from gxana_barlow import config as bconfig
from gxana_barlow import manifest
from gxana_barlow.variations import Variation, expand

STEPS = ("trees", "check", "bin", "tables", "weight", "plot")
DEFAULT_STEPS = ("trees", "bin", "tables", "weight", "plot")

def _output_dir(cfg: Dict[str, Any], environ: Env) -> str:
    return config.expand_env(bconfig.block(cfg)["output_dir"], environ)


def _xs_output(cfg: Dict[str, Any], environ: Env) -> str:
    return config.expand_env(config.require(config.require(cfg, "xsection"), "output_dir"), environ)


def _stems(cfg: Dict[str, Any]) -> List[Tuple[str, str]]:
    return [(period, config.tree_stem(cfg, period, "data")) for period in config.require(cfg, "periods")]


def _families(variations: Sequence[Variation]) -> List[Tuple[str, List[Variation]]]:
    groups: Dict[str, List[Variation]] = {}
    for v in variations:
        groups.setdefault(v.family, []).append(v)
    return list(groups.items())


def _path(template: str, environ: Env, **fields: str) -> str:
    return config.expand_env(template, environ).format(**fields)


def _variation_file(bcfg: Dict[str, Any], output_dir: str, stem: str, family: str) -> str:
    rel = bcfg["trees"]["output"].format(stem=stem, family=family, mc_sample=bcfg["mc_sample"])
    return f"{output_dir}/{rel}"


def _binned(variation_file: str) -> str:
    p = Path(variation_file)
    return str(p.parent / f"binned_{p.name}")


def _plan_trees(cfg, bcfg, output_dir, variations, environ) -> List[Command]:
    exe = executable("gxana_barlow_trees", environ)
    trees = bcfg["trees"]
    mc = bcfg["mc_sample"]
    filters = trees.get("filters") or {}
    commands = []
    for family, group in _families(variations):
        for _, stem in _stems(cfg):
            argv = [exe, "--tree", trees["tree"],
                    "--input", _path(trees["input"], environ, stem=stem, mc_sample=mc),
                    "--input-mc", _path(trees["input_mc"], environ, stem=stem, mc_sample=mc)]
            for name, expr in (trees.get("defines") or {}).items():
                argv += ["--define", f"{name}={expr}"]
            for expr in filters.get("data") or []:
                argv += ["--filter-data", expr]
            for expr in filters.get("mc") or []:
                argv += ["--filter-mc", expr]
            for branch in trees["branches"]:
                argv += ["--branch", branch]
            for v in group:
                argv += ["--variation", f"{v.tree}={v.cut}"]
            argv += ["--out", _variation_file(bcfg, output_dir, stem, family),
                     "--threads", str(trees.get("threads", 0))]
            commands.append(Command(argv, "trees"))
    return commands


def _plan_check(cfg, bcfg, output_dir, variations, environ) -> List[Command]:
    exe = executable("gxana_barlow_trees", environ)
    check = bcfg["check"]
    mc = bcfg["mc_sample"]
    commands = []
    for family, group in _families(variations):
        for _, stem in _stems(cfg):
            argv = [exe, "--check", "--tree", bcfg["trees"]["tree"],
                    "--out", _variation_file(bcfg, output_dir, stem, family),
                    "--nominal", _path(check["nominal"], environ, stem=stem, mc_sample=mc),
                    "--nominal-mc", _path(check["nominal_mc"], environ, stem=stem, mc_sample=mc),
                    "--name", f"flatTree_{stem}", "--weight", bcfg["weight"],
                    "--yields", f"{output_dir}/output_yields.txt", "--fit-dir", f"{output_dir}/fits"]
            for v in group:
                argv += ["--variation", f"{v.tree}={v.cut}"]
            commands.append(Command(argv, "check"))
    return commands


def _plan_bin(cfg, bcfg, output_dir, variations, environ) -> List[Command]:
    exe = executable("gxana_xsec_bin", environ)
    energy = ",".join(num(e) for e in config.require(cfg, "energy_edges"))
    t = ",".join(num(v) for v in flatten_t_bins(config.require(cfg, "t_bins")))
    commands = []
    for family, _ in _families(variations):
        for _, stem in _stems(cfg):
            vfile = _variation_file(bcfg, output_dir, stem, family)
            commands.append(Command([exe, "variation", vfile, _binned(vfile), "--energy", energy, "--t", t], "bin"))
    return commands


def _tables_inputs(cfg, bcfg, output_dir, stem, period, family, environ) -> Tuple[str, str, str]:
    xcfg = config.require(cfg, "xsection")
    binned = _binned(_variation_file(bcfg, output_dir, stem, family))
    thrown = xs._thrown_output(_xs_output(cfg, environ), config.tree_stem(cfg, period, bcfg["mc_sample"]))
    flux_dir = config.expand_env(xcfg["inputs"]["flux_dir"], environ)
    return binned, thrown, f"{flux_dir}/{config.period_settings(cfg, period)['flux']}"


def _plan_tables(cfg, bcfg, output_dir, variations, environ) -> List[Command]:
    exe = executable("gxana_xsec_tables", environ)
    fit = bcfg["fit"]
    commands = []
    for family, _ in _families(variations):
        for period, stem in _stems(cfg):
            binned, thrown, flux = _tables_inputs(cfg, bcfg, output_dir, stem, period, family, environ)
            argv = [exe, "--fit", fit["model"]]
            for name, values in fit["params"].items():
                argv += ["--param", f"{name}=" + ",".join(num(v) for v in values)]
            argv += ["--out", f"{output_dir}/xsection_data", "--plots", f"{output_dir}/fits",
                     "--weight", bcfg["weight"], "--cheby", num(fit.get("cheby", 2)), "--label", bcfg["label"],
                     f"flatTree_{stem}:{binned}:{binned}:{thrown}:{flux}"]
            commands.append(Command(argv, "tables"))
    return commands


def weight_commands(in_dir: str, out_dir: str, ids: Sequence[str], energy_edges: Sequence[float]) -> List[Command]:
    """Run-period weighted average of every variation: the total cross section and,
    per energy bin, the differential one (legacy GetWeightedXsecFile.py patterns)."""
    commands = []
    for vid in ids:
        suffix = f"vary_{vid}"
        commands.append(Command(python_module("gxana_xsection",
            "weighted_average", in_dir, out_dir, "--pattern", f"totxsec*_{suffix}.txt"), "weight"))
        for e in energy_edges[:-1]:
            commands.append(Command(python_module("gxana_xsection",
                "weighted_average", in_dir, out_dir, "--pattern", f"diffxsec*_{suffix}_emin_{e:.2f}*.txt"), "weight"))
    return commands


def _csv(values: Sequence[Any]) -> str:
    return ",".join(num(v) for v in values)


def plot_commands(cfg: Dict[str, Any], variations: Sequence[Variation], nominal_dir: str, var_dir: str,
                  out_dir: str, exe: str) -> List[Command]:
    bcfg = bconfig.block(cfg)
    families = bcfg["families"]
    commands = []
    for family, group in _families(variations):
        fam = families[family]
        style = fam["style"]
        argv = [exe, "--nominal-dir", nominal_dir, "--var-dir", var_dir, "--out-dir", out_dir,
                "--family", family, "--label", fam["label"]]
        for v in group:
            argv += ["--variation", f"{v.id}={v.value}"]
        argv += energy_args(config.require(cfg, "energy_edges"))
        canvas = style["canvas"]
        argv += ["--canvas", "default" if canvas == "default" else _csv(canvas),
                 "--legend-diff", _csv(style["legend_diff"]), "--legend-tot", _csv(style["legend_tot"]),
                 "--y-floor", num(style["y_floor"]), "--y-pad-diff", num(style["y_pad_diff"]),
                 "--canvas-def-w", num(style["canvas_def_w"]),
                 "--title-offset-y", num(style["title_offset_y"]),
                 "--title-offsets-diff", _csv(style["title_offsets_diff"]),
                 "--title-offsets-tot", _csv(style["title_offsets_tot"]),
                 "--tot-y-ndiv", "1" if style["tot_y_ndiv"] else "0",
                 "--threshold", num(bcfg["threshold"])]
        commands.append(Command(argv, "plot"))
    return commands


def _nominal_dir(cfg, environ) -> str:
    return f"{_xs_output(cfg, environ)}/weighted_data/{bconfig.block(cfg)['label']}"


def plan(cfg: Dict[str, Any], steps: Sequence[str], variations: Sequence[Variation],
         environ: Env = None) -> List[Command]:
    check_steps(steps, STEPS)
    bcfg = bconfig.block(cfg)
    output_dir = _output_dir(cfg, environ)
    label = bcfg["label"]
    commands: List[Command] = []
    for step in STEPS:
        if step not in steps:
            continue
        if step == "trees":
            commands += _plan_trees(cfg, bcfg, output_dir, variations, environ)
        elif step == "check":
            commands += _plan_check(cfg, bcfg, output_dir, variations, environ)
        elif step == "bin":
            commands += _plan_bin(cfg, bcfg, output_dir, variations, environ)
        elif step == "tables":
            commands += _plan_tables(cfg, bcfg, output_dir, variations, environ)
        elif step == "weight":
            commands += weight_commands(f"{output_dir}/xsection_data/{label}", f"{output_dir}/weighted_data/{label}",
                                        [v.id for v in variations], config.require(cfg, "energy_edges"))
        elif step == "plot":
            commands += plot_commands(cfg, variations, _nominal_dir(cfg, environ),
                                      f"{output_dir}/weighted_data/{label}", f"{output_dir}/plots",
                                      executable("gxana_barlow_plot", environ))
    return commands


def preflight(cfg: Dict[str, Any], step: str, variations: Sequence[Variation], environ: Env = None) -> List[str]:
    """Every input `step` reads that does not exist (spec §9), in order, once each."""
    bcfg = bconfig.block(cfg)
    output_dir = _output_dir(cfg, environ)
    label = bcfg["label"]
    mc = bcfg["mc_sample"]
    families = [f for f, _ in _families(variations)]
    needed: List[str] = []
    if step == "trees":
        for _, stem in _stems(cfg):
            needed += [_path(bcfg["trees"]["input"], environ, stem=stem, mc_sample=mc),
                       _path(bcfg["trees"]["input_mc"], environ, stem=stem, mc_sample=mc)]
    elif step in ("check", "bin"):
        for family in families:
            for _, stem in _stems(cfg):
                needed.append(_variation_file(bcfg, output_dir, stem, family))
        if step == "check":
            for _, stem in _stems(cfg):
                needed += [_path(bcfg["check"][k], environ, stem=stem, mc_sample=mc) for k in ("nominal", "nominal_mc")]
    elif step == "tables":
        for family in families:
            for period, stem in _stems(cfg):
                needed += list(_tables_inputs(cfg, bcfg, output_dir, stem, period, family, environ))
    elif step == "weight":
        in_dir = f"{output_dir}/xsection_data/{label}"
        for v in variations:
            for _, stem in _stems(cfg):
                needed.append(f"{in_dir}/totxsec_flatTree_{stem}_vary_{v.id}.txt")
                needed += [f"{in_dir}/diffxsec_flatTree_{stem}_vary_{v.id}_emin_{lo}_emax_{hi}.txt"
                           for lo, hi in energy_bins(config.require(cfg, "energy_edges"))]
    elif step == "plot":
        nominal = _nominal_dir(cfg, environ)
        var_dir = f"{output_dir}/weighted_data/{label}"
        needed.append(f"{nominal}/totxsec_weighted_output.txt")
        needed += [f"{nominal}/weighted_diffxsec_emin_{lo}_emax_{hi}.txt" for lo, hi in energy_bins(config.require(cfg, "energy_edges"))]
        for v in variations:
            needed.append(f"{var_dir}/weighted_totxsec_vary_{v.id}.txt")
            needed += [f"{var_dir}/weighted_diffxsec_vary_{v.id}_emin_{lo}_emax_{hi}.txt"
                       for lo, hi in energy_bins(config.require(cfg, "energy_edges"))]
    seen = set()
    missing = []
    for path in needed:
        if path not in seen and not Path(path).exists():
            missing.append(path)
        seen.add(path)
    return missing


def _error(message: str) -> int:
    print(f"gxana: error: {message}", file=sys.stderr)
    return 1


def run_barlow(cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
               runner: Runner = subprocess.run, environ: Env = None) -> int:
    check_steps(steps, STEPS)
    bcfg = bconfig.block(cfg)
    bconfig.validate(bcfg, steps)
    output_dir = Path(_output_dir(cfg, environ))
    label = bcfg["label"]
    if not dry_run:
        for sub in ("variation_trees", f"xsection_data/{label}", f"fits/{label}", f"weighted_data/{label}", "plots"):
            (output_dir / sub).mkdir(parents=True, exist_ok=True)
    variations: List[Variation] = []

    def before(step: str) -> Optional[int]:
        nonlocal variations
        if dry_run or step == "trees":
            variations = expand(bcfg)
        else:
            if not (output_dir / manifest.MANIFEST).is_file():
                manifest.write(output_dir, manifest.build(bcfg))
                print(f"gxana: note: no {manifest.MANIFEST} in {output_dir}; wrote it from the config "
                      "(run --steps trees to make the variation trees)", file=sys.stderr)
            try:
                variations = manifest.load_checked(output_dir, bcfg)
            except manifest.ManifestError as err:
                return _error(str(err))
        if not dry_run:
            missing = preflight(cfg, step, variations, environ)
            if missing:
                return _error(f"{step}: missing inputs:\n" + "\n".join(f"  {p}" for p in missing))
            if step == "check":
                (output_dir / "output_yields.txt").write_text("")
        return None

    def after(step: str) -> None:
        if step == "trees" and not dry_run:
            manifest.write(output_dir, manifest.build(bcfg))

    return run_steps(steps, STEPS, lambda step: plan(cfg, [step], variations, environ),
                     dry_run=dry_run, runner=runner, before=before, after=after)
