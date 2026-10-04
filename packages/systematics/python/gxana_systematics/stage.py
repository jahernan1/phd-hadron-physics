"""`gxana run systematics`: the systematic studies of analyses/<channel>/config/systematics.yaml.

  fit      gxana_xsec_tables per variant group -> <out>/variants/data/<label>/
  qvalue   gxana_xsection.qvalue_rescale        -> <out>/variants/data/<qvalue label>/
  weight   gxana_xsection.weighted_average      -> <out>/variants/weighted_data/<label>/
  spread   gxana_systematics.{spread,sfactor} + gxana_syst_plot -> <out>/<study>/
  track    gxana_syst_track + gxana_systematics.track           -> <out>/<study>/
  runperiod  <channel>/<runperiod.macro> (opt-in)               -> <out>/runperiod/
  compare  gxana_systematics.runcompare + gxana_syst_plot (opt-in) -> <out>/<study>/
  summary  gxana_systematics.summary                            -> <out>/summary/
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana import config as gconfig
from gxana.bins import energy_args
from gxana.stages import xsection as xs
from gxana.stages.runner import (Command, Env, Runner, check_steps, executable, num, python_module, root_macro,
                                 run_steps)
from gxana_systematics import config

STEPS = ("fit", "qvalue", "weight", "spread", "track", "runperiod", "compare", "summary")
DEFAULT_STEPS = ("fit", "qvalue", "weight", "spread", "track", "summary")
MOVED_TO_BARLOW = ("bin", "tables", "barlow")
_MOVED = {step: "barlow" for step in MOVED_TO_BARLOW}


def output_dir(cfg: Dict[str, Any], environ: Env) -> str:
    return gconfig.expand_env(config.block(cfg)["output_dir"], environ)


def _xs_output(cfg: Dict[str, Any], environ: Env) -> str:
    return gconfig.expand_env(gconfig.require(cfg, "xsection")["output_dir"], environ)


def _pool(cfg, environ, kind: str) -> str:
    return f"{output_dir(cfg, environ)}/variants/{kind}"


def label_dir(cfg: Dict[str, Any], label: str, environ: Env) -> str:
    scfg = config.block(cfg)
    if label in config.pool_labels(scfg):
        return f"{_pool(cfg, environ, 'weighted_data')}/{label}"
    if label == config.nominal(cfg):
        return f"{_xs_output(cfg, environ)}/weighted_data/{label}"
    raise gconfig.ConfigError(f"label {label!r} is neither a systematics variant nor the nominal")


def selected(cfg: Dict[str, Any], study_names: Optional[Sequence[str]]):
    """The studies to run and the variant groups / qvalue variants they need."""
    scfg = config.block(cfg)
    chosen = config.studies(scfg, study_names)
    needed = set()
    for name, study in chosen:
        if study["kind"] in ("spread", "compare"):
            needed |= set(config.study_labels(name, study, per_period=False))
    qvalues = [q for q in config.qvalue_variants(scfg) if q["label"] in needed]
    needed |= {q["source"] for q in qvalues}
    groups = [g for g in config.fit_groups(scfg) if any(e["label"] in needed for e in g["labels"])]
    return chosen, groups, qvalues


def _plan_fit(cfg, groups, environ) -> List[Command]:
    return [Command(c.argv, "fit") for c in xs.tables_commands(
        cfg, groups, _pool(cfg, environ, "data"), _pool(cfg, environ, "fits"), environ)]


def _plan_qvalue(cfg, qvalues, environ) -> List[Command]:
    commands = []
    for q in qvalues:
        src = Path(f"{_pool(cfg, environ, 'data')}/{q['source']}")
        out = Path(f"{_pool(cfg, environ, 'data')}/{q['label']}")
        file1 = sorted(src.glob("diffout*.txt"))
        file2 = sorted(src.glob("diffxsec*.txt"))
        if len(file1) != len(file2):
            raise gconfig.ConfigError(f"qvalue: {src} has {len(file1)} diffout*.txt but {len(file2)} diffxsec*.txt")
        for f1, f2 in zip(file1, file2):
            commands.append(Command(python_module("gxana_xsection",
                "qvalue_rescale", str(f1), "data_yield", "qval_yield", str(f2), str(out / f2.name)), "qvalue"))
    return commands


def _plan_weight(cfg, groups, qvalues, environ) -> List[Command]:
    commands = []
    edges = gconfig.require(cfg, "energy_edges")
    labels = [(e["label"], True) for g in groups for e in g["labels"]] + [(q["label"], False) for q in qvalues]
    for label, has_total in labels:
        commands += xs.weighted_average_commands(f"{_pool(cfg, environ, 'data')}/{label}",
                                                 f"{_pool(cfg, environ, 'weighted_data')}/{label}", edges, "weight",
                                                 n_periods=len(gconfig.require(cfg, "periods")), total=has_total)
    return commands


def study_dir(cfg, name: str, environ: Env) -> str:
    return f"{output_dir(cfg, environ)}/{name}"


def stats_path(cfg, name: str, study: Dict[str, Any], environ: Env) -> str:
    return f"{study_dir(cfg, name, environ)}/{study['stats']}"


def _examples(cfg, study, environ) -> List[Tuple[str, str]]:
    ex = study.get("examples")
    if not ex:
        return []
    stem = gconfig.tree_stem(cfg, gconfig.require(ex["bin"], "period"), "data")
    pdf = f"data_flatTree_{stem}_{gconfig.require(ex['bin'], 'tree')}.pdf"
    return [(name, f"{_pool(cfg, environ, 'fits')}/{label}/{pdf}") for name, label in ex["fits"].items()]


def _plan_spread(cfg, chosen, environ) -> List[Command]:
    scfg = config.block(cfg)
    nominal = config.nominal(cfg)
    commands: List[Command] = []
    checked = False
    for name, study in chosen:
        kind = study["kind"]
        if kind == "sfactor":
            argv = python_module("gxana_systematics", "sfactor", "--out", stats_path(cfg, name, study, environ),
                               "--periods-dir", f"{_xs_output(cfg, environ)}/data/{nominal}",
                               "--n-periods", str(len(gconfig.require(cfg, "periods")))) + energy_args(gconfig.require(cfg, "energy_edges"))
            commands.append(Command(argv, "spread"))
        elif kind == "spread":
            if (not checked and nominal in config.pool_labels(scfg)
                    and nominal in config.study_labels(name, study)):
                commands.append(Command(python_module("gxana_systematics",
                    "tables", "--same", label_dir(cfg, nominal, environ),
                    f"{_xs_output(cfg, environ)}/weighted_data/{nominal}"), "spread"))
                checked = True
            argv = python_module("gxana_systematics", "spread", "--out", stats_path(cfg, name, study, environ))
            for label in study["spread"]:
                argv += ["--member", f"{label}={label_dir(cfg, label, environ)}"]
            commands.append(Command(argv, "spread"))
            commands += plot_commands(cfg, name, study, environ)
            for fig, pdf in _examples(cfg, study, environ):
                commands.append(Command(["cp", pdf, f"{study_dir(cfg, name, environ)}/plots/fit_examples/{fig}.pdf"],
                                        "spread"))
    return commands


PERIOD_LAYOUTS = ("run_grid", "stddev_band")
BAND_LAYOUTS = ("pair_band", "all_band", "stddev_band")


def period_prefixes(cfg, label: str, environ: Env) -> List[str]:
    """<xs>/data/<label>/diffxsec_flatTree_<stem>, one per run period (gxana_syst_plot run_grid inputs)."""
    return [f"{_xs_output(cfg, environ)}/data/{label}/diffxsec_flatTree_{gconfig.tree_stem(cfg, period, 'data')}"
            for period in gconfig.require(cfg, "periods")]


def plot_commands(cfg, name: str, study: Dict[str, Any], environ: Env, step: str = "spread") -> List[Command]:
    """One gxana_syst_plot invocation per entry of the study's `plots`."""
    exe = executable("gxana_syst_plot", environ)
    commands = []
    for plot in study.get("plots") or []:
        argv = [exe, "--layout", plot["layout"], "--name", plot["name"],
                "--out-dir", f"{study_dir(cfg, name, environ)}/plots"]
        if plot["layout"] in PERIOD_LAYOUTS:
            inputs = period_prefixes(cfg, gconfig.require(study, "per_period"), environ)
        else:
            inputs = [label_dir(cfg, label, environ) for label in plot.get("labels") or []]
        for path in inputs:
            argv += ["--input", path]
        if plot["layout"] in BAND_LAYOUTS:
            argv += ["--band", stats_path(cfg, name, study, environ)]
        for entry in plot.get("legend") or []:
            argv += ["--legend", entry]
        if plot.get("legend_header"):
            argv += ["--legend-header", plot["legend_header"]]
        if plot.get("first_style"):
            argv += ["--first-style", plot["first_style"]]
        for item in plot.get("annotate") or []:
            argv += ["--annotate", item]
        for key, opt in (("axis_format", "--axis-format"), ("x_axis_format", "--x-axis-format"),
                         ("xmax", "--xmax"), ("ymax", "--ymax")):
            if key in plot:
                argv += [opt, num(plot[key])]
        commands.append(Command(argv, step))
    return commands


