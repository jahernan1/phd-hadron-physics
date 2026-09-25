"""`gxana` command-line entry point (docs/REFACTOR_SPEC.md §9)."""
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import Optional, Sequence

from gxana import analysis_data, doctor, externals
from gxana.config import ConfigError, load_channel
from gxana.paths import MissingEnvError
from gxana.stages import select, xsection


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="gxana", description="GlueX K+K+Xi- thesis analysis toolkit")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("doctor", help="check the environment")

    cfg = sub.add_parser("config", help="inspect channel configuration")
    cfg_sub = cfg.add_subparsers(dest="config_command", required=True)
    show = cfg_sub.add_parser("show", help="print merged channel YAML")
    show.add_argument("--channel", required=True)

    run = sub.add_parser("run", help="run a pipeline stage")
    stages = run.add_subparsers(dest="stage", required=True)
    sel = stages.add_parser("select", help="run a DSelector with PROOF-Lite")
    sel.add_argument("--channel", required=True)
    sel.add_argument("--period", required=True)
    sel.add_argument("--sample", default="data")
    sel.add_argument("--thrown", action="store_true", help="process thrown (MC truth) trees")
    sel.add_argument("--tag", help="suffix appended to the saved file names")
    sel.add_argument("--cores", type=int, default=16)
    sel.add_argument("--selector", help="override selector .C path")
    sel.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    xsec = stages.add_parser("xsection", help="bin, fit, weight and rescale cross-section tables")
    xsec.add_argument("--channel", default="kpkpxim")
    xsec.add_argument("--steps", help="comma-separated subset of: " + ",".join(xsection.STEPS) +
                       " (default: " + ",".join(xsection.DEFAULT_STEPS) +
                       "; qvalue is opt-in, e.g. --steps bin,tables,weight,components,qvalue"
                       " -- it needs xsection.qvalue_label set to a data/ label the tables"
                       " step has already written)")
    xsec.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    data = sub.add_parser("data", help="preserved analysis data under $GXANA_ANALYSIS_DATA")
    data_sub = data.add_subparsers(dest="data_command", required=True)
    for name, text in (("path", "print the channel's data directory"),
                       ("status", "compare files on disk with analyses/<channel>/analysis_data.yaml"),
                       ("lock", "record sha256 and size of every file in the manifest")):
        cmd = data_sub.add_parser(name, help=text)
        cmd.add_argument("--channel", required=True)

    ext = sub.add_parser("externals", help="pinned upstream sources (packages/montecarlo/external.lock)")
    ext_sub = ext.add_subparsers(dest="externals_command", required=True)
    for name, text in (("fetch", "clone at the locked sha, apply patches, verify"),
                       ("status", "compare checkouts with the lock")):
        cmd = ext_sub.add_parser(name, help=text)
        cmd.add_argument("names", nargs="*", help="lock entries (default: every fetch: true entry)")
        cmd.add_argument("--dest", help="checkout directory (only with exactly one NAME)")

    return parser


def _select(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    job = select.plan_select(cfg, args.period, args.sample, thrown=args.thrown, tag=args.tag,
                             cores=args.cores, selector=args.selector)
    if not args.dry_run:
        return select.run_select(job)
    print(f"channel:   {job.channel}")
    print(f"trees:     {job.tree_dir}")
    print(f"selector:  {job.selector}")
    print(f"save name: {job.save_name}")
    print(f"run dir:   {job.run_dir}")
    print(f"hist out:  {job.hist_dir}/[thrown_]{job.save_name}.root")
    print(f"flat out:  {job.flat_dir}/flatTree_[thrown_]{job.save_name}.root")
    print("--- ROOT input ---")
    rah = os.environ.get("ROOT_ANALYSIS_HOME", "$ROOT_ANALYSIS_HOME")
    print(select.root_script(job, "<tree name from rootls>", rah), end="")
    return 0


def _xsection(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(xsection.DEFAULT_STEPS)
    return xsection.run_xsection(cfg, steps, dry_run=args.dry_run)


def _data(args: argparse.Namespace) -> int:
    manifest = analysis_data.load_manifest(args.channel)
    base = analysis_data.data_dir(manifest)
    if args.data_command == "path":
        print(base)
        return 0
    if not base.is_dir():
        print(f"gxana: error: {base} does not exist (set GXANA_ANALYSIS_DATA; docs/analysis_data.md)",
              file=sys.stderr)
        return 2
    if args.data_command == "lock":
        count = analysis_data.lock(manifest, base)
        print(f"locked {count} files in {manifest.path}")
        return 0
    results = analysis_data.status(manifest, base)
    for result in results:
        detail = f": {result.detail}" if result.detail else ""
        print(f"[{result.state:>4}] {result.path}{detail}")
    return 1 if any(r.state in ("miss", "diff") for r in results) else 0


def _externals(args: argparse.Namespace) -> int:
    lock = externals.load_lock()
    names = args.names or [n for n, e in lock.items() if e.fetch]
    unknown = [n for n in names if n not in lock]
    if unknown:
        print(f"gxana: error: unknown external(s) {unknown}; known: {sorted(lock)}", file=sys.stderr)
        return 2
    if args.dest and len(args.names) != 1:
        print("gxana: error: --dest needs exactly one NAME", file=sys.stderr)
        return 2
    if args.externals_command == "status" and not args.names:
        names = list(lock)
    bad = 0
    for name in names:
        ext = lock[name]
        dest = Path(args.dest) if args.dest else externals.default_dest(ext)
        if args.externals_command == "fetch":
            externals.fetch(ext, dest)
            continue
        if not ext.fetch:
            print(f"[pin ] {name} {ext.ref} {ext.sha[:12]} (from the GlueX version set)")
            continue
        if not dest.exists():
            print(f"[miss] {name}: {dest}")
            bad += 1
            continue
        problems = externals.check(ext, dest)
        print(f"[{' ok ' if not problems else 'diff'}] {name}: {dest}")
        for problem in problems:
            print(f"       {problem}")
        bad += bool(problems)
    return 1 if bad else 0


def main(argv: Optional[Sequence[str]] = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    args = build_parser().parse_args(argv)
    try:
        if args.command == "doctor":
            return doctor.main()
        if args.command == "config":
            import yaml

            print(yaml.safe_dump(load_channel(args.channel), sort_keys=False), end="")
            return 0
        if args.command == "run" and args.stage == "select":
            return _select(args)
        if args.command == "run" and args.stage == "xsection":
            return _xsection(args)
        if args.command == "data":
            return _data(args)
        if args.command == "externals":
            return _externals(args)
    except (ConfigError, MissingEnvError, select.SelectError, externals.ExternalsError) as err:
        print(f"gxana: error: {err}", file=sys.stderr)
        return 2
    raise AssertionError(f"unhandled command {args}")


if __name__ == "__main__":
    sys.exit(main())
