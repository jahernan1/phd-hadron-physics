"""The `barlow` block of analyses/<channel>/config/barlow.yaml: access and validation."""
from __future__ import annotations

import hashlib
import json
from typing import Any, Dict, Sequence

from gxana.config import ConfigError, require

OPS = ("<", ">", "<=", ">=")
STYLE_FIELDS = ("canvas", "legend_diff", "legend_tot", "y_floor", "y_pad_diff", "canvas_def_w",
                "title_offset_y", "title_offsets_diff", "title_offsets_tot", "tot_y_ndiv")


def block(cfg: Dict[str, Any]) -> Dict[str, Any]:
    return require(cfg, "barlow")


def _need(mapping: Dict[str, Any], key: str, where: str) -> Any:
    value = mapping.get(key)
    if value in (None, "", [], {}):
        raise ConfigError(f"{where}: missing {key!r}")
    return value


def _numbers(value: Any, count: int) -> bool:
    return (isinstance(value, list) and len(value) == count
            and all(isinstance(x, (int, float)) and not isinstance(x, bool) for x in value))


def _validate_style(style: Dict[str, Any], where: str) -> None:
    for field in STYLE_FIELDS:
        if field not in style:
            raise ConfigError(f"{where}: missing {field!r}")
    for box in ("legend_diff", "legend_tot"):
        if not _numbers(style[box], 4) or not all(0 <= x <= 1 for x in style[box]):
            raise ConfigError(f"{where}.{box}: need 4 numbers in [0, 1], got {style[box]!r}")
    canvas = style["canvas"]
    if canvas != "default" and not (_numbers(canvas, 2) and all(isinstance(c, int) and c > 0 for c in canvas)):
        raise ConfigError(f"{where}.canvas: 'default' or [width, height], got {canvas!r}")
    for pair in ("title_offsets_diff", "title_offsets_tot"):
        if not _numbers(style[pair], 2):
            raise ConfigError(f"{where}.{pair}: need 2 numbers, got {style[pair]!r}")


def validate(bcfg: Dict[str, Any], steps: Sequence[str] = ()) -> None:
    """Raise ConfigError on the first problem; run before any command (spec §5)."""
    for key in ("label", "mc_sample", "output_dir", "weight", "threshold", "fit", "trees", "nominal", "families"):
        _need(bcfg, key, "barlow")
    trees = bcfg["trees"]
    for key in ("tree", "input", "input_mc", "output", "branches"):
        _need(trees, key, "barlow.trees")
    nominal = bcfg["nominal"]
    ids = set()
    for family, fam in bcfg["families"].items():
        where = f"barlow.families.{family}"
        if family not in nominal:
            raise ConfigError(f"{where}: no barlow.nominal entry for family {family!r}")
        if fam.get("op") not in OPS:
            raise ConfigError(f"{where}.op: {fam.get('op')!r} is not one of {list(OPS)}")
        values = fam.get("values")
        if not values:
            raise ConfigError(f"{where}.values: empty")
        if not fam.get("label"):
            raise ConfigError(f"{where}.label: not set")
        for value in values:
            if not isinstance(value, str):
                raise ConfigError(f"{where}.values: {value!r} must be a quoted string "
                                  f"(ids, file names and legend text use its exact spelling)")
            vid = f"{family}_{value}"
            if vid in ids:
                raise ConfigError(f"barlow: duplicate variation id {vid!r}")
            ids.add(vid)
        _validate_style(fam.get("style") or {}, f"{where}.style")
    if "check" in steps:
        check = bcfg.get("check") or {}
        for key in ("nominal", "nominal_mc"):
            if not check.get(key):
                raise ConfigError(f"barlow.check.{key}: required by the check step")


def config_hash(bcfg: Dict[str, Any]) -> str:
    """sha256 of the canonical (sorted-key JSON) barlow block."""
    text = json.dumps(bcfg, sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(text.encode()).hexdigest()
