"""Write the synthetic inputs of the toy walkthrough (examples/toy/README.md).

    uv run python examples/toy/make_toy.py OUT

OUT (a directory outside the checkout) receives:

  OUT/gxana_root/   the GXANA_ROOT of the walkthrough: links to this checkout's build/,
                    packages/ and rootlogon.C, and analyses/toy -> examples/toy/channel,
                    so `gxana ... --channel toy` reads examples/toy/channel/config
  OUT/data/toy/     GXANA_DATA inputs: per run period the data, reconstructed-MC and
                    thrown-MC flat trees, the tagged-flux file (flux/), and truth.txt
                    (the injected cross section, read by check_toy.py and the figure)
  OUT/output/       GXANA_OUTPUT, empty until `gxana run xsection`

Tree names, file names, branches, mass windows and the target come from the toy
channel config; the injected physics is TRUTH below. Runs `root` (make_toy.C) once
per run period.
"""
from __future__ import annotations

import argparse
import math
import os
import shutil
import subprocess
import sys
from pathlib import Path

from gxana import config

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CHANNEL = "toy"

# Injected physics: dsigma/dt = A0 * exp(-b t) (nb/GeV^2), the same at every beam energy,
# generated on t_min < -t < t_max only; a Gaussian mass peak (GeV).
TRUTH = {"A0": 30.0, "b": 1.5, "t_min": 0.1, "t_max": 2.4, "mean": 1.0, "sigma": 0.01}
# Per run period: tagged photons on 6-12 GeV (relative sizes like GlueX Phase I) and the
# random seed. Background events per produced signal event; thrown MC events per period.
FLUX = {"2017-01": (0.6e12, 1701), "2018-01": (1.6e12, 1801), "2018-08": (1.0e12, 1808)}
BACKGROUND_PER_SIGNAL = 0.8
N_THROWN = 60000


def sigma_total_nb(truth=TRUTH) -> float:
    """Integral of dsigma/dt over t_min < -t < t_max (nb)."""
    a0, b = truth["A0"], truth["b"]
    return a0 / b * (math.exp(-b * truth["t_min"]) - math.exp(-b * truth["t_max"]))


def dsigma_dt_bin(t_lo: float, t_hi: float, truth=TRUTH) -> float:
    """Mean of dsigma/dt over the -t bin (nb/GeV^2): what a bin of the table measures."""
    a0, b = truth["A0"], truth["b"]
    return a0 / b * (math.exp(-b * t_lo) - math.exp(-b * t_hi)) / (t_hi - t_lo)


def target_per_barn(target) -> float:
    """Target atoms per barn, as gxana::xsec::TargetDensity computes it."""
    z0, z1 = target["z"]
    return target["atoms"] * 6.022e23 * (z1 - z0) * target["density"] * 1e-24 / target["molar_mass"]


def make_overlay(out: Path) -> Path:
    """OUT/gxana_root with the links described in the module docstring."""
    root = out / "gxana_root"
    (root / "analyses").mkdir(parents=True, exist_ok=True)
    links = {root / "build": REPO / "build", root / "packages": REPO / "packages",
             root / "rootlogon.C": REPO / "rootlogon.C", root / "analyses" / CHANNEL: HERE / "channel"}
    for link, target in links.items():
        if link.is_symlink():
            link.unlink()
        elif link.exists():
            raise SystemExit(f"make_toy.py: {link} exists and is not a link; choose an empty OUT")
        link.symlink_to(target)
    return root


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").partition("\n")[0])
    parser.add_argument("out", help="output directory, outside the checkout")
    args = parser.parse_args(argv)
    out = Path(args.out).expanduser().resolve()
    if out == REPO or REPO in out.parents:
        raise SystemExit(f"make_toy.py: {out} is inside the checkout; choose a directory outside it "
                         "(e.g. ${TMPDIR:-/tmp}/gxana-toy)")
    if not (REPO / "build" / "bin" / "gxana_xsec_tables").is_file():
        raise SystemExit("make_toy.py: build/bin/gxana_xsec_tables not found; build the C++ packages first "
                         "(uv run cmake -S . -B build && uv run cmake --build build -j)")
    if shutil.which("root") is None:
        raise SystemExit("make_toy.py: `root` not on PATH; install ROOT first")

    root = make_overlay(out)
    data, output = out / "data", out / "output"
    environ = {"GXANA_ROOT": str(root), "GXANA_DATA": str(data), "GXANA_OUTPUT": str(output)}
    cfg = config.load_channel(CHANNEL, root=root)
    phys, xcfg = config.physics(cfg), cfg["xsection"]
    if sorted(cfg["periods"]) != sorted(FLUX):
        raise SystemExit(f"make_toy.py: FLUX has periods {sorted(FLUX)}, the config {sorted(cfg['periods'])}")
    windows = xcfg["mass_windows"]
    flux_dir = Path(config.expand_env(xcfg["inputs"]["flux_dir"], environ))
    flux_dir.mkdir(parents=True, exist_ok=True)
    output.mkdir(parents=True, exist_ok=True)

    per_barn = target_per_barn(xcfg["target"])
    br = phys["branching_ratio"]["value"]
    for period, (flux_total, seed) in FLUX.items():
        stems = {"stem": config.tree_stem(cfg, period, "data"),
                 "mc_stem": config.tree_stem(cfg, period, xcfg["mc_sample"])}
        paths = {kind: config.expand_env(xcfg["inputs"][kind], environ).format(**stems)
                 for kind in ("data", "mc", "thrown")}
        n_signal = sigma_total_nb() * 1e-9 * per_barn * flux_total * br
        call_args = [str(flux_dir / cfg["periods"][period]["flux"]), paths["data"], paths["mc"], paths["thrown"],
                     phys["flat_tree"], phys["thrown_flat_tree"], flux_total, n_signal,
                     BACKGROUND_PER_SIGNAL * n_signal, N_THROWN, TRUTH["b"], TRUTH["t_min"], TRUTH["t_max"],
                     TRUTH["mean"], TRUTH["sigma"], windows["lo"], windows["data_hi"], seed]
        call = f"{HERE / 'make_toy.C'}(" + ",".join(
            f'"{a}"' if isinstance(a, str) else repr(a) for a in call_args) + ")"
        proc = subprocess.run(["root", "-l", "-b", "-q", call], env={**os.environ, **environ})
        if proc.returncode != 0:
            return proc.returncode

    truth = data / CHANNEL / "truth.txt"
    truth.write_text("# toy truth: dsigma/dt = A0*exp(-b*t) nb/GeV^2 on t_min < -t < t_max\n"
                     + "".join(f"{k} {v}\n" for k, v in TRUTH.items()))
    print(f"wrote {truth}")
    print("\nFor the walkthrough, set:\n"
          f"  export GXANA_ROOT={root}\n  export GXANA_DATA={data}\n  export GXANA_OUTPUT={output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
