"""`gxana run qfactors`: Q-factor signal weights with the QFactors fork
(packages/qfactors, which also holds the fit models) and the channel run
config (config/qfactors.yaml).

The engine compiles its settings in: run.py sed-edits configSettings.h and
builds `main` against configPDFs.h, so each period runs in its own work
directory <work_dir>/<file_tag>/ holding a copy of the engine, the chosen
model (a configPDFs*.h file in the engine's top level, or a channel model
file given by its path relative to the repository root) as configPDFs.h,
run_settings.py (run.py reads it via QFACTORS_SETTINGS) and makePlotsVars.txt. logs/, histograms/ and
diagnosticPlots/ are symlinks into the output tree. configSettings.h is
rendered here with run.py's own substitutions (same keys, same order, same
unanchored patterns), so `--steps prepare` leaves a work directory where
`./main <iProcess>` runs by hand.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable, Dict, List, Mapping, NamedTuple, Optional, Sequence

from gxana import config
from gxana.paths import gxana_root

Runner = Callable[..., subprocess.CompletedProcess]
STEPS = ("prepare", "fit", "plots")
DEFAULT_STEPS = ("fit", "plots")
# run.py's _SET_* names, in its order.
SETTINGS_KEYS = (
    "fitWeights", "sigWeights", "altWeights", "varStringBase", "discrimVars", "extraVars", "neighborReqs",
    "nProcess", "kDim", "nentries", "numberEventsToSavePerProcess", "standardizationType",
    "redistributeBkgSigFits", "nRndRepSubset", "doKRandomNeighbors", "nBS", "runTag", "seedShift",
    "saveBShistsAlso", "alwaysSaveTheseEvents", "saveBranchOfNeighbors", "saveMemUsage",
    "saveEventLevelProcessSpeed", "emailWhenFinished", "runBatch", "runAllPhaseCombos", "extraLibs",
)
# run.py reconfigureSettings(): (configSettings.h key, _SET_ name, quoted); None = per-job value.
_RECONFIGURE = (
    ("rootFileLoc", None, True), ("rootTreeName", None, True), ("fileTag", None, True),
    ("runTag", "runTag", True), ("cwd", None, True), ("standardizationType", "standardizationType", True),
    ("s_discrimVar", "discrimVars", True), ("s_extraVar", "extraVars", True),
    ("s_neighborReqs", "neighborReqs", True), ("s_fitWeight", "fitWeights", True),
    ("s_sigWeight", "sigWeights", True), ("s_altWeight", "altWeights", True),
    ("standardizationType", "standardizationType", True), ("alwaysSaveTheseEvents", "alwaysSaveTheseEvents", True),
    ("s_phaseVar", "varStringBase", True), ("nProcess", "nProcess", False), ("kDim", "kDim", False),
    ("ckDim", "kDim", False), ("redistributeBkgSigFits", "redistributeBkgSigFits", False),
    ("doKRandomNeighbors", "doKRandomNeighbors", False),
    ("numberEventsToSavePerProcess", "numberEventsToSavePerProcess", False), ("seedShift", "seedShift", False),
    ("nentries", "nentries", False), ("nBS", "nBS", False), ("saveBShistsAlso", "saveBShistsAlso", False),
    ("saveEventLevelProcessSpeed", "saveEventLevelProcessSpeed", False),
    ("saveBranchOfNeighbors", "saveBranchOfNeighbors", False), ("saveMemUsage", "saveMemUsage", False),
    ("override_nentries", None, False),
)
# run.py runOverCombo() changeDim(): (const int name, _SET_ name, empty string is an error)
_DIMS = (
    ("phaseSpaceDim", "varStringBase", True), ("discrimVarDim", "discrimVars", True),
    ("extraVarDim", "extraVars", False), ("fitWeightsDim", "fitWeights", False),
    ("sigWeightsDim", "sigWeights", False), ("altWeightsDim", "altWeights", False),
)
# Work-dir symlink -> subdirectory of plots_dir ("" = output_dir itself)
_LINKS = (("logs", None), ("histograms", "histograms"), ("diagnosticPlots", "diagnosticPlots"))


class QFactorsError(RuntimeError):
    """The Q-factor run cannot be staged or launched."""


class QJob(NamedTuple):
    period: str
    stem: str
    file_tag: str
    combo_tag: str
    input_file: Path
    tree: str
    model: str
    engine_dir: Path
    pdf_config: Path
    work_dir: Path
    output_dir: Path
    plots_dir: Path
    settings: Dict[str, Any]
    extra_settings: Dict[str, str]
    diagnostic_vars: List[str]
    result: Path


def _check_settings(settings: Dict[str, Any]) -> None:
    unknown = sorted(set(settings) - set(SETTINGS_KEYS))
    if unknown:
        raise config.ConfigError(f"unknown qfactors setting(s) {unknown}; known: {list(SETTINGS_KEYS)}")
    missing = [k for k in SETTINGS_KEYS if k not in settings]
    if missing:
        raise config.ConfigError(f"missing qfactors setting(s) {missing}")
    for key in ("runBatch", "runAllPhaseCombos"):
        if settings[key]:
            raise config.ConfigError(f"qfactors setting {key} must be 0: gxana runs one phase-space combination "
                                     "locally (condor submission and combo scans are not supported)")
    if settings["runTag"] != "":
        raise config.ConfigError("qfactors setting runTag must be empty: output directories are set by "
                                 "output_dir/plots_dir")
    if not isinstance(settings["extraLibs"], list):
        raise config.ConfigError("qfactors setting extraLibs must be a list")


def plan_qfactors(cfg: Dict[str, Any], period: str, model: Optional[str] = None,
                  input_file: Optional[Path] = None, overrides: Optional[Mapping[str, Any]] = None,
                  diagnostic_vars: Optional[Sequence[str]] = None,
                  environ: Optional[Mapping[str, str]] = None) -> QJob:
    q = config.require(cfg, "qfactors")
    config.period_settings(cfg, period)
    settings = dict(q["settings"])
    settings.update(overrides or {})
    _check_settings(settings)
    stem = config.tree_stem(cfg, period, q["sample"])
    file_tag = f"{stem}{q['variant']}"
    combo_tag = f"{file_tag}_{'1' * len(settings['varStringBase'].split(';'))}"
    root = gxana_root(environ)
    model = model or q["model"]
    engine_dir = root / q["engine_dir"]
    if not (engine_dir / "main.C").is_file():
        raise config.ConfigError(f"{engine_dir} is not a checked-out QFactors engine; "
                                 "run `git submodule update --init packages/qfactors`")
    known = sorted(p.name for p in engine_dir.glob("configPDFs*.h"))
    if model in known:
        pdf_config = engine_dir / model
    elif "/" in model and (root / model).is_file():  # a channel model, relative to the repository root
        pdf_config = root / model
    else:
        raise config.ConfigError(f"unknown Q-factor model {model!r}; known: {known} (configPDFs*.h files in "
                                 f"{engine_dir}), or the path, relative to the repository root, of an existing "
                                 "channel model file (e.g. analyses/<channel>/config/qfactors_models/configPDFs_X.h)")
    if input_file is None:
        input_file = Path(config.expand_env(q["input"], environ).format(stem=stem, variant=q["variant"]))
    output_dir = Path(config.expand_env(q["output_dir"], environ))
    return QJob(
        period=period, stem=stem, file_tag=file_tag, combo_tag=combo_tag, input_file=Path(input_file),
        tree=q["tree"], model=model, engine_dir=engine_dir, pdf_config=pdf_config,
        work_dir=Path(config.expand_env(q["work_dir"], environ)) / file_tag, output_dir=output_dir,
        plots_dir=Path(config.expand_env(q["plots_dir"], environ)), settings=settings,
        extra_settings={str(k): str(v) for k, v in (q.get("extra_settings") or {}).items()},
        diagnostic_vars=list(diagnostic_vars if diagnostic_vars is not None else q["diagnostic_vars"]),
        result=output_dir / combo_tag / f"postQVal_flatTree_{combo_tag}.root",
    )


def _sub(text: str, key: str, value: Any, quoted: bool) -> str:
    """run.py spawnProcessChangeSetting: sed s@KEY=".*";@...@g / s@KEY=.*;@...@g (unanchored, greedy)."""
    if quoted:
        pattern, repl = re.escape(key) + r'=".*";', f'{key}="{value}";'
    else:
        pattern, repl = re.escape(key) + r"=.*;", f"{key}={value};"
    return re.sub(pattern, lambda _m: repl, text)


def _dim(value: str, required: bool, name: str) -> int:
    if value == "":
        if required:
            raise QFactorsError(f"{name} cannot be an empty string")
        return 0
    return len(value.split(";"))


def render_config_settings(template: str, job: QJob) -> str:
    """configSettings.h as run.py writes it (reconfigureSettings, then changeDim), plus extra_settings."""
    s = job.settings
    per_job = {"rootFileLoc": str(job.input_file), "rootTreeName": job.tree, "fileTag": job.combo_tag,
               "cwd": str(job.work_dir.resolve()), "override_nentries": 0 if s["nentries"] == -1 else 1}
    text = template
    for key, name, quoted in _RECONFIGURE:
        text = _sub(text, key, per_job[key] if name is None else s[name], quoted)
    for key, name, required in _DIMS:
        text = re.sub(r"const int " + re.escape(key) + r"=.*;",
                      lambda _m, k=key, n=name, r=required: f"const int {k}={_dim(s[n], r, n)};", text)
    for key, value in job.extra_settings.items():
        text = _sub(text, key, value, False)
    return text


def settings_py(job: QJob) -> str:
    lines = ["# Generated by gxana run qfactors from config/qfactors.yaml; run.py reads it via QFACTORS_SETTINGS.",
             f"rootFileLocs = [({str(job.input_file)!r}, {job.tree!r}, {job.file_tag!r})]"]
    lines += [f"_SET_{key} = {job.settings[key]!r}" for key in SETTINGS_KEYS]
    return "\n".join(lines) + "\n"


def _engine_files(engine_dir: Path) -> List[str]:
    if not (engine_dir / ".git").exists() or not (engine_dir / "main.C").is_file():
        raise QFactorsError(f"{engine_dir} is not a checked-out QFactors engine; "
                            "run `git submodule update --init packages/qfactors`")
    out = subprocess.run(["git", "-C", str(engine_dir), "ls-files", "-z"], capture_output=True, text=True)
    files = [f for f in out.stdout.split("\0") if f]
    if out.returncode != 0 or not files:
        raise QFactorsError(f"cannot list the engine files in {engine_dir}: {out.stderr.strip()}")
    return files


def _link(link: Path, target: Path) -> None:
    target.mkdir(parents=True, exist_ok=True)
    if link.is_symlink():
        link.unlink()
    elif link.exists():
        raise QFactorsError(f"{link.parent.name}/{link.name} is a real directory in {link.parent}; move it away "
                            f"(outputs belong in {target})")
    link.symlink_to(target, target_is_directory=True)


def stage(job: QJob) -> None:
    """Copy the engine, overlay the model and settings, link the output directories."""
    files = _engine_files(job.engine_dir)
    job.work_dir.mkdir(parents=True, exist_ok=True)
    for rel in files:
        dst = job.work_dir / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(job.engine_dir / rel, dst)
    if job.model != "configPDFs.h":  # the engine copy above already put configPDFs.h there
        shutil.copy2(job.pdf_config, job.work_dir / "configPDFs.h")
    (job.work_dir / "run_settings.py").write_text(settings_py(job))
    (job.work_dir / "makePlotsVars.txt").write_text("".join(f"{v}\n" for v in job.diagnostic_vars))
    template = (job.engine_dir / "configSettings.h").read_text()
    (job.work_dir / "configSettings.h").write_text(render_config_settings(template, job))
    for name, sub in _LINKS:
        target = job.output_dir if sub is None else job.plots_dir / sub
        _link(job.work_dir / name, target)
        (target / job.combo_tag).mkdir(parents=True, exist_ok=True)


def compile_main(job: QJob, runner: Runner = subprocess.run, syntax_only: bool = False) -> None:
    """Build main the way run.py does (g++ + root-config + RooFit libraries + extraLibs)."""
    flags = runner(["root-config", "--cflags"] if syntax_only else ["root-config", "--cflags", "--glibs", "--libs"],
                   capture_output=True, text=True)
    if flags.returncode != 0:
        raise QFactorsError("root-config failed; set up ROOT first (source env/setup.sh --gluex)")
    if syntax_only:
        argv = ["g++", "-fsyntax-only", "main.C", *flags.stdout.split()]
    else:
        argv = ["g++", "-o", "main", "main.C", *flags.stdout.split(), "-lRooStats", "-lRooFitCore", "-lRooFit",
                *job.settings["extraLibs"]]
    result = runner(argv, cwd=job.work_dir, capture_output=True, text=True)
    if result.returncode != 0:
        raise QFactorsError(f"main.C failed to compile with {job.pdf_config.name}:\n"
                            f"{(result.stdout or '') + (result.stderr or '')}"[-4000:])


def run_py_arg(steps: Sequence[str]) -> Optional[str]:
    unknown = [s for s in steps if s not in STEPS]
    if unknown:
        raise QFactorsError(f"unknown step(s) {unknown}; choose from {list(STEPS)}")
    if "fit" not in steps and "plots" not in steps:
        return None
    return ("1" if "fit" in steps else "0") + ("1" if "plots" in steps else "0")


def run_qfactors(job: QJob, steps: Sequence[str], environ: Optional[Mapping[str, str]] = None,
                 runner: Runner = subprocess.run, which: Callable[[str], Optional[str]] = shutil.which,
                 log: Callable[[str], None] = print) -> int:
    arg = run_py_arg(steps)
    if not job.input_file.is_file():
        raise QFactorsError(f"input flat tree not found: {job.input_file}")
    for tool in ("root-config", "g++") + (("hadd", "root") if arg else ()):
        if which(tool) is None:
            raise QFactorsError(f"{tool} not on PATH; set up ROOT first (source env/setup.sh --gluex)")
    stage(job)
    log(f"Staged {job.model} Q-factor run for {job.period} in {job.work_dir}")
    if "prepare" in steps:
        compile_main(job, runner)
        log(f"Compiled {job.work_dir / 'main'}; run ./main <iProcess> there by hand if needed")
    if arg is None:
        return 0
    env = dict(os.environ if environ is None else environ)
    env["QFACTORS_SETTINGS"] = str(job.work_dir / "run_settings.py")
    argv = [sys.executable, "run.py", arg]
    log(f"Running QFACTORS_SETTINGS={env['QFACTORS_SETTINGS']} {' '.join(argv[1:])} in {job.work_dir}")
    result = runner(argv, cwd=job.work_dir, env=env)
    if result.returncode == 0 and "plots" in steps:
        if not job.result.is_file():  # run.py exits 0 even when main, hadd or mergeQresults failed
            log(f"ERROR: run.py finished but {job.result} was not written; "
                f"read {job.output_dir / job.combo_tag}/err*.txt")
            return 1
        log(f"Result: {job.result}")
    return result.returncode
