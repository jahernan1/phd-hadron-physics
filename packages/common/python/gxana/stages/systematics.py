"""`gxana run systematics`: Barlow cut-variation systematics.

Replaces the middle of the legacy AnalysisNote UML chain
(SplitVariationTrees.C, GetXSecFilesUML.C, GetWeightedXsecFile.py) with the
xsection package, driven by analyses/<channel>/config/systematics.yaml:

  GetVariationTreesUML.C   (channel macro, run by hand first) writes
                           variation_trees/flatTree_<stem>_<cut>_<mc>_variations.root
  bin      gxana_xsec_bin variation   (SplitVariationTrees.C)
  tables   gxana_xsec_tables          (GetXSecFilesUML.C, fit JohnsonMCShapeSyst)
  weight   gxana_xsection.weighted_average per variation over the run periods
  barlow   barlow/PlotXSecBarlow*.C against the nominal weighted tables
"""
from __future__ import annotations

import re
import shlex
import subprocess
from pathlib import Path
from typing import Any, Callable, Dict, List, Mapping, NamedTuple, Optional, Sequence

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs

STEPS = ("bin", "tables", "weight", "barlow")
DEFAULT_STEPS = STEPS

Runner = Callable[..., subprocess.CompletedProcess]


class SystematicsError(RuntimeError):
    """A systematics step cannot run (missing inputs)."""


class Command(NamedTuple):
    argv: List[str]
    step: str
    cwd: Optional[str] = None


_VARY = re.compile(r"_(vary_.+)\.txt$")


