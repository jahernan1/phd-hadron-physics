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
KIND_STEPS = {"cutscan": ("fill", "fit", "plot")}


def _periods(cfg: Dict[str, Any]) -> List[str]:
    return list(gconfig.require(cfg, "periods"))


def expand(pattern: str, cfg: Dict[str, Any], period: Optional[str], environ: Env) -> str:
    """${GXANA_*} references, then {period} and {stem} (the period's data tree stem); {what} is left
    for gxana_study_cutscan. Any other placeholder is a ConfigError."""
    text = gconfig.expand_env(pattern, environ)
    values = {"what": "{what}"}
    if period is not None:
        values["period"] = period
        values["stem"] = gconfig.tree_stem(cfg, period, "data")
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


# kind -> (commands of one step, output directories, missing inputs of one step)
PLANNERS = {"cutscan": (_cutscan, _cutscan_dirs, _cutscan_missing)}


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
    config.studies(cfg, study_names)
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