def _unavailable_label(cfg, plot: Dict[str, Any], environ: Env, runtime: bool) -> Optional[Tuple[str, str]]:
    """(label, reason) of the first label of `plot` that cannot be drawn, else None."""
    for label in plot.get("labels") or []:
        try:
            d = label_dir(cfg, label, environ)
        except gconfig.ConfigError:
            return label, "is not configured"
        if runtime and not any(Path(d).glob("weighted_diffxsec_emin_*.txt")):
            return label, f"has no weighted_diffxsec_emin_*.txt in {d}"
    return None


def _plan_compare(cfg, chosen, environ, runtime: bool = False) -> List[Command]:
    commands: List[Command] = []
    for name, study in chosen:
        if study["kind"] != "compare":
            continue
        if study.get("per_period"):
            label = study["per_period"]
            argv = python_module("gxana_systematics", "runcompare", "--out", stats_path(cfg, name, study, environ),
                               "--periods-dir", f"{_xs_output(cfg, environ)}/data/{label}",
                               "--n-periods", str(len(gconfig.require(cfg, "periods")))) + energy_args(gconfig.require(cfg, "energy_edges"))
            commands.append(Command(argv, "compare"))
            commands += plot_commands(cfg, name, study, environ, step="compare")
            continue
        for plot in study.get("plots") or []:
            missing = _unavailable_label(cfg, plot, environ, runtime)
            if missing:
                print(f"gxana: note: compare study {name!r} plot {plot['name']!r} skipped: "
                      f"label {missing[0]!r} {missing[1]}")
                continue
            commands += plot_commands(cfg, name, {**study, "plots": [plot]}, environ, step="compare")
    return commands


