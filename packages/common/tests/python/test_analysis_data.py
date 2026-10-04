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


STAGE_MANIFEST = MANIFEST + """stage:
  mc_sample: sim
  next: gxana run xsection --channel demo --steps tables
  files:
    - {from: "inputs/{stem}.root", to: "${GXANA_OUTPUT}/demo/binned/", mode: copy}
    - {from: "inputs/{mc_stem}.root", to: "${GXANA_DATA}/flatTrees/", mode: link}
"""


@pytest.fixture
def staged_repo(repo, monkeypatch):
    (repo / "analyses" / "demo" / "analysis_data.yaml").write_text(STAGE_MANIFEST)
    (repo / "gad" / "demo" / "inputs" / "P1.root").write_bytes(b"data")
    (repo / "gad" / "demo" / "inputs" / "P1_sim.root").write_bytes(b"mc")
    monkeypatch.setenv("GXANA_OUTPUT", str(repo / "out"))
    monkeypatch.setenv("GXANA_DATA", str(repo / "data"))
    monkeypatch.setattr("gxana.config.tree_stem", lambda cfg, period, sample: "P1" if sample == "data" else "P1_sim")
    monkeypatch.setattr("gxana.cli.load_channel", lambda channel: {"periods": {"p1": {}}})
    return repo


def test_stage_modes_follow_manifest(staged_repo):
    m = ad.load_manifest("demo")
    actions = ad.stage_plan(m, ad.data_dir(m), {"periods": {"p1": {}}})
    assert [(a.mode, a.state, a.dest) for a in actions] == [
        ("copy", "new", staged_repo / "out" / "demo" / "binned" / "P1.root"),
        ("link", "new", staged_repo / "data" / "flatTrees" / "P1_sim.root"),
    ]
    ad.apply_stage(actions)
    copied, linked = actions[0].dest, actions[1].dest
    assert copied.is_file() and not copied.is_symlink() and copied.read_bytes() == b"data"
    assert linked.is_symlink() and linked.resolve() == (staged_repo / "gad/demo/inputs/P1_sim.root").resolve()
    again = ad.stage_plan(m, ad.data_dir(m), {"periods": {"p1": {}}})
    assert [a.state for a in again] == ["ok", "ok"]


def test_stage_never_overwrites(staged_repo, capsys):
    dest = staged_repo / "out" / "demo" / "binned" / "P1.root"
    dest.parent.mkdir(parents=True)
    dest.write_bytes(b"rebinned")
    assert main(["data", "stage", "--channel", "demo"]) == 1
    out = capsys.readouterr().out
    assert "[conflict]" in out and str(dest) in out
    assert dest.read_bytes() == b"rebinned"
    assert not (staged_repo / "data" / "flatTrees" / "P1_sim.root").exists()  # all or nothing


def test_stage_dry_run_and_next_hint(staged_repo, capsys):
    assert main(["data", "stage", "--channel", "demo", "--dry-run"]) == 0
    out = capsys.readouterr().out
    assert "[     new] copy" in out and "[     new] link" in out
    assert not (staged_repo / "out").exists()
    assert main(["data", "stage", "--channel", "demo"]) == 0
    assert "next: gxana run xsection --channel demo --steps tables" in capsys.readouterr().out


def test_stage_missing_source_fails(staged_repo, capsys):
    (staged_repo / "gad" / "demo" / "inputs" / "P1_sim.root").unlink()
    assert main(["data", "stage", "--channel", "demo"]) == 1
    assert "[ missing]" in capsys.readouterr().out
    assert not (staged_repo / "out").exists()


def test_stage_without_block_is_config_error(repo, monkeypatch, capsys):
    monkeypatch.setattr("gxana.cli.load_channel", lambda channel: {"periods": {}})
    assert main(["data", "stage", "--channel", "demo"]) == 2
    assert "no 'stage' block" in capsys.readouterr().err


def test_lock_keeps_stage_block(staged_repo):
    assert main(["data", "lock", "--channel", "demo"]) == 0
    assert "stage" in ad.load_manifest("demo").data
