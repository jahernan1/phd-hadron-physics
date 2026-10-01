"""`gxana run studies`: plans the argv of gxana_study_<kind> per study, step and period (studies.yaml)."""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence

from gxana import config as gconfig
from gxana.stages.runner import Command, Env, Runner, check_steps, executable, num, run_steps
from gxana_studies import config

STEPS = ("fill", "fit", "plot")
DEFAULT_STEPS = STEPS
KIND_STEPS = {"cutscan": ("fill", "fit", "plot"), "datamc": ("fill", "plot")}


def _periods(cfg: Dict[str, Any]) -> List[str]:
    return list(gconfig.require(cfg, "periods"))


def expand(pattern: str, cfg: Dict[str, Any], period: Optional[str], environ: Env,
           mc_sample: Optional[str] = None) -> str:
    """${GXANA_*} references, then {period}, {stem} (the period's data tree stem) and {mc_stem} (the
    period's tree stem of mc_sample); {what} is left for gxana_study_cutscan. Any other placeholder is
    a ConfigError."""
    text = gconfig.expand_env(pattern, environ)
    values = {"what": "{what}"}
    if period is not None:
        values["period"] = period
        values["stem"] = gconfig.tree_stem(cfg, period, "data")
        if "{mc_stem}" in text:
            values["mc_stem"] = gconfig.tree_stem(cfg, period, mc_sample)
    try:
        return text.format(**values)
    except (KeyError, IndexError, ValueError) as err:
        raise gconfig.ConfigError(f"{pattern!r}: unknown or malformed placeholder ({err})") from None


def _out(s: Dict[str, Any], pattern: str, cfg: Dict[str, Any], period: Optional[str], environ: Env) -> str:
    """An output path: a relative pattern is under the study's out_dir."""
    return os.path.join(expand(s["out_dir"], cfg, None, environ), expand(pattern, cfg, period, environ))


def _steps_argv(steps: Sequence[Dict[str, str]], prefix: str = "") -> List[str]:
    argv: List[str] = []
    for step in steps:
        if "filter" in step:
            argv += ["--filter", prefix + step["filter"]]
        else:
            argv += ["--define", f"{prefix}{step['define']}={step['expr']}"]
    return argv


def _axis(block: Dict[str, Any]) -> str:
    return block["var"] + ":" + ",".join(num(v) for v in block["bins"])


def _cutscan(cfg: Dict[str, Any], name: str, s: Dict[str, Any], step: str, environ: Env) -> List[Command]:
    exe = executable("gxana_study_cutscan", environ)
    o = s["outputs"]
    cmds = []
    for period in _periods(cfg):
        hist = _out(s, o["hist"], cfg, period, environ)
        tables = _out(s, o["tables"], cfg, period, environ)
        if step == "fill":
            argv = [exe, "fill", "--input", expand(s["input"], cfg, period, environ), "--tree", s["tree"],
                    "--out", hist]
            if "weight" in s:
                argv += ["--weight", s["weight"]]
            argv += _steps_argv(s["steps"]) + ["--mass", _axis(s["mass"]), "--scan", _axis(s["scan"])]
        elif step == "fit":
            fit = s["fit"]
            argv = [exe, "fit", "--hist", hist, "--tables", tables, "--grid-pdf", _out(s, o["grid"], cfg, period, environ),
                    "--first-bin", num(s["scan"]["first_bin"]), "--panel-label", s["panel_label"],
                    "--mass-title", fit["mass_title"], "--range", ",".join(num(v) for v in fit["range"])]
            for p in config.FIT_PARAMS:
                argv += ["--param", f"{p}={fit['params'][p]}"]
        else:
            argv = [exe, "plot", "--tables", tables, "--title", s["plot_title"], "--cut", num(s["cut"]),
                    "--name", name]
            for pdf in o["plots"]:
                argv += ["--pdf", _out(s, pdf, cfg, period, environ)]
        cmds.append(Command(argv, step))
    return cmds


def _cutscan_dirs(cfg: Dict[str, Any], s: Dict[str, Any], environ: Env) -> List[Path]:
    o = s["outputs"]
    return [Path(_out(s, pattern, cfg, period, environ)).parent
            for period in _periods(cfg) for pattern in [o["hist"], o["tables"], o["grid"]] + list(o["plots"])]


def _cutscan_missing(cfg: Dict[str, Any], name: str, s: Dict[str, Any], step: str, environ: Env) -> List[str]:
    again = f"gxana run studies --channel {gconfig.require(cfg, 'channel')} --study {name} --steps"
    missing = []
    for period in _periods(cfg):
        if step == "fill":
            need = [(expand(s["input"], cfg, period, environ), f"input of study {name}")]
        elif step == "fit":
            need = [(_out(s, s["outputs"]["hist"], cfg, period, environ), f"{again} fill")]
        else:
            tables = _out(s, s["outputs"]["tables"], cfg, period, environ)
            need = [(tables.replace("{what}", w), f"{again} fit") for w in ("FOM", "SB")]
        missing += [f"{path} ({how})" for path, how in need if not Path(path).is_file()]
    return missing


def _hist_file(cfg: Dict[str, Any], s: Dict[str, Any], environ: Env) -> str:
    return _out(s, s["hist_file"], cfg, None, environ)


def _datamc_inputs(cfg: Dict[str, Any], s: Dict[str, Any], period: str, environ: Env) -> List[str]:
    return [expand(s["inputs"][k], cfg, period, environ, s.get("mc_sample")) for k in config.SAMPLES]


