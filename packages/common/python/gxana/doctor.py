"""`gxana doctor`: check that the environment can run the pipeline."""
from __future__ import annotations

import os
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, List, Mapping, Optional

REQUIRED_ENV = ("GXANA_ROOT", "GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH")
REQUIRED_TOOLS = ("root", "rootls", "hadd")
OPTIONAL_TOOLS = ("cmake",)


@dataclass(frozen=True)
class Check:
    name: str
    status: str  # ok | warn | fail
    detail: str


def run_checks(
    environ: Optional[Mapping[str, str]] = None,
    which: Callable[[str], Optional[str]] = shutil.which,
) -> List[Check]:
    env = os.environ if environ is None else environ
    checks = []
    for var in REQUIRED_ENV:
        value = env.get(var)
        checks.append(Check(var, "ok" if value else "fail", value or "not set (source env/setup.sh)"))
    data = env.get("GXANA_ANALYSIS_DATA")
    if data and Path(data).is_dir():
        checks.append(Check("GXANA_ANALYSIS_DATA", "ok", data))
    else:
        checks.append(Check("GXANA_ANALYSIS_DATA", "warn",
                            f"{data or 'not set'}: no preserved analysis data; golden tests skip "
                            "(docs/analysis_data.md)"))
    for tool in REQUIRED_TOOLS:
        found = which(tool)
        checks.append(Check(tool, "ok" if found else "fail", found or "not on PATH"))
    for tool in OPTIONAL_TOOLS:
        found = which(tool)
        checks.append(Check(tool, "ok" if found else "warn", found or "not on PATH (needed to build C++ packages)"))
    rah = env.get("ROOT_ANALYSIS_HOME")
    if rah and Path(rah, "scripts", "Load_DSelector.C").is_file():
        checks.append(Check("ROOT_ANALYSIS_HOME", "ok", rah))
    else:
        checks.append(Check("ROOT_ANALYSIS_HOME", "warn",
                            "gluex_root_analysis not found; needed for `gxana run select` (source env/setup.sh --gluex)"))
    # gxenv is a convenience wrapper, not required by any stage; never fails doctor.
    gxenv = which("gxenv")
    checks.append(Check("gxenv", "ok" if gxenv else "warn", gxenv or "not on PATH (optional convenience wrapper)"))
    try:
        import yaml

        checks.append(Check("pyyaml", "ok", yaml.__version__))
    except ImportError:
        checks.append(Check("pyyaml", "fail", "not importable"))
    return checks


def main(
    environ: Optional[Mapping[str, str]] = None,
    which: Callable[[str], Optional[str]] = shutil.which,
    out: Callable[[str], None] = print,
) -> int:
    checks = run_checks(environ, which)
    for check in checks:
        out(f"[{check.status:>4}] {check.name}: {check.detail}")
    return 1 if any(check.status == "fail" for check in checks) else 0
