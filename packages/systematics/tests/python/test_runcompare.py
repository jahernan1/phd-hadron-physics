import numpy as np
import pytest

from gxana_systematics import runcompare


def test_scaled_std_dev():
    periods = [np.array([[0.2, y, 0.1, 0.5]]) for y in (1.0, 2.0, 3.0)]
    (row,) = runcompare.rows(periods)
    x, ex, wmean, std_scaled, mean, std, s = row
    assert (x, ex, mean, std) == pytest.approx((0.2, 0.1, 2.0, 1.0))
    assert s == pytest.approx(np.sqrt(4 * 2 / 2))
    assert std_scaled == pytest.approx(std * s)
