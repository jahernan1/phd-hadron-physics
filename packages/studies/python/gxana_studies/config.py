"""The `studies` block of analyses/<channel>/config/studies.yaml (`gxana run studies`)."""
from __future__ import annotations

from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana.config import ConfigError, check_block, require

KEYS = {
    "cutscan": ("kind", "out_dir", "input", "tree", "steps", "weight", "mass", "scan", "fit", "panel_label",
                "plot_title", "cut", "outputs"),
}
OPTIONAL = {"cutscan": ("weight",)}
FIT_PARAMS = ("a0", "a1", "mu", "lambda", "gamma", "delta", "nbkgd", "nxi")  # gxana_study_cutscan --param names


def _text(value: Any, where: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ConfigError(f"{where}: need a non-empty string, got {value!r}")
    return value


def _numbers(value: Any, count: int, where: str) -> List[Any]:
    if (not isinstance(value, list) or len(value) != count
            or any(isinstance(v, bool) or not isinstance(v, (int, float)) for v in value)):
        raise ConfigError(f"{where}: need a list of {count} numbers, got {value!r}")
    return value


def steps(value: Any, where: str) -> List[Dict[str, str]]:
    """A list of {filter: EXPR} or {define: NAME, expr: EXPR}, applied in order."""
    if not isinstance(value, list):
        raise ConfigError(f"{where}: need a list of steps, got {value!r}")
    for i, step in enumerate(value):
        w = f"{where}[{i}]"
        if isinstance(step, dict) and set(step) == {"filter"}:
            _text(step["filter"], f"{w}.filter")
        elif isinstance(step, dict) and set(step) == {"define", "expr"}:
            _text(step["define"], f"{w}.define")
            _text(step["expr"], f"{w}.expr")
        else:
            raise ConfigError(f"{w}: need {{filter: EXPR}} or {{define: NAME, expr: EXPR}}, got {step!r}")
    return value


def _output_key(out_dir: str, path: str) -> str:
    """An output pattern as it will be resolved: absolute patterns stand alone."""
    return path if path.startswith(("/", "${")) else f"{out_dir}/{path}"


def _check_cutscan(cfg: Dict[str, Any], s: Dict[str, Any], where: str) -> List[str]:
    """Checks a cutscan study; returns its output patterns (for the duplicate check)."""
    for key in ("out_dir", "input", "tree", "panel_label", "plot_title"):
        _text(s[key], f"{where}.{key}")
    if "weight" in s:
        _text(s["weight"], f"{where}.weight")
    steps(s["steps"], f"{where}.steps")
    mass = check_block(s["mass"], ("var", "bins"), f"{where}.mass", ("var", "bins"))
    _text(mass["var"], f"{where}.mass.var")
    _numbers(mass["bins"], 3, f"{where}.mass.bins")
    scan = check_block(s["scan"], ("var", "bins", "first_bin"), f"{where}.scan", ("var", "bins", "first_bin"))
    _text(scan["var"], f"{where}.scan.var")
    _numbers(scan["bins"], 3, f"{where}.scan.bins")
    if isinstance(scan["first_bin"], bool) or not isinstance(scan["first_bin"], int) or scan["first_bin"] < 1:
        raise ConfigError(f"{where}.scan.first_bin: need an integer >= 1, got {scan['first_bin']!r}")
    fit = check_block(s["fit"], ("mass_title", "range", "params"), f"{where}.fit", ("mass_title", "range", "params"))
    _text(fit["mass_title"], f"{where}.fit.mass_title")
    _numbers(fit["range"], 2, f"{where}.fit.range")
    params = check_block(fit["params"], FIT_PARAMS, f"{where}.fit.params", FIT_PARAMS)
    for name in FIT_PARAMS:
        _text(params[name], f"{where}.fit.params.{name}")  # factory bracket text, kept verbatim
    _numbers([s["cut"]], 1, f"{where}.cut")
    out = check_block(s["outputs"], ("hist", "tables", "grid", "plots"), f"{where}.outputs",
                      ("hist", "tables", "grid", "plots"))
    for key in ("hist", "tables", "grid"):
        _text(out[key], f"{where}.outputs.{key}")
    if "{what}" not in out["tables"]:
        raise ConfigError(f"{where}.outputs.tables: needs {{what}} (FOM, SB, Yield)")
    if not isinstance(out["plots"], list) or not out["plots"]:
        raise ConfigError(f"{where}.outputs.plots: need a non-empty list of paths")
    for i, path in enumerate(out["plots"]):
        _text(path, f"{where}.outputs.plots[{i}]")
    return [_output_key(s["out_dir"], p) for p in [out["hist"], out["tables"], out["grid"]] + list(out["plots"])]


CHECKS = {"cutscan": _check_cutscan}


def block(cfg: Dict[str, Any]) -> Dict[str, Any]:
    studies = require(cfg, "studies")
    if not isinstance(studies, dict) or not studies:
        raise ConfigError("studies: need a mapping of study name -> study")
    return studies


def validate(cfg: Dict[str, Any]) -> None:
    """Every study: known kind, no unknown or missing key, typed values, and no output pattern
    shared with another study (a YAML merge key that forgets `outputs` would otherwise overwrite)."""
    seen: Dict[str, str] = {}
    for name, s in block(cfg).items():
        where = f"studies.{name}"
        kind = s.get("kind") if isinstance(s, dict) else None
        if kind not in KEYS:
            raise ConfigError(f"{where}.kind: one of {', '.join(KEYS)}, got {kind!r}")
        check_block(s, KEYS[kind], where, [k for k in KEYS[kind] if k not in OPTIONAL[kind]])
        for path in CHECKS[kind](cfg, s, where):
            if path in seen:
                raise ConfigError(f"{where}: writes {path}, as studies.{seen[path]} does")
            seen[path] = name


def studies(cfg: Dict[str, Any], names: Optional[Sequence[str]] = None) -> List[Tuple[str, Dict[str, Any]]]:
    """The validated studies, in config order; `names` selects (an unknown name is an error)."""
    validate(cfg)
    all_studies = block(cfg)
    for name in names or []:
        if name not in all_studies:
            raise ConfigError(f"unknown study {name!r}; known: {list(all_studies)}")
    return [(n, s) for n, s in all_studies.items() if names is None or n in names]
