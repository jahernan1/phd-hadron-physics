"""Resolve site paths from GXANA_* environment variables (docs/REFACTOR_SPEC.md §7.3)."""
from __future__ import annotations

import os
from pathlib import Path
from typing import Mapping, Optional, Tuple

ENV_VARS: Tuple[str, ...] = (
    "GXANA_ROOT", "GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA",
)

# Legacy FSU prefixes -> env-var templates. First match wins, so specific
# prefixes come before the general AnalysisNote/ and analysis/ entries.
LEGACY_PREFIXES: Tuple[Tuple[str, str], ...] = (
    ("/d/grid17/hjesse/KpKpKmL012017012018082018Real_31July.root",
     "${GXANA_DATA}/KpKpKmL012017012018082018Real_31July.root"),
    ("/d/grid17/hjesse/AnalysisNote/QFactors/logs/", "${GXANA_OUTPUT}/kpkpxim/qfactors/"),
    ("/d/grid17/hjesse/AnalysisNote/flatTrees/", "${GXANA_DATA}/flatTrees/"),
    ("/d/grid17/hjesse/AnalysisNote/fluxFiles/", "${GXANA_DATA}/flux/"),
    ("/d/grid17/hjesse/AnalysisNote/", "${GXANA_OUTPUT}/kpkpxim/"),
    ("/d/grid17/hjesse/Trees/", "${GXANA_DATA}/Trees/"),
    ("/d/grid17/hjesse/analysis/kpkpxim/flux/", "${GXANA_DATA}/flux/"),
    ("/d/grid17/hjesse/analysis/kpkpxim/", "${GXANA_OUTPUT}/kpkpxim/selector_hists/"),
    ("/d/grid17/hjesse/analysis/", "${GXANA_OUTPUT}/kpkpxim/selector_hists/"),
    ("/d/grid17/hjesse/macros/", "${GXANA_OUTPUT}/legacy_macros/"),
    ("/d/grid17/hjesse/Clas_data.csv", "${GXANA_ROOT}/analyses/kpkpxim/xsection/external_data/Clas_data.csv"),
    ("/d/grid17/hjesse/temp", "${GXANA_SCRATCH}"),
)


class MissingEnvError(RuntimeError):
    """A required GXANA_* variable is unset or empty."""


def env_path(var: str, *parts: str, environ: Optional[Mapping[str, str]] = None) -> Path:
    """Return $var joined with parts; raise MissingEnvError if $var is unset."""
    if var not in ENV_VARS:
        raise ValueError(f"unknown variable {var!r}; expected one of {ENV_VARS}")
    env = os.environ if environ is None else environ
    value = env.get(var)
    if not value:
        raise MissingEnvError(f"{var} is not set; run `source env/setup.sh`")
    return Path(value).joinpath(*parts)


def repo_root() -> Path:
    """GXANA_ROOT if set, else the checkout containing this file."""
    value = os.environ.get("GXANA_ROOT")
    if value:
        return Path(value)
    # <root>/packages/common/python/gxana/paths.py
    return Path(__file__).resolve().parents[4]


ANALYSIS_DATA_DIRNAME = "gluex_analysis_data"


def analysis_data_root(environ: Optional[Mapping[str, str]] = None) -> Path:
    """$GXANA_ANALYSIS_DATA, else <repo>/gluex_analysis_data (the env/setup.sh default).

    Preserved analysis data: golden inputs and reference outputs kept outside
    git, as GlueX keeps dissertation data under /work/halld/gluex_analysis_data/
    (docs/analysis_data.md).
    """
    env = os.environ if environ is None else environ
    value = env.get("GXANA_ANALYSIS_DATA")
    return Path(value) if value else repo_root() / ANALYSIS_DATA_DIRNAME


def legacy_to_env(path: str) -> str:
    """Rewrite a legacy absolute FSU path to its ${GXANA_*} template; others unchanged."""
    for prefix, replacement in LEGACY_PREFIXES:
        if path.startswith(prefix):
            return replacement + path[len(prefix):]
    return path
