"""Golden tests: rerun cross-section stages on the preserved thesis data and
compare with the legacy outputs. Everything here skips unless the
files are under $GXANA_ANALYSIS_DATA/kpkpxim (default
<repo>/gluex_analysis_data/kpkpxim); see docs/analysis_data.md."""
from __future__ import annotations

import shutil
from pathlib import Path
from typing import Callable, List

import pytest

from gxana.paths import analysis_data_root, repo_root


@pytest.fixture(scope="session")
def golden() -> Path:
    base = analysis_data_root() / "kpkpxim"
    if not base.is_dir():
        pytest.skip(f"no preserved analysis data at {base} (docs/analysis_data.md)")
    return base


@pytest.fixture(scope="session")
def need(golden: Path) -> Callable[..., List[Path]]:
    """need("flux/a.root", ...) -> paths under golden; skips the test if any is missing."""

    def _need(*rels: str) -> List[Path]:
        paths = [golden / rel for rel in rels]
        missing = [str(p) for p in paths if not p.exists()]
        if missing:
            pytest.skip("missing golden files: " + ", ".join(missing))
        return paths

    return _need


@pytest.fixture(scope="session")
def build_bin() -> Path:
    path = repo_root() / "build" / "bin"
    if not path.is_dir():
        pytest.skip("C++ packages not built (uv run cmake --build build)")
    return path


@pytest.fixture(scope="session")
def root_exe() -> str:
    exe = shutil.which("root")
    if exe is None:
        pytest.skip("root not on PATH")
    return exe
