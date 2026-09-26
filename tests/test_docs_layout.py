"""The root README lists every package and channel, and points at the release files."""
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
README = (ROOT / "README.md").read_text()


def _visible_dirs(base: Path):
    return sorted(d for d in base.iterdir() if d.is_dir() and not d.name.startswith((".", "_")))


def _submodule_uninitialized(d: Path) -> bool:
    entries = list(d.iterdir())
    return not entries or (not (d / "README.md").is_file() and not (d / ".git").exists())


def test_every_package_and_channel_listed():
    for d in _visible_dirs(ROOT / "packages") + _visible_dirs(ROOT / "analyses"):
        assert f"`{d.relative_to(ROOT).as_posix()}`" in README, d.name


def test_every_package_has_a_readme():
    for d in _visible_dirs(ROOT / "packages"):
        if d.name == "qfactors" and _submodule_uninitialized(d):
            pytest.skip("packages/qfactors submodule not initialized (clone without --recurse-submodules)")
        assert (d / "README.md").is_file(), d.name


def test_release_pointers():
    for must in ("LICENSE", "NOTICE.md", "CITATION.cff", "--recurse-submodules"):
        assert must in README, must
    assert "being restructured" not in README
