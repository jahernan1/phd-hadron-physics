import copy

import pytest

from gxana import config
from gxana_barlow import config as bconfig
from gxana_barlow.variations import Variation, expand

# Legacy GetVariationTreesUML.C:26-35, verbatim.
LEGACY_NOMINAL = ("chisqndf<8&total_mm2_abs<0.02&&xim_pathlensig>2&&lambda_pathlensig>0"
                  "&&kphigh_prap>2&&t_dist<2.4")
LEGACY_CUTS = ["chisqndf<6", "chisqndf<7", "chisqndf<9", "chisqndf<10",
               "total_mm2_abs<0.01", "total_mm2_abs<0.015", "total_mm2_abs<0.025", "total_mm2_abs<0.03",
               "xim_pathlensig>1", "xim_pathlensig>1.5", "xim_pathlensig>2.5", "xim_pathlensig>3",
               "lambda_pathlensig>0.5", "lambda_pathlensig>1",
               "kphigh_prap>1.6", "kphigh_prap>1.8", "kphigh_prap>2.1", "kphigh_prap>2.2"]
LEGACY_OLD = {"chisqndf": "chisqndf<8", "total_mm2": "total_mm2_abs<0.02", "xim_pathlensig": "xim_pathlensig>2",
              "lambda_pathlensig": "lambda_pathlensig>0", "kphigh_prap": "kphigh_prap>2"}


def legacy_replace(text, old, new):
    """Python transcription of GetVariationTreesUML.C replaceValueInDelimitedString:
    getline on '&', skip a second '&', replace the first exact match, rejoin with '&&'."""
    parts, i = [], 0
    while i <= len(text):
        j = text.find("&", i)
        if j < 0:
            parts.append(text[i:])
            break
        parts.append(text[i:j])
        i = j + 1
        if i < len(text) and text[i] == "&":
            i += 1
    for k, part in enumerate(parts):
        if part == old:
            parts[k] = new
            break
    return "&&".join(parts)


def legacy_variations():
    out = []
    for cut in LEGACY_CUTS:
        old = next(v for k, v in LEGACY_OLD.items() if k in cut)
        op = ">" if ">" in cut else "<"
        name, value = cut.split(op)
        out.append((f"{name}_{value}", legacy_replace(LEGACY_NOMINAL, old, cut)))
    return out


def _bcfg():
    return copy.deepcopy(bconfig.block(config.load_channel("kpkpxim")))


def test_kpkpxim_expands_to_the_legacy_cut_strings():
    got = [(v.id, v.cut) for v in expand(_bcfg())]
    assert len(got) == 18
    assert got == legacy_variations()


def test_variation_fields():
    first = expand(_bcfg())[0]
    assert first == Variation(
        id="chisqndf_6", family="chisqndf", value="6",
        cut="chisqndf<6&&total_mm2_abs<0.02&&xim_pathlensig>2&&lambda_pathlensig>0&&kphigh_prap>2&&t_dist<2.4",
        tree="vary_chisqndf_6", label="#chi^{2}_{#nu} < 6")
    assert first.to_dict()["tree"] == "vary_chisqndf_6"
    ids = [v.id for v in expand(_bcfg())]
    assert "kphigh_prap_2.1" in ids and "total_mm2_abs_0.015" in ids


def test_kpkpxim_config_validates():
    bconfig.validate(_bcfg(), ["trees", "check", "bin", "tables", "weight", "plot"])


def _broken(mutate):
    bcfg = _bcfg()
    mutate(bcfg)
    return bcfg


