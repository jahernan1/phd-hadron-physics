"""Record what `gxana run` does, so a refactor can prove it changed nothing.

For the kpkpxim config and a synthetic second channel (`synthch`: other period
names and labels, one period fewer, an energy edge with three decimals), each
with GXANA_ROOT pointing at a tree with and without build/bin/<exe>, it records

  plans   the return value of every stage planner (xsection, barlow,
          systematics, select, mc, qfactors), each step alone, the default and
          the full step list, every systematics study alone and all together,
          the systematics runtime planner on an empty and a populated output
          tree, and the unknown-step errors;
  dryrun  stdout, stderr and exit code of `gxana run <stage> ... --dry-run`;
  trace   run_xsection / run_barlow / run_systematics with a fake runner that
          records (argv, kwargs) and fails at command k (never, 1, last), plus
          every file and directory each run created.

Temporary paths, the repository path and sys.executable are replaced by
<TMP>, <REPO> and <PYTHON>, so the fixtures do not depend on the machine.

  python tests/stage_plans/record.py              compare with baseline/ (exit 1 + diff if different)
  python tests/stage_plans/record.py --out DIR    also write the fresh recording to DIR
  python tests/stage_plans/record.py --write      rewrite baseline/*.json.gz (only on the unmodified code)
"""
from __future__ import annotations

import argparse
import contextlib
import dataclasses
import difflib
import gzip
import hashlib
import io
import json
import os
import shlex
import sys
import tempfile
from pathlib import Path
from typing import Any, Callable, Dict, Iterator, List, Optional, Sequence, Tuple

import yaml

from gxana import cli, config
from gxana.stages import mc, qfactors, select
from gxana.stages import xsection as xs
from gxana_barlow import config as bconfig
from gxana_barlow import manifest
from gxana_barlow import stage as bst
from gxana_barlow.variations import expand
from gxana_systematics import config as sconfig
from gxana_systematics import stage as sst

HERE = Path(__file__).resolve().parent
BASELINE = HERE / "baseline"
REPO = HERE.parents[1]
CHANNELS = ("kpkpxim", "synthch")
ROOTS = ("bin", "nobin")
KINDS = ("plans", "dryrun", "trace")
FIXTURE_NAMES = tuple(f"{root}_{channel}_{kind}.json" for root in ROOTS for channel in CHANNELS for kind in KINDS)
EXES = ("gxana_xsec_bin", "gxana_xsec_tables", "gxana_barlow_trees", "gxana_barlow_plot",
        "gxana_syst_plot", "gxana_syst_track")
SYNTH_EDGES = [6.40, 7.40, 7.855, 8.45, 9.26, 10.18, 11.40]


# ---------------------------------------------------------------- fixture trees

def _synth_text(text: str) -> str:
    return text.replace("kpkpxim", "synthch").replace("johnson", "jsyn").replace("2018-01", "2019-11")


def _make_root(base: Path, with_bin: bool) -> Path:
    """A GXANA_ROOT with both channel configs, a stub QFactors engine and, if with_bin, build/bin stubs."""
    root = base / ("root_bin" if with_bin else "root_nobin")
    for channel in CHANNELS:
        (root / "analyses" / channel / "config").mkdir(parents=True)
    for src in sorted((REPO / "analyses" / "kpkpxim" / "config").glob("*.yaml")):
        text = src.read_text()
        (root / "analyses" / "kpkpxim" / "config" / src.name).write_text(text)
        data = yaml.safe_load(_synth_text(text)) or {}
        if src.name == "binning.yaml":
            data["energy_edges"] = list(SYNTH_EDGES)
        if src.name == "periods.yaml":
            del data["periods"]["2017-01"]
        (root / "analyses" / "synthch" / "config" / src.name).write_text(yaml.safe_dump(data, sort_keys=False))
    engine = root / "packages" / "qfactors"
    engine.mkdir(parents=True)
    for name in ("main.C", "configPDFs.h", "configPDFs_Johnson.h"):
        (engine / name).write_text("")
    if with_bin:
        (root / "build" / "bin").mkdir(parents=True)
        for exe in EXES:
            (root / "build" / "bin" / exe).write_text("")
    return root


