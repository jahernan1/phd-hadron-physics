"""The `systematics` block of analyses/<channel>/config/systematics.yaml."""
from __future__ import annotations

from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana.config import ConfigError, require

KINDS = ("spread", "sfactor", "track", "constant", "compare")
MEASURING = ("spread", "sfactor", "track", "constant")
LAYOUTS = ("grid3", "pair_band", "all_band", "grid2", "run_grid", "stddev_band")
STUDY_KEYS = {
    "spread": {"kind", "stats", "spread", "plots", "examples"},
    "sfactor": {"kind", "stats"},
    "track": {"kind", "tree", "thrown_tree", "data_weight", "mc_weight", "theta_cut_deg", "low", "high",
              "override", "report", "particles", "legend_header"},
    "constant": {"kind", "value"},
    "compare": {"kind", "plots", "per_period", "stats"},
}
PLOT_KEYS = {"layout", "name", "labels", "legend", "legend_header", "first_style", "annotate", "axis_format",
             "x_axis_format", "xmax", "ymax"}


def block(cfg: Dict[str, Any]) -> Dict[str, Any]:
    return require(cfg, "systematics")


def nominal(cfg: Dict[str, Any]) -> str:
    scfg = block(cfg)
    if scfg.get("nominal"):
        return scfg["nominal"]
    return require(require(require(cfg, "xsection"), "tex"), "label")


def fit_groups(scfg: Dict[str, Any]) -> List[Dict[str, Any]]:
    return [v for v in require(scfg, "variants") if "model" in v]


def qvalue_variants(scfg: Dict[str, Any]) -> List[Dict[str, str]]:
    return [v["qvalue"] for v in require(scfg, "variants") if "qvalue" in v]


def pool_labels(scfg: Dict[str, Any]) -> List[str]:
    labels = [e["label"] for g in fit_groups(scfg) for e in g["labels"]]
    return labels + [q["label"] for q in qvalue_variants(scfg)]


def studies(scfg: Dict[str, Any], names: Optional[Sequence[str]] = None) -> List[Tuple[str, Dict[str, Any]]]:
    all_studies = require(scfg, "studies")
    for name in names or []:
        if name not in all_studies:
            raise ConfigError(f"unknown study {name!r}; known: {list(all_studies)}")
    return [(n, s) for n, s in all_studies.items() if names is None or n in names]


def study_labels(name: str, study: Dict[str, Any], per_period: bool = True) -> List[str]:
    """Every label the study names; per_period=False leaves out the compare per_period
    label (it reads xsection/data/<label>, not the variant pool)."""
    labels: List[str] = list(study.get("spread") or [])
    for plot in study.get("plots") or []:
        labels += plot.get("labels") or []
    labels += list(((study.get("examples") or {}).get("fits") or {}).values())
    if per_period and study.get("per_period"):
        labels.append(study["per_period"])
    seen: List[str] = []
    for label in labels:
        if label not in seen:
            seen.append(label)
    return seen


def _check_plot(where: str, plot: Dict[str, Any]) -> None:
    unknown = set(plot) - PLOT_KEYS
    if unknown:
        raise ConfigError(f"{where}: unknown keys {sorted(unknown)}")
    if plot.get("layout") not in LAYOUTS:
        raise ConfigError(f"{where}: layout must be one of {list(LAYOUTS)}, got {plot.get('layout')!r}")
    if not plot.get("name"):
        raise ConfigError(f"{where}: missing 'name'")


def validate(cfg: Dict[str, Any]) -> None:
    scfg = block(cfg)
    nom = nominal(cfg)
    labels = pool_labels(scfg)
    dupes = sorted({l for l in labels if labels.count(l) > 1})
    if dupes:
        raise ConfigError(f"systematics.variants: duplicate label {dupes[0]!r}")
    fit_labels = [e["label"] for g in fit_groups(scfg) for e in g["labels"]]
    for q in qvalue_variants(scfg):
        if q.get("source") not in fit_labels:
            raise ConfigError(f"systematics.variants: qvalue {q.get('label')!r} source {q.get('source')!r} "
                              f"is not a fit label of the pool")
    known = set(labels) | {nom}
    kinds = {}
    for name, study in require(scfg, "studies").items():
        where = f"systematics.studies.{name}"
        kind = study.get("kind")
        if kind not in KINDS:
            raise ConfigError(f"{where}: kind must be one of {list(KINDS)}, got {kind!r}")
        kinds[name] = kind
        unknown = set(study) - STUDY_KEYS[kind]
        if unknown:
            raise ConfigError(f"{where}: unknown keys {sorted(unknown)}")
        for i, plot in enumerate(study.get("plots") or []):
            _check_plot(f"{where}.plots[{i}]", plot)
        if kind == "spread":
            members = require(study, "spread")
            require(study, "stats")
            if len(members) < 2:
                raise ConfigError(f"{where}.spread: needs at least 2 members, got {members}")
            dupes = [l for i, l in enumerate(members) if l in members[:i]]
            if dupes:
                raise ConfigError(f"{where}.spread: duplicate member {dupes[0]!r}")
        if kind in ("spread", "sfactor", "track"):
            missing = [l for l in study_labels(name, study) if l not in known]
            if missing:
                raise ConfigError(f"{where}: label {missing[0]!r} is not a variant label or the nominal {nom!r}")
        if kind == "sfactor":
            require(study, "stats")
        if kind == "constant":
            require(study, "value")
        if kind == "track":
            for key in ("tree", "thrown_tree", "particles", "theta_cut_deg", "low", "high", "report"):
                require(study, key)
            if study["report"] not in ("data", "mc"):
                raise ConfigError(f"{where}.report: 'data' or 'mc', got {study['report']!r}")
            names = [p.get("name") for p in study["particles"]]
            for key in study.get("override") or {}:
                if key not in names:
                    raise ConfigError(f"{where}.override: {key!r} is not a particle name {names}")
    summary = scfg.get("summary") or {}
    for key in ("point_by_point", "normalization"):
        for name in summary.get(key) or []:
            if kinds.get(name) not in MEASURING:
                raise ConfigError(f"systematics.summary.{key}: {name!r} is not a spread, sfactor, track or "
                                  f"constant study")
