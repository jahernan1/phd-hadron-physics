"""`gxana run xsection`: bin, fit, weight, integrate, split and Q-value-rescale
cross-section tables, plus the dissertation LaTeX tables.

Config-driven replacement for the legacy MakeBinnedTrees.C and
MakeXSecFitVariations.C mains, and the GetWeightedXsecFile.py,
GetXSecComponentFiles.py, MakeQValXSecFile.py drivers (RunXSec.py, the
legacy top-level driver, never ran as checked in -- there is no single
legacy invocation to preserve, so each step below is translated from its
own driver script/macro; see analyses/kpkpxim/config/xsection.yaml).
"""
from __future__ import annotations

import shlex
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, List, Mapping, NamedTuple, Optional, Sequence

from gxana import config
from gxana.paths import repo_root

STEPS = ("bin", "tables", "weight", "integrate", "components", "qvalue", "fitfigs", "tex")

# `qvalue` needs xsection.qvalue_source to name a data/<label> directory the
# tables step populated (kpkpxim: hybrid_combo, the JohnsonMCShape study fit, as the legacy rescale used). It is opt-in
# via --steps ...,qvalue. `fitfigs` is opt-in: it needs every
# xsection.fit_figures label weighted (qvalues from the qvalue step) and the
# per-bin fit PDFs, and writes fit_variations_stats.txt. `tex` is opt-in too:
# it needs the systematics comparison tables (fit_variations_stats.txt,
# combo_variations_stats.txt).
DEFAULT_STEPS = ("bin", "tables", "weight", "integrate", "components")

Runner = Callable[..., subprocess.CompletedProcess]


class Command(NamedTuple):
    argv: List[str]
    step: str
    cwd: Optional[str] = None  # working directory; None = the caller's


def _gxana_root(environ: Optional[Mapping[str, str]]) -> Path:
    root = (environ or {}).get("GXANA_ROOT")
    return Path(root) if root else repo_root()


def _executable(name: str, environ: Optional[Mapping[str, str]]) -> str:
    candidate = _gxana_root(environ) / "build" / "bin" / name
    return str(candidate) if candidate.is_file() else name


def _root_macro(environ: Optional[Mapping[str, str]], macro_call: str) -> List[str]:
    """`root -l -b -q <GXANA_ROOT>/rootlogon.C <macro_call>`: rootlogon.C loads
    the gxana libraries and include paths the macros need from any cwd."""
    return ["root", "-l", "-b", "-q", str(_gxana_root(environ) / "rootlogon.C"), macro_call]


def _python_module(module: str, *args: str) -> List[str]:
    return [sys.executable, "-m", f"gxana_xsection.{module}", *args]


def _num(value: Any) -> str:
    return str(value)


def _flatten_t_bins(t_bins: Sequence[Sequence[float]]) -> List[float]:
    edges = [t_bins[0][0]]
    for lo, hi in t_bins:
        if lo != edges[-1]:
            raise config.ConfigError(f"t_bins are not contiguous: {t_bins}")
        edges.append(hi)
    return edges


def _bin_output(output_dir: str, prefix: str, stem: str) -> str:
    return f"{output_dir}/binned_trees/{prefix}flatTree_{stem}_nominal_kphighrap.root"


def _thrown_output(output_dir: str, mc_stem: str) -> str:
    return f"{output_dir}/binned_trees/binned_thrown_flatTree_{mc_stem}.root"


def _plan_bin(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    energy_str: str, t_str: str, environ: Optional[Mapping[str, str]],
) -> List[Command]:
    exe = _executable("gxana_xsec_bin", environ)
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
             _thrown_output(output_dir, mc_stem)),
        )
        for mode, in_path, out_path in jobs:
            commands.append(Command(
                [exe, mode, in_path, out_path, "--energy", energy_str, "--t", t_str], "bin"))
    return commands


def _tables_paths(cfg: Dict[str, Any], xcfg: Dict[str, Any], period: str, output_dir: str) -> Any:
    mc_sample = xcfg["mc_sample"]
    data_stem = config.tree_stem(cfg, period, "data")
    mc_stem = config.tree_stem(cfg, period, mc_sample)
    return (
        data_stem,
        _bin_output(output_dir, "binned_", data_stem),
        _bin_output(output_dir, "binned_", mc_stem),
        _thrown_output(output_dir, mc_stem),
    )


