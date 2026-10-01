"""The command layer of the `gxana run` stages that plan a list of commands per
step (xsection, barlow, systematics): the Command type, the step check and the
print / dry-run / run loop. Imported by gxana_barlow and gxana_systematics."""
from __future__ import annotations

import shlex
import subprocess
from typing import Callable, List, Mapping, NamedTuple, Optional, Sequence

from gxana import config

Runner = Callable[..., subprocess.CompletedProcess]
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
    command of the step succeeded. The caller checks `steps` first."""
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
