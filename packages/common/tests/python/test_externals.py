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
