"""`gxana run systematics`: the non-Barlow systematics suite (no steps yet).

The Barlow cut-variation steps (bin, tables, weight, barlow) moved to
`gxana run barlow` (packages/barlow, analyses/<channel>/config/barlow.yaml).
"""
from __future__ import annotations

import subprocess
from typing import Any, Callable, Dict, Mapping, Optional, Sequence

from gxana import config

STEPS: tuple = ()
DEFAULT_STEPS: tuple = ()
MOVED_TO_BARLOW = ("bin", "tables", "weight", "barlow")

Runner = Callable[..., subprocess.CompletedProcess]


def run_systematics(
    cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
    runner: Runner = subprocess.run, environ: Optional[Mapping[str, str]] = None,
) -> int:
    for step in steps:
        if step in MOVED_TO_BARLOW:
            raise config.ConfigError(
                f"step {step!r} moved to `gxana run barlow` (steps: trees,check,bin,tables,weight,plot)")
        if step not in STEPS:
            raise config.ConfigError(f"unknown step {step!r}; known: {list(STEPS)}")
    if not steps:
        print("gxana: no systematics steps yet; the Barlow check is `gxana run barlow`")
    return 0
