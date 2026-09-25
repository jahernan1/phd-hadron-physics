"""config/mc.yaml, the MCwrapper confs and hd_root configs agree with periods/samples (spec §8)."""
import re

import pytest

from gxana import config
from gxana.paths import repo_root

CFG = config.load_channel("kpkpxim")
MC = CFG["mc"]
SIM = repo_root() / MC["simulation_dir"]


def conf_value(text, key):
    m = re.search(rf"^\s*{key}\s*=\s*([^#\n]*?)\s*(?:#.*)?$", text, re.M)
    assert m, key
    return m.group(1)


@pytest.mark.parametrize("period", sorted(MC["periods"]))
def test_period_conf(period):
    p = config.period_settings(CFG, period)
    mp = MC["periods"][period]
    text = (SIM / "mcwrapper" / mp["conf"]).read_text()
    assert (repo_root() / "env/version_sets" / f"{mp['sim_version_set']}.xml.in").is_file()
    assert conf_value(text, "ENVIRONMENT_FILE") == "${GXANA_EXTERNALS}/version_sets/" + mp["sim_version_set"] + ".xml"
    assert conf_value(text, "GENERATOR") == "gen_amp_V2"
    plugins = conf_value(text, "CUSTOM_PLUGINS")
    assert plugins.startswith("file:${GXANA_ROOT}/analyses/kpkpxim/simulation/hd_root/")
    hd_root = (SIM / "hd_root" / plugins.rsplit("/", 1)[1]).read_text()
    flags = re.search(r"^Reaction1:Flags\s+(\S+)", hd_root, re.M).group(1)
    assert flags + "_" == p["fit_prefix"]   # MC tree name must match what run select expects
    assert isinstance(mp["events"], int) and mp["events"] > 0


@pytest.mark.parametrize("sample", sorted(MC["samples"]))
def test_sample(sample):
    s = config.sample_settings(CFG, sample)
    ms = MC["samples"][sample]
    assert s.get("mc") is True
    assert (SIM / "gen_amp_cfg" / ms["generator_config"]).is_file()
    for period in ms.get("periods", MC["periods"]):
        assert period in MC["periods"]
        config.tree_stem(CFG, period, sample)   # launch resolvable for every produced period


def test_cfgs_read_sampling_from_preserved_data():
    for cfg in (SIM / "gen_amp_cfg").glob("*.cfg"):
        roots = re.findall(r"(\S+\.root)", cfg.read_text())
        assert roots, cfg.name
        assert all(r.startswith("${GXANA_ANALYSIS_DATA}/kpkpxim/simulation/sampling/") for r in roots), cfg.name
