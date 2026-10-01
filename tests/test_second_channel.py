"""A second channel drives `gxana run xsection|barlow|systematics` from its own config.

The channel is the test fixture tests/fixtures/channels/analyses/kpkpkmlamb/config: the
committed kpkpkmlamb config plus a synthetic MC sample, flux files and the physics,
xsection, barlow and systematics blocks (kpkpkmlamb has no MC, so it has no cross
section; nothing here runs a fit)."""
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import xsection as xs
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