def _plan_runperiod(cfg, environ) -> List[Command]:
    runperiod = config.block(cfg).get("runperiod")
    if not runperiod:
        print("gxana: note: runperiod step skipped: systematics.runperiod is not configured")
        return []
    channel = gconfig.require(cfg, "channel")
    nominal = config.nominal(cfg)
    argv = root_macro(environ, channel, runperiod["macro"],
                      [nominal, f"{output_dir(cfg, environ)}/runperiod", f"{_xs_output(cfg, environ)}/data/{nominal}/"])
    return [Command(argv, "runperiod")]


def _track_inputs(cfg, environ) -> List[Tuple[str, str, str, str]]:
    """(period, data, mc, thrown) per period, from xsection.inputs."""
    xcfg = gconfig.require(cfg, "xsection")
    inputs = xcfg["inputs"]
    mc = xcfg["mc_sample"]
    out = []
    for period in gconfig.require(cfg, "periods"):
        stem = gconfig.tree_stem(cfg, period, "data")
        mc_stem = gconfig.tree_stem(cfg, period, mc)
        data = gconfig.expand_env(inputs["data"], environ).format(stem=stem)
        mc_path = gconfig.expand_env(inputs["mc"], environ).format(mc_stem=mc_stem)
        thrown = gconfig.expand_env(inputs["thrown"], environ).format(mc_stem=mc_stem)
        out.append((period, data, mc_path, thrown))
    return out


