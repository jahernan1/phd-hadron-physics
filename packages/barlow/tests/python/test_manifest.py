import copy
import json

import pytest

from gxana import config
from gxana_barlow import config as bconfig
from gxana_barlow import manifest


def _bcfg():
    return copy.deepcopy(bconfig.block(config.load_channel("kpkpxim")))


def test_round_trip(tmp_path):
    bcfg = _bcfg()
    path = manifest.write(tmp_path / "out", manifest.build(bcfg))
    assert path == tmp_path / "out" / "variations.json"
    data = json.loads(path.read_text())
    assert data["config_hash"] == bconfig.config_hash(bcfg)
    assert data["variations"][0] == {
        "id": "chisqndf_6", "family": "chisqndf", "value": "6",
        "cut": "chisqndf<6&&total_mm2_abs<0.02&&xim_pathlensig>2&&lambda_pathlensig>0&&kphigh_prap>2&&t_dist<2.4",
        "tree": "vary_chisqndf_6", "label": "#chi^{2}_{#nu} < 6"}
    variations = manifest.load_checked(tmp_path / "out", bcfg)
    assert [v.id for v in variations][:2] == ["chisqndf_6", "chisqndf_7"] and len(variations) == 18


def test_changed_config_is_stale(tmp_path):
    bcfg = _bcfg()
    manifest.write(tmp_path, manifest.build(bcfg))
    bcfg["families"]["chisqndf"]["values"][0] = "5"
    with pytest.raises(manifest.ManifestError, match=r"config changed since trees; rerun --steps trees"):
        manifest.load_checked(tmp_path, bcfg)


def test_missing_manifest(tmp_path):
    with pytest.raises(manifest.ManifestError, match=r"no variations\.json in .*; run --steps trees first"):
        manifest.read(tmp_path)
