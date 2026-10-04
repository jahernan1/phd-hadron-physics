#!/usr/bin/env python3
"""Re-run hd_root with a ReactionFilter over MCwrapper REST files, one worker per run number.

Generalized from the thesis-era AnalysisNote/MC/run_hdroot.py, whose
directories and hd_root settings were hardcoded (the defaults below are
those values). Writes <out-dir>/{monitoring_hists,trees,thrown}/ and
<out-dir>/all_runs_{stdout,stderr}.log.
"""
from __future__ import annotations

import argparse
import glob
import os
import shutil
import subprocess
import sys
from multiprocessing import Lock, Pool, Value
from typing import List, Optional, Sequence

_state = {}


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=(__doc__ or "").partition("\n")[0])
    p.add_argument("--mc-dir", required=True, help="MCwrapper output dir holding hddm/dana_rest_*.hddm")
    p.add_argument("--out-dir", required=True)
    p.add_argument("--reaction", default="1_14__11_11_23")
    p.add_argument("--flags", default="B4_U1_M23", help="Reaction1:Flags")
    p.add_argument("--plugins", default="ReactionFilter,mcthrown_tree")
    p.add_argument("--generator", default="gen_amp_V2")
    p.add_argument("--tree-prefix", default="kpkpxim", help="tree name = <prefix>__<flags>")
    p.add_argument("--nthreads", type=int, default=1, help="hd_root threads")
    p.add_argument("--processes", type=int, default=16, help="parallel run numbers")
    args = p.parse_args(argv)
    args.mc_dir = os.path.abspath(args.mc_dir)
    args.out_dir = os.path.abspath(args.out_dir)
    return args


def rest_files(s: argparse.Namespace, run_number: Optional[str] = None) -> List[str]:
    if run_number is None:
        return sorted(glob.glob(f"{s.mc_dir}/hddm/dana_rest_{s.generator}*.hddm"))
    return sorted(glob.glob(f"{s.mc_dir}/hddm/dana_rest_{s.generator}_{run_number}_???.hddm"))


def run_numbers(files: Sequence[str], generator: str) -> List[str]:
    start = len("dana_rest_" + generator) + 1
    return sorted({os.path.basename(f)[start:start + 6] for f in files})


def hdroot_command(s: argparse.Namespace, fname: str) -> str:
    return (f"hd_root --nthreads={s.nthreads} -PPLUGINS={s.plugins} -PReaction1={s.reaction} "
            f"-PReaction1:Flags={s.flags} {fname}")


def move_commands(s: argparse.Namespace, run_number: str, fname: str) -> List[str]:
    tree, idx = f"{s.tree_prefix}__{s.flags}", fname[-8:-5]
    return [
        f"mv hd_root.root ../monitoring_hists/hd_root_{s.generator}_{run_number}_{idx}.root",
        f"mv tree_{tree}.root ../trees/tree_{tree}_{s.generator}_{run_number}_{idx}.root",
        f"mv tree_thrown.root ../thrown/tree_thrown_{s.generator}_{run_number}_{idx}.root",
    ]


def _init(settings, completed, lock, total):
    _state.update(s=settings, completed=completed, lock=lock, total=total)


def _run_command(command: str) -> int:
    s = _state["s"]
    with open(os.path.join(s.out_dir, "all_runs_stdout.log"), "a") as out, \
         open(os.path.join(s.out_dir, "all_runs_stderr.log"), "a") as err:
        out.write(command + "\n")
        out.flush()
        result = subprocess.call(command, shell=True, stdout=out, stderr=err)
        if result != 0:
            err.write(f"\nCommand failed with return code {result}: {command}\n")
    return result


def _show_progress() -> None:
    with _state["lock"]:
        _state["completed"].value += 1
        done, total = _state["completed"].value, _state["total"]
        sys.stdout.write(f"\rProgress: {100.0 * done / total:.2f}% ({done} of {total})")
        sys.stdout.flush()


def _run_over_rest_files(run_number: str) -> None:
    s = _state["s"]
    work = os.path.join(s.out_dir, run_number)
    os.makedirs(work, exist_ok=True)
    os.chdir(work)
    for fname in rest_files(s, run_number):
        _run_command(hdroot_command(s, fname))
        for cmd in move_commands(s, run_number, fname):
            _run_command(cmd)
        _show_progress()
    os.chdir(s.out_dir)
    shutil.rmtree(work)


def main(argv: Optional[Sequence[str]] = None) -> int:
    s = parse_args(argv)
    files = rest_files(s)
    if not files:
        print(f"no {s.mc_dir}/hddm/dana_rest_{s.generator}*.hddm files", file=sys.stderr)
        return 1
    os.makedirs(s.out_dir, exist_ok=True)
    for name in ("monitoring_hists", "trees", "thrown"):
        path = os.path.join(s.out_dir, name)
        if os.path.exists(path):
            shutil.rmtree(path)
        os.mkdir(path)
    for log in ("all_runs_stdout.log", "all_runs_stderr.log"):
        path = os.path.join(s.out_dir, log)
        if os.path.exists(path):
            os.remove(path)
    completed, lock = Value("i", 0), Lock()
    with Pool(s.processes, initializer=_init, initargs=(s, completed, lock, len(files))) as pool:
        pool.map(_run_over_rest_files, run_numbers(files, s.generator))
    print("\nAll tasks completed!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
