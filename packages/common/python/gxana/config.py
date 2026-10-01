"""Per-channel YAML configuration (analyses/<channel>/config/*.yaml, spec §8)."""
from __future__ import annotations

import os
import re
from pathlib import Path
from typing import Any, Dict, Mapping, Optional, Sequence

from gxana.paths import ENV_VARS, MissingEnvError, repo_root

TREE_KINDS = ("trees", "thrown")
_ENV_REF = re.compile(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}")


class ConfigError(RuntimeError):
    """Invalid or incomplete channel configuration."""


def require(cfg: Dict[str, Any], key: str) -> Any:
    """Return cfg[key], or raise ConfigError naming the key and channel."""
    try:
        return cfg[key]
    except KeyError:
        raise ConfigError(f"channel {cfg.get('channel', '?')!r} config missing required key {key!r}") from None


def check_block(block: Any, allowed: Sequence[str], where: str, required: Sequence[str] = ()) -> Dict[str, Any]:
    """Return `block` (a mapping) after checking its keys: one not in `allowed` (a misspelt
    key would otherwise be ignored and the app's default used) or a `required` one that is
    missing is a ConfigError naming `where.key`."""
    if not isinstance(block, dict):
        raise ConfigError(f"{where}: must be a mapping, got {block!r}")
    for key in block:
        if key not in allowed:
            raise ConfigError(f"{where}.{key}: unknown key; allowed: {', '.join(allowed)}")
    for key in required:
        if key not in block:
            raise ConfigError(f"{where}.{key}: required")
    return block


PHYSICS_KEYS = ("flat_tree", "thrown_flat_tree", "observable", "qvalue_branch", "branching_ratio", "reaction_title")


def physics_block(cfg: Dict[str, Any]) -> Dict[str, Any]:
    """The channel's `physics` block (channel.yaml) with its keys checked: no unknown key,
    observable {branch, title} and branching_ratio {value, error} complete."""
    where = f"channel {cfg.get('channel', '?')!r} physics"
    phys = check_block(cfg["physics"], PHYSICS_KEYS, where)
    if "observable" in phys:
        check_block(phys["observable"], ("branch", "title"), f"{where}.observable", ("branch", "title"))
    if "branching_ratio" in phys:
        check_block(phys["branching_ratio"], ("value", "error"), f"{where}.branching_ratio", ("value", "error"))
    return phys


def load_channel(channel: str, root: Optional[Path] = None) -> Dict[str, Any]:
    import yaml

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


def _text(mapping: Dict[str, Any], key: str, where: str) -> str:
    value = mapping.get(key)
    if not isinstance(value, str) or not value.strip():
        raise ConfigError(f"{where}.{key}: need a non-empty string, got {value!r}")
    return value


def _number(mapping: Dict[str, Any], key: str, where: str) -> float:
    value = mapping.get(key)
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ConfigError(f"{where}.{key}: need a number, got {value!r}")
    return value


def physics(cfg: Dict[str, Any]) -> Dict[str, Any]:
    """The channel's `physics` block (channel.yaml), checked: flat_tree, thrown_flat_tree and
    reaction_title strings, observable {branch, title}, qvalue_branch (a branch, or null for a
    channel without Q-factors) and branching_ratio {value, error} numbers. Raises ConfigError
    naming the first problem."""
    phys = require(cfg, "physics")
    where = f"channel {cfg.get('channel', '?')!r} physics"
    physics_block(cfg)  # unknown keys, incomplete observable / branching_ratio
    for key in ("flat_tree", "thrown_flat_tree", "reaction_title"):
        _text(phys, key, where)
    observable = phys.get("observable")
    if not isinstance(observable, dict):
        raise ConfigError(f"{where}.observable: need {{branch, title}}, got {observable!r}")
    for key in ("branch", "title"):
        _text(observable, key, f"{where}.observable")
    if "qvalue_branch" not in phys:
        raise ConfigError(f"{where}.qvalue_branch: required (null for a channel without Q-factors)")
    if phys["qvalue_branch"] is not None:
        _text(phys, "qvalue_branch", where)
    br = phys.get("branching_ratio")
    if not isinstance(br, dict):
        raise ConfigError(f"{where}.branching_ratio: need {{value, error}}, got {br!r}")
    for key in ("value", "error"):
        _number(br, key, f"{where}.branching_ratio")
    return phys


