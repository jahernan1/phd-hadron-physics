"""`gxana` command-line entry point (docs/REFACTOR_SPEC.md §9)."""
from __future__ import annotations

import argparse
import os
import sys
from typing import Optional, Sequence

from gxana import doctor
from gxana.config import ConfigError, load_channel
from gxana.paths import MissingEnvError
from gxana.stages import select


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
    except (ConfigError, MissingEnvError, select.SelectError) as err:
        print(f"gxana: error: {err}", file=sys.stderr)
        return 2
    raise AssertionError(f"unhandled command {args}")


if __name__ == "__main__":
    sys.exit(main())
