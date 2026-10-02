"""The `studies` block of analyses/<channel>/config/studies.yaml (`gxana run studies`)."""
from __future__ import annotations

from typing import Any, Dict, List, Optional, Sequence, Tuple

from gxana.config import ConfigError, check_block, require

KEYS = {
    "cutscan": ("kind", "out_dir", "input", "tree", "steps", "weight", "mass", "scan", "fit", "panel_label",
                "plot_title", "cut", "outputs", "threads"),
    "datamc": ("kind", "out_dir", "hist_file", "mc_sample", "inputs", "tree", "thrown_tree", "tags", "samples",
               "vars", "truth_vars", "threads"),
}
OPTIONAL = {"cutscan": ("weight", "threads"), "datamc": ("mc_sample", "truth_vars", "threads")}
SAMPLES = ("data", "mc", "thrown")
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


def _check_datamc(cfg: Dict[str, Any], s: Dict[str, Any], where: str) -> List[str]:
    """Checks a datamc study; returns its histogram file pattern (for the duplicate check)."""
    for key in ("out_dir", "hist_file", "tree", "thrown_tree"):
        _text(s[key], f"{where}.{key}")
    inputs = check_block(s["inputs"], SAMPLES, f"{where}.inputs", SAMPLES)
    for name in SAMPLES:
        _text(inputs[name], f"{where}.inputs.{name}")
        if "{mc_stem}" in inputs[name] and "mc_sample" not in s:
            raise ConfigError(f"{where}.mc_sample: required by {{mc_stem}} in inputs.{name}")
    if "mc_sample" in s:
        _text(s["mc_sample"], f"{where}.mc_sample")
    periods = list(require(cfg, "periods"))
    tags = check_block(s["tags"], periods, f"{where}.tags", periods)
    for period, tag in tags.items():
        _text(tag, f"{where}.tags.{period}")
    samples = check_block(s["samples"], SAMPLES, f"{where}.samples")
    for name, sample in samples.items():
        sample = check_block(sample, ("steps", "weight"), f"{where}.samples.{name}")
        steps(sample.get("steps", []), f"{where}.samples.{name}.steps")
        if "weight" in sample:
            _text(sample["weight"], f"{where}.samples.{name}.weight")
    names: List[str] = []
    for key in ("vars", "truth_vars"):
        if key not in s:
            continue
        if not isinstance(s[key], list) or not s[key]:
            raise ConfigError(f"{where}.{key}: need a non-empty list")
        for i, var in enumerate(s[key]):
            w = f"{where}.{key}[{i}]"
            var = check_block(var, ("var", "title", "legend"), w, ("var", "title"))
            _text(var["var"], f"{w}.var")
            if not isinstance(var["title"], str):
                raise ConfigError(f"{w}.title: need a string, got {var['title']!r}")
            if var.get("legend", "tr") not in ("tl", "tr"):
                raise ConfigError(f"{w}.legend: tl or tr, got {var['legend']!r}")
            if key == "vars":
                names.append(var["var"])
            elif var["var"] not in names:
                raise ConfigError(f"{w}.var: {var['var']!r} is not in vars (its binning comes from the data histogram)")
    return [_output_key(s["out_dir"], s["hist_file"])]


def _threads(s: Dict[str, Any], where: str) -> None:
    v = s.get("threads", 0)
    if isinstance(v, bool) or not isinstance(v, int) or v < 0:
        raise ConfigError(f"{where}.threads: need an integer >= 0, got {v!r}")


CHECKS = {"cutscan": _check_cutscan, "datamc": _check_datamc}


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
        _threads(s, where)
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
