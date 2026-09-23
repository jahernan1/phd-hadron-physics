"""Tests for gxana.config (channel YAML config loader)."""
import pytest

from gxana import config


@pytest.fixture(scope="module")
def cfg():
    return config.load_channel("kpkpxim")


def test_merges_all_files(cfg):
    assert cfg["reaction"] == "kpkpxim"
    assert set(cfg["periods"]) == {"2017-01", "2018-01", "2018-08"}
    assert len(cfg["energy_edges"]) == 9
    assert len(cfg["t_bins"]) == 7


# Expected values are tree paths used in legacy runMultiDSelector.sh / runDSelector.sh.
@pytest.mark.parametrize(
    "period,sample,kind,expected",
    [
        ("2017-01", "data", "trees", "Trees/tree_kpkpxim__M23_2017-01_ana56/trees/"),
        ("2018-01", "data", "trees", "Trees/tree_kpkpxim__B4_M23_2018-01_ana03/trees/"),
        ("2017-01", "gen_amp_V2_ac_YstarRest", "thrown",
         "Trees/tree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest/thrown/"),
        ("2018-01", "gen_amp_V2_ac_YstarRest", "thrown",
         "Trees/tree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest/thrown/"),
        ("2017-01", "F1", "trees", "Trees/tree_kpkpxim__B4_F1_M23_2017-01_ana47/trees/"),
        ("2018-01", "F1", "trees", "Trees/tree_kpkpxim__B4_F1_M23_2018-01_ana16/trees/"),
    ],
)
def test_tree_dir_matches_legacy(cfg, period, sample, kind, expected):
    assert config.tree_dir(cfg, period, sample, kind) == expected


# Controller amendment: F1 2018-08 launch = ana15, per the legacy line in
# _workdir/runMultiDSelector.sh (read-only):
#   ./runDSelector.sh -t Trees/tree_kpkpxim__B4_F1_M23_2018-08_ana15/trees/ \
#       -d DSelector/DSelector_kpkpxim_F1.C -S kpkpxim_F1
def test_f1_2018_08_tree_dir_matches_legacy(cfg):
    assert config.tree_dir(cfg, "2018-08", "F1") == "Trees/tree_kpkpxim__B4_F1_M23_2018-08_ana15/trees/"


def test_thrown_requires_mc_or_thrown_selector(cfg):
    with pytest.raises(config.ConfigError, match="has no thrown trees"):
        config.tree_dir(cfg, "2018-08", "data", "thrown")


# Controller amendment: thrown trees resolve only for MC samples (samples.yaml
# 'mc' field). F1 is not MC, even though it defines its own thrown_selector,
# so requesting thrown for it must still raise ConfigError.
def test_thrown_requires_mc_sample(cfg):
    with pytest.raises(config.ConfigError, match="has no thrown trees"):
        config.tree_dir(cfg, "2018-01", "F1", "thrown")


def test_unknown_period_and_sample(cfg):
    with pytest.raises(config.ConfigError, match="unknown period"):
        config.tree_dir(cfg, "2019-11", "data")
    with pytest.raises(config.ConfigError, match="unknown sample"):
        config.tree_dir(cfg, "2018-08", "nope")


def test_selector_and_output_resolution(cfg):
    assert config.selector_name(cfg, "data") == "DSelector_kpkpxim.C"
    assert config.selector_name(cfg, "gen_amp_V2_ac_YstarRest", thrown=True) == "DSelector_thrown_kpkpxim.C"
    assert config.selector_name(cfg, "F1") == "DSelector_kpkpxim_F1.C"
    assert config.selector_name(cfg, "F1", thrown=True) == "DSelector_thrown_kpkpxim_F1.C"
    assert config.output_basename(cfg, "data") == "kpkpxim.root"
    assert config.output_basename(cfg, "F1") == "kpkpxim_F1.root"


def test_duplicate_key_across_files(tmp_path):
    cfg_dir = tmp_path / "analyses" / "x" / "config"
    cfg_dir.mkdir(parents=True)
    (cfg_dir / "a.yaml").write_text("channel: x\n")
    (cfg_dir / "b.yaml").write_text("channel: y\n")
    with pytest.raises(config.ConfigError, match="'channel' already defined"):
        config.load_channel("x", root=tmp_path)


def test_missing_channel(tmp_path):
    with pytest.raises(config.ConfigError, match="no config files"):
        config.load_channel("absent", root=tmp_path)


def test_missing_top_level_key_names_key_and_channel(tmp_path):
    cfg_dir = tmp_path / "analyses" / "x" / "config"
    cfg_dir.mkdir(parents=True)
    (cfg_dir / "channel.yaml").write_text(
        "channel: x\nperiods: {p: {launch: 1, fit_prefix: f}}\nsamples: {s: {}}\nreaction: x\n"
    )
    cfg = config.load_channel("x", root=tmp_path)
    with pytest.raises(config.ConfigError, match="channel 'x' config missing required key 'tree_dir_template'"):
        config.tree_dir(cfg, "p", "s")
