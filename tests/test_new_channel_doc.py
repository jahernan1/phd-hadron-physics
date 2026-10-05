"""docs/NEW_CHANNEL.md documents every channel-config key the stage code reads.

Keys are collected from the modules that read analyses/<channel>/config and
analysis_data.yaml: string subscripts (`x["key"]`), `.get("key")`, the
`require`/`_text`/`_number`/`_need`/`_override` helpers, string tuples a `for`
loop iterates over, and the key lists the checks use (check_block allowed
lists, STYLE_FIELDS, SETTINGS_KEYS, ...). Each must appear in the guide as a
code span, `key` or `{key}`. Also checks the channel.kv lines `gxana config
export` writes."""
import ast
import re
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import measurements, qfactors
from gxana.stages import xsection as xs
from gxana_barlow import config as bconfig
from gxana_barlow import stage as bstage
from gxana_studies import config as studies_config
from gxana_systematics import config as sconfig

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs" / "NEW_CHANNEL.md"
READERS = (
    "packages/common/python/gxana/config.py",
    "packages/common/python/gxana/analysis_data.py",
    "packages/common/python/gxana/stages/select.py",
    "packages/common/python/gxana/stages/mc.py",
    "packages/common/python/gxana/stages/qfactors.py",
    "packages/common/python/gxana/stages/measurements.py",
    "packages/common/python/gxana/stages/xsection.py",
    "packages/barlow/python/gxana_barlow/config.py",
    "packages/barlow/python/gxana_barlow/stage.py",
    "packages/barlow/python/gxana_barlow/variations.py",
    "packages/systematics/python/gxana_systematics/config.py",
    "packages/systematics/python/gxana_systematics/stage.py",
    "packages/studies/python/gxana_studies/config.py",
    "packages/studies/python/gxana_studies/stage.py",
)
HELPERS = {"require", "_text", "_number", "_need", "_override", "get"}
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
# Strings the scan finds that are not config keys.
NOT_KEYS = {
    "GXANA_SIM_VERSION_SET", "QFACTORS_SETTINGS", "ROOT_ANALYSIS_HOME",  # environment variables
    "FOM", "SB",                                                         # {what} values of a cut scan
    "variation_trees", "weighted_data",                                  # output subdirectories
}


def _strings(node):
    if isinstance(node, (ast.Tuple, ast.List, ast.Set)):
        for elt in node.elts:
            if isinstance(elt, ast.Constant) and isinstance(elt.value, str):
                yield elt.value
            else:
                yield from _strings(elt)
    elif isinstance(node, ast.BinOp):
        yield from _strings(node.left)
        yield from _strings(node.right)


def _scanned_keys():
    keys = set()
    for rel in READERS:
        for node in ast.walk(ast.parse((ROOT / rel).read_text())):
            if isinstance(node, ast.Subscript) and isinstance(node.slice, ast.Constant):
                keys.add(node.slice.value)
            elif isinstance(node, ast.Call):
                func = node.func
                name = func.attr if isinstance(func, ast.Attribute) else getattr(func, "id", None)
                index = 0 if name == "get" else 1
                arg = node.args[index] if len(node.args) > index else None
                if name in HELPERS and isinstance(arg, ast.Constant):
                    keys.add(arg.value)
            elif isinstance(node, (ast.For, ast.comprehension)):
                keys.update(_strings(node.iter))
    return {k for k in keys if isinstance(k, str) and IDENT.match(k)} - NOT_KEYS


def _listed_keys():
    lists = [config.PHYSICS_KEYS, xs.TARGET_KEYS, xs.MASS_WINDOWS, xs.TEX_KEYS, xs.FIGURES_KEYS,
             bstage.CHECK_WINDOWS, bconfig.STYLE_FIELDS, qfactors.SETTINGS_KEYS, measurements.STEPS,
             studies_config.FIT_PARAMS, studies_config.SAMPLES, sconfig.PLOT_KEYS]
    lists += list(studies_config.KEYS.values()) + list(sconfig.STUDY_KEYS.values())
    return {key for keys in lists for key in keys}


KEYS = sorted(_scanned_keys() | _listed_keys())


def test_scan_finds_keys():
    """Guards the scan itself: a refactor that hides the reads would empty it."""
    for key in ("tree_dir_template", "flux", "mass_windows", "families", "kDim", "theta_cut_deg", "first_bin"):
        assert key in KEYS, key


@pytest.mark.parametrize("key", KEYS)
def test_key_documented(key):
    assert re.search(r"`\{?" + re.escape(key) + r"\}?`", DOC.read_text()), \
        f"config key {key!r} is read by the stages but missing from docs/NEW_CHANNEL.md"


def test_channel_kv_lines_documented():
    """Every line kind of channel.kv, with period, sample and file names generalized."""
    text = DOC.read_text()
    kv = config.channel_kv("kpkpkmlamb", ROOT)
    periods = list(config.load_channel("kpkpkmlamb", ROOT)["periods"])
    for line in kv.splitlines():
        if line.startswith("#"):
            continue
        key = line.split("=", 1)[0]
        key = re.sub(r"^source\..+\.md5$", "source.<file>.md5", key)
        for period in periods:
            key = key.replace(f".{period}.", ".<period>.")
        key = re.sub(r"^stem\.<period>\..+$", "stem.<period>.<sample>", key)
        if key.startswith("period.<period>."):
            assert f"`.{key.rsplit('.', 1)[1]}=`" in text or f"`{key}=`" in text, key
        else:
            assert f"`{key}=`" in text, key