def tables_label_dir(output_dir: str, label: str) -> str:
    """The directory the tables step writes one fit label into: gxana_xsec_tables
    is given --out {output_dir}/data and writes each --label's tables to
    <out>/<label>/ (legacy MakeXSecFitVariations.C: data/<variation>/). The
    weight, components and qvalue steps read it back from here."""
    return f"{output_dir}/data/{label}"


def _plan_tables(
    cfg: Dict[str, Any], xcfg: Dict[str, Any], periods: Sequence[str], output_dir: str,
    flux_dir: str, environ: Optional[Mapping[str, str]],
) -> List[Command]:
    exe = _executable("gxana_xsec_tables", environ)
    weight = xcfg["weight"]
    out_dir = f"{output_dir}/data"
    fit_plots = config.expand_env(xcfg["fit_plots"], environ) if xcfg.get("fit_plots") else None
    commands = []
    for fit in xcfg["fits"]:
        argv = [exe, "--fit", fit["model"]]
        for name, values in fit["params"].items():
            argv += ["--param", f"{name}=" + ",".join(_num(v) for v in values)]
        argv += ["--out", out_dir]
        if fit_plots:
            argv += ["--plots", fit_plots]
        for entry in fit["labels"]:
            # A label may override the event weight (kpkpxim combo-selection
            # study: hybrid_combo, best_combo, acc_weight with the same JohnsonMCShape fit).
            argv += ["--weight", entry.get("weight", weight),
                     "--cheby", _num(entry["cheby"]), "--label", entry["label"]]
            for period in periods:
                stem, data_path, mc_path, thrown_path = _tables_paths(cfg, xcfg, period, output_dir)
                flux = config.period_settings(cfg, period)["flux"]
                argv.append(f"flatTree_{stem}:{data_path}:{mc_path}:{thrown_path}:{flux_dir}/{flux}")
        commands.append(Command(argv, "tables"))
    return commands


def _diffxsec_weight_commands(in_dir: str, out_dir: str, energy_edges: Sequence[float],
                              step: str) -> List[Command]:
    commands = []
    for e in energy_edges[:-1]:
        pattern = f"diffxsec*_emin_{e:.2f}*.txt"
        commands.append(Command(
            _python_module("weighted_average", in_dir, out_dir, "--pattern", pattern), step))
    return commands


def _plan_weight(xcfg: Dict[str, Any], output_dir: str, energy_edges: Sequence[float]) -> List[Command]:
    commands = []
    for label in xcfg["weighted_labels"]:
        in_dir = tables_label_dir(output_dir, label)
        out_dir = f"{output_dir}/weighted_data/{label}"
        commands.append(Command(
            _python_module("weighted_average", in_dir, out_dir, "--pattern", "totxsec*.txt"), "weight"))
        commands += _diffxsec_weight_commands(in_dir, out_dir, energy_edges, "weight")
    return commands


def _plan_integrate(xcfg: Dict[str, Any], output_dir: str) -> List[Command]:
    """Total cross section integrated over the differential -t bins: per label,
    intxsec_<name>.txt beside the per-period tables, then their weighted average
    in weighted_data/<label>/ (intxsec_weighted_output.txt)."""
    commands = []
    for label in xcfg["weighted_labels"]:
        in_dir = tables_label_dir(output_dir, label)
        out_dir = f"{output_dir}/weighted_data/{label}"
        commands.append(Command(_python_module("integrated_total", in_dir, in_dir), "integrate"))
        commands.append(Command(
            _python_module("weighted_average", in_dir, out_dir, "--pattern", "intxsec*.txt"), "integrate"))
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
                _python_module("components", in_dir, out_dir, "--pattern", f"totout*{period}*.txt"),
                "components"))
            for e in energy_edges[:-1]:
                pattern = f"diffout*{period}*_emin_{e:.2f}*.txt"
                commands.append(Command(
                    _python_module("components", in_dir, out_dir, "--pattern", pattern), "components"))
    return commands


