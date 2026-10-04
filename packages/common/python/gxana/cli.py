"""`gxana` command-line entry point (docs/REFACTOR_SPEC.md §9)."""
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import Optional, Sequence

from gxana import analysis_data, doctor, externals
from gxana.config import ConfigError, export_channel_kv, load_channel
from gxana.paths import MissingEnvError, env_path
from gxana.stages import barlow, mc, measurements, qfactors, select, studies, systematics, xsection


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="gxana", description="GlueX analysis toolkit (channels in analyses/)")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("doctor", help="check the environment")

    cfg = sub.add_parser("config", help="inspect channel configuration")
    cfg_sub = cfg.add_subparsers(dest="config_command", required=True)
    show = cfg_sub.add_parser("show", help="print merged channel YAML")
    show.add_argument("--channel", required=True)
    export = cfg_sub.add_parser("export", help="write $GXANA_OUTPUT/<channel>/config/channel.kv for the C++ macros")
    export.add_argument("--channel", required=True)
    export.add_argument("--out", help="output file (default: $GXANA_OUTPUT/<channel>/config/channel.kv)")

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
                       "; tex is opt-in and reads the gxana run systematics stats files in"
                       " xsection.tex.columns)")
    xsec.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    barp = stages.add_parser("barlow", help="Barlow cut-variation check: trees, fit, weight, plot")
    barp.add_argument("--channel", default="kpkpxim")
    barp.add_argument("--steps", help="comma-separated subset of: " + ",".join(barlow.STEPS) +
                      " (default: " + ",".join(barlow.DEFAULT_STEPS) + "; check is opt-in and needs"
                      " barlow.check; plot needs `gxana run xsection` for the nominal weighted tables)")
    barp.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    sysp = stages.add_parser("systematics", help="systematic studies: variant spreads, PDG scale factor, "
                                                   "track efficiency, normalization (systematics.yaml)")
    sysp.add_argument("--channel", default="kpkpxim")
    sysp.add_argument("--steps", help="comma-separated subset of: " + ",".join(systematics.STEPS) +
                      " (default: " + ",".join(systematics.DEFAULT_STEPS) + "; runperiod and compare are"
                      " opt-in; needs `gxana run xsection --steps bin,tables,weight` for the nominal)")
    sysp.add_argument("--study", help="comma-separated study names from systematics.yaml (default: all)")
    sysp.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    stp = stages.add_parser("studies", help="analysis studies from studies.yaml (cut scans, ...)")
    stp.add_argument("--channel", required=True)
    stp.add_argument("--steps", help="comma-separated subset of: " + ",".join(studies.STEPS) +
                     " (default: all; each study runs the steps of its kind)")
    stp.add_argument("--study", help="comma-separated study names from studies.yaml (default: all)")
    stp.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    msp = stages.add_parser("measurements", help="measurement macros from measurements.yaml (prep, fit)")
    msp.add_argument("--channel", required=True)
    msp.add_argument("--steps", help="comma-separated subset of: " + ",".join(measurements.STEPS) +
                     " (default: all; each item runs the steps it has)")
    msp.add_argument("--item", help="comma-separated item names from measurements.yaml (default: all)")
    msp.add_argument("--dry-run", action="store_true", help="print the plan, run nothing")

    mcp = stages.add_parser("mc", help="render thesis MCwrapper inputs and submit gluex_MC.py")
    mcp.add_argument("--channel", default="kpkpxim")
    mcp.add_argument("--period", required=True)
    mcp.add_argument("--sample", required=True)
    mcp.add_argument("--dry-run", action="store_true", help="print the plan, write and run nothing")

    qfp = stages.add_parser("qfactors", help="Q-factor signal weights with the QFactors fork")
    qfp.add_argument("--channel", default="kpkpxim")
    qfp.add_argument("--period", required=True)
    qfp.add_argument("--model", help="configPDFs*.h file in packages/qfactors, or the repo-relative path of a "
                     "channel model file (default: qfactors.model)")
    qfp.add_argument("--steps", help="comma-separated subset of: " + ",".join(qfactors.STEPS) +
                     " (default: " + ",".join(qfactors.DEFAULT_STEPS) + "; prepare = stage and compile"
                     " main only; fit = run the main processes; plots = hadd + mergeQresults into"
                     " postQVal_flatTree_*.root, the xsection input, + makePlots)")
    qfp.add_argument("--dry-run", action="store_true", help="print the plan, write and run nothing")

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
        cmd.add_argument("--dest", help="checkout directory (only with exactly one NAME); halld_sim has no "
                                        "single default -- it checks out per sim version set at "
                                        "$GXANA_EXTERNALS/halld_sim-<set> "
                                        "(see packages/montecarlo/scripts/build_halld_sim.sh)")

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