@pytest.mark.parametrize("mutate, message", [
    (lambda b: b["families"].update(foo={**b["families"]["chisqndf"]}), "no barlow.nominal entry for family 'foo'"),
    (lambda b: b["nominal"].pop("chisqndf"), "no barlow.nominal entry for family 'chisqndf'"),
    (lambda b: b["families"]["chisqndf"].update(values=[]), "values: empty"),
    (lambda b: b["families"]["chisqndf"].update(values=["6", "6"]), "duplicate variation id 'chisqndf_6'"),
    (lambda b: b["families"]["chisqndf"].update(values=[6]), "must be a quoted string"),
    (lambda b: b["families"]["chisqndf"].update(values=[0.10]), "must be a quoted string"),
    (lambda b: b["families"]["chisqndf"].update(op="=="), "op: '==' is not one of"),
    (lambda b: b["families"]["chisqndf"]["style"].pop("legend_tot"), "style: missing 'legend_tot'"),
    (lambda b: b["families"]["chisqndf"]["style"].update(legend_diff=[0.1, 0.2, 1.3, 0.9]), "legend_diff: need 4 numbers in [0, 1]"),
    (lambda b: b["families"]["chisqndf"]["style"].update(legend_diff=[0.1, 0.2, 0.3]), "legend_diff: need 4 numbers in [0, 1]"),
    (lambda b: b["families"]["chisqndf"]["style"].update(canvas="big"), "canvas: 'default' or [width, height]"),
    (lambda b: b["families"]["chisqndf"]["style"].update(title_offsets_tot=[1.1]), "title_offsets_tot: need 2 numbers"),
    (lambda b: b["families"]["chisqndf"].update(label=""), "label: not set"),
    (lambda b: b.pop("label"), "barlow: missing 'label'"),
    (lambda b: b.pop("mc_sample"), "barlow: missing 'mc_sample'"),
    (lambda b: b["trees"].pop("tree"), "barlow.trees: missing 'tree'"),
    (lambda b: b["trees"].pop("input"), "barlow.trees: missing 'input'"),
    (lambda b: b["trees"].pop("input_mc"), "barlow.trees: missing 'input_mc'"),
    (lambda b: b["trees"].update(branches=[]), "barlow.trees: missing 'branches'"),
    (lambda b: b["trees"].update(output="variations.root"), "barlow.trees.output: needs {stem} and {family}"),
    (lambda b: b["trees"].update(output="{stem}_{mc_sample}.root"), "barlow.trees.output: needs {stem} and {family}"),
    (lambda b: b["trees"].update(output="{family}_{mc_sample}.root"), "barlow.trees.output: needs {stem} and {family}"),
    (lambda b: b["families"].update(chisqndf=["<"]), "barlow.families.chisqndf: must be a mapping"),
    (lambda b: b["families"]["chisqndf"].update(style="big"), "barlow.families.chisqndf.style: must be a mapping"),
])
def test_validation_errors(mutate, message):
    with pytest.raises(config.ConfigError, match=_escape(message)):
        bconfig.validate(_broken(mutate))


def _escape(text):
    import re
    return re.escape(text)


def test_nominal_term_without_family_is_allowed():
    bcfg = _bcfg()
    bcfg["nominal"]["extra"] = "extra>0"
    bconfig.validate(bcfg)
    assert expand(bcfg)[0].cut.endswith("&&extra>0&&t_dist<2.4")


def test_check_step_needs_the_nominal_trees():
    bcfg = _bcfg()
    bcfg.pop("check")
    bconfig.validate(bcfg, ["trees", "plot"])
    with pytest.raises(config.ConfigError, match=r"barlow\.check\.nominal: required by the check step"):
        bconfig.validate(bcfg, ["check"])
    bcfg["check"] = {"nominal": "a.root"}
    with pytest.raises(config.ConfigError, match=r"barlow\.check\.nominal_mc"):
        bconfig.validate(bcfg, ["check"])


def test_config_hash_is_stable_and_sensitive():
    a, b = _bcfg(), _bcfg()
    assert bconfig.config_hash(a) == bconfig.config_hash(b)
    assert len(bconfig.config_hash(a)) == 64
    b["families"]["chisqndf"]["values"][0] = "5"
    assert bconfig.config_hash(a) != bconfig.config_hash(b)


@pytest.mark.parametrize("mutate", [
    lambda b: b["families"]["chisqndf"]["style"].update(y_floor=99),
    lambda b: b["families"]["chisqndf"].update(label="other"),
    lambda b: b.update(threshold=9.0),
    lambda b: b["fit"].update(cheby=3),
    lambda b: b.update(weight="other"),
    lambda b: b.update(label="other"),
    lambda b: b.update(check={"nominal": "x.root", "nominal_mc": "y.root"}),
])
def test_config_hash_ignores_what_does_not_shape_the_trees(mutate):
    a, b = _bcfg(), _bcfg()
    mutate(b)
    assert bconfig.config_hash(a) == bconfig.config_hash(b)


@pytest.mark.parametrize("mutate", [
    lambda b: b["families"]["chisqndf"]["values"].append("11"),
    lambda b: b["families"]["chisqndf"].update(op="<="),
    lambda b: b["trees"]["defines"].update(extra="a+b"),
    lambda b: b["trees"]["filters"]["data"].append("x>1"),
    lambda b: b["trees"]["branches"].append("newbranch"),
    lambda b: b["nominal"].update(chisqndf="chisqndf<7"),
    lambda b: b["fixed"].append("extra>0"),
    lambda b: b.update(mc_sample="other_mc"),
])
def test_config_hash_tracks_what_shapes_the_trees(mutate):
    a, b = _bcfg(), _bcfg()
    mutate(b)
    assert bconfig.config_hash(a) != bconfig.config_hash(b)
