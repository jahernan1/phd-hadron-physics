"""Golden: gxana_syst_plot draws what the archived comparison macros drew.

Both sides run on the same inputs, made here from the preserved weighted tables:
- accidentals: acc_weight, best_combo, hybrid_combo (preserved) with the legacy two-member
  combo stats; archived PlotComboComparison.C vs the grid3 and pair_band layouts;
- fit: the preserved johnson, hybrid_combo, acc_weight, best_combo tables stand in for the eight
  fit labels (cycled); archived PlotFitComparison.C vs the grid3 and all_band layouts;
- qval_yield: the preserved hybrid_combo and the preserved per-period qvalues tables weighted here
  (gxana_xsection.weighted_average); archived PlotQValueComparison.C vs the grid2 layout;
- run_compare: the preserved per-period johnson tables; archived PlotRunComparison.C vs the
  run_grid and stddev_band layouts, the band from gxana_systematics.runcompare (whose numbers
  are also checked against the macro's run_comp_stddev_scaled.txt).
The archived macros read WeightedDiffXSecTGraphs_<label>.root (DiffXSecTGraphs_<stem>_<label>.root
for one run period) from the cwd and write $GXANA_OUTPUT/kpkpxim/xsection/plots/<name>.pdf; the
graphs are made with analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C(dir, out).
"""
from __future__ import annotations

import copy
import os
import shutil
import subprocess
from pathlib import Path

import numpy as np
import pytest

from gxana import config
from gxana.paths import repo_root
from gxana_systematics import runcompare, stage
from gxana_xsection.weighted_average import weight_files

pytestmark = pytest.mark.golden

FIT = ["hybrid_combo", "johnson", "qvalues", "johnson_cheby1", "voigt", "voigt_cheby1", "mcPdf", "mcPdf_cheby1"]
PDFS = [f"weighted_diffxsec_{n}.pdf" for n in
        ("ComboSelection", "Combos_StdDev", "SignalFitVoigt", "SignalFitMC", "SignalFitJohn", "AllFits",
         "QValYield", "RunComparison", "RunCompStdDevScaled")]
# PlotRunComparison.C's PlotWeightedXSec legend (copied from the combo macro); the config names
# the run periods. The drawing is compared with the legacy text.
RUN_GRID_LEGACY_LEGEND = ["RF Sub|lep", "Best #chi^{2}_{#nu}|lep", "Hybrid #chi^{2}_{#nu}|lep"]
LEGACY = {"accidentals": ("PlotComboComparison.C", "combo_variations_stats.txt", ["acc_weight", "hybrid_combo"]),
          "fit": ("PlotFitComparison.C", "fit_variations_stats.txt", None)}


def _run(argv, **kwargs):
    proc = subprocess.run(argv, capture_output=True, text=True, **kwargs)
    assert proc.returncode == 0, f"{argv}\n{proc.stdout[-2000:]}\n{proc.stderr[-2000:]}"
    return proc


