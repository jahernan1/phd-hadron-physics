"""external.lock is well formed and every patch credits its upstream (spec D8, §11)."""
import re
from pathlib import Path

import yaml

HERE = Path(__file__).resolve().parents[1]
LOCK = yaml.safe_load((HERE / "external.lock").read_text())["externals"]
HEX40 = re.compile(r"^[0-9a-f]{40}$")
HEX64 = re.compile(r"^[0-9a-f]{64}$")


def test_expected_externals():
    assert set(LOCK) == {"halld_sim", "gluex_MCwrapper", "AmpTools", "HDGeant4"}
    assert LOCK["halld_sim"]["sha"] == "bcff7a5c4e8453b1e75e83facdc6f2ad95ba5719"
    assert LOCK["gluex_MCwrapper"]["sha"] == "c4e918a2fe0e48bdf67f69326ed1820458f26b9e"
    assert [LOCK[n]["fetch"] for n in ("halld_sim", "gluex_MCwrapper", "AmpTools", "HDGeant4")] == [True, True, False, False]


def test_fields():
    for name, e in LOCK.items():
        assert HEX40.match(e["sha"]), name
        assert e["url"].startswith("https://github.com/"), name
        assert isinstance(e["ref"], str), name
        for rel, digest in (e.get("files") or {}).items():
            assert HEX64.match(digest), f"{name}: {rel}"


def test_patch_list_matches_directory():
    for name in ("halld_sim", "gluex_MCwrapper"):
        listed = LOCK[name]["patches"]
        on_disk = sorted(f"{name}/{p.name}" for p in (HERE / "patches" / name).glob("*.patch"))
        assert listed == on_disk
    assert len(LOCK["halld_sim"]["patches"]) == 5
    assert len(LOCK["gluex_MCwrapper"]["patches"]) == 2


def test_patch_headers_credit_upstream():
    for name in ("halld_sim", "gluex_MCwrapper"):
        e = LOCK[name]
        for rel in e["patches"]:
            text = (HERE / "patches" / rel).read_text()
            # format-patch quotes display names containing '.'
            assert re.search(r'^From: "?Jesse A\. Hernandez"? <jahphys@gmail\.com>$', text, re.M), rel
            assert "\nSubject: [PATCH" in text, rel
            owner_repo = e["url"].split("github.com/")[1]
            assert f"Upstream: {owner_repo}@{e['sha']} ({e['ref']}); upstream copyright retained." in text, rel


def test_patches_touch_only_locked_files():
    for name in ("halld_sim", "gluex_MCwrapper"):
        e = LOCK[name]
        touched = set()
        for rel in e["patches"]:
            touched |= set(re.findall(r"^\+\+\+ b/(\S+)", (HERE / "patches" / rel).read_text(), re.M))
        assert touched == set(e["files"])
