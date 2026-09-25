"""gxana.externals against a local git fixture (offline) and, opt-in, the real upstreams."""
import os
import subprocess
from pathlib import Path

import pytest

from gxana import externals
from gxana.analysis_data import sha256_file
from gxana.cli import main

ID = ["-c", "user.name=t", "-c", "user.email=t@t"]


def git(cwd, *args):
    return subprocess.run(["git", *ID, *args], cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


@pytest.fixture
def fake_root(tmp_path):
    """<root>/packages/montecarlo/{external.lock,patches/up/0001-*.patch} over a local upstream repo."""
    up = tmp_path / "upstream"
    up.mkdir()
    git(up, "init", "-q")
    (up / "a.txt").write_text("one\n")
    (up / "keep.txt").write_text("same\n")
    git(up, "add", "-A")
    git(up, "commit", "-q", "-m", "base")
    sha = git(up, "rev-parse", "HEAD")
    work = tmp_path / "work"
    git(tmp_path, "clone", "-q", str(up), str(work))
    (work / "a.txt").write_text("one\ntwo\n")
    git(work, "commit", "-q", "-am", "edit a")
    root = tmp_path / "root"
    pdir = root / "packages" / "montecarlo" / "patches" / "up"
    pdir.mkdir(parents=True)
    git(work, "format-patch", "-q", "-o", str(pdir), sha)
    patch = sorted(pdir.iterdir())[0].name
    (root / "packages" / "montecarlo" / "external.lock").write_text(
        f"externals:\n  up:\n    url: {up}\n    ref: v1\n    sha: {sha}\n    fetch: true\n"
        f"    patches: [up/{patch}]\n    files: {{a.txt: {sha256_file(work / 'a.txt')}}}\n"
        f"  pinned:\n    url: {up}\n    ref: v1\n    sha: {sha}\n    fetch: false\n")
    return root


def test_fetch_applies_patches_and_verifies(fake_root, tmp_path):
    ext = externals.load_lock(fake_root)["up"]
    dest = tmp_path / "ext" / "up"
    externals.fetch(ext, dest, log=lambda _: None)
    assert (dest / "a.txt").read_text() == "one\ntwo\n"
    assert externals.check(ext, dest) == []


def test_refetch_is_noop(fake_root, tmp_path):
    ext = externals.load_lock(fake_root)["up"]
    dest = tmp_path / "up"
    externals.fetch(ext, dest, log=lambda _: None)
    logs = []
    externals.fetch(ext, dest, log=logs.append)
    assert "already matches" in logs[0]


def test_existing_modified_checkout_is_refused(fake_root, tmp_path):
    ext = externals.load_lock(fake_root)["up"]
    dest = tmp_path / "up"
    externals.fetch(ext, dest, log=lambda _: None)
    (dest / "a.txt").write_text("local edit\n")
    with pytest.raises(externals.ExternalsError, match="does not match the lock") as err:
        externals.fetch(ext, dest, log=lambda _: None)
    assert "a.txt" in str(err.value)
    assert (dest / "a.txt").read_text() == "local edit\n"   # never overwritten


def test_existing_non_git_dir_is_refused(fake_root, tmp_path):
    ext = externals.load_lock(fake_root)["up"]
    dest = tmp_path / "up"
    dest.mkdir()
    with pytest.raises(externals.ExternalsError, match="not a git checkout"):
        externals.fetch(ext, dest, log=lambda _: None)


def test_wrong_locked_digest_fails_after_fetch(fake_root, tmp_path):
    lock = fake_root / "packages" / "montecarlo" / "external.lock"
    text = lock.read_text()
    digest = text.split("a.txt: ")[1].split("}")[0]
    lock.write_text(text.replace(digest, "0" * 64))
    ext = externals.load_lock(fake_root)["up"]
    with pytest.raises(externals.ExternalsError, match="a.txt: sha256"):
        externals.fetch(ext, tmp_path / "up", log=lambda _: None)


def test_fetch_failure_after_clone_suggests_remove_dest(fake_root, tmp_path):
    lock_path = fake_root / "packages" / "montecarlo" / "external.lock"
    pdir = fake_root / "packages" / "montecarlo" / "patches" / "up"
    good_patch = sorted(pdir.iterdir())[0].name
    bogus = pdir / "0002-bogus.patch"
    bogus.write_text("not a valid patch\n")
    text = lock_path.read_text()
    lock_path.write_text(text.replace(f"patches: [up/{good_patch}]", f"patches: [up/{good_patch}, up/{bogus.name}]"))
    ext = externals.load_lock(fake_root)["up"]
    dest = tmp_path / "up"
    with pytest.raises(externals.ExternalsError, match="remove .*and retry"):
        externals.fetch(ext, dest, log=lambda _: None)
    assert dest.exists()   # half-built clone left in place for inspection, not auto-removed


def test_pinned_only_is_not_fetched(fake_root, tmp_path):
    ext = externals.load_lock(fake_root)["pinned"]
    with pytest.raises(externals.ExternalsError, match="pinned only"):
        externals.fetch(ext, tmp_path / "p", log=lambda _: None)


def test_bad_sha_and_missing_patch(fake_root):
    lock = fake_root / "packages" / "montecarlo" / "external.lock"
    good = lock.read_text()
    lock.write_text(good.replace("ref: v1\n    sha: ", "ref: v1\n    sha: X", 1))
    with pytest.raises(externals.ExternalsError, match="40 lowercase hex"):
        externals.load_lock(fake_root)
    lock.write_text(good.replace("patches: [up/", "patches: [up/missing-"))
    with pytest.raises(externals.ExternalsError, match="patch not found"):
        externals.load_lock(fake_root)


@pytest.fixture
def fake_root_halld_sim(tmp_path):
    """Like fake_root, but the fetch:true entry is named halld_sim and the
    root has env/version_sets/{A,B}.xml.in, to test the per-version-set
    default destination ($GXANA_EXTERNALS/halld_sim-<set>)."""
    up = tmp_path / "upstream"
    up.mkdir()
    git(up, "init", "-q")
    (up / "a.txt").write_text("one\n")
    git(up, "add", "-A")
    git(up, "commit", "-q", "-m", "base")
    sha = git(up, "rev-parse", "HEAD")
    root = tmp_path / "root"
    (root / "packages" / "montecarlo" / "patches").mkdir(parents=True)
    (root / "packages" / "montecarlo" / "external.lock").write_text(
        f"externals:\n  halld_sim:\n    url: {up}\n    ref: v1\n    sha: {sha}\n    fetch: true\n")
    vs = root / "env" / "version_sets"
    vs.mkdir(parents=True)
    (vs / "A.xml.in").write_text("<gxml/>\n")
    (vs / "B.xml.in").write_text("<gxml/>\n")
    return root


def test_cli_halld_sim_default_dest_status_per_set(fake_root_halld_sim, tmp_path, monkeypatch, capsys):
    monkeypatch.setenv("GXANA_ROOT", str(fake_root_halld_sim))
    monkeypatch.setenv("GXANA_EXTERNALS", str(tmp_path / "ext"))
    assert main(["externals", "status"]) == 1
    out = capsys.readouterr().out
    assert f"[miss] halld_sim: {tmp_path}/ext/halld_sim-A" in out
    assert f"[miss] halld_sim: {tmp_path}/ext/halld_sim-B" in out
    ext = externals.load_lock(fake_root_halld_sim)["halld_sim"]
    externals.fetch(ext, tmp_path / "ext" / "halld_sim-A", log=lambda _: None)
    assert main(["externals", "status"]) == 1   # B still missing
    out = capsys.readouterr().out
    assert f"[ ok ] halld_sim: {tmp_path}/ext/halld_sim-A" in out
    assert f"[miss] halld_sim: {tmp_path}/ext/halld_sim-B" in out


def test_cli_halld_sim_default_dest_fetch_skips(fake_root_halld_sim, tmp_path, monkeypatch, capsys):
    monkeypatch.setenv("GXANA_ROOT", str(fake_root_halld_sim))
    monkeypatch.setenv("GXANA_EXTERNALS", str(tmp_path / "ext"))
    assert main(["externals", "fetch"]) == 0
    out = capsys.readouterr().out
    assert "halld_sim" in out
    assert "build_halld_sim.sh" in out or "--dest" in out
    assert not (tmp_path / "ext" / "halld_sim").exists()


def test_cli_halld_sim_explicit_dest_still_single_checkout(fake_root_halld_sim, tmp_path, monkeypatch, capsys):
    monkeypatch.setenv("GXANA_ROOT", str(fake_root_halld_sim))
    dest = tmp_path / "custom" / "halld_sim"
    assert main(["externals", "fetch", "halld_sim", "--dest", str(dest)]) == 0
    assert (dest / "a.txt").is_file()
    assert main(["externals", "status", "halld_sim", "--dest", str(dest)]) == 0
    out = capsys.readouterr().out
    assert f"[ ok ] halld_sim: {dest}" in out


def test_cli_status_and_dest_rule(fake_root, tmp_path, monkeypatch, capsys):
    monkeypatch.setenv("GXANA_ROOT", str(fake_root))
    monkeypatch.setenv("GXANA_EXTERNALS", str(tmp_path / "ext"))
    assert main(["externals", "status"]) == 1                  # not fetched yet
    assert "[miss] up" in capsys.readouterr().out
    assert main(["externals", "fetch"]) == 0
    assert main(["externals", "status"]) == 0
    out = capsys.readouterr().out
    assert "[ ok ] up" in out and "[pin ] pinned" in out
    assert main(["externals", "fetch", "--dest", str(tmp_path / "x")]) == 2   # --dest needs one NAME


@pytest.mark.network
@pytest.mark.skipif(os.environ.get("GXANA_NETWORK_TESTS") != "1", reason="set GXANA_NETWORK_TESTS=1")
@pytest.mark.parametrize("name", ["halld_sim", "gluex_MCwrapper"])
def test_real_upstreams_reproduce_author_files(name, tmp_path):
    ext = externals.load_lock()[name]
    dest = tmp_path / name
    externals.fetch(ext, dest, log=lambda _: None)
    assert externals.check(ext, dest) == []
