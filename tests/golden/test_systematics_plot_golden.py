"""Golden: gxana_syst_plot draws what the archived comparison macros drew.

Both sides run on the same inputs, made here from the preserved weighted tables:
- accidentals: acc_weight, best_combo, hybrid_combo (preserved) with the legacy two-member
  combo stats; archived PlotComboComparison.C vs the grid3 and pair_band layouts;
- fit: the preserved johnson, hybrid_combo, acc_weight, best_combo tables stand in for the eight
  fit labels (cycled); archived PlotFitComparison.C vs the grid3 and all_band layouts.
The archived macros read WeightedDiffXSecTGraphs_<label>.root from the cwd and write
$GXANA_OUTPUT/kpkpxim/xsection/plots/<name>.pdf; the graphs are made with
analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C(dir, out).
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
from gxana_systematics import stage

pytestmark = pytest.mark.golden

FIT = ["hybrid_combo", "johnson", "qvalues", "johnson_cheby1", "voigt", "voigt_cheby1", "mcPdf", "mcPdf_cheby1"]
PDFS = [f"weighted_diffxsec_{n}.pdf" for n in
        ("ComboSelection", "Combos_StdDev", "SignalFitVoigt", "SignalFitMC", "SignalFitJohn", "AllFits")]
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
    (weighted,) = need("reference/xsection/weighted")
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
    return legacy_out / "kpkpxim/xsection/plots", new


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


def test_plots_match_the_archived_macros(pdfs, tmp_path):
    legacy, new = pdfs
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
