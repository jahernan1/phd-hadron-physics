"""`gxana run mc`: render the thesis MCwrapper inputs for one period/sample
and submit gluex_MC.py (replaces the ifarm runAllMC.sh; config/mc.yaml).

The MCwrapper conf templates are the as-run thesis confs with ${GXANA_*}
paths; the stage expands them and overrides the per-job keys
(DATA_OUTPUT_BASE_DIR, GENERATOR_CONFIG, ENVIRONMENT_FILE, WORKFLOW_NAME,
plus per-sample overrides such as BKG). MCwrapper writes
<DATA_OUTPUT_BASE_DIR>/root/{trees,thrown}/; the printed ln -s commands put
them where `gxana run select` reads (tree_dir_template).
"""
from __future__ import annotations

import os
import re
import shlex
import shutil
import subprocess
from pathlib import Path
from typing import Any, Callable, Dict, List, Mapping, NamedTuple, Optional, Tuple

from gxana import config
from gxana.paths import env_path, gxana_root

Runner = Callable[..., subprocess.CompletedProcess]
_KEY_LINE = r"^(\s*{key}\s*=\s*)([^#\n]*?)(\s*(?:#.*)?)$"


class McError(RuntimeError):
    """The MC job cannot be rendered or submitted."""


class McJob(NamedTuple):
    period: str
    sample: str
    stem: str
    run_dir: Path
    cfg_template: Path
    conf_template: Path
    generator_config: Path
    conf: Path
    version_set: str
    version_set_xml: Path
    overrides: Dict[str, str]
    argv: List[str]
    links: List[Tuple[Path, Path]]


def override_keys(text: str, values: Mapping[str, str]) -> str:
    """Set KEY=value on the one uncommented KEY line, keeping its trailing comment."""
    out = text
    for key, value in values.items():
        pattern = re.compile(_KEY_LINE.format(key=re.escape(key)), re.M)
        out, count = pattern.subn(lambda m: m.group(1) + value + m.group(3), out)
        if count != 1:
            raise McError(f"expected one {key}= line in the MCwrapper conf, found {count}")
    return out


def plan_mc(cfg: Dict[str, Any], period: str, sample: str,
            environ: Optional[Mapping[str, str]] = None) -> McJob:
    mc = config.require(cfg, "mc")
    p = config.period_settings(cfg, period)
    if period not in mc["periods"]:
        raise config.ConfigError(f"no MC production configured for period {period!r}; mc.periods: {sorted(mc['periods'])}")
    if sample not in mc["samples"]:
        raise config.ConfigError(f"no MC production for sample {sample!r} in mc.yaml; known: {sorted(mc['samples'])}")
    mp, ms = mc["periods"][period], mc["samples"][sample]
    if "periods" in ms and period not in ms["periods"]:
        raise config.ConfigError(f"sample {sample!r} is produced only for {ms['periods']}")
    channel = config.require(cfg, "channel")
    stem = config.tree_stem(cfg, period, sample)
    sim = gxana_root(environ) / mc["simulation_dir"]
    run_dir = env_path("GXANA_OUTPUT", channel, "mc", f"tree_{stem}", environ=environ)
    version_set = mp["sim_version_set"]
    version_set_xml = env_path("GXANA_EXTERNALS", "version_sets", f"{version_set}.xml", environ=environ)
    generator_config = run_dir / ms["generator_config"]
    conf = run_dir / mp["conf"]
    overrides = {
        "DATA_OUTPUT_BASE_DIR": str(run_dir),
        "GENERATOR_CONFIG": str(generator_config),
        "ENVIRONMENT_FILE": str(version_set_xml),
        "WORKFLOW_NAME": f"{channel}_{period}_{sample}",
    }
    overrides.update({str(k): str(v) for k, v in (ms.get("overrides") or {}).items()})
    lo, hi = p["runs"]
    argv = ["gluex_MC.py", str(conf), f"{lo}-{hi}", str(mp["events"]), f"batch={mc['batch']}"]
    data = env_path("GXANA_DATA", environ=environ)
    links = [(data / config.tree_dir(cfg, period, sample, kind).rstrip("/"), run_dir / "root" / kind)
             for kind in config.TREE_KINDS]
    return McJob(period, sample, stem, run_dir, sim / "gen_amp_cfg" / ms["generator_config"],
                 sim / "mcwrapper" / mp["conf"], generator_config, conf, version_set, version_set_xml,
                 overrides, argv, links)


def _read_template(path: Path) -> str:
    try:
        return path.read_text()
    except FileNotFoundError as err:
        raise McError(f"MCwrapper template not found: {path}") from err


def render(job: McJob, environ: Optional[Mapping[str, str]] = None) -> Dict[Path, str]:
    cfg_text = config.expand_env(_read_template(job.cfg_template), environ)
    conf_text = override_keys(config.expand_env(_read_template(job.conf_template), environ), job.overrides)
    return {job.generator_config: cfg_text, job.conf: conf_text}


def link_commands(job: McJob) -> List[str]:
    return [f"mkdir -p {shlex.quote(str(link.parent))} && "
            f"ln -sfn {shlex.quote(str(target))} {shlex.quote(str(link))}" for link, target in job.links]


def run_mc(job: McJob, environ: Optional[Mapping[str, str]] = None, runner: Runner = subprocess.run,
           which: Callable[[str], Optional[str]] = shutil.which, log: Callable[[str], None] = print) -> int:
    env = os.environ if environ is None else environ
    active = env.get("GXANA_SIM_VERSION_SET")
    if active != job.version_set:
        raise McError(f"{job.period} MC needs the {job.version_set} environment (active: {active or 'none'}); "
                      f"run `source env/setup.sh --sim={job.version_set}`")
    if not job.version_set_xml.is_file():
        raise McError(f"{job.version_set_xml} missing; run `source env/setup.sh --sim={job.version_set}`")
    if which("gluex_MC.py") is None:
        raise McError("gluex_MC.py not on PATH; the sim environment provides it via MCWRAPPER_CENTRAL "
                      "(gxana externals fetch gluex_MCwrapper)")
    files = render(job, env)
    job.run_dir.mkdir(parents=True, exist_ok=True)
    for path, text in files.items():
        path.write_text(text)
    log(f"Submitting {job.sample} {job.period}: {' '.join(job.argv)}")
    result = runner(job.argv, cwd=job.run_dir)
    if result.returncode == 0:
        log("When the jobs finish, link the outputs where `gxana run select` reads them:")
        for line in link_commands(job):
            log(f"  {line}")
    return result.returncode
