"""Per-channel YAML configuration (analyses/<channel>/config/*.yaml, spec §8).

`${GXANA_*}` expansion inside YAML values is not supported yet (Plan 3).
"""
from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, Optional

import yaml

from gxana.paths import repo_root

TREE_KINDS = ("trees", "thrown")


class ConfigError(RuntimeError):
    """Invalid or incomplete channel configuration."""


def load_channel(channel: str, root: Optional[Path] = None) -> Dict[str, Any]:
    cfg_dir = (root or repo_root()) / "analyses" / channel / "config"
    files = sorted(cfg_dir.glob("*.yaml"))
    if not files:
        raise ConfigError(f"no config files in {cfg_dir}")
    merged: Dict[str, Any] = {}
    for path in files:
        data = yaml.safe_load(path.read_text()) or {}
        if not isinstance(data, dict):
            raise ConfigError(f"{path}: top level must be a mapping")
        for key, value in data.items():
            if key in merged:
                raise ConfigError(f"{path}: key {key!r} already defined in another file")
            merged[key] = value
    return merged


def period_settings(cfg: Dict[str, Any], period: str) -> Dict[str, Any]:
    try:
        return cfg["periods"][period]
    except KeyError:
        raise ConfigError(f"unknown period {period!r}; known: {sorted(cfg['periods'])}") from None


def sample_settings(cfg: Dict[str, Any], sample: str) -> Dict[str, Any]:
    try:
        return cfg["samples"][sample]
    except KeyError:
        raise ConfigError(f"unknown sample {sample!r}; known: {sorted(cfg['samples'])}") from None


def tree_dir(cfg: Dict[str, Any], period: str, sample: str, kind: str = "trees") -> str:
    """Tree directory relative to $GXANA_DATA, e.g. 'Trees/tree_kpkpxim__M23_2017-01_ana56/trees/'."""
    if kind not in TREE_KINDS:
        raise ConfigError(f"kind must be one of {TREE_KINDS}, got {kind!r}")
    p = period_settings(cfg, period)
    s = sample_settings(cfg, sample)
    # Thrown trees exist only for MC samples (samples.yaml 'mc' field), even
    # when a non-MC sample defines its own thrown_selector for other uses.
    if kind == "thrown" and not s.get("mc"):
        raise ConfigError(f"sample {sample!r} has no thrown trees")
    launch = s.get("launch", p["launch"])
    if isinstance(launch, dict):
        if period not in launch:
            raise ConfigError(f"sample {sample!r} has no launch for period {period!r}")
        launch = launch[period]
    return cfg["tree_dir_template"].format(
        reaction=cfg["reaction"],
        fit_prefix=s.get("fit_prefix", p["fit_prefix"]),
        period=period,
        launch=launch,
        mc_suffix=f"_{sample}" if s.get("mc") else "",
        kind=kind,
    )


def selector_name(cfg: Dict[str, Any], sample: str, thrown: bool = False) -> str:
    s = sample_settings(cfg, sample)
    if thrown:
        return s.get("thrown_selector", cfg["thrown_selector"])
    return s.get("selector", cfg["default_selector"])


def output_basename(cfg: Dict[str, Any], sample: str) -> str:
    return sample_settings(cfg, sample).get("output_basename", cfg["output_basename"])
