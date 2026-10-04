"""Golden: gxana_xsec_bin reproduces the legacy binned trees from the preserved flat trees."""
import subprocess
from pathlib import Path
from typing import Dict, FrozenSet, NamedTuple, Optional, Tuple, cast

import pytest
from golden_data import PERIOD_TREES

from gxana.config import load_channel
from gxana.stages.xsection import bin_physics_args

pytestmark = pytest.mark.golden

MACRO = Path(__file__).with_name("tree_summary.C")
CASES = [
    ("data", "flat_trees/postQVal_flatTree_{t}_nominal_kphighrap_1111111.root",
     "binned_trees/binned_flatTree_{t}_nominal_kphighrap.root"),
    # The staged binned MC/thrown trees are the thesis inputs (they reproduce
    # the qval/mc/thrown yields of every row of reference/xsection/johnson).
    # The staged flat MC/thrown trees are a later, larger production that
    # contains every event of them: +1.6/+1.7/+2.9 % reconstructed and
    # +1.6/+1.7/+2.8 % thrown entries in 6.40-11.40 GeV (2017-01/2018-01/
    # 2018-08). Binning them therefore cannot equal the staged binned trees;
    # a plain TTree selection on the flat trees gives the port's counts
    # (docs/KNOWN_ISSUES.md, MC sample provenance).
    pytest.param("mc", "flat_trees/flatTree_{t}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                 "binned_trees/binned_flatTree_{t}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                 marks=pytest.mark.xfail(
                     strict=True,
                     reason="staged flat MC trees are a later superset (+1.6-2.9 % entries) of the binned thesis inputs")),
    pytest.param("thrown", "flat_trees/flatTree_thrown_{t}_gen_amp_V2_ac_YstarRest.root",
                 "binned_trees/binned_thrown_flatTree_{t}_gen_amp_V2_ac_YstarRest.root",
                 marks=pytest.mark.xfail(
                     strict=True,
                     reason="staged flat thrown trees are a later superset (+1.6-2.8 % entries) of the binned thesis inputs")),
]


def edges():
    cfg = load_channel("kpkpxim")
    t_edges = [pair[0] for pair in cfg["t_bins"]] + [cfg["t_bins"][-1][1]]
    return ",".join(str(e) for e in cfg["energy_edges"]), ",".join(str(e) for e in t_edges)


class TreeSummary(NamedTuple):
    entries: int
    sum_e: float
    sum_t: float
    branches: FrozenSet[str]
    fit_sums: Tuple[Optional[float], Optional[float], Optional[float]]  # decayxim_M, hybrid_combo, qvalue_decayxim_M


def summary(root_exe: str, path: Path) -> Dict[str, TreeSummary]:
    out = subprocess.run([root_exe, "-l", "-b", "-q", f'{MACRO}("{path}")'],
                         capture_output=True, text=True, check=True)
    entries: Dict[str, Tuple[int, float, float]] = {}
    branches: Dict[str, FrozenSet[str]] = {}
    fit_sums: Dict[str, Tuple[Optional[float], Optional[float], Optional[float]]] = {}
    for line in out.stdout.splitlines():
        if line.startswith("TREE "):
            _, name, count, sum_e, sum_t = line.split()
            entries[name] = (int(count), float(sum_e), float(sum_t))
        elif line.startswith("BRANCHES "):
            _, name, names = line.split(" ", 2)
            branches[name] = frozenset(names.split(",")) if names else frozenset()
        elif line.startswith("SUMS "):
            parts = line.split()
            name = parts[1]
            fit_sums[name] = cast(Tuple[Optional[float], Optional[float], Optional[float]],
                                tuple(None if v == "NA" else float(v) for v in parts[2:]))  # SUMS lines carry 3 values
    assert entries, f"no trees in {path}: {out.stdout}{out.stderr}"
    return {name: TreeSummary(count, sum_e, sum_t, branches[name], fit_sums[name])
            for name, (count, sum_e, sum_t) in entries.items()}


def run_bin(build_bin, mode, src, out):
    """gxana_xsec_bin as `gxana run xsection --steps bin` calls it (variation: as barlow's bin step)."""
    energy, t = edges()
    channel = [] if mode == "variation" else bin_physics_args(load_channel("kpkpxim"), mode)
    subprocess.run([str(build_bin / "gxana_xsec_bin"), mode, str(src), str(out), "--energy", energy, "--t", t,
                    *channel], check=True)


@pytest.mark.parametrize("tree", PERIOD_TREES)
@pytest.mark.parametrize("mode,flat,binned", CASES)
def test_binning_matches_legacy(need, build_bin, root_exe, tmp_path, tree, mode, flat, binned):
    src, ref = need(flat.format(t=tree), binned.format(t=tree))
    out = tmp_path / "binned.root"
    run_bin(build_bin, mode, src, out)
    new, old = summary(root_exe, out), summary(root_exe, ref)
    assert sorted(new) == sorted(old)
    for name, ref_tree in old.items():
        new_tree = new[name]
        assert new_tree.entries == ref_tree.entries, name
        assert (new_tree.sum_e, new_tree.sum_t) == pytest.approx((ref_tree.sum_e, ref_tree.sum_t), rel=1e-12), name
        # xsection.branches (analyses/kpkpxim/config/xsection.yaml) is the legacy
        # branch list without its repeats: total_mm2/chisqndf are listed twice
        # legacy side. A legacy Snapshot with a repeated column name can genuinely
        # produce two TBranch objects sharing that name (confirmed on ROOT 6.40:
        # a raw TTree::Branch() call does not reject a repeated name, unlike
        # RDataFrame::Snapshot's own duplicate-column check). tree_summary.C
        # already collects branch names into a std::set, so both sides here are
        # the deduped sets; compare those rather than raw (possibly duplicated)
        # branch lists.
        assert new_tree.branches == ref_tree.branches, name
        for new_sum, ref_sum in zip(new_tree.fit_sums, ref_tree.fit_sums):
            if ref_sum is None:
                assert new_sum is None, name
            else:
                assert new_sum == pytest.approx(ref_sum, rel=1e-12), name


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
        new_tree, ref_tree = new["flatTree_kpkpxim/" + name], old[name]
        assert new_tree.entries == ref_tree.entries, name
        assert (new_tree.sum_e, new_tree.sum_t) == pytest.approx((ref_tree.sum_e, ref_tree.sum_t), rel=1e-12), name
