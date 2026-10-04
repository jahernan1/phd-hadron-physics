import hashlib

import pytest

from gxana import analysis_data as ad
from gxana.cli import main
from gxana.config import ConfigError

MANIFEST = """# demo manifest
# second comment line
subdir: demo
directories:
  inputs: input trees
files:
  inputs/a.root: {sha256: null, bytes: null}
  inputs/b.root:
"""


@pytest.fixture
def repo(tmp_path, monkeypatch):
    chan = tmp_path / "analyses" / "demo"
    chan.mkdir(parents=True)
    (chan / "analysis_data.yaml").write_text(MANIFEST)
    inputs = tmp_path / "gad" / "demo" / "inputs"
    inputs.mkdir(parents=True)
    (inputs / "a.root").write_bytes(b"aaa")
    monkeypatch.setenv("GXANA_ROOT", str(tmp_path))
    monkeypatch.setenv("GXANA_ANALYSIS_DATA", str(tmp_path / "gad"))
    return tmp_path


def states(manifest, base):
    return {s.path: s.state for s in ad.status(manifest, base)}


def test_load_manifest(repo):
    m = ad.load_manifest("demo")
    assert m.subdir == "demo"
    assert m.files == {"inputs/a.root": {"sha256": None, "bytes": None}, "inputs/b.root": {}}
    assert m.header == "# demo manifest\n# second comment line\n"


def test_data_dir_uses_env(repo):
    assert ad.data_dir(ad.load_manifest("demo")) == repo / "gad" / "demo"


def test_status_before_lock(repo):
    m = ad.load_manifest("demo")
    assert states(m, ad.data_dir(m)) == {"inputs/a.root": "open", "inputs/b.root": "miss"}


def test_status_reports_unlisted_files(repo):
    m = ad.load_manifest("demo")
    base = ad.data_dir(m)
    (base / "new.txt").write_text("x")
    assert states(m, base)["new.txt"] == "new"


def test_lock_then_status(repo):
    m = ad.load_manifest("demo")
    base = ad.data_dir(m)
    (base / "extra.txt").write_text("x")
    (base / ".hidden").write_text("x")
    assert ad.lock(m, base) == 2
    again = ad.load_manifest("demo")
    assert again.header.startswith("# demo manifest\n")
    assert again.data["directories"] == {"inputs": "input trees"}
    assert again.files["inputs/a.root"] == {"sha256": hashlib.sha256(b"aaa").hexdigest(), "bytes": 3}
    assert states(again, base) == {"extra.txt": "ok", "inputs/a.root": "ok", "inputs/b.root": "miss"}
    (base / "inputs" / "a.root").write_bytes(b"changed")
    assert states(again, base)["inputs/a.root"] == "diff"


def test_missing_manifest_raises(repo):
    with pytest.raises(ConfigError, match="no analysis_data.yaml"):
        ad.load_manifest("nope")


def test_manifest_needs_subdir(repo):
    (repo / "analyses" / "demo" / "analysis_data.yaml").write_text("files: {}\n")
    with pytest.raises(ConfigError, match="subdir"):
        ad.load_manifest("demo")


def test_cli_path_and_status(repo, capsys):
    assert main(["data", "path", "--channel", "demo"]) == 0
    assert capsys.readouterr().out.strip() == str(repo / "gad" / "demo")
    assert main(["data", "status", "--channel", "demo"]) == 1
    out = capsys.readouterr().out
    assert "[miss] inputs/b.root" in out
    assert "[open] inputs/a.root" in out


def test_cli_lock(repo, capsys):
    assert main(["data", "lock", "--channel", "demo"]) == 0
    assert "locked 1 files" in capsys.readouterr().out


def test_cli_status_without_data_is_not_an_error(repo, monkeypatch, capsys):
    """A clone without preserved data: status says so, exit 0 (README Quickstart)."""
    monkeypatch.setenv("GXANA_ANALYSIS_DATA", str(repo / "absent"))
    assert main(["data", "status", "--channel", "demo"]) == 0
    out, err = capsys.readouterr()
    assert err == ""
    assert f"no preserved data at {repo / 'absent' / 'demo'}" in out
    assert "golden tests skip" in out and "docs/analysis_data.md" in out


def test_cli_lock_without_data_is_an_error(repo, monkeypatch, capsys):
    monkeypatch.setenv("GXANA_ANALYSIS_DATA", str(repo / "absent"))
    assert main(["data", "lock", "--channel", "demo"]) == 2
    assert "does not exist" in capsys.readouterr().err