def _qvalue_source_dir(xcfg: Dict[str, Any], output_dir: str) -> Path:
    # gxana: legacy MakeQValXSecFile.py hardcodes directory
    # .../xsection/data/hybrid_combo -- the JohnsonMCShape study's hybrid_combo accType dir
    # (legacy MakeXSecFiles.C getXSecFiles with variation="" writes
    # data/<accType>). qvalue_source makes that source explicit and
    # configurable; it must name a `fits` label the tables step writes.
    return Path(tables_label_dir(output_dir, config.require(xcfg, 'qvalue_source')))


def _plan_qvalue(xcfg: Dict[str, Any], output_dir: str) -> List[Command]:
    src_dir = _qvalue_source_dir(xcfg, output_dir)
    out_dir = Path(f"{output_dir}/data/qvalues")
    file1_list = sorted(src_dir.glob("diffout*.txt"))
    file2_list = sorted(src_dir.glob("diffxsec*.txt"))
    if len(file1_list) != len(file2_list):
        raise config.ConfigError(
            f"qvalue: {src_dir} has {len(file1_list)} diffout*.txt but {len(file2_list)} diffxsec*.txt files")
    commands = []
    for file1, file2 in zip(file1_list, file2_list):
        output_file = out_dir / file2.name
        commands.append(Command(
            _python_module("qvalue_rescale", str(file1), "data_yield", "qval_yield", str(file2), str(output_file)),
            "qvalue"))
    return commands


QVALUES_LABEL = "qvalues"  # the directory the qvalue step writes under data/


class _FitFigures(NamedTuple):
    labels: List[str]
    comparisons_dir: str
    macro: str
    graph_macro: str
    plots_dir: str
    examples: List[Any]  # (figure name, fit PDF path)


def _fit_figures(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                 environ: Optional[Mapping[str, str]]) -> _FitFigures:
    ff = config.require(xcfg, "fit_figures")
    channel_dir = _gxana_root(environ) / "analyses" / config.require(cfg, "channel")
    fit_plots = config.expand_env(config.require(xcfg, "fit_plots"), environ)
    ebin = config.require(ff, "example_bin")
    stem = config.tree_stem(cfg, config.require(ebin, "period"), "data")
    # gxana_xsec_tables --plots: <fit_plots>/<label>/data_<job name>_<tree>.pdf,
    # job name flatTree_<stem> (see _plan_tables).
    pdf = f"data_flatTree_{stem}_{config.require(ebin, 'tree')}.pdf"
    examples = [(name, f"{fit_plots}/{label}/{pdf}")
                for name, label in config.require(ff, "examples").items()]
    return _FitFigures(
        labels=list(config.require(ff, "comparison_labels")),
        comparisons_dir=config.expand_env(config.require(ff, "comparisons_dir"), environ),
        macro=str(channel_dir / "systematics" / config.require(ff, "comparison_macro")),
        graph_macro=str(channel_dir / "xsection" / "MakeWeightedDiffXSecTGraphs.C"),
        plots_dir=f"{output_dir}/plots",
        examples=examples,
    )


def _plan_fitfigs(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                  energy_edges: Sequence[float], environ: Optional[Mapping[str, str]]) -> List[Command]:
    """Weight the qvalue-step tables (diffxsec only: the qvalue step writes no
    totxsec), convert each comparison label to TGraphErrors, run the
    comparison macro in comparisons_dir (it opens its inputs and writes
    fit_variations_stats.txt relative to the cwd), copy the example fits."""
    ff = _fit_figures(cfg, xcfg, output_dir, environ)
    commands = _diffxsec_weight_commands(
        tables_label_dir(output_dir, QVALUES_LABEL), f"{output_dir}/weighted_data/{QVALUES_LABEL}",
        energy_edges, "fitfigs")
    for label in ff.labels:
        out = f"{ff.comparisons_dir}/WeightedDiffXSecTGraphs_{label}.root"
        call = f'{ff.graph_macro}("{output_dir}/weighted_data/{label}/","{out}")'
        commands.append(Command(_root_macro(environ, call), "fitfigs"))
    commands.append(Command(_root_macro(environ, ff.macro), "fitfigs", ff.comparisons_dir))
    for name, pdf in ff.examples:
        commands.append(Command(["cp", pdf, f"{ff.plots_dir}/fit_examples/{name}.pdf"], "fitfigs"))
    return commands


