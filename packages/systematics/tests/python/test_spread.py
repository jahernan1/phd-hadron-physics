import numpy as np
import pytest

from gxana_systematics import spread
from gxana_systematics.tables import Block


def _blocks(ys, xs=(0.2, 0.4), ex=(0.1, 0.1)):
    return [Block("6.40", "7.40", np.array([[x, y, e, 0.1, 1.0] for x, y, e in zip(xs, ys, ex)]))]


def test_mean_and_sample_std_dev():
    rows = spread.spread({"a": _blocks([1.0, 4.0]), "b": _blocks([3.0, 4.0]), "c": _blocks([2.0, 4.0])})
    assert rows[0] == pytest.approx((0.2, 0.1, 2.0, 1.0))  # std of 1,3,2 with N-1 = 1
    assert rows[1] == pytest.approx((0.4, 0.1, 4.0, 0.0))


def test_two_members_match_legacy_formula():
    rows = spread.spread({"a": _blocks([1.0, 2.0]), "b": _blocks([2.0, 2.0])})
    assert rows[0][3] == pytest.approx(abs(1.0 - 2.0) / np.sqrt(2))


def test_mismatched_points_raise():
    with pytest.raises(spread.SpreadError, match="'b'.*6.40"):
        spread.spread({"a": _blocks([1.0, 2.0]), "b": _blocks([1.0, 2.0], xs=(0.2, 0.5))})


def test_mismatched_energy_bins_raise():
    a = _blocks([1.0, 2.0])
    b = a + [Block("7.40", "7.86", a[0].rows)]
    with pytest.raises(spread.SpreadError, match="energy bins"):
        spread.spread({"a": a, "b": b})


def test_needs_two_members():
    with pytest.raises(spread.SpreadError, match="at least 2"):
        spread.spread({"a": _blocks([1.0, 2.0])})


def test_write_stats_format(tmp_path):
    path = tmp_path / "s.txt"
    spread.write_stats(str(path), [(0.225, 0.125, 4.72434012, 0.0512306123), (1.0, 0.1, 2.0, 0.000242538)])
    assert path.read_text() == "XVal XErr YMean StdDev\n0.225 0.125 4.72434 0.0512306\n1 0.1 2 0.000242538\n"


def test_cli_writes_file(tmp_path):
    for label, ys in (("a", [1.0, 2.0]), ("b", [3.0, 2.0])):
        d = tmp_path / label
        d.mkdir()
        (d / "weighted_diffxsec_emin_6.40_emax_7.40.txt").write_text(
            "h\n" + "".join(f"{x} {y} 0.1 0.1 1\n" for x, y in zip((0.2, 0.4), ys)))
    out = tmp_path / "stats.txt"
    assert spread.main(["--out", str(out), "--member", f"a={tmp_path/'a'}", "--member", f"b={tmp_path/'b'}"]) == 0
    assert out.read_text().splitlines()[1] == "0.2 0.1 2 1.41421"
