"""xsection/MakeWeightedDiffXSecTGraphs.C writes the per-energy graphs in
ascending beam energy with an E_gamma title, whatever the directory-listing
order (PlotFitComparison.C draws panels and writes fit_variations_stats.txt
rows in key order)."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
pytestmark = [pytest.mark.macros,
              pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

EMINS = [("10.18", "11.40"), ("6.40", "7.40"), ("8.45", "8.68"), ("7.40", "7.86")]


def test_graphs_sorted_by_energy_with_titles(tmp_path):
    label = tmp_path / "label"
    label.mkdir()
    for lo, hi in EMINS:
        (label / f"weighted_diffxsec_emin_{lo}_emax_{hi}.txt").write_text(
            "-t d\\sigma/dt \\delta_x \\delta_y S\n0.225 4.4 0.125 0.4 0.5\n")
    out = tmp_path / "graphs.root"
    macro = ROOT / "analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C"
    dump = tmp_path / "dump.C"
    dump.write_text(f'void dump() {{ TFile f("{out}"); for (auto k : *f.GetListOfKeys()) '
                    '{ auto g = (TGraph*)((TKey*)k)->ReadObj(); '
                    'std::cout << "KEY " << k->GetName() << " | " << g->GetTitle() << std::endl; } }')
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_OUTPUT=str(tmp_path))
    subprocess.run(["root", "-l", "-b", "-q", "rootlogon.C", f'{macro}("{label}/","{out}")'],
                   cwd=ROOT, env=env, check=True, capture_output=True, timeout=300)
    proc = subprocess.run(["root", "-l", "-b", "-q", str(dump)], cwd=ROOT, env=env,
                          check=True, capture_output=True, text=True, timeout=300)
    keys = [line[4:] for line in proc.stdout.splitlines() if line.startswith("KEY ")]
    assert keys == [f"weighted_diffxsec_emin_{lo}_emax_{hi} | #bf{{E_{{#gamma}} (GeV): ({lo}, {hi})}}"
                    for lo, hi in sorted(EMINS, key=lambda e: float(e[0]))]
