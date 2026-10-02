"""`gxana run measurements`: the measurement macros of analyses/<channel>/config/measurements.yaml.

Each item has a prep and/or a fit step, each one channel macro run as
`root -l -b -q rootlogon.C <macro>(<args>)` from measurements.output_dir, where the macros read and
write their ROOT files. output_dir and every make_dirs entry are created first: a macro saving a PDF
into a missing directory prints an error and still exits 0. Step by step (prep, then fit), item by
item in config order; the run stops at the first command that fails."""
from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana import config as gconfig
from gxana.stages.runner import Command, Env, Runner, check_steps, cxx_arg, gxana_root, root_macro, run_steps

STEPS = ("prep", "fit")
DEFAULT_STEPS = STEPS


def _text(value: Any, where: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise gconfig.ConfigError(f"{where}: need a non-empty string, got {value!r}")
    return value


def block(cfg: Dict[str, Any]) -> Dict[str, Any]:
    """The `measurements` block, checked: output_dir, optional make_dirs, items {name: {prep|fit: {macro, args}}}."""
    m = gconfig.check_block(gconfig.require(cfg, "measurements"), ("output_dir", "make_dirs", "items"),
                            "measurements", ("output_dir", "items"))
    _text(m["output_dir"], "measurements.output_dir")
    dirs = m.get("make_dirs", [])
    if not isinstance(dirs, list):
        raise gconfig.ConfigError(f"measurements.make_dirs: need a list of directories, got {dirs!r}")
    for i, d in enumerate(dirs):
        _text(d, f"measurements.make_dirs[{i}]")
    items = m["items"]
    if not isinstance(items, dict) or not items:
        raise gconfig.ConfigError("measurements.items: need a mapping of item name -> {prep, fit}")
    for name, item in items.items():
        where = f"measurements.items.{name}"
        if not gconfig.check_block(item, STEPS, where):
            raise gconfig.ConfigError(f"{where}: needs prep and/or fit")
        for step, call in item.items():
            gconfig.check_block(call, ("macro", "args"), f"{where}.{step}", ("macro",))
            _text(call["macro"], f"{where}.{step}.macro")
            if "args" in call:
                if not isinstance(call["args"], list):
                    raise gconfig.ConfigError(f"{where}.{step}.args: need a list, got {call['args']!r}")
                for value in call["args"]:
                    cxx_arg(value)
    return m


def output_dir(cfg: Dict[str, Any], environ: Env) -> str:
    return gconfig.expand_env(block(cfg)["output_dir"], environ)


def selected(cfg: Dict[str, Any], items: Optional[Sequence[str]]) -> List[Tuple[str, Dict[str, Any]]]:
    """The items to run, in config order; `items` selects (an unknown name is an error)."""
    all_items = block(cfg)["items"]
    for name in items or []:
        if name not in all_items:
            raise gconfig.ConfigError(f"unknown item {name!r}; known: {list(all_items)}")
    return [(n, i) for n, i in all_items.items() if items is None or n in items]


def plan(cfg: Dict[str, Any], steps: Sequence[str], items: Optional[Sequence[str]] = None,
         environ: Env = None) -> List[Command]:
    """The macro calls of `steps` (in STEPS order), item by item, each run from output_dir."""
    check_steps(steps, STEPS)
    channel = gconfig.require(cfg, "channel")
    cwd = output_dir(cfg, environ)
    cmds: List[Command] = []
    for step in STEPS:
        if step not in steps:
            continue
        for _, item in selected(cfg, items):
            if step in item:
                call = item[step]
                cmds.append(Command(root_macro(environ, channel, call["macro"], call.get("args")), step, cwd))
    return cmds


def check_macros(cfg: Dict[str, Any], steps: Sequence[str], items: Optional[Sequence[str]] = None,
                 environ: Env = None) -> None:
    """A configured macro file that does not exist is a ConfigError naming the item, step and path."""
    base = gxana_root(environ) / "analyses" / gconfig.require(cfg, "channel")
    for name, item in selected(cfg, items):
        for step in steps:
            if step in item and not (base / item[step]["macro"]).is_file():
                raise gconfig.ConfigError(f"measurements.items.{name}.{step}: macro {base / item[step]['macro']} "
                                          "does not exist")


def run_measurements(cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
                     runner: Runner = subprocess.run, environ: Env = None,
                     items: Optional[Sequence[str]] = None) -> int:
    """Print every command; unless dry_run, create output_dir and make_dirs, then run, stopping at the
    first failure. A missing macro file is a ConfigError before anything runs."""
    check_steps(steps, STEPS)
    if not any(step in item for _, item in selected(cfg, items) for step in steps):
        raise gconfig.ConfigError(f"no selected item has step(s) {','.join(steps)}")
    check_macros(cfg, steps, items, environ)
    if not dry_run:
        for d in [output_dir(cfg, environ)] + [gconfig.expand_env(d, environ)
                                               for d in block(cfg).get("make_dirs", [])]:
            Path(d).mkdir(parents=True, exist_ok=True)
    return run_steps(steps, STEPS, lambda step: plan(cfg, [step], items, environ),
                     dry_run=dry_run, runner=runner)