def _tex_settings(xcfg: Dict[str, Any], output_dir: str, environ: Optional[Mapping[str, str]]) -> Any:
    """(weighted-tables dir, output .tex, systematic source, additional files)
    from xsection.tex."""
    tex = config.require(xcfg, "tex")
    label = config.require(tex, "label")
    additional = [config.expand_env(a, environ) for a in tex.get("additional", [])]
    output = config.expand_env(config.require(tex, "output"), environ)
    source = tex.get("systematic_source", "run_fraction")
    return f"{output_dir}/weighted_data/{label}", output, source, additional


def _plan_tex(xcfg: Dict[str, Any], output_dir: str, environ: Optional[Mapping[str, str]]) -> List[Command]:
    weighted_dir, output, source, additional = _tex_settings(xcfg, output_dir, environ)
    argv = _python_module("tex_table", weighted_dir, "weighted*.txt", output, "--systematic-source", source)
    if additional:
        argv += ["--additional", *additional]
    return [Command(argv, "tex")]


def _resolve_xcfg(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> Any:
    """The `xsection` config block plus its expanded output_dir, resolved once
    so callers (plan_xsection and run_xsection's qvalue precheck) share the
    same lookup instead of each re-reading xsection.output_dir."""
    xcfg = config.require(cfg, "xsection")
    output_dir = config.expand_env(xcfg["output_dir"], environ)
    return xcfg, output_dir


def plan_xsection(
    cfg: Dict[str, Any], steps: Sequence[str], environ: Optional[Mapping[str, str]] = None,
) -> List[Command]:
    requested = set(steps)
    unknown = sorted(requested - set(STEPS))
    if unknown:
        raise config.ConfigError(f"unknown step {unknown[0]!r}; known: {list(STEPS)}")

    xcfg, output_dir = _resolve_xcfg(cfg, environ)
    inputs = xcfg["inputs"]
    flux_dir = config.expand_env(inputs["flux_dir"], environ)
    periods = list(config.require(cfg, "periods"))
    energy_edges = config.require(cfg, "energy_edges")
    t_edges = _flatten_t_bins(config.require(cfg, "t_bins"))
    energy_str = ",".join(_num(e) for e in energy_edges)
    t_str = ",".join(_num(t) for t in t_edges)

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
        elif step == "qvalue":
            commands += _plan_qvalue(xcfg, output_dir)
        elif step == "fitfigs":
            commands += _plan_fitfigs(cfg, xcfg, output_dir, energy_edges, environ)
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


def _qvalue_missing_inputs_message(xcfg: Dict[str, Any], output_dir: str) -> Optional[str]:
    src_dir = _qvalue_source_dir(xcfg, output_dir)
    if any(src_dir.glob("diffout*.txt")):
        return None
    return (
        f"gxana: error: no qvalue input files (diffout*.txt) in {src_dir}; "
        f"set xsection.qvalue_source in analyses/<channel>/config/xsection.yaml "
        f"to a data/ label the tables step has written (currently "
        f"qvalue_source={xcfg.get('qvalue_source')!r})"
    )


def _fitfigs_missing_inputs_message(cfg: Dict[str, Any], xcfg: Dict[str, Any], output_dir: str,
                                    environ: Optional[Mapping[str, str]]) -> Optional[str]:
    """The first missing fitfigs input, named with the command that makes it.
    weighted_data/<label>/ is created empty up front for weighted_labels, so
    a label counts as present only once it holds weighted_diffxsec*.txt."""
    ff = _fit_figures(cfg, xcfg, output_dir, environ)
    for label in ff.labels:
        if label == QVALUES_LABEL:
            qdir = Path(tables_label_dir(output_dir, QVALUES_LABEL))
            if not any(qdir.glob("diffxsec*.txt")):
                return (f"gxana: error: fitfigs step: label {QVALUES_LABEL!r} has no diffxsec*.txt in {qdir}; "
                        f"run `gxana run xsection --steps qvalue` first")
            continue
        wdir = Path(f"{output_dir}/weighted_data/{label}")
        if not any(wdir.glob("weighted_diffxsec*.txt")):
            return (f"gxana: error: fitfigs step: label {label!r} has no weighted_diffxsec*.txt in {wdir}; "
                    f"run `gxana run xsection --steps tables,weight` first (add {label!r} to "
                    f"xsection.fits and xsection.weighted_labels if it is not there)")
        # MakeWeightedDiffXSecTGraphs.C converts every *diffxsec*.txt; the tex
        # step's syst_weighted_diffxsec_*.txt would add graphs and make
        # PlotFitComparison.C fail on the graph count.
        extra = sorted(p.name for p in wdir.glob("*diffxsec*.txt")
                       if not p.name.startswith("weighted_diffxsec_"))
        if extra:
            return (f"gxana: error: fitfigs step: {wdir} holds *diffxsec*.txt files other than "
                    f"weighted_diffxsec_* ({', '.join(extra)}); the graph conversion would pick them up. "
                    f"Run fitfigs before tex, or remove the syst_ files the tex step wrote there")
    missing = [pdf for _, pdf in ff.examples if not Path(pdf).is_file()]
    if missing:
        return ("gxana: error: fitfigs step: missing example fit PDFs: " + ", ".join(missing) +
                "; run `gxana run xsection --steps tables` with xsection.fit_plots set, or edit "
                "xsection.fit_figures.example_bin")
    return None


def _tex_missing_inputs_message(xcfg: Dict[str, Any], output_dir: str,
                                environ: Optional[Mapping[str, str]]) -> Optional[str]:
    _, _, _, additional = _tex_settings(xcfg, output_dir, environ)
    missing = [a for a in additional if not Path(a).is_file()]
    if not missing:
        return None
    return (
        "gxana: error: tex step: missing systematics input files: " + ", ".join(missing) +
        "; write them first with the systematics comparison macros "
        "(PlotFitComparison.C and PlotComboComparison.C) or edit xsection.tex.additional "
        "in analyses/<channel>/config/xsection.yaml"
    )


def run_xsection(
    cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
    runner: Runner = subprocess.run, environ: Optional[Mapping[str, str]] = None,
) -> int:
    requested = set(steps)
    unknown = sorted(requested - set(STEPS))
    if unknown:
        raise config.ConfigError(f"unknown step {unknown[0]!r}; known: {list(STEPS)}")
    if not dry_run:
        for d in _output_dirs(cfg, environ):
            d.mkdir(parents=True, exist_ok=True)
    for step in STEPS:
        if step not in requested:
            continue
        if step == "qvalue" and not dry_run:
            xcfg, output_dir = _resolve_xcfg(cfg, environ)
            message = _qvalue_missing_inputs_message(xcfg, output_dir)
            if message is not None:
                print(message)
                return 1
            # qvalue_rescale writes into data/qvalues/ without creating it
            Path(tables_label_dir(output_dir, QVALUES_LABEL)).mkdir(parents=True, exist_ok=True)
        if step == "tex" and not dry_run:
            xcfg, output_dir = _resolve_xcfg(cfg, environ)
            message = _tex_missing_inputs_message(xcfg, output_dir, environ)
            if message is not None:
                print(message)
                return 1
            Path(_tex_settings(xcfg, output_dir, environ)[1]).parent.mkdir(parents=True, exist_ok=True)
        if step == "fitfigs" and not dry_run:
            xcfg, output_dir = _resolve_xcfg(cfg, environ)
            message = _fitfigs_missing_inputs_message(cfg, xcfg, output_dir, environ)
            if message is not None:
                print(message)
                return 1
            ff = _fit_figures(cfg, xcfg, output_dir, environ)
            for d in (f"{output_dir}/weighted_data/{QVALUES_LABEL}", ff.comparisons_dir,
                      f"{ff.plots_dir}/fit_examples"):
                Path(d).mkdir(parents=True, exist_ok=True)
        for cmd in plan_xsection(cfg, [step], environ=environ):
            line = shlex.join(cmd.argv)
            print(f"(cd {shlex.quote(cmd.cwd)} && {line})" if cmd.cwd else line)
            if dry_run:
                continue
            kwargs = {"cwd": cmd.cwd} if cmd.cwd else {}
            result = runner(cmd.argv, check=False, **kwargs)
            rc = getattr(result, "returncode", 0) or 0
            if rc != 0:
                return rc
    return 0