def period_settings(cfg: Dict[str, Any], period: str) -> Dict[str, Any]:
    periods = require(cfg, "periods")
    try:
        return periods[period]
    except KeyError:
        raise ConfigError(f"unknown period {period!r}; known: {sorted(periods)}") from None


def sample_settings(cfg: Dict[str, Any], sample: str) -> Dict[str, Any]:
    samples = require(cfg, "samples")
    try:
        return samples[sample]
    except KeyError:
        raise ConfigError(f"unknown sample {sample!r}; known: {sorted(samples)}") from None


def expand_env(value: str, environ: Optional[Mapping[str, str]] = None) -> str:
    """Replace ${GXANA_*} references; an unset variable is an error, never ''."""
    env = os.environ if environ is None else environ

    def sub(match: "re.Match[str]") -> str:
        name = match.group(1)
        if name not in ENV_VARS:
            raise ConfigError(f"only GXANA_* variables may be referenced, got ${{{name}}}")
        if not env.get(name):
            raise MissingEnvError(f"{name} is not set; run `source env/setup.sh`")
        return env[name]

    return _ENV_REF.sub(sub, value)


def _override(settings: Dict[str, Any], key: str, what: str) -> Optional[Any]:
    """settings[key] if present; an empty value is a config error, not 'unset'."""
    if key not in settings:
        return None
    if settings[key] in ("", None):
        raise ConfigError(f"{what}: {key!r} is empty")
    return settings[key]


def tree_stem(cfg: Dict[str, Any], period: str, sample: str) -> str:
    """Legacy tree / flat-tree stem '<reaction>__<fit_prefix><period>_<launch>[<tree_suffix>]'.

    tree_suffix defaults to '_<sample>' for MC samples and '' otherwise.
    """
    p = period_settings(cfg, period)
    s = sample_settings(cfg, sample)
    what = f"sample {sample!r}"
    launch = _override(s, "launch", what) or p["launch"]
    if isinstance(launch, dict):
        if period not in launch:
            raise ConfigError(f"sample {sample!r} has no launch for period {period!r}")
        launch = launch[period]
    fit_prefix = _override(s, "fit_prefix", what)
    if fit_prefix is None:
        fit_prefix = p["fit_prefix"]
    suffix = _override(s, "tree_suffix", what)
    if suffix is None:
        suffix = f"_{sample}" if s.get("mc") else ""
    return f"{require(cfg, 'reaction')}__{fit_prefix}{period}_{launch}{suffix}"


def tree_dir(cfg: Dict[str, Any], period: str, sample: str, kind: str = "trees") -> str:
    """Tree directory relative to $GXANA_DATA, e.g. 'Trees/tree_kpkpxim__M23_2017-01_ana56/trees/'."""
    if kind not in TREE_KINDS:
        raise ConfigError(f"kind must be one of {TREE_KINDS}, got {kind!r}")
    # Thrown trees exist only for MC samples (samples.yaml 'mc' field).
    if kind == "thrown" and not sample_settings(cfg, sample).get("mc"):
        raise ConfigError(f"sample {sample!r} has no thrown trees")
    return require(cfg, "tree_dir_template").format(stem=tree_stem(cfg, period, sample), kind=kind)


def selector_name(cfg: Dict[str, Any], sample: str, thrown: bool = False) -> str:
    s = sample_settings(cfg, sample)
    key = "thrown_selector" if thrown else "selector"
    return _override(s, key, f"sample {sample!r}") or require(cfg, "thrown_selector" if thrown else "default_selector")


def output_basename(cfg: Dict[str, Any], sample: str, thrown: bool = False) -> str:
    """Base of the file the selector writes (thrown selectors prefix it with 'thrown_').

    A thrown job uses the sample's output_basename only when the sample also
    names its own thrown_selector; the generic thrown selector writes the
    channel basename.
    """
    s = sample_settings(cfg, sample)
    what = f"sample {sample!r}"
    if thrown and _override(s, "thrown_selector", what) is None:
        return require(cfg, "output_basename")
    return _override(s, "output_basename", what) or require(cfg, "output_basename")
