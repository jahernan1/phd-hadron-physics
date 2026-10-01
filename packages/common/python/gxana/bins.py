"""Bin-edge labels and energy-bin arguments shared by the stages: the python side
of the names the C++ gxana_xsec_bin writes (gxana::xsec::BinEdgeLabel,
EnergyBinName). No numpy: the core gxana package needs only PyYAML."""
from __future__ import annotations

from typing import List, Sequence, Tuple


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