def _env(base: Path, root: Path) -> Dict[str, str]:
    return {"GXANA_ROOT": str(root), "GXANA_DATA": str(base / "data"), "GXANA_OUTPUT": str(base / "out"),
            "GXANA_SCRATCH": str(base / "scratch"), "GXANA_EXTERNALS": str(base / "ext"),
            "GXANA_ANALYSIS_DATA": str(base / "analysis_data")}


@contextlib.contextmanager
def _os_environ(env: Dict[str, str]) -> Iterator[None]:
    saved = dict(os.environ)
    for key in list(os.environ):
        if key.startswith("GXANA_") or key == "ROOT_ANALYSIS_HOME":
            del os.environ[key]
    os.environ.update(env)
    try:
        yield
    finally:
        os.environ.clear()
        os.environ.update(saved)


def _touch(path: str) -> None:
    p = Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    if not p.exists():
        p.write_text("")


def _stems(cfg: Dict[str, Any]) -> List[str]:
    return [config.tree_stem(cfg, period, "data") for period in cfg["periods"]]


def _bins(cfg: Dict[str, Any]) -> List[Tuple[str, str]]:
    edges = cfg["energy_edges"]
    return [(f"{lo:.2f}", f"{hi:.2f}") for lo, hi in zip(edges, edges[1:])]


def _materialize(entry: str, cfg: Dict[str, Any]) -> None:
    """Create the file(s) a systematics preflight / nominal-guard entry asks for."""
    path = entry.split(" (", 1)[0]
    directory, _, name = path.rpartition("/")
    if "*" not in name:
        _touch(path)
        return
    if name.startswith("weighted_diffxsec"):
        names = [f"weighted_diffxsec_emin_{lo}_emax_{hi}.txt" for lo, hi in _bins(cfg)]
    else:  # diffout*.txt, diffxsec*.txt, diffxsec*_emin_*.txt
        names = [f"{kind}_flatTree_{stem}_emin_{lo}_emax_{hi}.txt"
                 for kind in ("diffout", "diffxsec") for stem in _stems(cfg) for lo, hi in _bins(cfg)]
    for n in names:
        _touch(f"{directory}/{n}")


