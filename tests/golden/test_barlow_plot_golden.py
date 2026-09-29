"""Golden: gxana_barlow_plot on the preserved weighted tables.

1. sigma_B in every .txt equals an independent recomputation of the legacy
   formula (PlotXSecBarlow*.C calc_barlow) from the preserved tables, to 1e-6.
2. Every PDF matches, pixel for pixel within a small tolerance, the PDF the
   archived PlotXSecBarlow*.C macro made from the same inputs
   (reference/barlow/plots, tests/golden/make_barlow_reference_plots.py).
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import numpy as np
import pytest

from gxana import config
from gxana_barlow import stage as st
from gxana_barlow.variations import expand

pytestmark = pytest.mark.golden

FAMILIES = ["chisqndf", "total_mm2_abs", "xim_pathlensig", "lambda_pathlensig", "kphigh_prap"]
N_PDFS = len(FAMILIES) * 9


@pytest.fixture(scope="module")
def tables(need):
    return need("reference/xsection/weighted/johnson", "reference/systematics/weighted_data")


@pytest.fixture(scope="module")
def plots(tables, build_bin, tmp_path_factory):
    exe = build_bin / "gxana_barlow_plot"
    if not exe.exists():
        pytest.skip(f"{exe} not built")
    nominal, variations = tables
    cfg = config.load_channel("kpkpxim")
    out = tmp_path_factory.mktemp("barlow_plots")
    for cmd in st.plot_commands(cfg, expand(cfg["barlow"]), str(nominal), str(variations), str(out), str(exe)):
        proc = subprocess.run(cmd.argv, capture_output=True, text=True)
        assert proc.returncode == 0, f"{cmd.argv}\n{proc.stderr[-2000:]}"
    return out


def _rows(path: Path) -> np.ndarray:
    """Numeric rows of a table (TGraphErrors reads the first four columns and skips headers)."""
    rows = []
    for line in path.read_text().splitlines():
        try:
            rows.append([float(x) for x in line.split()[:4]])
        except ValueError:
            continue
    return np.array(rows)


def _sigma_b(nominal: np.ndarray, variation: np.ndarray) -> np.ndarray:
    sigma = np.sqrt(np.abs(nominal[:, 3] ** 2 - variation[:, 3] ** 2))
    with np.errstate(divide="ignore", invalid="ignore"):
        return np.where(sigma != 0.0, (nominal[:, 1] - variation[:, 1]) / sigma, 0.0)


def test_sigma_b_matches_the_legacy_formula(plots, tables):
    nominal_dir, var_dir = tables
    cfg = config.load_channel("kpkpxim")
    edges = [f"{e:.2f}" for e in cfg["energy_edges"]]
    checked = 0
    for family in FAMILIES:
        ids = [v.id for v in expand(cfg["barlow"]) if v.family == family]
        cases = [("barlow_weighted_totxsec_vary_" + family, "totxsec_weighted_output.txt",
                  lambda vid: f"weighted_totxsec_vary_{vid}.txt")]
        for lo, hi in zip(edges, edges[1:]):
            b = f"_emin_{lo}_emax_{hi}"
            cases.append((f"barlow_weighted_diffxsec_vary_{family}{b}", f"weighted_diffxsec{b}.txt",
                          lambda vid, b=b: f"weighted_diffxsec_vary_{vid}{b}.txt"))
        for stem, nominal_name, var_name in cases:
            got = [line.split() for line in (plots / f"{stem}.txt").read_text().splitlines()[1:]]
            nominal = _rows(nominal_dir / nominal_name)
            expected = []
            for vid in ids:
                sb = _sigma_b(nominal, _rows(var_dir / var_name(vid)))
                expected += [(vid, nominal[i, 0], s) for i, s in enumerate(sb)]
            assert len(got) == len(expected), stem
            for row, (vid, x, sb) in zip(got, expected):
                assert row[0] == vid and abs(float(row[1]) - x) < 1e-9
                # a NaN input point (kphigh_prap_2.2, last -t bin of 6.40-7.40) gives NaN in both
                assert np.isclose(float(row[6]), sb, rtol=0.0, atol=1e-6, equal_nan=True), (stem, row)
            checked += 1
    assert checked == N_PDFS


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


def test_plots_match_the_archived_macros(plots, need, tmp_path):
    (reference,) = need("reference/barlow/plots")
    refs = sorted(reference.glob("barlow_*.pdf"))
    assert len(refs) == N_PDFS
    worst = []
    for ref in refs:
        new = plots / ref.name
        assert new.exists(), ref.name
        a = _raster(ref, tmp_path / f"ref_{ref.stem}")
        b = _raster(new, tmp_path / f"new_{ref.stem}")
        assert a.shape == b.shape, (ref.name, a.shape, b.shape)
        frac = float(np.mean(np.abs(a.astype(int) - b.astype(int)) > 32))
        worst.append((frac, ref.name))
    assert max(worst)[0] <= 0.002, sorted(worst)[-5:]
