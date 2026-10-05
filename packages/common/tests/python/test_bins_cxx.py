"""gxana.bins against the names the C++ gxana_xsec_bin writes
(gxana::BinEdgeLabel, gxana::EnergyBinName in gxana/common/BinNames.h; the
gxana::xsec:: forwarders are what this calls), through ROOT and the built
libraries. Python rounds, C++ truncates: the three-decimal edges differ and the
difference is pinned here as kept behaviour (docs/history/PORT_NOTES.md)."""
import os
import shutil
import subprocess

import pytest

from gxana import bins, config
from gxana.paths import repo_root

ROOT = repo_root()
pytestmark = [
    pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH"),
    pytest.mark.skipif(not any((ROOT / "build" / "lib").glob("libGxanaXsec.*")),
                       reason="C++ packages not built (uv run cmake --build build)"),
]

EXTRA = [0.375, 6.405, 7.855, 10.18]
# edge: (python label, C++ label) where round and truncate disagree
DIFFERENT = {0.375: ("0.38", "0.37"), 6.405: ("6.41", "6.40"), 7.855: ("7.86", "7.85")}


def _edges():
    cfg = config.load_channel("kpkpxim")
    t_edges = [e for pair in cfg["t_bins"] for e in pair]
    return sorted(set(cfg["energy_edges"]) | set(t_edges) | set(EXTRA))


def _cxx(tmp_path, edges):
    pairs = list(zip(edges, edges[1:]))
    body = [f'    std::cout << "LABEL " << gxana::xsec::BinEdgeLabel({e!r}) << "\\n";' for e in edges]
    body += [f'    std::cout << "NAME " << gxana::xsec::EnergyBinName({lo!r}, {hi!r}) << "\\n";' for lo, hi in pairs]
    macro = tmp_path / "bin_labels.C"
    macro.write_text('#include <iostream>\n#include "gxana/xsection/Binning.h"\nvoid bin_labels() {\n'
                     + "\n".join(body) + "\n}\n")
    proc = subprocess.run(["root", "-l", "-b", "-q", "rootlogon.C", str(macro)], cwd=ROOT,
                          env=dict(os.environ, GXANA_ROOT=str(ROOT)), capture_output=True, text=True, timeout=300)
    out = proc.stdout.splitlines()
    labels = [line.split(" ", 1)[1] for line in out if line.startswith("LABEL ")]
    names = [line.split(" ", 1)[1] for line in out if line.startswith("NAME ")]
    assert len(labels) == len(edges) and len(names) == len(pairs), proc.stdout + proc.stderr
    return labels, names


def test_python_bin_labels_match_cxx_except_three_decimal_edges(tmp_path):
    edges = _edges()
    labels, names = _cxx(tmp_path, edges)
    for edge, cxx in zip(edges, labels):
        assert (bins.edge_label(edge), cxx) == DIFFERENT.get(edge, (cxx, cxx)), edge
    for (lo, hi), (plo, phi), cxx in zip(zip(edges, edges[1:]), bins.energy_bins(edges), names):
        same = lo not in DIFFERENT and hi not in DIFFERENT
        assert (f"emin_{plo}_emax_{phi}" == cxx) is same, (lo, hi, cxx)
