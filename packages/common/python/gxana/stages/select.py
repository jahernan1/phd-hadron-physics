"""`gxana run select`: run a DSelector over a TChain with PROOF-Lite.

Python port of legacy runDSelector.sh (docs/REFACTOR_SPEC.md §9). Output
naming and file handling are unchanged; paths come from GXANA_* variables and
analyses/<channel>/config instead of being hardcoded. Changes from legacy:
paths come from GXANA_*, the job runs in $GXANA_SCRATCH/run/<save> instead of
the repo root, a non-zero `root` exit now skips all output moves entirely
(legacy would still move outputs after a PROOF-Lite failure if any existed),
and the stage exits 1 if nothing was produced.
"""
from __future__ import annotations

import os
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Dict, List, Mapping, Optional, Tuple

from gxana import config
from gxana.paths import env_path, repo_root

Runner = Callable[..., subprocess.CompletedProcess]
STALE_PREFIXES = ("", "thrown_", "flatTree_", "flatTree_thrown_")


class SelectError(RuntimeError):
    """The selector job cannot run as configured."""


@dataclass(frozen=True)
class SelectJob:
    channel: str
    tree_dir: Path
    selector: Path
    output_basename: str
    save_name: str
    cores: int
    run_dir: Path
    sandbox: Path
    hist_dir: Path
    flat_dir: Path
    thrown_dir: Path


def plan_select(
    cfg: Dict[str, Any],
    period: str,
    sample: str,
    *,
    thrown: bool = False,
    tag: Optional[str] = None,
    cores: int = 16,
    selector: Optional[str] = None,
    environ: Optional[Mapping[str, str]] = None,
    root: Optional[Path] = None,
) -> SelectJob:
    kind = "thrown" if thrown else "trees"
    rel_tree = config.tree_dir(cfg, period, sample, kind)
    tree_parent = Path(rel_tree.rstrip("/")).parent.name  # tree_<reaction>__...
    if not tree_parent.startswith("tree_"):
        raise SelectError(f"tree directory {rel_tree!r} must be inside a 'tree_*' directory")
    save_name = tree_parent[len("tree_"):] + (f"_{tag}" if tag else "")
    if selector:
        selector_path = Path(selector).resolve()
    else:
        selector_path = ((root or repo_root()) / config.require(cfg, "selector_dir")
                          / config.selector_name(cfg, sample, thrown)).resolve()
    channel = config.require(cfg, "channel")
    return SelectJob(
        channel=channel,
        # Resolved to absolute: ROOT (root_script) runs with cwd=run_dir, so any
        # relative GXANA_* value or --selector path must not depend on that cwd.
        tree_dir=env_path("GXANA_DATA", rel_tree, environ=environ).resolve(),
        selector=selector_path,
        output_basename=config.output_basename(cfg, sample, thrown),
        save_name=save_name,
        cores=cores,
        run_dir=env_path("GXANA_SCRATCH", "run", save_name, environ=environ).resolve(),
        sandbox=env_path("GXANA_SCRATCH", "proof", environ=environ).resolve(),
        hist_dir=env_path("GXANA_OUTPUT", channel, "selector_hists", environ=environ).resolve(),
        flat_dir=env_path("GXANA_DATA", "Trees", "flatTree", "rawTrees", environ=environ).resolve(),
        thrown_dir=env_path("GXANA_DATA", "flatTrees", environ=environ).resolve(),
    )


def root_script(job: SelectJob, tree_name: str, root_analysis_home: str) -> str:
    return (
        f'gEnv->SetValue("ProofLite.Sandbox", "{job.sandbox}");\n'
        f".x {root_analysis_home}/scripts/Load_DSelector.C\n"
        f'TChain *ch = new TChain("{tree_name}");\n'
        f'ch->Add("{job.tree_dir}/*");\n'
        f'DPROOFLiteManager::Process_Chain(ch, "{job.selector}++", {job.cores});\n'
    )


