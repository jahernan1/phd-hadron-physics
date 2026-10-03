import pytest

from gxana import bins


def test_edge_label_two_decimals_rounding():
    assert [bins.edge_label(x) for x in (6.4, 11.4, 10.18, 0.1, 7.86)] == ["6.40", "11.40", "10.18", "0.10", "7.86"]
    # Python rounds; the C++ BinEdgeLabel truncates ("0.37", "6.40", "7.85"): kept, see PORT_NOTES.
    assert [bins.edge_label(x) for x in (0.375, 6.405, 7.855)] == ["0.38", "6.41", "7.86"]


def test_energy_bins_and_args():
    edges = [6.4, 7.4, 7.855, 11.4]
    assert bins.energy_bins(edges) == [("6.40", "7.40"), ("7.40", "7.86"), ("7.86", "11.40")]
    assert bins.energy_args(edges) == ["--energy", "6.40:7.40", "--energy", "7.40:7.86", "--energy", "7.86:11.40"]
    assert bins.energy_bins([6.4]) == [] and bins.energy_args([]) == []


def test_flatten_t_bins():
    from gxana import config
    assert bins.flatten_t_bins([[0.1, 0.35], [0.35, 0.53]]) == [0.1, 0.35, 0.53]
    with pytest.raises(config.ConfigError, match=r"^t_bins are not contiguous: \[\[0\.1, 0\.35\], \[0\.4, 0\.53\]\]$"):
        bins.flatten_t_bins([[0.1, 0.35], [0.4, 0.53]])