@pytest.fixture(scope="module")
def pdfs(need, build_bin, root_exe, tmp_path_factory):
    exe = build_bin / "gxana_syst_plot"
    if not exe.exists():
        pytest.skip(f"{exe} not built")
    weighted, qvalues, johnson = need("reference/xsection/weighted", "reference/xsection/qvalues",
                                      "reference/xsection/johnson")
    acc, best, hyb, joh = (weighted / n for n in ("acc_weight", "best_combo", "hybrid_combo", "johnson"))
    labels = {"acc_weight": acc, "best_combo": best, "hybrid_combo": hyb, "johnson": joh}
    labels.update(zip(FIT, [hyb, joh, acc, best, hyb, joh, acc, best]))

    tmp = tmp_path_factory.mktemp("syst_plot")
    repo = repo_root()
    for label, src in labels.items():
        dst = tmp / "in" / label
        dst.mkdir(parents=True)
        for f in sorted(src.glob("weighted_diffxsec_emin_*.txt")):
            shutil.copy(f, dst / f.name)

    # Legacy side: the archived macros on WeightedDiffXSecTGraphs_<label>.root in the cwd.
    legacy, legacy_out = tmp / "legacy", tmp / "legacy_out"
    legacy.mkdir()
    (legacy_out / "kpkpxim/xsection/plots").mkdir(parents=True)
    env = {**os.environ, "GXANA_ROOT": str(repo), "GXANA_OUTPUT": str(legacy_out)}
    logon = str(repo / "rootlogon.C")
    maker = repo / "analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C"
    for label in labels:
        _run([root_exe, "-l", "-b", "-q", logon,
              f'{maker}("{tmp / "in" / label}/","{legacy / f"WeightedDiffXSecTGraphs_{label}.root"}")'],
             cwd=legacy, env=env)
    for macro, _, _ in LEGACY.values():
        _run([root_exe, "-l", "-b", "-q", logon, str(repo / "archive/systematics_legacy/comparisons" / macro)],
             cwd=legacy, env=env)

    # New side: the stage's plot commands, pointed at the same inputs and the legacy stats.
    new = tmp / "new"
    new.mkdir()
    cfg = config.load_channel("kpkpxim")
    studies = cfg["systematics"]["studies"]
    for name, (_, stats, members) in LEGACY.items():
        study = copy.deepcopy(studies[name])
        if members:
            study["spread"] = members
        for cmd in stage.plot_commands(cfg, name, study, env):
            argv = list(cmd.argv)
            argv[0] = str(exe)
            for i, a in enumerate(argv[:-1]):
                if a == "--input":
                    argv[i + 1] = str(tmp / "in" / Path(argv[i + 1]).name)
                elif a == "--band":
                    argv[i + 1] = str(legacy / stats)
                elif a == "--out-dir":
                    argv[i + 1] = str(new)
            _run(argv)

    # qval_yield: the qvalues per-period tables weighted as the weight step weights them.
    qdir = tmp / "in" / "qvalues_yield"
    qdir.mkdir()
    edges = cfg["energy_edges"]
    for e in edges[:-1]:
        weight_files(str(qvalues), str(qdir), pattern=f"diffxsec*_emin_{e:.2f}*.txt")
    legacy_q = tmp / "legacy_qval"
    legacy_q.mkdir()
    for label, src in (("hybrid_combo", tmp / "in" / "hybrid_combo"), ("qvalues", qdir)):
        _run([root_exe, "-l", "-b", "-q", logon,
              f'{maker}("{src}/","{legacy_q / f"WeightedDiffXSecTGraphs_{label}.root"}")'], cwd=legacy_q, env=env)
    _run([root_exe, "-l", "-b", "-q", logon,
          str(repo / "archive/systematics_legacy/comparisons/PlotQValueComparison.C")], cwd=legacy_q, env=env)
    inputs = {"hybrid_combo": tmp / "in" / "hybrid_combo", "qvalues": qdir}
    for cmd in stage.plot_commands(cfg, "qval_yield", studies["qval_yield"], env, step="compare"):
        argv = list(cmd.argv)
        argv[0] = str(exe)
        for i, a in enumerate(argv[:-1]):
            if a == "--input":
                argv[i + 1] = str(inputs[Path(argv[i + 1]).name])
            elif a == "--out-dir":
                argv[i + 1] = str(new)
        _run(argv)

    # run_compare: one DiffXSecTGraphs_<stem>_hybrid_combo.root per period for the macro.
    legacy_run = tmp / "legacy_run"
    legacy_run.mkdir()
    for period in cfg["periods"]:
        stem = config.tree_stem(cfg, period, "data")
        pdir = tmp / "periods" / stem
        pdir.mkdir(parents=True)
        for f in sorted(johnson.glob(f"diffxsec_flatTree_{stem}_emin_*.txt")):
            shutil.copy(f, pdir / f.name)
        _run([root_exe, "-l", "-b", "-q", logon,
              f'{maker}("{pdir}/","{legacy_run / f"DiffXSecTGraphs_{stem}_hybrid_combo.root"}")'],
             cwd=legacy_run, env=env)
    _run([root_exe, "-l", "-b", "-q", logon,
          str(repo / "archive/systematics_legacy/comparisons/PlotRunComparison.C")], cwd=legacy_run, env=env)
    band = tmp / "run_comp_stddev_scaled.txt"
    energy = [a for lo, hi in zip(edges, edges[1:]) for a in ("--energy", f"{lo:.2f}:{hi:.2f}")]
    assert runcompare.main(["--out", str(band), "--periods-dir", str(johnson),
                            "--n-periods", str(len(cfg["periods"])), *energy]) == 0
    for cmd in stage.plot_commands(cfg, "run_compare", studies["run_compare"], env, step="compare"):
        argv = list(cmd.argv)
        argv[0] = str(exe)
        for i, a in enumerate(argv[:-1]):
            if a == "--input":
                argv[i + 1] = str(johnson / Path(argv[i + 1]).name)
            elif a == "--band":
                argv[i + 1] = str(band)
            elif a == "--out-dir":
                argv[i + 1] = str(new)
        if "run_grid" in argv:
            argv = [a for i, a in enumerate(argv) if a != "--legend" and argv[i - 1] != "--legend"]
            for entry in RUN_GRID_LEGACY_LEGEND:
                argv += ["--legend", entry]
        _run(argv)
    return legacy_out / "kpkpxim/xsection/plots", new, legacy_run / "run_comp_stddev_scaled.txt", band


