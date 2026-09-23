"""Golden: gxana_xsec_bin reproduces the legacy binned trees from the preserved flat trees."""
import subprocess
from pathlib import Path
from typing import Dict, Tuple

import pytest
from golden_data import PERIOD_TREES

from gxana.config import load_channel

pytestmark = pytest.mark.golden

MACRO = Path(__file__).with_name("tree_summary.C")
CASES = [
    ("data", "flat_trees/postQVal_flatTree_{t}_nominal_kphighrap_1111111.root",
     "binned_trees/binned_flatTree_{t}_nominal_kphighrap.root"),
    ("mc", "flat_trees/flatTree_{t}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
     "binned_trees/binned_flatTree_{t}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root"),
    ("thrown", "flat_trees/flatTree_thrown_{t}_gen_amp_V2_ac_YstarRest.root",
     "binned_trees/binned_thrown_flatTree_{t}_gen_amp_V2_ac_YstarRest.root"),
]


def edges():
    cfg = load_channel("kpkpxim")
    t_edges = [pair[0] for pair in cfg["t_bins"]] + [cfg["t_bins"][-1][1]]
    return ",".join(str(e) for e in cfg["energy_edges"]), ",".join(str(e) for e in t_edges)


def summary(root_exe: str, path: Path) -> Dict[str, Tuple[int, float, float]]:
    out = subprocess.run([root_exe, "-l", "-b", "-q", f'{MACRO}("{path}")'],
                         capture_output=True, text=True, check=True)
    rows = {}
    for line in out.stdout.splitlines():
        if line.startswith("TREE "):
            _, name, entries, sum_e, sum_t = line.split()
            rows[name] = (int(entries), float(sum_e), float(sum_t))
    assert rows, f"no trees in {path}: {out.stdout}{out.stderr}"
    return rows


def run_bin(build_bin, mode, src, out):
    energy, t = edges()
    subprocess.run([str(build_bin / "gxana_xsec_bin"), mode, str(src), str(out), "--energy", energy, "--t", t],
                   check=True)


@pytest.mark.parametrize("tree", PERIOD_TREES)
@pytest.mark.parametrize("mode,flat,binned", CASES)
def test_binning_matches_legacy(need, build_bin, root_exe, tmp_path, tree, mode, flat, binned):
    src, ref = need(flat.format(t=tree), binned.format(t=tree))
    out = tmp_path / "binned.root"
    run_bin(build_bin, mode, src, out)
    new, old = summary(root_exe, out), summary(root_exe, ref)
    assert sorted(new) == sorted(old)
    for name, (entries, sum_e, sum_t) in old.items():
        assert new[name][0] == entries, name
        assert new[name][1:] == pytest.approx((sum_e, sum_t), rel=1e-12), name


@pytest.mark.parametrize("tree", PERIOD_TREES)
def test_variation_binning_matches_nominal_t_bins(need, build_bin, root_exe, tmp_path, tree):
    # SplitVariationTrees skips the t_dist < 2.4 energy-bin cut, so only the
    # (E, -t) bins must agree with the nominal binning.
    src, ref = need(CASES[0][1].format(t=tree), CASES[0][2].format(t=tree))
    out = tmp_path / "variation.root"
    run_bin(build_bin, "variation", src, out)
    new, old = summary(root_exe, out), summary(root_exe, ref)
    t_bins = [name for name in old if "tmin" in name]
    assert len(t_bins) == 56
    for name in t_bins:
        entries, sum_e, sum_t = new["flatTree_kpkpxim/" + name]
        assert entries == old[name][0], name
        assert (sum_e, sum_t) == pytest.approx(old[name][1:], rel=1e-12), name
