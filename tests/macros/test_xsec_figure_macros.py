"""xsection/PlotDiffXSec.C and PlotTotXsecWithClas.C draw the dissertation figures from the
directories they are given, write the drawn graphs in panel order beside the PDFs, write
nothing into the working directory, and exit 1 on a missing input."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
MACROS = ROOT / "analyses" / "kpkpxim" / "xsection"
pytestmark = [pytest.mark.macros,
              pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

BINS = [("7.40", "7.86"), ("6.40", "7.40"), ("10.18", "11.40")]
STEMS = {"2017-01": "kpkpxim__M23_2017-01_ana56", "2018-01": "kpkpxim__B4_M23_2018-01_ana03",
         "2018-08": "kpkpxim__B4_M23_2018-08_ana02"}
W_HEADER = "-t d\\sigma/dt \\delta_x \\delta_y S\n"
S_HEADER = "-t d\\sigma/dt \\delta_x \\delta_y_syst \\delta_y S\n"


def _root(call, cwd):
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_OUTPUT=str(cwd / "unused"), ROOT_MAX_THREADS="1")
    return subprocess.run(["root", "-l", "-b", "-q", str(ROOT / "rootlogon.C"), call], cwd=cwd, env=env,
                          capture_output=True, text=True, timeout=300)


def _dump(path, cwd):
    proc = _root(f'{ROOT / "tests" / "macros" / "dump_graphs.C"}("{path}")', cwd)
    assert proc.returncode == 0, proc.stderr[-2000:]
    return [line.split("\t") for line in proc.stdout.splitlines() if line.startswith(("G\t", "F\t"))]


def _xsec_dir(tmp_path, syst_bins=BINS):
    base = tmp_path / "xsec"
    w, d = base / "weighted_data" / "lab", base / "data" / "lab"
    w.mkdir(parents=True)
    d.mkdir(parents=True)
    for i, (lo, hi) in enumerate(BINS):
        y = 1.0 + i
        (w / f"weighted_diffxsec_emin_{lo}_emax_{hi}.txt").write_text(
            W_HEADER + f"0.225 {y} 0.125 0.4 0.5\n0.44 {y / 2} 0.09 0.3 1.2\n")
        if (lo, hi) in syst_bins:
            (w / f"syst_weighted_diffxsec_emin_{lo}_emax_{hi}.txt").write_text(
                S_HEADER + f"0.225 {y} 0.125 0.1 0.4 0.5\n0.44 {y / 2} 0.09 0.2 0.3 1.2\n")
        for stem in STEMS.values():
            (d / f"diffxsec_flatTree_{stem}_emin_{lo}_emax_{hi}.txt").write_text(
                f"tBinCenter dsigmadt tBinWidth Yerr\n0.225 {y} 0.125 0.5\n0.44 {y / 2} 0.09 0.4\n")
    return base


def test_diffxsec_figures_and_graphs_in_panel_order(tmp_path):
    base, plots, cwd = _xsec_dir(tmp_path), tmp_path / "plots", tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotDiffXSec.C"}("{base}","lab","{plots}")', cwd)
    assert proc.returncode == 0, (proc.stdout + proc.stderr)[-3000:]
    for name in ("diffxsec_runs_lab", "diffxsec_phase1_systematics_lab", "diffxsec_phase1_weighted_lab"):
        assert (plots / f"{name}.pdf").stat().st_size > 0, name
    rows = _dump(plots / "SystWeightedDiffXSecTGraphs_lab.root", cwd)
    ascending = sorted(BINS, key=lambda b: float(b[0]))
    assert [r[2] for r in rows[::2]] == [f"#bf{{E_{{#gamma}} (GeV): ({lo}, {hi})}}" for lo, hi in ascending]
    assert [float(r[7]) for r in rows[:2]] == [0.1, 0.2]       # ey = the systematic column
    stat = _dump(plots / "WeightedDiffXSecTGraphs_lab.root", cwd)
    assert [float(r[7]) for r in stat[:2]] == [0.4, 0.3]       # ey = the statistical column
    for period in STEMS:
        assert (plots / f"DiffXSecTGraphs_{period}_lab.root").is_file()
    assert list(cwd.iterdir()) == []


def test_diffxsec_without_all_systematic_tables_fails(tmp_path):
    base = _xsec_dir(tmp_path, syst_bins=BINS[:1])
    cwd = tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotDiffXSec.C"}("{base}","lab","{tmp_path / "plots"}")', cwd)
    assert proc.returncode == 1
    assert "syst_weighted_diffxsec" in proc.stdout + proc.stderr
    assert str(base / "weighted_data" / "lab") in proc.stdout + proc.stderr
    assert not (tmp_path / "plots").exists()
    assert list(cwd.iterdir()) == []


def test_diffxsec_without_a_run_period_fails_before_writing(tmp_path):
    base = _xsec_dir(tmp_path)
    for path in (base / "data" / "lab").glob(f"*{STEMS['2018-01']}*"):
        path.unlink()
    cwd = tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotDiffXSec.C"}("{base}","lab","{tmp_path / "plots"}")', cwd)
    assert proc.returncode == 1
    assert "2018-01" in proc.stdout + proc.stderr
    assert not (tmp_path / "plots").exists()


ENERGIES = [(6.9, 0.5), (7.63, 0.23), (8.025, 0.165)]


def _total_dir(tmp_path, weighted=True):
    base = tmp_path / "var"
    d, w = base / "data" / "lab", base / "weighted_data" / "lab"
    d.mkdir(parents=True)
    w.mkdir(parents=True)
    for stem in STEMS.values():
        (d / f"totxsec_flatTree_{stem}.txt").write_text(
            "enBinCenter sigma enBinWidth Yerr\n" + "".join(f"{e} {7 - (e - 6.9)} {h} 0.3\n" for e, h in ENERGIES))
    if weighted:
        (w / "totxsec_weighted_output.txt").write_text(
            W_HEADER + "".join(f"{e} {7 - (e - 6.9)} {h} 0.2 1.0\n" for e, h in ENERGIES))
    return base


def test_total_figure_writes_the_drawn_graphs_and_fit(tmp_path):
    base, plots, cwd = _total_dir(tmp_path), tmp_path / "plots", tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotTotXsecWithClas.C"}("{base}","lab","{plots}")', cwd)
    assert proc.returncode == 0, (proc.stdout + proc.stderr)[-3000:]
    assert (plots / "totxsec_clas_gluex_Phase1.pdf").stat().st_size > 0
    rows = _dump(plots / "totxsec_clas_gluex_Phase1.root", cwd)
    assert {r[1] for r in rows} == {"clas", "weighted", "sp17", "sp18", "fa18", "fit"}
    assert [float(r[4]) for r in rows if r[0] == "G" and r[1] == "sp17"] == [e for e, _ in ENERGIES]
    (fit,) = [r for r in rows if r[0] == "F"]
    assert int(fit[3]) > 0
    assert list(cwd.iterdir()) == []


def test_total_figure_missing_input_fails(tmp_path):
    base = _total_dir(tmp_path, weighted=False)
    cwd = tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotTotXsecWithClas.C"}("{base}","lab","{tmp_path / "plots"}")', cwd)
    assert proc.returncode == 1
    assert "totxsec_weighted_output.txt" in proc.stdout + proc.stderr
    assert list(cwd.iterdir()) == []


def test_total_figure_reads_explicit_directories(tmp_path):
    # the preserved tables keep another layout (<label>, weighted/<label>; --systematics published)
    base = _total_dir(tmp_path)
    data, weighted = tmp_path / "ref" / "lab", tmp_path / "ref" / "weighted" / "lab"
    weighted.parent.mkdir(parents=True)
    (base / "data" / "lab").rename(data)
    (base / "weighted_data" / "lab").rename(weighted)
    plots, cwd = tmp_path / "plots", tmp_path / "cwd"
    cwd.mkdir()
    proc = _root(f'{MACROS / "PlotTotXsecWithClas.C"}("{base}","lab","{plots}","{data}","{weighted}")', cwd)
    assert proc.returncode == 0, (proc.stdout + proc.stderr)[-3000:]
    rows = _dump(plots / "totxsec_clas_gluex_Phase1.root", cwd)
    assert [float(r[4]) for r in rows if r[0] == "G" and r[1] == "sp17"] == [e for e, _ in ENERGIES]