def _period_dir(cfg: Dict[str, Any], period: str) -> str:
    """The period's ROOT directory (periods.yaml `dir`), or its name, as gxana::Period::Dir()."""
    return gconfig.period_settings(cfg, period).get("dir") or period


def _datamc(cfg: Dict[str, Any], name: str, s: Dict[str, Any], step: str, environ: Env) -> List[Command]:
    exe = executable("gxana_study_datamc", environ)
    if step == "fill":
        argv = [exe, "fill", "--out", _hist_file(cfg, s, environ), "--tree", s["tree"], "--thrown-tree",
                s["thrown_tree"]]
        for period in _periods(cfg):
            data, mc, thrown = _datamc_inputs(cfg, s, period, environ)
            argv += ["--period", f"{period}:{_period_dir(cfg, period)}:{data}:{mc}:{thrown}"]
        for sample in config.SAMPLES:
            block = s["samples"].get(sample, {})
            argv += _steps_argv(block.get("steps", []), sample + ":")
            if "weight" in block:
                argv += ["--weight", f"{sample}:{block['weight']}"]
        argv += [a for v in s["vars"] for a in ("--var", v["var"])]
        argv += [a for v in s.get("truth_vars", []) for a in ("--truth-var", v["var"])]
    else:
        argv = [exe, "plot", "--in", _hist_file(cfg, s, environ), "--out-dir", expand(s["out_dir"], cfg, None, environ)]
        for period in _periods(cfg):
            argv += ["--period", f"{_period_dir(cfg, period)}:{s['tags'][period]}"]
        for key, flag in (("vars", "--var"), ("truth_vars", "--truth-var")):
            argv += [a for v in s.get(key, []) for a in (flag, f"{v['var']}:{v.get('legend', 'tr')}:{v['title']}")]
    return [Command(argv, step)]


def _datamc_dirs(cfg: Dict[str, Any], s: Dict[str, Any], environ: Env) -> List[Path]:
    return [Path(expand(s["out_dir"], cfg, None, environ)), Path(_hist_file(cfg, s, environ)).parent]


def _datamc_missing(cfg: Dict[str, Any], name: str, s: Dict[str, Any], step: str, environ: Env) -> List[str]:
    if step == "fill":
        return [f"{path} (input of study {name})" for period in _periods(cfg)
                for path in _datamc_inputs(cfg, s, period, environ) if not Path(path).is_file()]
    hist = _hist_file(cfg, s, environ)
    again = f"gxana run studies --channel {gconfig.require(cfg, 'channel')} --study {name} --steps fill"
    return [] if Path(hist).is_file() else [f"{hist} ({again})"]


# kind -> (commands of one step, output directories, missing inputs of one step)
PLANNERS = {"cutscan": (_cutscan, _cutscan_dirs, _cutscan_missing),
            "datamc": (_datamc, _datamc_dirs, _datamc_missing)}


def plan(cfg: Dict[str, Any], steps: Sequence[str], study_names: Optional[Sequence[str]] = None,
         environ: Env = None) -> List[Command]:
    """Every command of `steps` (in STEPS order), study by study in config order, period by period."""
    check_steps(steps, STEPS)
    chosen = config.studies(cfg, study_names)
    cmds: List[Command] = []
    for step in STEPS:
        if step not in steps:
            continue
        for name, s in chosen:
            if step in KIND_STEPS[s["kind"]]:
                cmds += PLANNERS[s["kind"]][0](cfg, name, s, step, environ)
    return cmds


def output_dirs(cfg: Dict[str, Any], study_names: Optional[Sequence[str]], environ: Env) -> List[Path]:
    dirs: List[Path] = []
    for _, s in config.studies(cfg, study_names):
        dirs += PLANNERS[s["kind"]][1](cfg, s, environ)
    return sorted(set(dirs))


def preflight(cfg: Dict[str, Any], step: str, study_names: Optional[Sequence[str]], environ: Env) -> List[str]:
    """Missing inputs of `step`, each with what makes it."""
    missing: List[str] = []
    for name, s in config.studies(cfg, study_names):
        if step in KIND_STEPS[s["kind"]]:
            missing += PLANNERS[s["kind"]][2](cfg, name, s, step, environ)
    return missing


def run_studies(cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False, runner: Runner = subprocess.run,
                environ: Env = None, study_names: Optional[Sequence[str]] = None) -> int:
    """Print every command; unless dry_run, make the output directories, check each step's inputs
    (nothing of a step runs when one is missing) and run, stopping at the first failure."""
    check_steps(steps, STEPS)
    chosen = config.studies(cfg, study_names)
    if not any(step in KIND_STEPS[s["kind"]] for _, s in chosen for step in steps):
        raise gconfig.ConfigError(f"no selected study has step(s) {','.join(steps)}")
    if not dry_run:
        for d in output_dirs(cfg, study_names, environ):
            d.mkdir(parents=True, exist_ok=True)

    def before(step: str) -> Optional[int]:
        if dry_run:
            return None
        missing = preflight(cfg, step, study_names, environ)
        if missing:
            print(f"gxana: error: {step}: missing inputs:\n" + "\n".join(f"  {m}" for m in missing), file=sys.stderr)
            return 1
        return None

    return run_steps(steps, STEPS, lambda step: plan(cfg, [step], study_names, environ),
                     dry_run=dry_run, runner=runner, before=before)
