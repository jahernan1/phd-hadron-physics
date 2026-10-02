"""The root README lists every package and channel, points at the release files, and documents every gxana command and flag."""
import argparse
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


def _subcommands(parser):
    for action in parser._actions:
        if isinstance(action, argparse._SubParsersAction):
            return action.choices
    return {}


def _cli_commands():
    """(start of its README table row, parser) for every leaf gxana command."""
    from gxana.cli import build_parser

    out = []
    for name, p in _subcommands(build_parser()).items():
        children = _subcommands(p)
        if not children:
            out.append((f"| `gxana {name}", p))
        for child, cp in children.items():
            out.append((f"| `{child}` |" if name == "run" else f"| `gxana {name} {child}", cp))
    return out


def test_cli_commands_found():
    assert _cli_commands()


@pytest.mark.parametrize("cmd,parser", _cli_commands(), ids=lambda v: v if isinstance(v, str) else "")
def test_every_gxana_command_and_flag_documented(cmd, parser):
    """Documentation rule: each command, and each of its flags, is in the README command reference."""
    row = next((line for line in README.splitlines() if line.startswith(cmd)), None)
    assert row, cmd
    for action in parser._actions:
        flags = [o for o in action.option_strings if o.startswith("--") and o != "--help"]
        if flags and flags[0] != "--dry-run":
            assert flags[0] in row, f"{cmd}: {flags[0]}"
