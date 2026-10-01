"""A second channel drives `gxana run xsection|barlow|systematics` from its own config.

The channel is the test fixture tests/fixtures/channels/analyses/kpkpkmlamb/config: the
committed kpkpkmlamb config plus a synthetic MC sample, flux files and the physics,
xsection, barlow and systematics blocks (kpkpkmlamb has no MC, so it has no cross
section; nothing here runs a fit)."""
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import xsection as xs
from gxana_barlow import stage as bst
from gxana_barlow.variations import expand
from gxana_systematics import stage as sst
from gxana_xsection import components

ROOT = Path(__file__).resolve().parent / "fixtures" / "channels"
ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}


@pytest.fixture(scope="module")
def cfg():
    return config.load_channel("kpkpkmlamb", root=ROOT)


def _module_args(argv):
    """The arguments after `python -m <module>`."""
    return list(argv[argv.index("-m") + 2:])


def test_components_output_names_keep_the_channel_anchor(cfg, tmp_path):
    """components.py cuts each output name at the anchor; without --anchor every
    kpkpkmlamb table collapsed to its last character and the files overwrote each other."""
    stem = config.tree_stem(cfg, "2017-01", "data")
    in_dir, out_dir = tmp_path / "in", tmp_path / "out"
    in_dir.mkdir()
    out_dir.mkdir()
    (in_dir / f"totout_flatTree_{stem}.txt").write_text("enBinCenter EnErr data_yield yield_err\n7.3 0.9 10 3\n")
    first = xs.plan_xsection(cfg, ["components"], environ=ENV)[0].argv
    args = _module_args(first)
    assert args[2:4] == ["--pattern", "totout*2017-01*.txt"]
    components.main([str(in_dir), str(out_dir)] + args[2:])
    assert sorted(p.name for p in out_dir.iterdir()) == [f"data_yield_{stem}.txt"]


def _tables(cfg):
    """Every gxana_xsec_tables command of the three stages."""
    cmds = xs.plan_xsection(cfg, ["tables"], environ=ENV)
    cmds += bst.plan(cfg, ["tables"], expand(cfg["barlow"]), environ=ENV)
    cmds += sst.plan(cfg, ["fit"], None, ENV)
    return [c.argv for c in cmds]


def test_tables_get_the_channel_physics(cfg):
    tail = ["--observable", "ximstar_M", "--observable-title", "M(#LambdaK^{-}) (GeV/c^{2})",
            "--gate", "(best_combo)*(ximstar_M>1.80&&ximstar_M<1.84)", "--qvalue-branch", "none",
            "--br", "0.641,0.005", "--target", "50.4,79.1,0.07008,2.01588,2",
            "--mass-window", "lo=1.7", "--mass-window", "mc_hi=1.95", "--mass-window", "mc_signal_hi=1.92",
            "--mass-window", "mc_plot_hi=1.96", "--mass-window", "data_hi=2.0", "--mass-window", "data_edge=1.71",
            "--mass-window", "mcpdf_data_lo=1.705"]
    argvs = _tables(cfg)
    assert len(argvs) == 1 + 3 + 2
    for argv in argvs:
        assert argv[len(argv) - len(tail):] == tail


def test_bin_gets_the_channel_trees_and_branches(cfg):
    branches = [a for b in cfg["xsection"]["branches"] for a in ("--branch", b)]
    cmds = [c.argv for c in xs.plan_xsection(cfg, ["bin"], environ=ENV)]
    assert [c[1] for c in cmds[:3]] == ["data", "mc", "thrown"]
    assert cmds[0][3].endswith(f"binned_flatTree_{config.tree_stem(cfg, '2017-01', 'data')}_nominal_allCuts.root")
    assert cmds[0][8:] == ["--tree", "flatTree_kpkpkmlamb"] + branches   # no Q-factors: no --data-branch
    assert cmds[1][8:] == ["--tree", "flatTree_kpkpkmlamb"] + branches
    assert cmds[2][8:] == ["--tree", "flatTree_thrown_kpkpkmlamb"]


def test_barlow_check_gets_the_channel_observable_and_windows(cfg):
    tail = ["--observable", "ximstar_M", "--observable-title", "M(#LambdaK^{-}) (GeV/c^{2})",
            "--mass-window", "lo=1.7", "--mass-window", "mc_hi=1.95", "--mass-window", "mc_signal_hi=1.92",
            "--mass-window", "mc_plot_hi=1.96", "--mass-window", "data_lo=1.69", "--mass-window", "data_hi=2.0",
            "--mass-window", "scan_start=1.8"]
    cmds = [c.argv for c in bst.plan(cfg, ["check"], expand(cfg["barlow"]), environ=ENV)]
    assert len(cmds) == 3
    for argv in cmds:
        assert argv[len(argv) - len(tail):] == tail


def test_barlow_plot_gets_the_channel_title_and_ranges(cfg):
    tail = ["--reaction-title", "#gamma p#rightarrow K^{+}K^{+}K^{-}#Lambda", "--t-limits", "0,2.5",
            "--energy-limits", "6.2,11.6", "--graph-limits", "6,12"]
    cmds = [c.argv for c in bst.plan(cfg, ["plot"], expand(cfg["barlow"]), environ=ENV)]
    assert len(cmds) == 1
    assert cmds[0][len(cmds[0]) - len(tail):] == tail