def _read_pgm(path: Path) -> np.ndarray:
    data = path.read_bytes()
    tokens, pos = [], 0
    while len(tokens) < 4:  # magic, width, height, maxval; skip comments
        while data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":
            pos = data.index(b"\n", pos) + 1
            continue
        end = pos
        while not data[end:end + 1].isspace():
            end += 1
        tokens.append(data[pos:end])
        pos = end
    assert tokens[0] == b"P5", tokens
    width, height = int(tokens[1]), int(tokens[2])
    return np.frombuffer(data[pos + 1:pos + 1 + width * height], dtype=np.uint8).reshape(height, width)


def _raster(pdf: Path, out: Path) -> np.ndarray:
    pgm = Path(f"{out}.pgm")  # not with_suffix: names contain "6.40"
    if shutil.which("pdftoppm"):
        subprocess.run(["pdftoppm", "-gray", "-r", "50", "-singlefile", str(pdf), str(out)], check=True)
    elif shutil.which("gs"):
        subprocess.run(["gs", "-q", "-dNOPAUSE", "-dBATCH", "-dSAFER", "-sDEVICE=pgmraw", "-r50",
                        f"-sOutputFile={pgm}", str(pdf)], check=True)
    else:
        pytest.skip("no PDF rasterizer (pdftoppm or gs)")
    return _read_pgm(pgm)


def test_run_compare_numbers_match_the_archived_macro(pdfs):
    _, _, legacy, new = pdfs
    a = np.loadtxt(legacy, comments="#", ndmin=2)
    b = np.loadtxt(new, comments="#", ndmin=2)
    assert a.shape == b.shape == (56, 7)
    # the macro prints 6 significant digits; runcompare writes %.6g
    np.testing.assert_allclose(b, a, rtol=2e-5, atol=1e-12)


def test_plots_match_the_archived_macros(pdfs, tmp_path):
    legacy, new = pdfs[:2]
    worst = []
    for name in PDFS:
        assert (legacy / name).exists() and (new / name).exists(), name
        a = _raster(legacy / name, tmp_path / f"ref_{name}")
        b = _raster(new / name, tmp_path / f"new_{name}")
        assert a.shape == b.shape, (name, a.shape, b.shape)
        frac = float(np.mean(np.abs(a.astype(int) - b.astype(int)) > 32))
        worst.append((frac, name))
    print("pixel fractions:", sorted(worst))
    assert max(worst)[0] <= 0.002, sorted(worst)[-5:]
