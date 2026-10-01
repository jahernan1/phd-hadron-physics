import numpy as np
import pytest

from gxana_systematics import sfactor


def test_scale_factor_matches_pdg_definition():
    y = np.array([[1.0], [2.0], [3.0]])
    err = np.array([[0.5], [0.5], [0.5]])
    r = sfactor.scale_factor(y, err)
    w = 1 / 0.25
    mean = 2.0
    chi2 = w * ((1 - mean) ** 2 + (3 - mean) ** 2)
    assert r.mean[0] == pytest.approx(mean)
    assert r.stat_err[0] == pytest.approx((3 * w) ** -0.5)
    assert r.chi2[0] == pytest.approx(chi2)
    assert r.n[0] == 3
    assert r.s[0] == pytest.approx(np.sqrt(chi2 / 2))


def test_zero_error_period_is_dropped():
    # a zero error means the period has no measurement there (legacy weights[isinf] = 0)
    y = np.array([[1.0], [2.0], [5.0]])
    err = np.array([[0.5], [0.5], [0.0]])
    r = sfactor.scale_factor(y, err)
    assert r.n[0] == 2
    assert r.mean[0] == pytest.approx(1.5)
    assert np.isfinite(r.s[0])


def test_run_systematic_rule():
    stat = np.array([0.4, 0.4, 0.477055])
    s = np.array([0.596467, 1.0, 1.128338])
    assert sfactor.run_systematic(stat, s) == pytest.approx([0.0, 0.0, 0.477055 * 0.128338])


def test_cli_writes_rows(tmp_path):
    for stem, y, e in (("a", 1.0, 0.5), ("b", 2.0, 0.5), ("c", 3.0, 0.5)):
        (tmp_path / f"diffxsec_flatTree_{stem}_emin_6.40_emax_7.40.txt").write_text(
            f"tBinCenter dsigmadt tBinWidth Yerr\n0.225 {y} 0.125 {e}\n")
    out = tmp_path / "sfactor_stats.txt"
    assert sfactor.main(["--out", str(out), "--periods-dir", str(tmp_path), "--n-periods", "3",
                         "--energy", "6.40:7.40"]) == 0
    lines = out.read_text().splitlines()
    assert lines[0] == sfactor.HEADER
    x, ex, mean, stat, chi2, n, s, syst = lines[1].split()
    assert (x, ex, mean, n) == ("0.225", "0.125", "2", "3")
    assert float(syst) == pytest.approx(float(stat) * (float(s) - 1))
