"""`gxana run xsection`: bin, fit, weight, integrate and split
cross-section tables, plus the dissertation LaTeX tables and figures.

Config-driven replacement for the legacy MakeBinnedTrees.C and
MakeXSecFitVariations.C mains, and the GetWeightedXsecFile.py,
GetXSecComponentFiles.py drivers. Each step below is translated from its
own driver script/macro; see analyses/kpkpxim/config/xsection.yaml.
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional, Sequence

from gxana import config
from gxana.bins import edge_label, flatten_t_bins
from gxana.paths import gxana_root
from gxana.stages.runner import (Command, Runner, check_steps, executable, num, python_module, root_macro,
                                 run_steps)
from gxana_xsection import SCALE_FACTOR

STEPS = ("bin", "tables", "weight", "integrate", "components", "tex", "figures")

# `tex` and `figures` are opt-in: they need the `gxana run systematics` stats files of
# xsection.tex.columns (figures also the variant tables its plots read).
DEFAULT_STEPS = ("bin", "tables", "weight", "integrate", "components")

def _bin_output(xcfg: Dict[str, Any], output_dir: str, prefix: str, stem: str) -> str:
    return f"{output_dir}/binned_trees/{prefix}flatTree_{stem}{config.require(xcfg, 'binned_suffix')}.root"


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
             _bin_output(xcfg, output_dir, "binned_", data_stem)),
            ("mc", config.expand_env(inputs["mc"], environ).format(mc_stem=mc_stem),
             _bin_output(xcfg, output_dir, "binned_", mc_stem)),
            ("thrown", config.expand_env(inputs["thrown"], environ).format(mc_stem=mc_stem),
             thrown_output(output_dir, mc_stem)),
        )
        for mode, in_path, out_path in jobs:
            commands.append(Command(
                [exe, mode, in_path, out_path, "--energy", energy_str, "--t", t_str] + bin_physics_args(cfg, mode),
                "bin"))
    return commands


def bin_physics_args(cfg: Dict[str, Any], mode: str) -> List[str]:
    """The channel flags of gxana_xsec_bin MODE (data, mc, thrown), appended last: the
    flat-tree name (physics.flat_tree, physics.thrown_flat_tree), the binned columns
    (xsection.branches) and, for data, the Q-factor branch (physics.qvalue_branch, if any)."""
    phys = config.physics(cfg)
    if mode == "thrown":
        return ["--tree", phys["thrown_flat_tree"]]
    branches = config.require(config.require(cfg, "xsection"), "branches")
    if not branches or not all(isinstance(b, str) and b for b in branches):
        raise config.ConfigError(f"xsection.branches: need a list of branch names, got {branches!r}")
    args = ["--tree", phys["flat_tree"]]
    for branch in branches:
        args += ["--branch", branch]
    if mode == "data" and phys["qvalue_branch"]:
        args += ["--data-branch", phys["qvalue_branch"]]
    return args


def tables_paths(cfg: Dict[str, Any], xcfg: Dict[str, Any], period: str, output_dir: str) -> Any:
    mc_sample = xcfg["mc_sample"]
    data_stem = config.tree_stem(cfg, period, "data")
    mc_stem = config.tree_stem(cfg, period, mc_sample)
    return (
        data_stem,
        _bin_output(xcfg, output_dir, "binned_", data_stem),
        _bin_output(xcfg, output_dir, "binned_", mc_stem),
        thrown_output(output_dir, mc_stem),
    )


def tables_label_dir(output_dir: str, label: str) -> str:
    """The directory the tables step writes one fit label into: gxana_xsec_tables
    is given --out {output_dir}/data and writes each --label's tables to
    <out>/<label>/ (legacy MakeXSecFitVariations.C: data/<variation>/). The
    weight, integrate and components steps read it back from here."""
    return f"{output_dir}/data/{label}"


# gxana_xsec_tables --mass-window names, in command-line order (xsection.mass_windows).
TARGET_KEYS = ("z", "density", "molar_mass", "atoms")
MASS_WINDOWS = ("lo", "mc_hi", "mc_signal_hi", "mc_plot_hi", "data_hi", "data_edge", "mcpdf_data_lo")


def _numbers(value: Any, count: int) -> bool:
    return (isinstance(value, list) and len(value) == count
            and all(isinstance(v, (int, float)) and not isinstance(v, bool) for v in value))


def tables_physics_args(cfg: Dict[str, Any]) -> List[str]:
    """The channel flags of gxana_xsec_tables, appended after the JOBs: physics.observable,
    physics.qvalue_branch and physics.branching_ratio (channel.yaml), xsection.gate,
    xsection.target and xsection.mass_windows (numbers as written in the YAML)."""
    phys = config.physics(cfg)
    xcfg = config.require(cfg, "xsection")
    gate = config._text(xcfg, "gate", "xsection")
    target = config.check_block(config.require(xcfg, "target"), TARGET_KEYS, "xsection.target")
    z = target.get("z")
    if not (_numbers(z, 2) and z[0] < z[1]):
        raise config.ConfigError(f"xsection.target.z: need [zmin, zmax] with zmin < zmax, got {z!r}")
    numbers = [config._number(target, key, "xsection.target") for key in ("density", "molar_mass", "atoms")]
    windows = config.check_block(config.require(xcfg, "mass_windows"), MASS_WINDOWS, "xsection.mass_windows")
    if not _numbers([windows.get(n) for n in MASS_WINDOWS], len(MASS_WINDOWS)):
        raise config.ConfigError(f"xsection.mass_windows: need numbers {', '.join(MASS_WINDOWS)}, got {windows!r}")
    br = phys["branching_ratio"]
    args = ["--observable", phys["observable"]["branch"], "--observable-title", phys["observable"]["title"],
            "--gate", gate, "--qvalue-branch", phys["qvalue_branch"] or "none",
            "--br", f"{num(br['value'])},{num(br['error'])}",
            "--target", ",".join(num(v) for v in z + numbers)]
    for name in MASS_WINDOWS:
        args += ["--mass-window", f"{name}={num(windows[name])}"]
    return args


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
    weight = config.require(xcfg, "weight")
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
        commands.append(Command(argv + tables_physics_args(cfg), "tables"))
    return commands


def _plan_tables(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    flux_dir: str, environ: Optional[Mapping[str, str]],
) -> List[Command]:
    fit_plots = config.expand_env(xcfg["fit_plots"], environ) if xcfg.get("fit_plots") else None
    return tables_commands(cfg, xcfg["fits"], f"{output_dir}/data", fit_plots, environ)


def weighted_average_commands(in_dir: str, out_dir: str, energy_edges: Sequence[float], step: str, *,
                              n_periods: int, tag: str = "", total: bool = True) -> List[Command]:
    """`python -m gxana_xsection.weighted_average IN OUT --pattern P --n-periods N` for the
    total cross section (totxsec*[_<tag>].txt; only if total) and then per lower energy edge
    (diffxsec*[_<tag>]_emin_<edge>*.txt); N = the channel's number of run periods. Used by
    xsection, barlow (tag vary_<id>) and systematics (total=False for Q-value variants)."""
    suffix = f"_{tag}" if tag else ""
    periods = ["--n-periods", str(n_periods)]
    commands = []
    if total:
        commands.append(Command(python_module(
            "gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern", f"totxsec*{suffix}.txt", *periods),
            step))
    for e in energy_edges[:-1]:
        commands.append(Command(python_module(
            "gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern",
            f"diffxsec*{suffix}_emin_{edge_label(e)}*.txt", *periods), step))
    return commands


def _plan_weight(xcfg: Dict[str, Any], output_dir: str, energy_edges: Sequence[float],
                 n_periods: int) -> List[Command]:
    commands = []
    for label in xcfg["weighted_labels"]:
        commands += weighted_average_commands(tables_label_dir(output_dir, label),
                                              f"{output_dir}/weighted_data/{label}", energy_edges, "weight",
                                              n_periods=n_periods)
    return commands


def _plan_integrate(xcfg: Dict[str, Any], output_dir: str, n_periods: int) -> List[Command]:
    """Total cross section integrated over the differential -t bins: per label,
    intxsec_<name>.txt beside the per-period tables, then their weighted average
    in weighted_data/<label>/ (intxsec_weighted_output.txt)."""
    commands = []
    for label in xcfg["weighted_labels"]:
        in_dir = tables_label_dir(output_dir, label)
        out_dir = f"{output_dir}/weighted_data/{label}"
        commands.append(Command(python_module("gxana_xsection", "integrated_total", in_dir, in_dir), "integrate"))
        commands.append(Command(
            python_module("gxana_xsection", "weighted_average", in_dir, out_dir, "--pattern", "intxsec*.txt",
                          "--n-periods", str(n_periods)), "integrate"))
    return commands


def _plan_components(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    energy_edges: Sequence[float],
) -> List[Command]:
    # The output names start at the channel's reaction (components --anchor).
    anchor = ["--anchor", config.require(cfg, "reaction")]
    commands = []
    for period in periods:
        plabel = config.period_settings(cfg, period)["label"]
        for label in xcfg["component_labels"]:
            in_dir = tables_label_dir(output_dir, label)
            out_dir = f"{output_dir}/components/{plabel}/{label}"
            commands.append(Command(
                python_module("gxana_xsection", "components", in_dir, out_dir, "--pattern", f"totout*{period}*.txt",
                              *anchor),
                "components"))
            for e in energy_edges[:-1]:
                pattern = f"diffout*{period}*_emin_{edge_label(e)}*.txt"
                commands.append(Command(
                    python_module("gxana_xsection", "components", in_dir, out_dir, "--pattern", pattern, *anchor),
                    "components"))
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


def _figures_settings(xcfg: Dict[str, Any], output_dir: str, environ: Optional[Mapping[str, str]]) -> Any:
    """(weighted-tables dir, figures dir, {column: stats file or SCALE_FACTOR}, [(macro, args, requires)])
    from xsection.figures; columns default to xsection.tex.columns."""
    where = "xsection.figures"
    fig = config.check_block(config.require(xcfg, "figures"), ("output_dir", "label", "columns", "plots"), where,
                             ("output_dir", "label", "plots"))
    label = config._text(fig, "label", where)
    columns = fig.get("columns") if "columns" in fig else config.require(config.require(xcfg, "tex"), "columns")
    if not isinstance(columns, dict) or not columns or not all(isinstance(v, str) for v in columns.values()):
        raise config.ConfigError(f"{where}.columns: need a non-empty mapping of column name to file, "
                                 f"got {columns!r}")
    columns = {name: path if path == SCALE_FACTOR else config.expand_env(path, environ)
               for name, path in columns.items()}
    if not isinstance(fig["plots"], list) or not fig["plots"]:
        raise config.ConfigError(f"{where}.plots: need a list of {{macro, args, requires}}, got {fig['plots']!r}")
    plots = []
    for i, plot in enumerate(fig["plots"]):
        config.check_block(plot, ("macro", "args", "requires"), f"{where}.plots[{i}]", ("macro",))
        macro = config._text(plot, "macro", f"{where}.plots[{i}]")
        args = [config.expand_env(a, environ) if isinstance(a, str) else a for a in plot.get("args", [])]
        requires = [config.expand_env(p, environ) for p in plot.get("requires", [])]
        plots.append((macro, args, requires))
    figures_dir = config.expand_env(config._text(fig, "output_dir", where), environ)
    return f"{output_dir}/weighted_data/{label}", figures_dir, columns, plots


def _plan_figures(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                  environ: Optional[Mapping[str, str]]) -> List[Command]:
    """syst_tables on the weighted tables of figures.label, then each plot macro from figures.output_dir."""
    weighted_dir, figures_dir, columns, plots = _figures_settings(xcfg, output_dir, environ)
    argv = python_module("gxana_xsection", "syst_tables", weighted_dir)
    for name, path in columns.items():
        argv += ["--column", f"{name}={path}"]
    commands = [Command(argv, "figures")]
    channel = config.require(cfg, "channel")
    for macro, args, _ in plots:
        commands.append(Command(root_macro(environ, channel, macro, args), "figures", figures_dir))
    return commands


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
            commands += _plan_weight(xcfg, output_dir, energy_edges, len(periods))
        elif step == "integrate":
            commands += _plan_integrate(xcfg, output_dir, len(periods))
        elif step == "components":
            commands += _plan_components(cfg, xcfg, periods, output_dir, energy_edges)
        elif step == "tex":
            commands += _plan_tex(xcfg, output_dir, environ)
        elif step == "figures":
            commands += _plan_figures(cfg, xcfg, output_dir, environ)
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
    missing = [p for p in columns.values() if p != SCALE_FACTOR and not Path(p).is_file()]
    if not missing:
        return None
    return (
        "gxana: error: tex step: missing systematics input files: " + ", ".join(missing) +
        "; run `gxana run systematics --channel <channel>` first "
        "(xsection.tex.columns in analyses/<channel>/config/xsection.yaml)"
    )


def _input_hints(cfg: Dict[str, Any], period: str) -> Dict[str, str]:
    channel = config.require(cfg, "channel")
    sample = config.require(cfg, "xsection")["mc_sample"]
    stage = f"or gxana data stage --channel {channel}"
    return {
        "data": f"gxana run qfactors --channel {channel} --period {period}, {stage}",
        "mc": f"selection/flatTreePrep.C on the reconstructed MC (analyses/{channel}/README.md), {stage}",
        "thrown": f"gxana run select --channel {channel} --period {period} --sample {sample} --thrown, {stage}",
    }


def _missing_bin(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                 environ: Optional[Mapping[str, str]]) -> List[str]:
    missing: List[str] = []
    for period in config.require(cfg, "periods"):
        hints = _input_hints(cfg, period)
        stems = {"stem": config.tree_stem(cfg, period, "data"),
                 "mc_stem": config.tree_stem(cfg, period, xcfg["mc_sample"])}
        for kind in ("data", "mc", "thrown"):
            path = config.expand_env(xcfg["inputs"][kind], environ).format(**stems)
            if not Path(path).is_file():
                missing.append(f"{path} ({hints[kind]})")
    return missing


def _missing_tables(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                    environ: Optional[Mapping[str, str]]) -> List[str]:
    channel = config.require(cfg, "channel")
    flux_dir = config.expand_env(xcfg["inputs"]["flux_dir"], environ)
    missing: List[str] = []
    for period in config.require(cfg, "periods"):
        for path in tables_paths(cfg, xcfg, period, output_dir)[1:]:
            if not Path(path).is_file():
                missing.append(f"{path} (gxana run xsection --channel {channel} --steps bin, "
                               f"or gxana data stage --channel {channel})")
        flux = Path(flux_dir) / config.period_settings(cfg, period)["flux"]
        if not flux.is_file():
            missing.append(f"{flux} (xsection.inputs.flux_dir; preserved data, docs/analysis_data.md)")
    return missing


def _missing_tables_output(labels_key: str, *patterns: str):
    patterns = patterns or ("diffxsec*.txt",)

    def check(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
              environ: Optional[Mapping[str, str]]) -> List[str]:
        channel = config.require(cfg, "channel")
        missing: List[str] = []
        for label in xcfg[labels_key]:
            d = Path(tables_label_dir(output_dir, label))
            for pattern in patterns:
                if not any(d.glob(pattern)):
                    missing.append(f"{d}/{pattern} (gxana run xsection --channel {channel} --steps tables)")
        return missing
    return check


def _missing_figures(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                     environ: Optional[Mapping[str, str]]) -> List[str]:
    channel = config.require(cfg, "channel")
    weighted_dir, _, columns, plots = _figures_settings(xcfg, output_dir, environ)
    systematics = f"gxana run systematics --channel {channel}"
    missing: List[str] = []
    if not any(Path(weighted_dir).glob("weighted_diffxsec*.txt")):
        missing.append(f"{weighted_dir}/weighted_diffxsec*.txt (gxana run xsection --channel {channel} "
                       "--steps weight, after gxana data stage or the tables step)")
    missing += [f"{p} ({systematics})" for p in columns.values() if p != SCALE_FACTOR and not Path(p).is_file()]
    base = gxana_root(environ) / "analyses" / channel
    for macro, _, requires in plots:
        if not (base / macro).is_file():
            missing.append(f"{base / macro} (xsection.figures.plots macro)")
        for path in requires:
            if not Path(path).exists():
                hint = (systematics if "/systematics/" in path else
                        f"gxana data stage, then gxana run xsection --channel {channel} --steps tables")
                missing.append(f"{path} ({hint})")
    return missing


# Per-step input checks: step -> f(cfg, xcfg, output_dir, environ) -> missing entries.
# `tex` keeps its own check (_tex_missing_inputs_message); add a step here to give it a precheck.
_PREFLIGHT = {
    "bin": _missing_bin,
    "tables": _missing_tables,
    "weight": _missing_tables_output("weighted_labels"),
    "integrate": _missing_tables_output("weighted_labels"),
    "components": _missing_tables_output("component_labels", "totout*.txt", "diffout*.txt"),
    "figures": _missing_figures,
}


def preflight(cfg: Dict[str, Any], step: str, environ: Optional[Mapping[str, str]] = None) -> List[str]:
    """Missing inputs of `step`, each with the command that makes it."""
    check = _PREFLIGHT.get(step)
    if check is None:
        return []
    xcfg, output_dir = _resolve_xcfg(cfg, environ)
    return check(cfg, xcfg, output_dir, environ)


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
        if not dry_run and step != "tex":
            missing = preflight(cfg, step, environ)
            if missing:
                print(f"gxana: error: {step}: missing inputs:\n" + "\n".join(f"  {m}" for m in missing),
                      file=sys.stderr)
                return 1
            if step == "figures":
                xcfg, output_dir = _resolve_xcfg(cfg, environ)
                Path(_figures_settings(xcfg, output_dir, environ)[1]).mkdir(parents=True, exist_ok=True)
        return None

    return run_steps(steps, STEPS, lambda step: plan_xsection(cfg, [step], environ=environ),
                     dry_run=dry_run, runner=runner, before=before)
