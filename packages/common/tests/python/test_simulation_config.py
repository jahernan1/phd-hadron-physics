"""config/mc.yaml, the MCwrapper confs and hd_root configs agree with periods/samples."""
import re
from typing import cast

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
    flags = cast(re.Match, re.search(r"^Reaction1:Flags\s+(\S+)", hd_root, re.M)).group(1)
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


def test_sampling_inputs_are_in_the_manifest():
    import yaml
    manifest = yaml.safe_load((repo_root() / "analyses/kpkpxim/analysis_data.yaml").read_text())
    prefix = "${GXANA_ANALYSIS_DATA}/kpkpxim/"
    for cfg in (SIM / "gen_amp_cfg").glob("*.cfg"):
        for ref in re.findall(r"(\S+\.root)", cfg.read_text()):
            assert ref[len(prefix):] in manifest["files"], f"{cfg.name}: {ref}"


@pytest.mark.parametrize("variant", ["ac", "noac"])
def test_generator_options_are_line_one(variant):
    # MCwrapper's MakeMC.sh reads the gen_amp options with `head -n 1 <cfg> | sed -r 's/.//'`
    path = SIM / "gen_amp_cfg" / f"kpkpxim_2dhist_{variant}_YstarRest.cfg"
    first = path.read_text().splitlines()[0]
    assert first == "# -t 1.45 1 -mask 1 1 0"
    assert first[1:] == " -t 1.45 1 -mask 1 1 0"   # what the sed strips to


@pytest.mark.parametrize("variant", ["ac", "noac"])
def test_rendered_generator_cfg_keeps_options_on_line_one(variant):
    text = (SIM / "gen_amp_cfg" / f"kpkpxim_2dhist_{variant}_YstarRest.cfg").read_text()
    first = config.expand_env(text, {"GXANA_ANALYSIS_DATA": "/x"}).splitlines()[0]
    assert first[0] == "#" and "-t 1.45" in first and "-mask 1 1 0" in first