def tree_name(first_file: Path, runner: Runner = subprocess.run) -> str:
    try:
        result = runner(["rootls", str(first_file)], capture_output=True, text=True, check=True)
    except FileNotFoundError as exc:
        raise SelectError(f"rootls not found; is ROOT on PATH? ({exc})") from exc
    except subprocess.CalledProcessError as exc:
        raise SelectError(f"rootls failed on {first_file}: {exc}") from exc
    keys = result.stdout.split()
    if len(keys) != 1:
        raise SelectError(f"{first_file}: expected exactly one key, rootls gave {keys}")
    return keys[0]


def planned_moves(job: SelectJob) -> List[Tuple[Path, Path]]:
    out, run, save = job.output_basename, job.run_dir, job.save_name
    moves = []
    if (run / out).is_file():
        moves.append((run / out, job.hist_dir / f"{save}.root"))
    elif (run / f"thrown_{out}").is_file():
        moves.append((run / f"thrown_{out}", job.hist_dir / f"thrown_{save}.root"))
    if (run / f"flatTree_{out}").is_file():
        moves.append((run / f"flatTree_{out}", job.flat_dir / f"flatTree_{save}.root"))
    elif (run / f"flatTree_thrown_{out}").is_file():
        # Thrown flat trees skip flatTreePrep.C; they go straight to $GXANA_DATA/flatTrees/,
        # where xsection, systematics track, the measurements and the MC macros read them.
        moves.append((run / f"flatTree_thrown_{out}", job.thrown_dir / f"flatTree_thrown_{save}.root"))
    return moves


def place(src: Path, dst: Path) -> None:
    """Move src to dst, replacing dst. A symlinked dst (`gxana data stage` links the thrown trees to
    the preserved data) is replaced, never written through, also when the move falls back to a copy."""
    if dst.is_symlink():
        dst.unlink()
    part = dst.with_name(dst.name + ".part")
    if part.is_symlink() or part.exists():
        part.unlink()
    try:
        shutil.move(str(src), str(part))
        os.replace(part, dst)
    finally:
        if part.exists() or part.is_symlink():
            part.unlink()


def run_select(
    job: SelectJob,
    environ: Optional[Mapping[str, str]] = None,
    runner: Runner = subprocess.run,
    log: Callable[[str], None] = print,
) -> int:
    env = os.environ if environ is None else environ
    rah = env.get("ROOT_ANALYSIS_HOME")
    if not rah:
        raise SelectError("ROOT_ANALYSIS_HOME is not set; run `source env/setup.sh --gluex`")
    if not job.selector.is_file():
        raise SelectError(f"selector not found: {job.selector}")
    if not job.tree_dir.is_dir():
        raise SelectError(f"tree directory not found: {job.tree_dir}")
    files = sorted(p for p in job.tree_dir.iterdir() if p.is_file())
    if not files:
        raise SelectError(f"no files in {job.tree_dir}")
    name = tree_name(files[0], runner)
    log(f"Will process {len(files)} file(s) from {job.tree_dir} (tree {name})")

    job.run_dir.mkdir(parents=True, exist_ok=True)
    for prefix in STALE_PREFIXES:
        stale = job.run_dir / f"{prefix}{job.output_basename}"
        if stale.is_file():
            stale.unlink()
            log(f"Removed {stale}")
    job.sandbox.mkdir(parents=True, exist_ok=True)

    try:
        result = runner(["root", "-l", "-b"], input=root_script(job, name, rah), text=True, cwd=job.run_dir)
    except FileNotFoundError as exc:
        raise SelectError(f"root not found; is ROOT on PATH? ({exc})") from exc
    except subprocess.CalledProcessError as exc:
        raise SelectError(f"root failed: {exc}") from exc
    if result.returncode != 0:
        log(f"root exited with status {result.returncode}; nothing saved")
        return result.returncode

    moves = planned_moves(job)
    if not moves:
        log(f"No output files found in {job.run_dir}; nothing saved")
        return 1
    for src, dst in moves:
        dst.parent.mkdir(parents=True, exist_ok=True)
        place(src, dst)
        log(f"Saved {dst}")
    return 0