def _plan_track(cfg, chosen, environ) -> List[Command]:
    commands = []
    for name, study in chosen:
        if study["kind"] != "track":
            continue
        out = study_dir(cfg, name, environ)
        argv = [executable("gxana_syst_track", environ), "--out-dir", out,
                "--tree", study["tree"], "--thrown-tree", study["thrown_tree"],
                "--data-weight", study.get("data_weight", ""), "--mc-weight", study.get("mc_weight", ""),
                "--theta-cut", num(study["theta_cut_deg"]), "--low", num(study["low"]),
                "--high", num(study["high"]), "--legend-header", study.get("legend_header", "")]
        for period, data, mc_path, thrown in _track_inputs(cfg, environ):
            argv += ["--period", f"{period}:{data}:{mc_path}:{thrown}"]
        for p in study["particles"]:
            theta = ",".join(num(v) for v in p["theta"])
            pbins = ",".join(num(v) for v in p["p"])
            argv += ["--particle", f"{p['name']}:{p['p4']}:{p['thrown_p4']}:{theta}:{pbins}:{p['title']}"]
        commands.append(Command(argv, "track"))
        track_argv = python_module("gxana_systematics", "track", "--counts", f"{out}/track_counts.txt", "--out", f"{out}/track_efficiency.txt",
                          "--low", num(study["low"]), "--high", num(study["high"]),
                          "--report", study["report"])
        for pname, value in (study.get("override") or {}).items():
            track_argv += ["--override", f"{pname}={num(value)}"]
        commands.append(Command(track_argv, "track"))
    return commands


def _summary_columns(cfg, environ) -> List[Tuple[str, str]]:
    scfg = config.block(cfg)
    summ = scfg.get("summary") or {}
    return [(name, stats_path(cfg, name, scfg["studies"][name], environ))
            for name in summ.get("point_by_point") or []]


def _summary_normalization(cfg, environ) -> List[Tuple[str, str]]:
    scfg = config.block(cfg)
    summ = scfg.get("summary") or {}
    out = []
    for name in summ.get("normalization") or []:
        study = scfg["studies"][name]
        if study["kind"] == "constant":
            out.append((name, num(study["value"])))
        elif study["kind"] == "track":
            out.append((name, f"{study_dir(cfg, name, environ)}/track_efficiency.txt"))
        else:
            out.append((name, stats_path(cfg, name, study, environ)))
    return out


def _summary_inputs(cfg) -> List[str]:
    summ = config.block(cfg).get("summary") or {}
    return list(summ.get("point_by_point") or []) + list(summ.get("normalization") or [])


def _summary_excluded(cfg, study_names: Optional[Sequence[str]]) -> List[str]:
    """Summary input studies left out by --study (the summary step is skipped then)."""
    if study_names is None:
        return []
    return [name for name in _summary_inputs(cfg) if name not in study_names]


def _plan_summary(cfg, study_names, environ) -> List[Command]:
    if not (config.block(cfg).get("summary") or {}):
        return []
    excluded = _summary_excluded(cfg, study_names)
    if excluded:
        print(f"gxana: note: summary step skipped: --study excludes {', '.join(excluded)}")
        return []
    argv = python_module("gxana_systematics", "summary", "--nominal-dir", f"{_xs_output(cfg, environ)}/weighted_data/{config.nominal(cfg)}",
                       "--out-dir", f"{output_dir(cfg, environ)}/summary")
    for name, path in _summary_columns(cfg, environ):
        argv += ["--column", f"{name}={path}"]
    for name, spec in _summary_normalization(cfg, environ):
        argv += ["--normalization", f"{name}={spec}"]
    return [Command(argv, "summary")]


def plan(cfg: Dict[str, Any], steps: Sequence[str], study_names: Optional[Sequence[str]] = None,
         environ: Env = None, runtime: bool = False) -> List[Command]:
    """The commands of `steps`. runtime: also skip compare plots whose label tables do not exist."""
    check_steps(steps, STEPS, moved=_MOVED)
    chosen, groups, qvalues = selected(cfg, study_names)
    commands: List[Command] = []
    for step in STEPS:
        if step not in steps:
            continue
        if step == "fit":
            commands += _plan_fit(cfg, groups, environ)
        elif step == "qvalue":
            commands += _plan_qvalue(cfg, qvalues, environ)
        elif step == "weight":
            commands += _plan_weight(cfg, groups, qvalues, environ)
        elif step == "spread":
            commands += _plan_spread(cfg, chosen, environ)
        elif step == "track":
            commands += _plan_track(cfg, chosen, environ)
        elif step == "runperiod":
            commands += _plan_runperiod(cfg, environ)
        elif step == "compare":
            commands += _plan_compare(cfg, chosen, environ, runtime)
        elif step == "summary":
            commands += _plan_summary(cfg, study_names, environ)
    return commands


