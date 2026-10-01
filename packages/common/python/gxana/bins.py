"""Bin-edge labels and energy-bin arguments shared by the stages: the python side
of the names the C++ gxana_xsec_bin writes (gxana::xsec::BinEdgeLabel,
EnergyBinName). No numpy: the core gxana package needs only PyYAML."""
from __future__ import annotations

from typing import List, Sequence, Tuple

from gxana import config


def edge_label(x: float) -> str:
    """Two-decimal label of a bin edge, as in the table names and glob patterns.

    Rounds (Python format); the C++ BinEdgeLabel truncates. Both agree on every
    edge with at most two decimals (all configured edges); they differ on e.g.
    7.855 ("7.86" here, "7.85" in C++) -- see docs/KNOWN_ISSUES.md."""
    return f"{x:.2f}"


def energy_bins(edges: Sequence[float]) -> List[Tuple[str, str]]:
    """[(lo, hi), ...] labels of consecutive energy edges."""
    return [(edge_label(lo), edge_label(hi)) for lo, hi in zip(edges, edges[1:])]


def energy_args(edges: Sequence[float]) -> List[str]:
    """["--energy", "lo:hi", ...], one pair per energy bin."""
    argv: List[str] = []
    for lo, hi in energy_bins(edges):
        argv += ["--energy", f"{lo}:{hi}"]
    return argv


def flatten_t_bins(t_bins: Sequence[Sequence[float]]) -> List[float]:
    """[[lo, hi], ...] -> [lo, hi, ...]; ConfigError if a bin does not start where the last one ended."""
    edges = [t_bins[0][0]]
    for lo, hi in t_bins:
        if lo != edges[-1]:
            raise config.ConfigError(f"t_bins are not contiguous: {t_bins}")
        edges.append(hi)
    return edges