def _populate_barlow(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    variations = expand(bconfig.block(cfg))
    for step in bst.STEPS:
        for path in bst.preflight(cfg, step, variations, env):
            _touch(path)


def _populate_systematics(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    chosen = sst.selected(cfg, None)[0]
    for entry in sst.nominal_missing(cfg, list(sst.STEPS), chosen, None, env):
        _materialize(entry, cfg)
    for step in sst.STEPS:
        for entry in sst.preflight(cfg, step, None, env):
            _materialize(entry, cfg)


def _populate_tex(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    for path in cfg["xsection"]["tex"]["columns"].values():
        _touch(config.expand_env(path, env))


# ---------------------------------------------------------------- serialising

def _plain(value: Any) -> Any:
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, dict):
        return {str(k): _plain(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return [_plain(v) for v in value]
    return value


def _cmds(commands: Sequence[Any]) -> List[List[Any]]:
    # Barlow's Command had no cwd field before the port: record None for it.
    return [[shlex.join(c.argv), c.step, getattr(c, "cwd", None)] for c in commands]


def _job(job: Any) -> Dict[str, Any]:
    fields = job._asdict() if hasattr(job, "_asdict") else dataclasses.asdict(job)
    return _plain(dict(fields))


def _call(fn: Callable[[], Any]) -> Dict[str, Any]:
    out, err = io.StringIO(), io.StringIO()
    value, error = None, None
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        try:
            value = fn()
        except (Exception, SystemExit) as exc:  # errors are part of the recorded behaviour
            error = f"{type(exc).__name__}: {exc}"
    return {"value": value, "error": error, "stdout": out.getvalue(), "stderr": err.getvalue()}


def _listing(base: Path) -> Dict[str, Optional[str]]:
    entries: Dict[str, Optional[str]] = {}
    for p in sorted(base.rglob("*")):
        rel = p.relative_to(base).as_posix()
        if p.is_dir():
            entries[rel + "/"] = None
        else:
            data = p.read_bytes()
            entries[rel] = data.decode() if len(data) < 4096 else "sha256:" + hashlib.sha256(data).hexdigest()
    return entries


def _step_lists(steps: Sequence[str], default: Sequence[str]) -> List[List[str]]:
    return [[s] for s in steps] + [list(default), list(steps)]


# ---------------------------------------------------------------- recordings

def _plans(cfg: Dict[str, Any], env: Dict[str, str], populated_env: Dict[str, str]) -> Dict[str, Any]:
    rec: Dict[str, Any] = {}
    for steps in _step_lists(xs.STEPS, xs.DEFAULT_STEPS) + [["zzz", "plot"], ["tables", "nope"]]:
        rec[f"xsection {','.join(steps)}"] = _call(lambda: _cmds(xs.plan_xsection(cfg, steps, environ=env)))
    variations = expand(bconfig.block(cfg))
    for steps in _step_lists(bst.STEPS, bst.DEFAULT_STEPS) + [["zzz", "barlow"]]:
        rec[f"barlow {','.join(steps)}"] = _call(lambda: _cmds(bst.plan(cfg, steps, variations, environ=env)))
    for steps in _step_lists(sst.STEPS, sst.DEFAULT_STEPS) + [["zzz", "bin"], ["fit", "tables"], ["nope"]]:
        rec[f"systematics {','.join(steps)}"] = _call(lambda: _cmds(sst.plan(cfg, steps, None, env)))
    for name, _ in sconfig.studies(sconfig.block(cfg)):
        rec[f"systematics study {name}"] = _call(lambda: _cmds(sst.plan(cfg, list(sst.STEPS), [name], env)))
    for label, e in (("empty", env), ("populated", populated_env)):
        rec[f"systematics runtime {label}"] = _call(
            lambda: _cmds(sst.plan(cfg, list(sst.STEPS), None, e, runtime=True)))
    mc_sample = cfg["xsection"]["mc_sample"]
    for period in cfg["periods"]:
        for sample, thrown in (("data", False), (mc_sample, False), (mc_sample, True)):
            rec[f"select {period} {sample} thrown={thrown}"] = _call(
                lambda: _job(select.plan_select(cfg, period, sample, thrown=thrown, environ=env)))
        rec[f"qfactors {period}"] = _call(lambda: _job(qfactors.plan_qfactors(cfg, period, environ=env)))
    for period in cfg["mc"]["periods"]:
        for sample in cfg["mc"]["samples"]:
            rec[f"mc {period} {sample}"] = _call(lambda: _job(mc.plan_mc(cfg, period, sample, environ=env)))
    return rec


def _cli_cases(cfg: Dict[str, Any], channel: str) -> List[List[str]]:
    c = ["--channel", channel]
    cases: List[List[str]] = []
    for stage, mod in (("xsection", xs), ("barlow", bst), ("systematics", sst)):
        cases.append(["run", stage, *c, "--dry-run"])
        cases.append(["run", stage, *c, "--steps", ",".join(mod.STEPS), "--dry-run"])
    for name, _ in sconfig.studies(sconfig.block(cfg)):
        cases.append(["run", "systematics", *c, "--steps", ",".join(sst.STEPS), "--study", name, "--dry-run"])
    cases += [["run", "xsection", *c, "--steps", "zzz,plot", "--dry-run"],
              ["run", "barlow", *c, "--steps", "zzz,barlow", "--dry-run"],
              ["run", "systematics", *c, "--steps", "zzz,bin", "--dry-run"]]
    mc_sample = cfg["xsection"]["mc_sample"]
    for period in cfg["periods"]:
        cases.append(["run", "select", *c, "--period", period, "--dry-run"])
        cases.append(["run", "qfactors", *c, "--period", period, "--dry-run"])
        cases.append(["run", "qfactors", *c, "--period", period, "--steps", "prepare,fit,plots", "--dry-run"])
    cases.append(["run", "select", *c, "--period", next(iter(cfg["periods"])), "--sample", mc_sample,
                  "--thrown", "--dry-run"])
    for period in cfg["mc"]["periods"]:
        for sample in cfg["mc"]["samples"]:
            cases.append(["run", "mc", *c, "--period", period, "--sample", sample, "--dry-run"])
    return cases


def _dryrun(cfg: Dict[str, Any], channel: str) -> Dict[str, Any]:
    rec: Dict[str, Any] = {}
    for argv in _cli_cases(cfg, channel):
        rec[" ".join(argv)] = _call(lambda: cli.main(argv))
    return rec


class _Result:
    def __init__(self, returncode: int) -> None:
        self.returncode = returncode


def _fake_runner(calls: List[Any], fail_at: Optional[int]) -> Callable[..., _Result]:
    def run(argv: Sequence[str], **kwargs: Any) -> _Result:
        calls.append([shlex.join(argv), _plain(dict(sorted(kwargs.items())))])
        return _Result(3 if fail_at is not None and len(calls) == fail_at else 0)
    return run


def _trace_one(base: Path, root: Path, channel: str, name: str,
               setup: Callable[[Dict[str, Any], Dict[str, str]], None],
               run: Callable[[Dict[str, Any], Dict[str, str], Callable[..., _Result]], int],
               fail_at: Optional[int]) -> Dict[str, Any]:
    tdir = base / "trace" / name
    tdir.mkdir(parents=True)
    env = _env(tdir, root)
    calls: List[Any] = []
    with _os_environ(env):
        cfg = config.load_channel(channel, root=root)
        setup(cfg, env)
        before = _listing(tdir)
        result = _call(lambda: run(cfg, env, _fake_runner(calls, fail_at)))
        after = _listing(tdir)
    created = {k: v for k, v in after.items() if k not in before or before[k] != v}
    return {**result, "fail_at": fail_at, "calls": calls, "created": created}


def _nothing(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    return None


def _stale_manifest(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    out = Path(config.expand_env(bconfig.block(cfg)["output_dir"], env))
    manifest.write(out, {"config_hash": "stale", "variations": []})


def _nominal_only(cfg: Dict[str, Any], env: Dict[str, str]) -> None:
    chosen = sst.selected(cfg, None)[0]
    for entry in sst.nominal_missing(cfg, list(sst.STEPS), chosen, None, env):
        _materialize(entry, cfg)


def _trace(base: Path, root: Path, channel: str) -> Dict[str, Any]:
    def xsec(steps: Sequence[str], dry_run: bool = False):
        return lambda cfg, env, runner: xs.run_xsection(cfg, list(steps), dry_run=dry_run, runner=runner, environ=env)

    def barlow(steps: Sequence[str], dry_run: bool = False):
        return lambda cfg, env, runner: bst.run_barlow(cfg, list(steps), dry_run=dry_run, runner=runner, environ=env)

    def syst(steps: Sequence[str], dry_run: bool = False):
        return lambda cfg, env, runner: sst.run_systematics(cfg, list(steps), dry_run=dry_run, runner=runner,
                                                             environ=env)

    full = [
        ("xsection_default", _nothing, xsec(xs.DEFAULT_STEPS)),
        ("barlow_all", _populate_barlow, barlow(bst.STEPS)),
        ("systematics_all", _populate_systematics, syst(sst.STEPS)),
    ]
    single = [
        ("xsection_no_steps", _nothing, xsec([])),
        ("xsection_tex_missing", _nothing, xsec(["tex"])),
        ("xsection_tex_present", _populate_tex, xsec(["tex"])),
        ("xsection_dry_all", _nothing, xsec(xs.STEPS, dry_run=True)),
        ("barlow_bin_empty", _nothing, barlow(["bin"])),
        ("barlow_weight_stale", _stale_manifest, barlow(["weight"])),
        ("barlow_dry_all", _nothing, barlow(bst.STEPS, dry_run=True)),
        ("systematics_default_empty", _nothing, syst(sst.DEFAULT_STEPS)),
        ("systematics_fit_nominal_only", _nominal_only, syst(["fit"])),
        ("systematics_dry_all", _nothing, syst(sst.STEPS, dry_run=True)),
    ]
    rec: Dict[str, Any] = {}
    for name, setup, run in full:
        first = _trace_one(base, root, channel, f"{name}_never", setup, run, None)
        rec[f"{name} never"] = first
        n = len(first["calls"])
        for label, k in (("first", 1), ("last", n)):
            failed = _trace_one(base, root, channel, f"{name}_{label}", setup, run, k)
            # Up to the failure the calls and stdout are those of the full run (in another
            # directory): keep where it stopped instead of a second copy.
            calls = failed.pop("calls")
            stdout = failed.pop("stdout").splitlines()
            failed["n_calls"] = len(calls)
            failed["last_call"] = calls[-1] if calls else None
            failed["stdout_lines"] = len(stdout)
            failed["stdout_last"] = stdout[-1] if stdout else None
            rec[f"{name} {label}"] = failed
    for name, setup, run in single:
        rec[name] = _trace_one(base, root, channel, name, setup, run, None)
    return rec


# ---------------------------------------------------------------- driver

def _scrub(text: str, base: Path) -> str:
    for raw, placeholder in ((sys.executable, "<PYTHON>"), (str(base.resolve()), "<TMP>"), (str(base), "<TMP>"),
                             (str(REPO), "<REPO>")):
        text = text.replace(raw, placeholder)
    return text


def record_all() -> Dict[str, str]:
    """{fixture file name: JSON text} for the current code."""
    texts: Dict[str, str] = {}
    with tempfile.TemporaryDirectory(prefix="stage_plans_") as tmp:
        base = Path(tmp).resolve()
        for root_name in ROOTS:
            root = _make_root(base, with_bin=(root_name == "bin"))
            for channel in CHANNELS:
                work = base / f"{root_name}_{channel}"
                env = _env(work, root)
                populated = _env(work / "populated", root)
                with _os_environ(env):
                    cfg = config.load_channel(channel, root=root)
                    _populate_systematics(cfg, populated)
                    rec = {
                        "plans": _plans(cfg, env, populated),
                        "dryrun": _dryrun(cfg, channel),
                    }
                rec["trace"] = _trace(work, root, channel)
                for kind in KINDS:
                    text = json.dumps(rec[kind], indent=1, ensure_ascii=False) + "\n"
                    texts[f"{root_name}_{channel}_{kind}.json"] = _scrub(text, base)
    return texts


def baseline_text(name: str) -> str:
    """The recorded fixture `name` (stored gzipped as baseline/<name>.gz), or "" if absent."""
    path = BASELINE / f"{name}.gz"
    return gzip.decompress(path.read_bytes()).decode() if path.is_file() else ""


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--write", action="store_true", help="rewrite baseline/ from the current code")
    parser.add_argument("--out", type=Path, help="also write the fresh recording to this directory")
    args = parser.parse_args(argv)
    texts = record_all()
    if args.out:
        args.out.mkdir(parents=True, exist_ok=True)
        for name, text in texts.items():
            (args.out / name).write_text(text)
    if args.write:
        BASELINE.mkdir(exist_ok=True)
        for name, text in texts.items():
            (BASELINE / f"{name}.gz").write_bytes(gzip.compress(text.encode(), compresslevel=9, mtime=0))
        print(f"wrote {len(texts)} fixtures to {BASELINE}")
        return 0
    different = 0
    for name in FIXTURE_NAMES:
        expected = baseline_text(name)
        if texts[name] != expected:
            different += 1
            sys.stdout.writelines(difflib.unified_diff(
                expected.splitlines(keepends=True), texts[name].splitlines(keepends=True),
                f"baseline/{name}", f"recorded/{name}", n=2))
    print(f"{len(FIXTURE_NAMES) - different} of {len(FIXTURE_NAMES)} fixtures identical")
    return 1 if different else 0


if __name__ == "__main__":
    sys.exit(main())
