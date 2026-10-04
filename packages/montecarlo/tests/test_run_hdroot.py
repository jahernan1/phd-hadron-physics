"""run_hdroot.py reproduces the legacy hd_root / mv command lines (AnalysisNote/MC/run_hdroot.py)."""
import importlib.util
from pathlib import Path

import pytest

PATH = Path(__file__).resolve().parents[1] / "scripts" / "run_hdroot.py"
spec = importlib.util.spec_from_file_location("run_hdroot", PATH)
assert spec is not None and spec.loader is not None
rh = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rh)


@pytest.fixture
def s(tmp_path):
    return rh.parse_args(["--mc-dir", str(tmp_path / "mc"), "--out-dir", str(tmp_path / "out"),
                           "--flags", "B4_U1_M23"])


def test_flags_required(tmp_path):
    with pytest.raises(SystemExit):
        rh.parse_args(["--mc-dir", str(tmp_path / "mc"), "--out-dir", str(tmp_path / "out")])


def test_defaults_are_legacy_values(s):
    assert (s.reaction, s.flags, s.plugins, s.generator, s.tree_prefix, s.nthreads, s.processes) == (
        "1_14__11_11_23", "B4_U1_M23", "ReactionFilter,mcthrown_tree", "gen_amp_V2", "kpkpxim", 1, 16)


def test_hdroot_command(s):
    assert rh.hdroot_command(s, "/m/hddm/dana_rest_gen_amp_V2_050685_003.hddm") == (
        "hd_root --nthreads=1 -PPLUGINS=ReactionFilter,mcthrown_tree -PReaction1=1_14__11_11_23 "
        "-PReaction1:Flags=B4_U1_M23 /m/hddm/dana_rest_gen_amp_V2_050685_003.hddm")


def test_move_commands(s):
    assert rh.move_commands(s, "050685", "/m/hddm/dana_rest_gen_amp_V2_050685_003.hddm") == [
        "mv hd_root.root ../monitoring_hists/hd_root_gen_amp_V2_050685_003.root",
        "mv tree_kpkpxim__B4_U1_M23.root ../trees/tree_kpkpxim__B4_U1_M23_gen_amp_V2_050685_003.root",
        "mv tree_thrown.root ../thrown/tree_thrown_gen_amp_V2_050685_003.root",
    ]


def test_rest_files_and_run_numbers(s):
    hddm = Path(s.mc_dir) / "hddm"
    hddm.mkdir(parents=True)
    for name in ("dana_rest_gen_amp_V2_050685_000.hddm", "dana_rest_gen_amp_V2_050685_001.hddm",
                 "dana_rest_gen_amp_V2_050690_000.hddm", "dana_rest_other_050690_000.hddm"):
        (hddm / name).write_text("")
    files = rh.rest_files(s)
    assert len(files) == 3
    assert rh.run_numbers(files, s.generator) == ["050685", "050690"]
    assert [Path(f).name for f in rh.rest_files(s, "050685")] == [
        "dana_rest_gen_amp_V2_050685_000.hddm", "dana_rest_gen_amp_V2_050685_001.hddm"]


def test_dirs_required():
    with pytest.raises(SystemExit):
        rh.parse_args([])