def preflight(cfg: Dict[str, Any], step: str, study_names: Optional[Sequence[str]], environ: Env) -> List[str]:
    """Missing inputs of `step`, each with the command that makes it."""
    chosen, groups, qvalues = selected(cfg, study_names)
    channel = gconfig.require(cfg, "channel")
    xs_out = _xs_output(cfg, environ)
    missing: List[str] = []
    if step == "fit" and groups:
        for period in gconfig.require(cfg, "periods"):
            xcfg = gconfig.require(cfg, "xsection")
            for path in xs.tables_paths(cfg, xcfg, period, xs_out)[1:]:
                if not Path(path).is_file():
                    missing.append(f"{path} (gxana run xsection --channel {channel} --steps bin)")
    if step == "qvalue":
        for q in qvalues:
            src = Path(f"{_pool(cfg, environ, 'data')}/{q['source']}")
            if not any(src.glob("diffout*.txt")):
                missing.append(f"{src}/diffout*.txt (gxana run systematics --channel {channel} --steps fit)")
    if step == "weight":
        for label in [e["label"] for g in groups for e in g["labels"]] + [q["label"] for q in qvalues]:
            d = Path(f"{_pool(cfg, environ, 'data')}/{label}")
            if not any(d.glob("diffxsec*.txt")):
                missing.append(f"{d}/diffxsec*.txt (gxana run systematics --channel {channel} --steps fit,qvalue)")
    if step == "spread":
        nominal = config.nominal(cfg)
        for name, study in chosen:
            if study["kind"] == "sfactor":
                d = Path(f"{xs_out}/data/{nominal}")
                if not any(d.glob("diffxsec*_emin_*.txt")):
                    missing.append(f"{d}/diffxsec*_emin_*.txt (gxana run xsection --channel {channel} "
                                   f"--steps tables)")
            if study["kind"] == "spread":
                for label in config.study_labels(name, study):
                    d = Path(label_dir(cfg, label, environ))
                    if not any(d.glob("weighted_diffxsec_emin_*.txt")):
                        how = (f"gxana run systematics --channel {channel} --steps fit,qvalue,weight"
                               if label in config.pool_labels(config.block(cfg))
                               else f"gxana run xsection --channel {channel} --steps tables,weight")
                        missing.append(f"{d}/weighted_diffxsec_emin_*.txt ({how})")
                for _, pdf in _examples(cfg, study, environ):
                    if not Path(pdf).is_file():
                        missing.append(f"{pdf} (gxana run systematics --channel {channel} --steps fit)")
    if step == "track" and any(study["kind"] == "track" for _, study in chosen):
        stage_hint = f"or gxana data stage --channel {channel}"
        sample = config.require(cfg, "xsection")["mc_sample"]
        for period, data, mc_path, thrown in _track_inputs(cfg, environ):
            if not Path(data).is_file():
                missing.append(f"{data} (gxana run qfactors --channel {channel} --period {period}, {stage_hint})")
            if not Path(mc_path).is_file():
                missing.append(f"{mc_path} (selection/flatTreePrep.C on the reconstructed MC, {stage_hint})")
            if not Path(thrown).is_file():
                missing.append(f"{thrown} (gxana run select --channel {channel} --period {period} "
                               f"--sample {sample} --thrown, {stage_hint})")
    if step == "runperiod" and config.block(cfg).get("runperiod"):
        d = Path(f"{xs_out}/data/{config.nominal(cfg)}")
        if not any(d.glob("diffxsec*_emin_*.txt")):
            missing.append(f"{d}/diffxsec*_emin_*.txt (gxana run xsection --channel {channel} --steps tables)")
    if step == "compare":
        for name, study in chosen:
            if study["kind"] == "compare" and study.get("per_period"):
                d = Path(f"{xs_out}/data/{study['per_period']}")
                if not any(d.glob("diffxsec*_emin_*.txt")):
                    missing.append(f"{d}/diffxsec*_emin_*.txt (gxana run xsection --channel {channel} "
                                   f"--steps tables)")
    if step == "summary" and not _summary_excluded(cfg, study_names):
        paths = [p for _, p in _summary_columns(cfg, environ)]
        paths += [s for _, s in _summary_normalization(cfg, environ) if Path(s).suffix == ".txt"]
        for path in paths:
            if not Path(path).is_file():
                missing.append(f"{path} (gxana run systematics --channel {channel} --steps spread,track)")
    return missing


