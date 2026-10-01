import numpy as np
import pytest

from gxana_systematics import summary
from gxana_systematics.tables import Block


def test_combine_quadrature():
    blocks = [Block("6.40", "7.40", np.array([[0.2, 1, 0.1, 0.1, 1], [0.4, 1, 0.1, 0.1, 1]]))]
    rows = summary.combine(blocks, {"a": np.array([0.3, 0.0]), "b": np.array([0.4, 0.1])})
    assert rows[0] == ("6.40", "7.40", 0.2, 0.1, 0.3, 0.4, pytest.approx(0.5))
    assert rows[1][-1] == pytest.approx(0.1)


def test_combine_length_mismatch():
    blocks = [Block("6.40", "7.40", np.array([[0.2, 1, 0.1, 0.1, 1]]))]
    with pytest.raises(ValueError, match="'a'"):
        summary.combine(blocks, {"a": np.array([0.3, 0.1])})


def test_cli(tmp_path):
    w = tmp_path / "w"
    w.mkdir()
    (w / "weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text("h\n0.2 1 0.1 0.1 1\n")
    (tmp_path / "a.txt").write_text("XVal XErr YMean StdDev\n0.2 0.1 1 0.3\n")
    (tmp_path / "t.txt").write_text("particle data mc data_raw mc_raw\ntotal 0.2 0.2036 0.18 0.186\nreport mc 0.2036\n")
    assert summary.main(["--nominal-dir", str(w), "--out-dir", str(tmp_path / "s"), "--column", f"a={tmp_path/'a.txt'}",
                         "--normalization", f"track={tmp_path/'t.txt'}", "--normalization", "luminosity=0.05"]) == 0
    assert (tmp_path / "s/systematics_summary.txt").read_text().splitlines() == [
        "Emin Emax XVal XErr a Total", "6.40 7.40 0.2 0.1 0.3 0.3"]
    assert (tmp_path / "s/normalization.txt").read_text() == "track 0.2036\nluminosity 0.05\n"
