"""Tests for gxana.config (channel YAML config loader)."""
import pytest

from gxana import config
from gxana.paths import MissingEnvError, analysis_data_root


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
    assert config.selector_name(cfg, "F1", thrown=True) == "DSelector_thrown_kpkpxim.C"  # F1 data has no thrown trees
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


from gxana.paths import MissingEnvError  # noqa: E402


def _cfg():
    return config.load_channel("kpkpxim")


def test_tree_stem_data_and_mc():
    cfg = _cfg()
    assert config.tree_stem(cfg, "2017-01", "data") == "kpkpxim__M23_2017-01_ana56"
    assert (config.tree_stem(cfg, "2018-08", "gen_amp_V2_ac_YstarRest")
            == "kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest")
    assert config.tree_stem(cfg, "2018-01", "F1") == "kpkpxim__B4_F1_M23_2018-01_ana16"
    assert config.tree_stem(cfg, "2017-01", "F1_ystar2400_genr8") == "kpkpxim__B4_F1_M23_2017-01_ana47_ystar2400_genr8"


def test_tree_dir_unchanged_by_stem_refactor():
    cfg = _cfg()
    assert (config.tree_dir(cfg, "2018-08", "gen_amp_V2_ac_YstarRest", "thrown")
            == "Trees/tree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest/thrown/")


def test_period_only_sample_rejects_other_periods():
    cfg = _cfg()
    assert config.tree_stem(cfg, "2018-08", "gen_amp_V2_nobkg") == "kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nobkg"
    with pytest.raises(config.ConfigError, match="no launch for period '2017-01'"):
        config.tree_stem(cfg, "2017-01", "gen_amp_V2_nobkg")


def test_f1_mc_sample_uses_generic_thrown_selector():
    cfg = _cfg()
    assert config.selector_name(cfg, "F1_ystar2400_genr8") == "DSelector_kpkpxim_F1.C"
    assert config.selector_name(cfg, "F1_ystar2400_genr8", thrown=True) == "DSelector_thrown_kpkpxim.C"
    assert "thrown_selector" not in cfg["samples"]["F1"]


def test_empty_string_override_is_an_error():
    cfg = _cfg()
    cfg["samples"]["data"]["selector"] = ""
    with pytest.raises(config.ConfigError, match="empty"):
        config.selector_name(cfg, "data")


def test_expand_env():
    env = {"GXANA_DATA": "/x/data"}
    assert config.expand_env("${GXANA_DATA}/flatTrees/a.root", env) == "/x/data/flatTrees/a.root"
    with pytest.raises(MissingEnvError, match="GXANA_OUTPUT"):
        config.expand_env("${GXANA_OUTPUT}/q", env)
    with pytest.raises(config.ConfigError, match="HOME"):
        config.expand_env("${HOME}/q", env)


def test_periods_have_flux_files():
    cfg = _cfg()
    assert [config.period_settings(cfg, p)["flux"] for p in cfg["periods"]] == [
        "flux_30274_31057_r4.root", "flux_40856_42559.root", "flux_50685_51768.root"]


def test_physics_block_of_kpkpxim():
    phys = config.physics(config.load_channel("kpkpxim"))
    assert phys["observable"] == {"branch": "decayxim_M", "title": "M(#Lambda#pi^{-}) (GeV/c^{2})"}
    assert phys["qvalue_branch"] == "qvalue_decayxim_M"
    assert phys["branching_ratio"] == {"value": 0.641, "error": 0.005}


def test_physics_block_is_checked():
    import copy

    base = config.load_channel("kpkpxim")
    cases = [
        (lambda p: p.pop("flat_tree"), "physics.flat_tree: need a non-empty string"),
        (lambda p: p.pop("qvalue_branch"), "qvalue_branch: required"),
        (lambda p: p["observable"].pop("title"), "observable.title"),
        (lambda p: p["branching_ratio"].update(value="0.641"), "branching_ratio.value: need a number"),
    ]
    for change, message in cases:
        cfg = copy.deepcopy(base)
        change(cfg["physics"])
        with pytest.raises(config.ConfigError, match=message):
            config.physics(cfg)
    cfg = copy.deepcopy(base)
    cfg["physics"]["qvalue_branch"] = None  # a channel without Q-factors
    assert config.physics(cfg)["qvalue_branch"] is None
    del cfg["physics"]
    with pytest.raises(config.ConfigError, match="missing required key 'physics'"):
        config.physics(cfg)


def test_expand_env_analysis_data_defaults_like_setup_sh():
    """Unset GXANA_ANALYSIS_DATA resolves as gxana.paths.analysis_data_root (the env/setup.sh default)."""
    got = config.expand_env("${GXANA_ANALYSIS_DATA}/kpkpxim/flux", {"GXANA_OUTPUT": "/o"})
    assert got == f"{analysis_data_root({})}/kpkpxim/flux"
    assert config.expand_env("${GXANA_ANALYSIS_DATA}/x", {"GXANA_ANALYSIS_DATA": "/a"}) == "/a/x"


def test_expand_env_other_unset_variables_still_fail():
    with pytest.raises(MissingEnvError):
        config.expand_env("${GXANA_DATA}/x", {})
