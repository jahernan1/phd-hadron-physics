"""`gxana run xsection`: bin, fit, weight, integrate and split
cross-section tables, plus the dissertation LaTeX tables.

Config-driven replacement for the legacy MakeBinnedTrees.C and
MakeXSecFitVariations.C mains, and the GetWeightedXsecFile.py,
GetXSecComponentFiles.py drivers (RunXSec.py, the
legacy top-level driver, never ran as checked in -- there is no single
legacy invocation to preserve, so each step below is translated from its
own driver script/macro; see analyses/kpkpxim/config/xsection.yaml).
"""
from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional, Sequence

from gxana import config
from gxana.bins import edge_label, flatten_t_bins
from gxana.stages.runner import Command, Runner, check_steps, executable, num, python_module, run_steps

STEPS = ("bin", "tables", "weight", "integrate", "components", "tex")

# `tex` is opt-in: it needs the `gxana run systematics` stats files named in xsection.tex.columns.
DEFAULT_STEPS = ("bin", "tables", "weight", "integrate", "components")

def _bin_output(output_dir: str, prefix: str, stem: str) -> str:
    return f"{output_dir}/binned_trees/{prefix}flatTree_{stem}_nominal_kphighrap.root"


def thrown_output(output_dir: str, mc_stem: str) -> str:
    return f"{output_dir}/binned_trees/binned_thrown_flatTree_{mc_stem}.root"


def _plan_bin(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    energy_str: str, t_str: str, environ: Optional[Mapping[str, str]],
) -> List[Command]:
    exe = executable("gxana_xsec_bin", environ)
    inputs = xcfg["inputs"]
    mc_sample = xcfg["mc_sample"]
    commands = []
    for period in periods:
        data_stem = config.tree_stem(cfg, period, "data")
        mc_stem = config.tree_stem(cfg, period, mc_sample)
        jobs = (
            ("data", config.expand_env(inputs["data"], environ).format(stem=data_stem),
             _bin_output(output_dir, "binned_", data_stem)),
            ("mc", config.expand_env(inputs["mc"], environ).format(mc_stem=mc_stem),
             _bin_output(output_dir, "binned_", mc_stem)),
            ("thrown", config.expand_env(inputs["thrown"], environ).format(mc_stem=mc_stem),
             thrown_output(output_dir, mc_stem)),
        )
        for mode, in_path, out_path in jobs:
            commands.append(Command(
                [exe, mode, in_path, out_path, "--energy", energy_str, "--t", t_str], "bin"))
    return commands


def tables_paths(cfg: Dict[str, Any], xcfg: Dict[str, Any], period: str, output_dir: str) -> Any:
    mc_sample = xcfg["mc_sample"]
    data_stem = config.tree_stem(cfg, period, "data")
    mc_stem = config.tree_stem(cfg, period, mc_sample)
    return (
        data_stem,
        _bin_output(output_dir, "binned_", data_stem),
        _bin_output(output_dir, "binned_", mc_stem),
        thrown_output(output_dir, mc_stem),
    )


def tables_label_dir(output_dir: str, label: str) -> str:
    """The directory the tables step writes one fit label into: gxana_xsec_tables
    is given --out {output_dir}/data and writes each --label's tables to
    <out>/<label>/ (legacy MakeXSecFitVariations.C: data/<variation>/). The
    weight, integrate and components steps read it back from here."""
    return f"{output_dir}/data/{label}"


def tables_commands(
    cfg: Dict[str, Any], fits: Sequence[Dict[str, Any]], out_dir: str, plots_dir: Optional[str],
    environ: Optional[Mapping[str, str]],
) -> List[Command]:
    """One gxana_xsec_tables process per fit group (labels in order share and carry
    over the parameters), reading the binned trees of xsection.output_dir. Also used
    by `gxana run systematics` for its variant pool."""
    xcfg = config.require(cfg, "xsection")
    output_dir = config.expand_env(xcfg["output_dir"], environ)
    flux_dir = config.expand_env(xcfg["inputs"]["flux_dir"], environ)
    periods = list(config.require(cfg, "periods"))
    exe = executable("gxana_xsec_tables", environ)
    weight = xcfg["weight"]
    commands = []
    for fit in fits:
        argv = [exe, "--fit", fit["model"]]
        for name, values in fit["params"].items():
            argv += ["--param", f"{name}=" + ",".join(num(v) for v in values)]
        argv += ["--out", out_dir]
        if plots_dir:
            argv += ["--plots", plots_dir]
        for entry in fit["labels"]:
            # A label may override the event weight (kpkpxim accidental-subtraction
            # study: hybrid_combo, best_combo, acc_weight with the same JohnsonMCShape fit).
            argv += ["--weight", entry.get("weight", weight),
                     "--cheby", num(entry["cheby"]), "--label", entry["label"]]
            for period in periods:
                stem, data_path, mc_path, thrown_path = tables_paths(cfg, xcfg, period, output_dir)
                flux = config.period_settings(cfg, period)["flux"]
                argv.append(f"flatTree_{stem}:{data_path}:{mc_path}:{thrown_path}:{flux_dir}/{flux}")
        commands.append(Command(argv, "tables"))
    return commands


