"""Golden: the systematics stage's weight step reproduces the legacy weighted tables.

Runs the commands `gxana run systematics --steps weight` plans (one
gxana_xsection.weighted_average call per variation and energy bin, discovered
from the totxsec_*_vary_*.txt files) on a copy of the staged legacy
xsection_data and compares the output to the staged legacy weighted_data
(produced by the retired GetWeightedXsecFile.py) with
gxana_xsection.compare.compare_dirs. The nominal-cut files also in the
reference weighted_data are not produced by the stage (those come from
`gxana run xsection`) and are ignored.
"""
import os
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import systematics as sy
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

REF = "reference/systematics"

# 18 variation cuts x (1 totxsec + 8 diffxsec energy-bin patterns) = 162 weighted_data files.
N_VARIATIONS = 18
N_ENERGY_BINS = 8


def test_weight_step_matches_legacy_systematics(need, tmp_path):
    src, ref = need(f"{REF}/xsection_data", f"{REF}/weighted_data")

    in_dir = tmp_path / "xsection_data" / "johnson"
    out_dir = tmp_path / "weighted_data" / "johnson"
    in_dir.mkdir(parents=True)
    out_dir.mkdir(parents=True)
    for f in src.glob("*.txt"):
        (in_dir / f.name).write_bytes(f.read_bytes())

    suffixes = sy.discover_variations(str(in_dir))
    assert len(suffixes) == N_VARIATIONS
    edges = config.load_channel("kpkpxim")["energy_edges"]
    commands = sy.weight_commands(str(in_dir), str(out_dir), suffixes, edges)
    assert len(commands) == N_VARIATIONS * (1 + N_ENERGY_BINS)
    for cmd in commands:
        proc = subprocess.run(cmd.argv, capture_output=True, text=True)
        assert proc.returncode == 0, f"{cmd.argv}\n{proc.stdout}\n{proc.stderr}"

    # The package's tables carry a different header and one extra column (S, the
    # scale factor); the retired legacy script wrote "# X Y_weighted EX EY_weighted".
    # Compare the four legacy columns under the legacy header.
    legacy = tmp_path / "legacy_columns"
    legacy.mkdir()
    for f in out_dir.glob("*.txt"):
        rows = [line.split()[:4] for line in f.read_text().splitlines()[1:] if line.strip()]
        # An empty bin (all inputs zero-error, so nan average): the legacy script
        # printed error 0, the package prints inf (1/sqrt(0)).
        rows = [r[:3] + ["0.000000"] if r[1] == "nan" and r[3] == "inf" else r for r in rows]
        (legacy / f.name).write_text("# X Y_weighted EX EY_weighted\n" + "".join(" ".join(r) + "\n" for r in rows))
    assert all(len(line.split()) == 5 for f in out_dir.glob("*.txt") for line in f.read_text().splitlines()[1:2])

    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    report = compare_dirs(legacy, ref, rtol=rtol, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == N_VARIATIONS * (1 + N_ENERGY_BINS)
