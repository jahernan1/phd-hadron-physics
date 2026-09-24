"""Compare whitespace-separated result tables numerically (golden tests, spec D20).

Tables are the legacy cross-section text outputs: optional header lines, then
rows of numbers. Non-numeric tokens must match exactly; numbers must agree
within rtol/atol (math.isclose); NaN matches NaN. Stdlib only.
"""
from __future__ import annotations

import argparse
import math
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Sequence


def _rows(path: Path) -> List[List[str]]:
    return [line.split() for line in path.read_text().splitlines() if line.strip()]


def _number(token: str) -> Optional[float]:
    try:
        return float(token)
    except ValueError:
        return None


def _rel(a: float, b: float) -> float:
    if a == b:
        return 0.0
    if math.isnan(a) or math.isnan(b) or math.isinf(a) or math.isinf(b):
        return math.inf
    return abs(a - b) / max(abs(a), abs(b))


@dataclass
class FileResult:
    name: str
    problems: List[str] = field(default_factory=list)
    max_rel: float = 0.0


def compare_tables(new: Path, ref: Path, rtol: float = 1e-9, atol: float = 0.0) -> FileResult:
    new, ref = Path(new), Path(ref)
    result = FileResult(new.name)
    rows_new, rows_ref = _rows(new), _rows(ref)
    if len(rows_new) != len(rows_ref):
        result.problems.append(f"{len(rows_new)} rows vs {len(rows_ref)} in reference")
        result.max_rel = math.inf  # structural mismatch: no meaningful numeric deviation
    for line, (row_new, row_ref) in enumerate(zip(rows_new, rows_ref), start=1):
        if len(row_new) != len(row_ref):
            result.problems.append(f"line {line}: {len(row_new)} columns vs {len(row_ref)}")
            result.max_rel = math.inf
            continue
        for col, (tok_new, tok_ref) in enumerate(zip(row_new, row_ref), start=1):
            num_new, num_ref = _number(tok_new), _number(tok_ref)
            if num_new is None or num_ref is None:
                if tok_new != tok_ref:
                    result.problems.append(f"line {line} col {col}: {tok_new!r} vs {tok_ref!r}")
                continue
            if math.isnan(num_new) and math.isnan(num_ref):
                continue
            rel = _rel(num_new, num_ref)
            result.max_rel = max(result.max_rel, rel)
            if math.isnan(num_new) or math.isnan(num_ref) or not math.isclose(
                    num_new, num_ref, rel_tol=rtol, abs_tol=atol):
                result.problems.append(f"line {line} col {col}: {tok_new} vs {tok_ref} (rel {rel:.2e})")
    return result


@dataclass
class DirReport:
    results: List[FileResult]
    unproduced: List[str]  # in the reference directory, not produced
    unreferenced: List[str]  # produced, no reference file
    only_new: bool = False

    @property
    def ok(self) -> bool:
        return (not self.unreferenced and (self.only_new or not self.unproduced)
                and all(not r.problems for r in self.results))

    @property
    def max_rel(self) -> float:
        return max((r.max_rel for r in self.results), default=0.0)

    def summary(self) -> str:
        lines = [f"{len(self.results)} files compared, max relative deviation {self.max_rel:.3e}"]
        if self.unreferenced:
            lines.append("no reference for: " + ", ".join(self.unreferenced))
        if self.unproduced and not self.only_new:
            lines.append("not produced: " + ", ".join(self.unproduced))
        for result in self.results:
            for problem in result.problems[:5]:
                lines.append(f"{result.name}: {problem}")
            if len(result.problems) > 5:
                lines.append(f"{result.name}: ... {len(result.problems) - 5} more")
        return "\n".join(lines)


def compare_dirs(new_dir: Path, ref_dir: Path, pattern: str = "*.txt", rtol: float = 1e-9,
                 atol: float = 0.0, only_new: bool = False) -> DirReport:
    new_dir, ref_dir = Path(new_dir), Path(ref_dir)
    new_names = {p.name for p in new_dir.glob(pattern) if p.is_file()}
    ref_names = {p.name for p in ref_dir.glob(pattern) if p.is_file()}
    results = [compare_tables(new_dir / name, ref_dir / name, rtol, atol)
               for name in sorted(new_names & ref_names)]
    return DirReport(results, sorted(ref_names - new_names), sorted(new_names - ref_names), only_new)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(prog="python -m gxana_xsection.compare",
                                     description="Compare result tables in NEW against REF.")
    parser.add_argument("new")
    parser.add_argument("ref")
    parser.add_argument("--pattern", default="*.txt")
    parser.add_argument("--rtol", type=float, default=1e-9)
    parser.add_argument("--atol", type=float, default=0.0)
    parser.add_argument("--only-new", action="store_true", help="ignore reference files that were not produced")
    args = parser.parse_args(argv)
    report = compare_dirs(Path(args.new), Path(args.ref), args.pattern, args.rtol, args.atol, args.only_new)
    print(report.summary())
    return 0 if report.ok else 1


if __name__ == "__main__":
    sys.exit(main())