def _plan_tables(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    flux_dir: str, environ: Optional[Mapping[str, str]],
) -> List[Command]:
    fit_plots = config.expand_env(xcfg["fit_plots"], environ) if xcfg.get("fit_plots") else None
    return tables_commands(cfg, xcfg["fits"], f"{output_dir}/data", fit_plots, environ)


def weighted_average_commands(in_dir: str, out_dir: str, energy_edges: Sequence[float], step: str, *,
                              tag: str = "", total: bool = True) -> List[Command]:
    """`python -m gxana_xsection.weighted_average IN OUT --pattern P` for the total cross
    section (totxsec*[_<tag>].txt; only if total) and then per lower energy edge
    (diffxsec*[_<tag>]_emin_<edge>*.txt). Used by xsection, barlow (tag vary_<id>) and
    systematics (total=False for Q-value variants)."""
    suffix = f"_{tag}" if tag else ""
    commands = []
    if total:
        commands.append(Command(python_module(
            "gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern", f"totxsec*{suffix}.txt"), step))
    for e in energy_edges[:-1]:
        commands.append(Command(python_module(
            "gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern",
            f"diffxsec*{suffix}_emin_{edge_label(e)}*.txt"), step))
    return commands


def _plan_weight(xcfg: Dict[str, Any], output_dir: str, energy_edges: Sequence[float]) -> List[Command]:
    commands = []
    for label in xcfg["weighted_labels"]:
        commands += weighted_average_commands(tables_label_dir(output_dir, label),
                                              f"{output_dir}/weighted_data/{label}", energy_edges, "weight")
    return commands


def _plan_integrate(xcfg: Dict[str, Any], output_dir: str) -> List[Command]:
    """Total cross section integrated over the differential -t bins: per label,
    intxsec_<name>.txt beside the per-period tables, then their weighted average
    in weighted_data/<label>/ (intxsec_weighted_output.txt)."""
    commands = []
    for label in xcfg["weighted_labels"]:
        in_dir = tables_label_dir(output_dir, label)
        out_dir = f"{output_dir}/weighted_data/{label}"
        commands.append(Command(python_module("gxana_xsection", "integrated_total", in_dir, in_dir), "integrate"))
        commands.append(Command(
            python_module("gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern", "intxsec*.txt"), "integrate"))
    return commands


def _plan_components(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    energy_edges: Sequence[float],
) -> List[Command]:
    commands = []
    for period in periods:
        plabel = config.period_settings(cfg, period)["label"]
        for label in xcfg["component_labels"]:
            in_dir = tables_label_dir(output_dir, label)
            out_dir = f"{output_dir}/components/{plabel}/{label}"
            commands.append(Command(
                python_module("gxana_xsection", "components", in_dir, out_dir, "--pattern", f"totout*{period}*.txt"),
                "components"))
            for e in energy_edges[:-1]:
                pattern = f"diffout*{period}*_emin_{e:.2f}*.txt"
                commands.append(Command(
                    python_module("gxana_xsection", "components", in_dir, out_dir, "--pattern", pattern), "components"))
    return commands


def _tex_settings(xcfg: Dict[str, Any], output_dir: str, environ: Optional[Mapping[str, str]]) -> Any:
    """(weighted-tables dir, output .tex, {column: stats file}, run_fraction) from xsection.tex."""
    tex = config.require(xcfg, "tex")
    label = config.require(tex, "label")
    columns = {name: config.expand_env(path, environ) for name, path in config.require(tex, "columns").items()}
    output = config.expand_env(config.require(tex, "output"), environ)
    return f"{output_dir}/weighted_data/{label}", output, columns, tex.get("run_fraction", 0.051)


