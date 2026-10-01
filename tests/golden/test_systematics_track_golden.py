"""Golden: the track-efficiency study reproduces the dissertation table (tab:track_eff) and the
archived macros' figures.

The preserved flat trees are staged where xsection.inputs expects them: the Q-factor output
under $GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/ and the MC and thrown
trees under $GXANA_DATA/flatTrees/. `gxana run systematics --steps track --study track` runs on
them; the archived get_hists.C and get_track_efficiency.C run on the same staged inputs in a
temporary cwd (they write particle_kinematics.root and the figures there).
"""
from __future__ import annotations

import os
import re
import subprocess
from pathlib import Path

import numpy as np
import pytest

from gxana import config
from gxana.paths import repo_root
from gxana_systematics import stage

from test_systematics_plot_golden import _raster

pytestmark = pytest.mark.golden

PARTICLES = ["kp1", "kp2", "pim1", "pim2", "proton"]
EXPECTED_MC_RAW = {"kp1": 0.03, "kp2": 0.0494, "pim1": 0.0383, "pim2": 0.0359, "proton": 0.0329}  # diss tab:track_eff


def _run(argv, **kwargs):
    proc = subprocess.run(argv, capture_output=True, text=True, **kwargs)
    assert proc.returncode == 0, f"{argv}\n{proc.stdout[-2000:]}\n{proc.stderr[-2000:]}"
    return proc


@pytest.fixture(scope="module")
def track_run(need, build_bin, root_exe, tmp_path_factory):
    if not (build_bin / "gxana_syst_track").exists():
        pytest.skip(f"{build_bin / 'gxana_syst_track'} not built")
    cfg = config.load_channel("kpkpxim")
    mc = cfg["xsection"]["mc_sample"]
    tmp = tmp_path_factory.mktemp("syst_track")
    out, data = tmp / "out", tmp / "data"
    (data / "flatTrees").mkdir(parents=True)
    for period in cfg["periods"]:
        stem = config.tree_stem(cfg, period, "data")
        mc_stem = config.tree_stem(cfg, period, mc)
        name = f"postQVal_flatTree_{stem}_nominal_kphighrap_1111111.root"
        mc_name = f"flatTree_{mc_stem}_nominal_kphighrap.root"
        thrown_name = f"flatTree_thrown_{mc_stem}.root"
        src, src_mc, src_thrown = need(f"flat_trees/{name}", f"flat_trees/{mc_name}", f"flat_trees/{thrown_name}")
        qdir = out / "kpkpxim/qfactors" / f"{stem}_nominal_kphighrap_1111111"
        qdir.mkdir(parents=True)
        (qdir / name).symlink_to(src)
        (data / "flatTrees" / mc_name).symlink_to(src_mc)
        (data / "flatTrees" / thrown_name).symlink_to(src_thrown)

    repo = repo_root()
    env = {**os.environ, "GXANA_ROOT": str(repo), "GXANA_OUTPUT": str(out), "GXANA_DATA": str(data)}
    assert stage.run_systematics(cfg, ["track"], study_names=["track"], environ=env) == 0
    new = Path(stage.study_dir(cfg, "track", env))

    legacy = tmp / "legacy"
    legacy.mkdir()
    logon = str(repo / "rootlogon.C")
    archived = repo / "archive/systematics_legacy/track_efficiency"
    _run([root_exe, "-l", "-b", "-q", logon, str(archived / "get_hists.C")], cwd=legacy, env=env)
    proc = _run([root_exe, "-l", "-b", "-q", logon, str(archived / "get_track_efficiency.C")], cwd=legacy, env=env)
    return new, legacy, proc.stdout


def _rows(path: Path):
    lines = path.read_text().splitlines()
    cols = lines[0].split()[1:]
    return {p[0]: dict(zip(cols, map(float, p[1:]))) for p in (line.split() for line in lines[1:-1])}


def test_track_efficiency_reproduces_the_dissertation_table(track_run):
    new, _, _ = track_run
    rows = _rows(new / "track_efficiency.txt")
    print(new.joinpath("track_efficiency.txt").read_text())
    for name, want in EXPECTED_MC_RAW.items():
        assert round(rows[name]["mc_raw"], 4) == pytest.approx(want, abs=1e-4)
    assert rows["proton"]["mc"] == 0.05
    assert rows["total"]["mc"] == pytest.approx(0.2036, abs=1e-4)   # per-track sum; the text quotes 20.29 %
    assert rows["total"]["mc_raw"] == pytest.approx(0.186468, abs=1e-6)
    assert rows["total"]["data_raw"] == pytest.approx(0.184258, abs=1e-6)
    report = new.joinpath("track_efficiency.txt").read_text().splitlines()[-1].split()
    assert report[:2] == ["report", "mc"] and float(report[2]) == pytest.approx(rows["total"]["mc"])
    for name in PARTICLES:
        assert (new / f"{name}_kin_angle_phase1_mc_data_mc.pdf").exists(), name


def test_counts_match_the_archived_macro(track_run):
    new, _, stdout = track_run
    legacy = {}
    for m in re.finditer(r"(\w+)_kin: \(Data, MC\)\n\tNlow \(([^,]+), ([^)]+)\)\n\tNhigh \(([^,]+), ([^)]+)\)", stdout):
        legacy[m.group(1)] = [float(m.group(i)) for i in (2, 4, 3, 5)]  # nlow, nhigh data; nlow, nhigh mc
    lines = new.joinpath("track_counts.txt").read_text().splitlines()
    assert lines[0] == "particle nlow_data nhigh_data nlow_mc nhigh_mc"
    counts = {p[0]: [float(v) for v in p[1:]] for p in (line.split() for line in lines[1:])}
    assert list(counts) == PARTICLES and sorted(legacy) == sorted(PARTICLES)
    for name in PARTICLES:  # cout prints 6 significant digits, as %.6g does
        assert counts[name] == pytest.approx(legacy[name], rel=1e-5), name


def test_figures_match_the_archived_macro(track_run, tmp_path):
    new, legacy, _ = track_run
    worst = []
    for name in PARTICLES:
        pdf = f"{name}_kin_angle_phase1_mc_data_mc.pdf"
        assert (legacy / pdf).exists(), pdf
        a = _raster(legacy / pdf, tmp_path / f"ref_{pdf}")
        b = _raster(new / pdf, tmp_path / f"new_{pdf}")
        assert a.shape == b.shape, (pdf, a.shape, b.shape)
        frac = float(np.mean(np.abs(a.astype(int) - b.astype(int)) > 32))
        worst.append((frac, pdf))
    print("pixel fractions:", sorted(worst))
    assert max(worst)[0] <= 0.002, sorted(worst)[-5:]
