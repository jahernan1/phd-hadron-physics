"""The kinematics study against the macro it replaces, for one period (2018-08): a frozen copy of
archive/root_macros/GetKinematicsDataMC_RF.C (tests/legacy/) with its per-period function
GetDataMCPlots, and the `kinematics` study of `gxana run studies` planned for that one period (the
configuration with the other periods removed), both single-threaded on the preserved trees. The same
36 PDF files, the same printed lines in the same order and, when Ghostscript is installed, identical
rasters for a sample of four PDFs. Skipped, each with its own reason, without ROOT, without the built
app, without the preserved trees; a skipped run proves nothing (`pytest -rs` lists the skips)."""
import copy
import os
import shutil
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.paths import analysis_data_root
from gxana_studies import stage

ROOT = Path(__file__).resolve().parents[4]
LEGACY = Path(__file__).resolve().parents[1] / "legacy"
APP = ROOT / "build" / "bin" / "gxana_study_datamc"
PERIOD = "2018-08"
TAG = "2018-08_ver02_kphighrap"
PRINTED = ("Sum of Weighted", "Bin Width")
# one data/MC plot with the legend top right and one top left, one truth plot of each
SAMPLE = (f"chisqndf_weighted_qvalue_acc_{TAG}_ac.pdf", f"beam_vertexZ_weighted_qvalue_acc_{TAG}_ac.pdf",
          f"pim2_P3_weighted_qvalue_acc_{TAG}_MC_Truth_ac.pdf",
          f"xim_costheta_gen_amp_weighted_qvalue_acc_{TAG}_MC_Truth_ac.pdf")

pytestmark = [pytest.mark.golden,
              pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH"),
              pytest.mark.skipif(not APP.is_file(), reason="gxana_study_datamc not built")]


def _inputs(cfg):
    """(link, preserved file) of the period's three trees, in the layout the macro reads."""
    stem = config.tree_stem(cfg, PERIOD, "data")
    kin = cfg["studies"]["kinematics"]
    flat = analysis_data_root() / "kpkpxim" / "flat_trees"
    q = f"{stem}_nominal_kphighrap_1111111"
    mc = f"flatTree_{stem}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root"
    thrown = f"flatTree_thrown_{stem}_gen_amp_V2_ac_YstarRest.root"
    assert kin["tags"][PERIOD] == TAG
    return [(Path("output") / "kpkpxim" / "qfactors" / q / f"postQVal_flatTree_{q}.root", flat / f"postQVal_flatTree_{q}.root"),
            (Path("data") / "flatTrees" / mc, flat / mc),
            (Path("data") / "flatTrees" / thrown, flat / thrown),  # the study reads it here
            (Path("data") / "Trees" / "flatTree" / "rawTrees" / thrown, flat / thrown)]  # the frozen macro here


def _printed(text):
    return [line for line in text.splitlines() if line.startswith(PRINTED)]


def _pdfs(base):
    return {p.relative_to(base).as_posix(): p for p in sorted(base.rglob("*.pdf"))}


def _raster(pdf):
    return subprocess.run(["gs", "-q", "-dNOPAUSE", "-dBATCH", "-dSAFER", "-sDEVICE=pgmraw", "-r100",
                           "-sOutputFile=-", str(pdf)], capture_output=True, check=True).stdout


@pytest.fixture(scope="module")
def runs(tmp_path_factory):
    cfg = copy.deepcopy(config.load_channel("kpkpxim"))
    cfg["periods"] = {PERIOD: cfg["periods"][PERIOD]}
    cfg["studies"]["kinematics"]["tags"] = {PERIOD: cfg["studies"]["kinematics"]["tags"][PERIOD]}
    links = _inputs(cfg)
    missing = [str(src) for _, src in links if not src.is_file()]
    if missing:
        pytest.skip("no preserved trees: " + ", ".join(missing))
    tmp = tmp_path_factory.mktemp("kinematics")
    for dst, src in links:  # "data/..." is shared, "output/..." is per run
        for link in ([tmp / dst] if dst.parts[0] == "data" else [tmp / n / dst for n in ("old", "new")]):
            link.parent.mkdir(parents=True, exist_ok=True)
            link.symlink_to(src)
    out = {n: tmp / n / "output" / "kpkpxim" / "data_mc_kinematics" for n in ("old", "new")}
    out["old"].mkdir(parents=True)  # the macro does not create its output directory
    env = {n: dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp / "data"),
                   GXANA_OUTPUT=str(tmp / n / "output"), ROOT_MAX_THREADS="1", PYTHONUNBUFFERED="1")
           for n in ("old", "new")}

    stem = config.tree_stem(cfg, PERIOD, "data")
    args = ", ".join(f'"{a}"' for a in (f"{stem}_nominal_kphighrap_1111111",
                                        f"flatTree_{stem}_gen_amp_V2_ac_YstarRest_nominal_kphighrap",
                                        f"flatTree_thrown_{stem}_gen_amp_V2_ac_YstarRest", TAG))
    driver = tmp / "driver.C"
    driver.write_text(f'void driver() {{ gROOT->ProcessLine(".L {LEGACY / "GetKinematicsDataMC_RF.C"}");\n'
                      f'  gROOT->ProcessLine(R"CALL(GetDataMCPlots({args}, 0))CALL"); }}\n')
    legacy = subprocess.run(["root", "-l", "-b", "-q", str(ROOT / "rootlogon.C"), str(driver)], cwd=tmp / "old",
                            env=env["old"], capture_output=True, text=True, timeout=1200)
    assert legacy.returncode == 0, legacy.stderr[-2000:]

    for d in stage.output_dirs(cfg, ["kinematics"], env["new"]):
        d.mkdir(parents=True, exist_ok=True)  # as `gxana run studies` does
    printed = []
    for command in stage.plan(cfg, ["fill", "plot"], ["kinematics"], env["new"]):
        argv = [str(APP)] + command.argv[1:]
        done = subprocess.run(argv, env=env["new"], capture_output=True, text=True, timeout=1200)
        assert done.returncode == 0, done.stderr[-2000:]
        printed += _printed(done.stdout)
    return {"old": _pdfs(out["old"]), "new": _pdfs(out["new"]), "legacy_lines": _printed(legacy.stdout),
            "study_lines": printed}


def test_kinematics_study_reproduces_the_macro(runs):
    assert sorted(runs["old"]) == sorted(runs["new"])
    assert len(runs["old"]) == 36
    assert len(runs["legacy_lines"]) == 1 + 2 * 36  # the weighted sum, a bin-width pair per PDF
    assert runs["study_lines"] == runs["legacy_lines"]


@pytest.mark.skipif(shutil.which("gs") is None, reason="Ghostscript (gs) not on PATH: PDF rasters not compared")
def test_kinematics_sample_rasters_are_identical(runs):
    for name in SAMPLE:
        old = next(k for k in runs["old"] if k.endswith(name))
        new = next(k for k in runs["new"] if k.endswith(name))
        assert _raster(runs["new"][new]) == _raster(runs["old"][old]), name