def _plan_tex(xcfg: Dict[str, Any], output_dir: str, environ: Optional[Mapping[str, str]]) -> List[Command]:
    weighted_dir, output, columns, run_fraction = _tex_settings(xcfg, output_dir, environ)
    argv = python_module("gxana_xsection", "tex_table", weighted_dir, "weighted*.txt", output, "--run-fraction", num(run_fraction))
    for name, path in columns.items():
        argv += ["--column", f"{name}={path}"]
    return [Command(argv, "tex")]


def _resolve_xcfg(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> Any:
    """The `xsection` config block plus its expanded output_dir, resolved once
    so callers (plan_xsection and run_xsection's tex precheck) share the
    same lookup instead of each re-reading xsection.output_dir."""
    xcfg = config.require(cfg, "xsection")
    output_dir = config.expand_env(xcfg["output_dir"], environ)
    return xcfg, output_dir


def plan_xsection(
    cfg: Dict[str, Any], steps: Sequence[str], environ: Optional[Mapping[str, str]] = None,
) -> List[Command]:
    check_steps(steps, STEPS, first="sorted")
    requested = set(steps)

    xcfg, output_dir = _resolve_xcfg(cfg, environ)
    inputs = xcfg["inputs"]
    flux_dir = config.expand_env(inputs["flux_dir"], environ)
    periods = list(config.require(cfg, "periods"))
    energy_edges = config.require(cfg, "energy_edges")
    t_edges = flatten_t_bins(config.require(cfg, "t_bins"))
    energy_str = ",".join(num(e) for e in energy_edges)
    t_str = ",".join(num(t) for t in t_edges)

    commands: List[Command] = []
    for step in STEPS:
        if step not in requested:
            continue
        if step == "bin":
            commands += _plan_bin(cfg, xcfg, periods, output_dir, energy_str, t_str, environ)
        elif step == "tables":
            commands += _plan_tables(cfg, xcfg, periods, output_dir, flux_dir, environ)
        elif step == "weight":
            commands += _plan_weight(xcfg, output_dir, energy_edges)
        elif step == "integrate":
            commands += _plan_integrate(xcfg, output_dir)
        elif step == "components":
            commands += _plan_components(cfg, xcfg, periods, output_dir, energy_edges)
        elif step == "tex":
            commands += _plan_tex(xcfg, output_dir, environ)
    return commands


def _output_dirs(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> List[Path]:
    xcfg = config.require(cfg, "xsection")
    output_dir = Path(config.expand_env(xcfg["output_dir"], environ))
    periods = list(config.require(cfg, "periods"))
    dirs = [output_dir / "binned_trees", output_dir / "data"]
    for label in xcfg["weighted_labels"]:
        dirs.append(output_dir / "weighted_data" / label)
    for period in periods:
        plabel = config.period_settings(cfg, period)["label"]
        for label in xcfg["component_labels"]:
            dirs.append(output_dir / "components" / plabel / label)
    return dirs


def _tex_missing_inputs_message(xcfg: Dict[str, Any], output_dir: str,
                                environ: Optional[Mapping[str, str]]) -> Optional[str]:
    _, _, columns, _ = _tex_settings(xcfg, output_dir, environ)
    missing = [p for p in columns.values() if not Path(p).is_file()]
    if not missing:
        return None
    return (
        "gxana: error: tex step: missing systematics input files: " + ", ".join(missing) +
        "; run `gxana run systematics --channel <channel>` first "
        "(xsection.tex.columns in analyses/<channel>/config/xsection.yaml)"
    )


def run_xsection(
    cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
    runner: Runner = subprocess.run, environ: Optional[Mapping[str, str]] = None,
) -> int:
    check_steps(steps, STEPS, first="sorted")
    if not dry_run:
        for d in _output_dirs(cfg, environ):
            d.mkdir(parents=True, exist_ok=True)

    def before(step: str) -> Optional[int]:
        if step == "tex" and not dry_run:
            xcfg, output_dir = _resolve_xcfg(cfg, environ)
            message = _tex_missing_inputs_message(xcfg, output_dir, environ)
            if message is not None:
                print(message)
                return 1
            Path(_tex_settings(xcfg, output_dir, environ)[1]).parent.mkdir(parents=True, exist_ok=True)
        return None

    return run_steps(steps, STEPS, lambda step: plan_xsection(cfg, [step], environ=environ),
                     dry_run=dry_run, runner=runner, before=before)
