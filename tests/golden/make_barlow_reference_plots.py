"""Make reference/barlow/plots once from the archived PlotXSecBarlow*.C macros.

Stages the preserved nominal (reference/xsection/weighted/johnson) and variation
(reference/systematics/weighted_data) weighted tables where the macros look for
them ($GXANA_OUTPUT/kpkpxim/{xsection,systematics}/weighted_data/johnson), runs
each configured family's macro with the repo rootlogon.C, and copies the PDFs to
$GXANA_ANALYSIS_DATA/kpkpxim/reference/barlow/plots. Then record them with
`gxana data lock --channel kpkpxim`. Usage: uv run python tests/golden/make_barlow_reference_plots.py
"""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from gxana.paths import analysis_data_root, repo_root

MACROS = {  # configured families only (kplow_prap has no variation trees)
    "chisqndf": "PlotXSecBarlowChiSqNdf",
    "total_mm2_abs": "PlotXSecBarlowMissingMass",
    "xim_pathlensig": "PlotXSecBarlowXimFlightSig",
    "lambda_pathlensig": "PlotXSecBarlowLambdaFlightSig",
    "kphigh_prap": "PlotXSecBarlowKHighRapidity",
}


def main() -> int:
    base = analysis_data_root() / "kpkpxim"
    root = repo_root()
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp)
        shutil.copytree(base / "reference/xsection/weighted/johnson", out / "kpkpxim/xsection/weighted_data/johnson")
        shutil.copytree(base / "reference/systematics/weighted_data", out / "kpkpxim/systematics/weighted_data/johnson")
        env = dict(os.environ, GXANA_ROOT=str(root), GXANA_OUTPUT=str(out))
        for macro in MACROS.values():
            subprocess.run(["root", "-l", "-b", "-q", "rootlogon.C",
                            f'archive/systematics_legacy/{macro}.C("johnson")'], cwd=root, env=env, check=True)
        pdfs = sorted((out / "kpkpxim/systematics/plots").glob("barlow_*.pdf"))
        dest = base / "reference/barlow/plots"
        dest.mkdir(parents=True, exist_ok=True)
        for pdf in pdfs:
            shutil.copy2(pdf, dest / pdf.name)
    print(f"wrote {len(pdfs)} PDFs to {dest}")
    return 0 if len(pdfs) == 45 else 1


if __name__ == "__main__":
    sys.exit(main())
