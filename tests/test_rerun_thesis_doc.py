"""docs/RERUN_THESIS.md stays in step with the code.

Every `gxana ...` command in the guide parses with the real CLI (placeholders
P and S filled in; the map tables leave `--channel kpkpxim` out, so it is added
where a command has none), every test file it names exists, and every relative
link and #anchor it uses resolves (GitHub heading slugs)."""
import re
import shlex
from pathlib import Path

import pytest

from gxana.cli import build_parser

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs" / "RERUN_THESIS.md"
TEXT = DOC.read_text()


def _commands():
    cmds = set()
    for line in re.findall(r"`(gxana [^`]+)`", TEXT) + re.findall(r"^\s*(?:uv run )?(gxana .+)$", TEXT, re.M):
        line = line.split("#")[0].strip().rstrip("\\").strip()
        line = re.sub(r"\[(--[\w-]+)\]", r"\1", line)
        line = line.replace("--period P", "--period 2017-01").replace("--sample S", "--sample data")
        if "--channel" not in line:
            line += " --channel kpkpxim"
        cmds.add(line)
    return sorted(cmds)


def _slug(heading: str) -> str:
    s = heading.strip().lower().replace("`", "")
    s = "".join(c for c in s if c.isalnum() or c in " -_")
    return s.replace(" ", "-")


def _anchors(md: Path):
    return {_slug(h) for h in re.findall(r"^#+\s+(.+)$", md.read_text(), re.M)}


def test_commands_found():
    assert len(_commands()) > 10


@pytest.mark.parametrize("cmd", _commands())
def test_command_parses(cmd):
    args = shlex.split(cmd)[1:]
    try:
        build_parser().parse_args(args)
    except SystemExit as exc:
        pytest.fail(f"gxana rejects: {cmd} (exit {exc.code})")


def test_named_tests_exist():
    names = set(re.findall(r"\b(test_\w+\.py)\b", TEXT))
    assert names
    present = {p.name for p in ROOT.glob("tests/**/test_*.py")} | {p.name for p in ROOT.glob("packages/*/tests/**/test_*.py")}
    assert names <= present, sorted(names - present)


def test_links_resolve():
    links = re.findall(r"\]\(([^)\s]+)\)", TEXT)
    assert links
    for link in links:
        if link.startswith(("http://", "https://")):
            continue
        path, _, anchor = link.partition("#")
        target = (DOC.parent / path).resolve() if path else DOC
        assert target.exists(), link
        if anchor:
            assert target.suffix == ".md", link
            assert anchor in _anchors(target), link
