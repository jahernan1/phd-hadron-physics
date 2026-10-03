import copy

import pytest

from gxana import config as gconfig
from gxana_systematics import config


def _cfg():
    return copy.deepcopy(gconfig.load_channel("kpkpxim"))


def test_kpkpxim_config_is_valid():
    config.validate(_cfg())


def test_nominal_defaults_to_tex_label():
    cfg = _cfg()
    del cfg["systematics"]["nominal"]
    assert config.nominal(cfg) == cfg["xsection"]["tex"]["label"]


def test_pool_labels_order():
    assert config.pool_labels(config.block(_cfg())) == [
        "hybrid_combo", "best_combo", "acc_weight", "johnson", "johnson_cheby1", "voigt", "voigt_cheby1",
        "mcPdf", "mcPdf_cheby1", "qvalues"]


def test_unknown_spread_label_is_error():
    cfg = _cfg()
    cfg["systematics"]["studies"]["fit"]["spread"].append("nope")
    with pytest.raises(gconfig.ConfigError, match="fit.*'nope'"):
        config.validate(cfg)


def test_duplicate_pool_label_is_error():
    cfg = _cfg()
    cfg["systematics"]["variants"][1]["labels"].append({"label": "voigt", "cheby": 2})
    with pytest.raises(gconfig.ConfigError, match="duplicate.*'voigt'"):
        config.validate(cfg)


def test_qvalue_source_must_be_fit_label():
    cfg = _cfg()
    cfg["systematics"]["variants"][-1]["qvalue"]["source"] = "qvalues"
    with pytest.raises(gconfig.ConfigError, match="qvalue.*source"):
        config.validate(cfg)


def test_unknown_kind_and_keys_are_errors():
    cfg = _cfg()
    cfg["systematics"]["studies"]["luminosity"]["kind"] = "magic"
    with pytest.raises(gconfig.ConfigError, match="kind"):
        config.validate(cfg)
    cfg = _cfg()
    cfg["systematics"]["studies"]["luminosity"]["colour"] = 1
    with pytest.raises(gconfig.ConfigError, match="colour"):
        config.validate(cfg)


def test_summary_must_reference_measuring_studies():
    cfg = _cfg()
    cfg["systematics"]["summary"]["point_by_point"].append("bunch")
    with pytest.raises(gconfig.ConfigError, match="summary.*bunch"):
        config.validate(cfg)


def test_spread_needs_two_members():
    cfg = _cfg()
    cfg["systematics"]["studies"]["accidentals"]["spread"] = ["acc_weight"]
    with pytest.raises(gconfig.ConfigError, match="at least 2"):
        config.validate(cfg)


def test_compare_labels_outside_pool_are_allowed():
    # bunch/bkgd name labels not configured yet: they are skipped at run time, not config errors
    config.validate(_cfg())
    assert "oneRfBunch" in config.study_labels("bunch", config.block(_cfg())["studies"]["bunch"])


def test_studies_filter():
    names = [n for n, _ in config.studies(config.block(_cfg()), ["fit", "run"])]
    assert names == ["run", "fit"]
    with pytest.raises(gconfig.ConfigError, match="unknown study 'nope'"):
        config.studies(config.block(_cfg()), ["nope"])


def test_misspelled_track_override_is_error():
    cfg = _cfg()
    cfg["systematics"]["studies"]["track"]["override"] = {"protn": 0.05}
    with pytest.raises(gconfig.ConfigError, match="track.override.*'protn'"):
        config.validate(cfg)


def test_duplicate_spread_member_is_error():
    cfg = _cfg()
    cfg["systematics"]["studies"]["accidentals"]["spread"].append("best_combo")
    with pytest.raises(gconfig.ConfigError, match="accidentals.spread.*duplicate.*'best_combo'"):
        config.validate(cfg)
