"""The command layer of the `gxana run` stages that plan a list of commands per
step (xsection, barlow, systematics): the Command type, the step check and the
print / dry-run / run loop. Imported by gxana_barlow and gxana_systematics."""
from __future__ import annotations

import shlex
import subprocess
import sys
from typing import Any, Callable, List, Mapping, NamedTuple, Optional, Protocol, Sequence

from gxana import config
from gxana.paths import gxana_root


class Runner(Protocol):
    """subprocess.run-like: called as runner(argv, check=False[, cwd=...]); the result is only read
    for its returncode attribute (getattr, so a result without one, or None, counts as 0)."""

    def __call__(self, argv: Any, /, *args: Any, **kwargs: Any) -> object: ...


Env = Optional[Mapping[str, str]]


class Command(NamedTuple):
    argv: List[str]
    step: str
    cwd: Optional[str] = None  # working directory; None = the caller's


def check_steps(steps: Sequence[str], known: Sequence[str], *, moved: Optional[Mapping[str, str]] = None,
                first: str = "given") -> None:
    """ConfigError for a step that is not in `known`.

    moved: step -> the `gxana run` stage it moved to; checked step by step in the given
    order, before the unknown check of that step. first: which unknown step the message
    names when there are several -- "given" (first in the given order: barlow,
    systematics) or "sorted" (alphabetically first: xsection)."""
    if first not in ("given", "sorted"):
        raise ValueError(f"first must be 'given' or 'sorted', got {first!r}")
    moved = moved or {}
    for step in steps:
        if step in moved:
            raise config.ConfigError(f"step {step!r} moved to `gxana run {moved[step]}`")
        if first == "given" and step not in known:
            raise config.ConfigError(f"unknown step {step!r}; known: {list(known)}")
    unknown = sorted(set(steps) - set(known))
    if unknown:
        raise config.ConfigError(f"unknown step {unknown[0]!r}; known: {list(known)}")


def command_line(cmd: Command) -> str:
    """The printed form of a command: shlex-quoted argv, wrapped in (cd <cwd> && ...) if cwd is set."""
    line = shlex.join(cmd.argv)
    return f"(cd {shlex.quote(cmd.cwd)} && {line})" if cmd.cwd else line


def run_commands(commands: Sequence[Command], dry_run: bool, runner: Runner) -> int:
    """Print each command; unless dry_run, run it (cwd= passed only when set) and return
    the first non-zero exit code (a result without returncode, or None, counts as 0)."""
    for cmd in commands:
        print(command_line(cmd))
        if dry_run:
            continue
        kwargs = {"cwd": cmd.cwd} if cmd.cwd else {}
        rc = getattr(runner(cmd.argv, check=False, **kwargs), "returncode", 0) or 0
        if rc != 0:
            return rc
    return 0


def run_steps(steps: Sequence[str], known: Sequence[str], plan_step: Callable[[str], List[Command]], *,
              dry_run: bool, runner: Runner, before: Optional[Callable[[str], Optional[int]]] = None,
              after: Optional[Callable[[str], None]] = None) -> int:
    """Run the requested `steps` in the order of `known`. Per step: before(step) -- a
    non-None return ends the run with that code; plan_step(step) -- planned only now, so
    a plan can read what the previous step wrote; run_commands; after(step) once every
    command of the step succeeded. The caller checks `steps` first. `steps` must be a list or tuple: it is
    membership-tested once per known step, so a step given twice runs once, in the
    order of `known`."""
    for step in known:
        if step not in steps:
            continue
        if before is not None:
            rc = before(step)
            if rc is not None:
                return rc
        rc = run_commands(plan_step(step), dry_run, runner)
        if rc != 0:
            return rc
        if after is not None:
            after(step)
    return 0


def executable(name: str, environ: Env) -> str:
    """$GXANA_ROOT/build/bin/<name> if that file exists, else the bare name (looked up on PATH),
    so a plan depends on whether the C++ packages are built."""
    candidate = gxana_root(environ) / "build" / "bin" / name
    return str(candidate) if candidate.is_file() else name


def python_module(package: str, module: str, *args: str) -> List[str]:
    """argv running `python -m <package>.<module> args...` with this interpreter."""
    return [sys.executable, "-m", f"{package}.{module}", *args]


def num(value: Any) -> str:
    """A config number as written on a command line (str(), so 6.4 -> "6.4", 2 -> "2")."""
    return str(value)


def cxx_arg(value: Any) -> str:
    """A config value as a C++ literal in a ROOT macro call: true/false, a number as num(), a string in
    double quotes. A string holding '"' or '\\' (it would need escaping) or any other type is a
    ConfigError."""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return num(value)
    if isinstance(value, str) and '"' not in value and "\\" not in value:
        return f'"{value}"'
    raise config.ConfigError(f"macro argument {value!r}: need a number, a boolean or a string without '\"' and '\\'")


def root_macro(environ: Env, channel: str, macro: str, args: Optional[Sequence[Any]] = None) -> List[str]:
    """argv running analyses/<channel>/<macro> after the repository's rootlogon.C with `root -l -b -q`.
    args (each through cxx_arg) go in parentheses; None calls the macro with its defaults."""
    root = gxana_root(environ)
    call = f"{root}/analyses/{channel}/{macro}"
    if args is not None:
        call += "(" + ",".join(cxx_arg(a) for a in args) + ")"
    return ["root", "-l", "-b", "-q", str(root / "rootlogon.C"), call]