def _reads_nominal(cfg, steps: Sequence[str], chosen, study_names) -> bool:
    """Whether a selected step reads the nominal xsection tables."""
    scfg = config.block(cfg)
    nominal = config.nominal(cfg)
    for name, study in chosen:
        kind = study["kind"]
        if "spread" in steps and kind == "sfactor":
            return True
        if "spread" in steps and kind == "spread" and nominal in config.study_labels(name, study):
            return True
        if "compare" in steps and kind == "compare" and study.get("per_period") == nominal:
            return True
    if "runperiod" in steps and scfg.get("runperiod"):
        return True
    return "summary" in steps and bool(scfg.get("summary")) and not _summary_excluded(cfg, study_names)


def nominal_missing(cfg, steps: Sequence[str], chosen, study_names, environ: Env) -> List[str]:
    """The nominal xsection table patterns that a selected step reads and that do not exist."""
    if not _reads_nominal(cfg, steps, chosen, study_names):
        return []
    xs_out, nominal = _xs_output(cfg, environ), config.nominal(cfg)
    patterns = (f"{xs_out}/weighted_data/{nominal}/weighted_diffxsec_emin_*.txt",
                f"{xs_out}/data/{nominal}/diffxsec*_emin_*.txt")
    return [p for p in patterns if not any(Path(p).parent.glob(Path(p).name))]


def _mkdirs(cfg, groups, qvalues, environ) -> None:
    for kind in ("data", "fits", "weighted_data"):
        for label in [e["label"] for g in groups for e in g["labels"]] + [q["label"] for q in qvalues]:
            Path(f"{_pool(cfg, environ, kind)}/{label}").mkdir(parents=True, exist_ok=True)


def run_systematics(cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
                    runner: Runner = subprocess.run, environ: Env = None,
                    study_names: Optional[Sequence[str]] = None) -> int:
    check_steps(steps, STEPS, moved=_MOVED)
    config.validate(cfg)
    chosen, groups, qvalues = selected(cfg, study_names)
    if not dry_run:
        missing = nominal_missing(cfg, steps, chosen, study_names, environ)
        if missing:
            channel = gconfig.require(cfg, "channel")
            print(f"gxana: error: the nominal {config.nominal(cfg)!r} cross section is missing (make it with "
                  f"`gxana run xsection --channel {channel} --steps tables,weight`):\n"
                  + "\n".join(f"  {m}" for m in missing), file=sys.stderr)
            return 1
        _mkdirs(cfg, groups, qvalues, environ)

    def before(step: str) -> Optional[int]:
        if step == "spread" and not dry_run:
            for name, study in chosen:
                if study["kind"] in ("spread", "sfactor"):
                    Path(f"{study_dir(cfg, name, environ)}/plots/fit_examples").mkdir(parents=True, exist_ok=True)
        if step == "track" and not dry_run:
            for name, study in chosen:
                if study["kind"] == "track":
                    Path(study_dir(cfg, name, environ)).mkdir(parents=True, exist_ok=True)
        if step == "compare" and not dry_run:
            for name, study in chosen:
                if study["kind"] == "compare":
                    Path(f"{study_dir(cfg, name, environ)}/plots").mkdir(parents=True, exist_ok=True)
        if step == "runperiod" and not dry_run:
            Path(f"{output_dir(cfg, environ)}/runperiod").mkdir(parents=True, exist_ok=True)
        if not dry_run:
            missing = preflight(cfg, step, study_names, environ)
            if missing:
                print(f"gxana: error: {step}: missing inputs:\n" + "\n".join(f"  {m}" for m in missing),
                      file=sys.stderr)
                return 1
        return None

    return run_steps(steps, STEPS, lambda step: plan(cfg, [step], study_names, environ, runtime=not dry_run),
                     dry_run=dry_run, runner=runner, before=before)
