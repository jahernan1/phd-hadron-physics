"""Every migrated ROOT macro loads under cling with the gxana libraries.

Selectors are excluded: they need gluex_root_analysis (DSelector base) and
are only loadable in the GlueX container.
"""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
EXCLUDE_DIRS = ("selectors",)
pytestmark = [pytest.mark.macros,
              pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]


def _skips():
    path = Path(__file__).with_name("skip.txt")
    if not path.exists():
        return set()
    return {line.split("#")[0].strip() for line in path.read_text().splitlines() if line.split("#")[0].strip()}


def macro_files():
    skips = _skips()
    out = []
    for ext in ("*.C", "*.cpp", "*.cxx"):
        for p in sorted((ROOT / "analyses").rglob(ext)):
            rel = p.relative_to(ROOT).as_posix()
            if any(part in EXCLUDE_DIRS for part in p.relative_to(ROOT).parts) or rel in skips:
                continue
            out.append(rel)
    return out


def _env(tmp_path):
    env = dict(os.environ, GXANA_ROOT=str(ROOT))
    for var in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA"):
        env[var] = str(tmp_path / var.lower())
    return env


def _load(rel, env):
    return subprocess.run(["root", "-l", "-b", "-q", "rootlogon.C", f'tests/macros/load_macro.C("{rel}")'],
                          cwd=ROOT, env=env, capture_output=True, text=True, timeout=300)


@pytest.mark.parametrize("rel", macro_files() or ["<none>"])
def test_macro_loads(rel, tmp_path):
    if rel == "<none>":
        pytest.skip("no migrated macros yet")
    proc = _load(rel, _env(tmp_path))
    out = proc.stdout + proc.stderr
    assert proc.returncode == 0 and "error:" not in out and "LOAD FAILED" not in out, out[-4000:]


def test_unset_env_names_variable(tmp_path):
    macro = tmp_path / "uses_env.C"
    macro.write_text('#include "gxana/common/Paths.h"\nstd::string d = gxana::EnvPath("GXANA_OUTPUT", "x");\n')
    env = _env(tmp_path)
    del env["GXANA_OUTPUT"]
    proc = _load(str(macro), env)
    assert "GXANA_OUTPUT is not set" in proc.stdout + proc.stderr
