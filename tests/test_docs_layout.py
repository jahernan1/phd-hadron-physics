"""The root README lists every package and channel, and points at the release files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
README = (ROOT / "README.md").read_text()


def _visible_dirs(base: Path):
    return sorted(d for d in base.iterdir() if d.is_dir() and not d.name.startswith((".", "_")))


def test_every_package_and_channel_listed():
    for d in _visible_dirs(ROOT / "packages") + _visible_dirs(ROOT / "analyses"):
        assert f"`{d.relative_to(ROOT).as_posix()}`" in README, d.name


def test_every_package_has_a_readme():
    for d in _visible_dirs(ROOT / "packages"):
        assert (d / "README.md").is_file(), d.name


def test_release_pointers():
    for must in ("LICENSE", "NOTICE.md", "CITATION.cff", "--recurse-submodules"):
        assert must in README, must
    assert "being restructured" not in README