def _resolve(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> Any:
    scfg = config.require(cfg, "systematics")
    return scfg, config.expand_env(config.require(scfg, "output_dir"), environ)


def _xsection_output(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> str:
    return config.expand_env(config.require(config.require(cfg, "xsection"), "output_dir"), environ)


def _binned_variations(scfg: Dict[str, Any], output_dir: str, in_path: str) -> str:
    return f"{output_dir}/variation_trees/binned_{Path(in_path).name}"


def _variation_input(cfg: Dict[str, Any], scfg: Dict[str, Any], period: str, cut: str,
                     environ: Optional[Mapping[str, str]]) -> str:
    mc_sample = config.require(config.require(cfg, "xsection"), "mc_sample")
    return config.expand_env(config.require(scfg, "variation_trees"), environ).format(
        stem=config.tree_stem(cfg, period, "data"), cut=cut, mc_sample=mc_sample)


def _plan_bin(cfg, scfg, periods, output_dir, energy_str, t_str, environ) -> List[Command]:
    exe = xs._executable("gxana_xsec_bin", environ)
    commands = []
    for cut in scfg["cuts"]:
        for period in periods:
            in_path = _variation_input(cfg, scfg, period, cut, environ)
            commands.append(Command(
                [exe, "variation", in_path, _binned_variations(scfg, output_dir, in_path),
                 "--energy", energy_str, "--t", t_str], "bin"))
    return commands


def _plan_tables(cfg, scfg, periods, output_dir, environ) -> List[Command]:
    exe = xs._executable("gxana_xsec_tables", environ)
    xcfg = config.require(cfg, "xsection")
    xs_output = _xsection_output(cfg, environ)
    flux_dir = config.expand_env(xcfg["inputs"]["flux_dir"], environ)
    fit = config.require(scfg, "fit")
    label = config.require(scfg, "label")
    commands = []
    for cut in scfg["cuts"]:
        for period in periods:
            stem = config.tree_stem(cfg, period, "data")
            binned = _binned_variations(scfg, output_dir, _variation_input(cfg, scfg, period, cut, environ))
            thrown = xs._thrown_output(xs_output, config.tree_stem(cfg, period, xcfg["mc_sample"]))
            flux = config.period_settings(cfg, period)["flux"]
            argv = [exe, "--fit", fit["model"]]
            for name, values in fit["params"].items():
                argv += ["--param", f"{name}=" + ",".join(xs._num(v) for v in values)]
            argv += ["--out", f"{output_dir}/xsection_data", "--plots", f"{output_dir}/fits",
                     "--weight", scfg.get("weight", xcfg["weight"]),
                     "--cheby", xs._num(fit.get("cheby", 2)), "--label", label,
                     f"flatTree_{stem}:{binned}:{binned}:{thrown}:{flux_dir}/{flux}"]
            commands.append(Command(argv, "tables"))
    return commands


def discover_variations(in_dir: str) -> List[str]:
    """The distinct vary_<cut>_<value> suffixes of the per-period tables in in_dir."""
    found = set()
    for path in Path(in_dir).glob("totxsec_*_vary_*.txt"):
        match = _VARY.search(path.name)
        if match:
            found.add(match.group(1))
    return sorted(found)


def weight_commands(in_dir: str, out_dir: str, suffixes: Sequence[str], energy_edges: Sequence[float]) -> List[Command]:
    """Run-period weighted average of every variation: the total cross section and,
    per energy bin, the differential one (legacy GetWeightedXsecFile.py patterns)."""
    commands = []
    for suffix in suffixes:
        commands.append(Command(xs._python_module(
            "weighted_average", in_dir, out_dir, "--pattern", f"totxsec*_{suffix}.txt"), "weight"))
        for e in energy_edges[:-1]:
            commands.append(Command(xs._python_module(
                "weighted_average", in_dir, out_dir, "--pattern", f"diffxsec*_{suffix}_emin_{e:.2f}*.txt"), "weight"))
    return commands


def _missing_tables_message(in_dir: str) -> str:
    return (f"gxana: error: no variation tables (totxsec_*_vary_*.txt) in {in_dir}; run the bin and tables "
            f"steps first (after GetVariationTreesUML.C has written the variation trees)")


def _plan_weight(cfg, scfg, output_dir, environ) -> List[Command]:
    label = config.require(scfg, "label")
    in_dir = f"{output_dir}/xsection_data/{label}"
    suffixes = discover_variations(in_dir)
    if not suffixes:
        raise SystematicsError(_missing_tables_message(in_dir))
    return weight_commands(in_dir, f"{output_dir}/weighted_data/{label}", suffixes,
                           config.require(cfg, "energy_edges"))


def _root_dir(environ: Optional[Mapping[str, str]]) -> Path:
    value = (environ or {}).get("GXANA_ROOT")
    return Path(value) if value else repo_root()


def _plan_barlow(cfg, scfg, output_dir, channel_dir: str, environ) -> List[Command]:
    root = _root_dir(environ)
    label = config.require(scfg, "label")
    commands = []
    for macro in config.require(scfg, "barlow_macros"):
        # -q: a macro that does not compile exits non-zero (1); a crash exits 128+signal.
        # A macro that only warns (e.g. a missing input file) still exits 0.
        commands.append(Command(
            ["root", "-l", "-b", "-q", str(root / "rootlogon.C"),
             f'{root / "analyses" / channel_dir / "systematics" / macro}("{label}")'],
            "barlow", cwd=output_dir))
    return commands


def plan_systematics(
    cfg: Dict[str, Any], steps: Sequence[str], environ: Optional[Mapping[str, str]] = None,
) -> List[Command]:
    requested = set(steps)
    unknown = sorted(requested - set(STEPS))
    if unknown:
        raise config.ConfigError(f"unknown step {unknown[0]!r}; known: {list(STEPS)}")
    scfg, output_dir = _resolve(cfg, environ)
    periods = list(config.require(cfg, "periods"))
    commands: List[Command] = []
    for step in STEPS:
        if step not in requested:
            continue
        if step == "bin":
            energy_str = ",".join(xs._num(e) for e in config.require(cfg, "energy_edges"))
            t_str = ",".join(xs._num(t) for t in xs._flatten_t_bins(config.require(cfg, "t_bins")))
            commands += _plan_bin(cfg, scfg, periods, output_dir, energy_str, t_str, environ)
        elif step == "tables":
            commands += _plan_tables(cfg, scfg, periods, output_dir, environ)
        elif step == "weight":
            commands += _plan_weight(cfg, scfg, output_dir, environ)
        elif step == "barlow":
            commands += _plan_barlow(cfg, scfg, output_dir, cfg.get("channel", "kpkpxim"), environ)
    return commands


def _output_dirs(cfg: Dict[str, Any], environ: Optional[Mapping[str, str]]) -> List[Path]:
    scfg, output_dir = _resolve(cfg, environ)
    label = config.require(scfg, "label")
    base = Path(output_dir)
    return [base / "variation_trees", base / "xsection_data" / label, base / "fits" / label,
            base / "weighted_data" / label, base / "plots"]


def run_systematics(
    cfg: Dict[str, Any], steps: Sequence[str], dry_run: bool = False,
    runner: Runner = subprocess.run, environ: Optional[Mapping[str, str]] = None,
) -> int:
    requested = set(steps)
    unknown = sorted(requested - set(STEPS))
    if unknown:
        raise config.ConfigError(f"unknown step {unknown[0]!r}; known: {list(STEPS)}")
    if not dry_run:
        for d in _output_dirs(cfg, environ):
            d.mkdir(parents=True, exist_ok=True)
    scfg, output_dir = _resolve(cfg, environ)
    for step in STEPS:
        if step not in requested:
            continue
        if step == "barlow" and not dry_run:
            nominal = Path(_xsection_output(cfg, environ)) / "weighted_data" / scfg["label"]
            if not nominal.is_dir():
                print(f"gxana: error: nominal weighted tables {nominal} not found; run `gxana run xsection` first")
                return 1
        try:
            commands = plan_systematics(cfg, [step], environ=environ)
        except SystematicsError as err:
            if dry_run and "tables" in requested:
                print(f"# weight: commands depend on the tables step output ({err})")
                continue
            print(err)
            return 1
        for cmd in commands:
            print(shlex.join(cmd.argv))
            if dry_run:
                continue
            kwargs = {"cwd": cmd.cwd} if cmd.cwd else {}
            result = runner(cmd.argv, check=False, **kwargs)
            rc = getattr(result, "returncode", 0) or 0
            if rc != 0:
                return rc
    return 0
