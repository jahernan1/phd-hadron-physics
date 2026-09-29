"""Total cross section integrated over the differential -t bins.

The dissertation defines the total cross section two ways: directly from the
energy-only bins (all -t; the `totxsec_<name>.txt` tables gxana_xsec_tables
writes), and by integrating the differential cross section,
sigma(E) = integral of dsigma/dt dt over the -t bins. The differential bins
cover only 0.10 < -t < 2.40 GeV^2, so the integrated value carries that
effective -t cut and differs from the direct one.

Input tables are `diffxsec_<name>_emin_<E>_emax_<E>.txt` with columns
`tBinCenter dsigmadt tBinWidth Yerr` (tBinWidth is the half-width). Per energy
bin, sigma = sum_i dsigmadt_i * 2 * tBinWidth_i and
err = sqrt(sum_i (Yerr_i * 2 * tBinWidth_i)^2), the same as ROOT
`TH1::IntegralAndError(..., "width")` used by the legacy GetDiffXSec.C. A -t
bin gated to dsigmadt = 0 contributes zero. Output rows follow the
`totxsec_<name>.txt` layout: `enBinCenter sigma enBinWidth Yerr`
(enBinWidth is the half-width), written to `intxsec_<name>.txt`.
"""
from __future__ import annotations

import os
import re
from typing import List, Optional, Tuple

import numpy as np

HEADER = "enBinCenter\tsigma\tenBinWidth\tYerr"
_NAME_RE = re.compile(r"^diffxsec_(?P<name>.+)_emin_(?P<emin>[0-9.]+)_emax_(?P<emax>[0-9.]+)\.txt$")


def read_diffxsec(file_path: str):
    """Returns tBinCenter, dsigmadt, tBinWidth, Yerr arrays (header line skipped)."""
    data = np.loadtxt(file_path, skiprows=1, ndmin=2)
    if data.shape[1] != 4:
        raise ValueError(f"File {file_path} must have exactly 4 columns: tBinCenter dsigmadt tBinWidth Yerr")
    return data.T


def integrate_table(dsigmadt, t_half_width, yerr) -> Tuple[float, float]:
    """Sum of dsigma/dt over the bins weighted by the full bin widths, and its error."""
    width = 2.0 * np.asarray(t_half_width, dtype=float)
    sigma = float(np.sum(np.asarray(dsigmadt, dtype=float) * width))
    err = float(np.sqrt(np.sum((np.asarray(yerr, dtype=float) * width) ** 2)))
    return sigma, err


def _bin_files(directory: str, name: str) -> List[Tuple[float, float, str]]:
    bins = []
    for base in os.listdir(directory):
        match = _NAME_RE.match(base)
        if match and match.group("name") == name:
            bins.append((float(match.group("emin")), float(match.group("emax")), os.path.join(directory, base)))
    return sorted(bins)


def discover_names(directory: str) -> List[str]:
    """The distinct <name> prefixes of the diffxsec tables in directory, sorted."""
    names = set()
    for base in os.listdir(directory):
        match = _NAME_RE.match(base)
        if match:
            names.add(match.group("name"))
    return sorted(names)


def integrate_period(directory: str, name: str, output_dir: str) -> str:
    """Integrates each energy bin of `diffxsec_<name>_emin_*_emax_*.txt` and
    writes `<output_dir>/intxsec_<name>.txt`. Returns the output path."""
    bins = _bin_files(directory, name)
    if not bins:
        raise FileNotFoundError(f"No diffxsec_{name}_emin_*_emax_*.txt files found in directory: {directory}")
    rows = []
    for emin, emax, path in bins:
        _, dsigmadt, t_half_width, yerr = read_diffxsec(path)
        sigma, err = integrate_table(dsigmadt, t_half_width, yerr)
        rows.append([(emin + emax) / 2.0, sigma, (emax - emin) / 2.0, err])
    os.makedirs(output_dir, exist_ok=True)
    output_file = os.path.join(output_dir, f"intxsec_{name}.txt")
    np.savetxt(output_file, np.array(rows), header=HEADER, fmt="%.6f", delimiter="\t", comments="")
    print(f"Integrated {len(bins)} energy bins of {name}: {output_file}")
    return output_file


def integrate_directory(directory: str, output_dir: str, name: Optional[str] = None) -> List[str]:
    """integrate_period for `name`, or for every <name> found in directory."""
    names = [name] if name else discover_names(directory)
    if not names:
        raise FileNotFoundError(f"No diffxsec_*_emin_*_emax_*.txt files found in directory: {directory}")
    return [integrate_period(directory, n, output_dir) for n in names]


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description="Integrate diffxsec tables over -t into intxsec tables.")
    parser.add_argument("directory")
    parser.add_argument("output_dir")
    parser.add_argument("--name", default=None, help="one period name (default: every name found)")
    return parser


def main(argv=None):
    args = _build_arg_parser().parse_args(argv)
    integrate_directory(args.directory, args.output_dir, name=args.name)


if __name__ == "__main__":
    main()