def _mc(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    job = mc.plan_mc(cfg, args.period, args.sample)
    if not args.dry_run:
        return mc.run_mc(job)
    print(f"environment: source env/setup.sh --sim={job.version_set}")
    print(f"run dir:     {job.run_dir}")
    print(f"generator:   {job.cfg_template} -> {job.generator_config}")
    print(f"mcwrapper:   {job.conf_template} -> {job.conf}")
    for key, value in job.overrides.items():
        print(f"  {key}={value}")
    print("command:     " + " ".join(job.argv))
    for line in mc.link_commands(job):
        print(f"then:        {line}")
    return 0


def _qfactors(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    job = qfactors.plan_qfactors(cfg, args.period, model=args.model)
    steps = args.steps.split(",") if args.steps else list(qfactors.DEFAULT_STEPS)
    arg = qfactors.run_py_arg(steps)
    if not args.dry_run:
        return qfactors.run_qfactors(job, steps)
    print(f"model:     {job.model} ({job.pdf_config})")
    print(f"input:     {job.input_file} [{job.tree}]")
    print(f"work dir:  {job.work_dir}")
    print(f"logs ->    {job.output_dir}")
    print(f"plots ->   {job.plots_dir}")
    print(f"result:    {job.result}")
    if "prepare" in steps:
        print("prepare:   stage + g++ -o main main.C $(root-config --cflags --glibs --libs) -lRooStats -lRooFitCore -lRooFit")
    if arg:
        print(f"command:   QFACTORS_SETTINGS={job.work_dir / 'run_settings.py'} python3 run.py {arg}")
    return 0


def _xsection(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(xsection.DEFAULT_STEPS)
    return xsection.run_xsection(cfg, steps, dry_run=args.dry_run)


def _barlow(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(barlow.DEFAULT_STEPS)
    return barlow.run_barlow(cfg, steps, dry_run=args.dry_run)


def _systematics(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(systematics.DEFAULT_STEPS)
    studies = args.study.split(",") if args.study else None
    return systematics.run_systematics(cfg, steps, dry_run=args.dry_run, study_names=studies)


def _studies(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(studies.DEFAULT_STEPS)
    names = args.study.split(",") if args.study else None
    return studies.run_studies(cfg, steps, dry_run=args.dry_run, study_names=names)


def _measurements(args: argparse.Namespace) -> int:
    cfg = load_channel(args.channel)
    steps = args.steps.split(",") if args.steps else list(measurements.DEFAULT_STEPS)
    items = args.item.split(",") if args.item else None
    return measurements.run_measurements(cfg, steps, dry_run=args.dry_run, items=items)


def _data(args: argparse.Namespace) -> int:
    manifest = analysis_data.load_manifest(args.channel)
    base = analysis_data.data_dir(manifest)
    if args.data_command == "path":
        print(base)
        return 0
    if not base.is_dir():
        if args.data_command == "status":
            # A public clone has no preserved data: a state, not an error (README Quickstart).
            print(f"no preserved data at {base}: golden tests skip and `gxana data stage` has nothing "
                  "to stage; to use it, place the data there or set GXANA_ANALYSIS_DATA "
                  "(docs/analysis_data.md)")
            return 0
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
        if name == "halld_sim" and not args.dest:
            # No single checkout: each sim version set (env/version_sets/
            # <set>.xml.in) gets its own halld_sim-<set> build.
            if args.externals_command == "fetch":
                print(f"[skip] {name}: no --dest given; halld_sim checks out per sim version set at "
                      f"$GXANA_EXTERNALS/{name}-<set> -- run "
                      f"`packages/montecarlo/scripts/build_halld_sim.sh <set>` or pass --dest")
                continue
            sets = externals.version_set_names()
            if not sets:
                print(f"[miss] {name}: no version sets in env/version_sets/")
                bad += 1
                continue
            for set_name in sets:
                bad += _status_line(name, ext, externals.versioned_dest(ext, set_name))
            continue
        dest = Path(args.dest) if args.dest else externals.default_dest(ext)
        if args.externals_command == "fetch":
            externals.fetch(ext, dest)
            continue
        if not ext.fetch:
            print(f"[pin ] {name} {ext.ref} {ext.sha[:12]} (from the GlueX version set)")
            continue
        bad += _status_line(name, ext, dest)
    return 1 if bad else 0


def _status_line(name: str, ext: externals.External, dest: Path) -> int:
    """Print one `externals status` line for dest; return 1 if bad, else 0."""
    if not dest.exists():
        print(f"[miss] {name}: {dest}")
        return 1
    problems = externals.check(ext, dest)
    print(f"[{' ok ' if not problems else 'diff'}] {name}: {dest}")
    for problem in problems:
        print(f"       {problem}")
    return int(bool(problems))


def main(argv: Optional[Sequence[str]] = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    args = build_parser().parse_args(argv)
    try:
        if args.command == "doctor":
            return doctor.main()
        if args.command == "config":
            import yaml

            if args.config_command == "export":
                out = Path(args.out) if args.out else env_path("GXANA_OUTPUT", args.channel, "config", "channel.kv")
                print(f"wrote {export_channel_kv(args.channel, out)}")
                return 0
            print(yaml.safe_dump(load_channel(args.channel), sort_keys=False), end="")
            return 0
        if args.command == "run" and args.stage == "select":
            return _select(args)
        if args.command == "run" and args.stage == "xsection":
            return _xsection(args)
        if args.command == "run" and args.stage == "barlow":
            return _barlow(args)
        if args.command == "run" and args.stage == "systematics":
            return _systematics(args)
        if args.command == "run" and args.stage == "studies":
            return _studies(args)
        if args.command == "run" and args.stage == "measurements":
            return _measurements(args)
        if args.command == "run" and args.stage == "mc":
            return _mc(args)
        if args.command == "run" and args.stage == "qfactors":
            return _qfactors(args)
        if args.command == "data":
            return _data(args)
        if args.command == "externals":
            return _externals(args)
    except (ConfigError, MissingEnvError, select.SelectError, externals.ExternalsError, mc.McError,
            qfactors.QFactorsError) as err:
        print(f"gxana: error: {err}", file=sys.stderr)
        return 2
    raise AssertionError(f"unhandled command {args}")


if __name__ == "__main__":
    sys.exit(main())
