"""No legacy site paths outside archive/ (spec D15)."""
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATTERN = re.compile(r"/d/grid1[37]/|/work/halld/home/|/w/halld-scshelf")
SCOPES = ("analyses", "packages", "env", "scripts")
# Files that legitimately hold the legacy prefixes as data.
ALLOWED = {"packages/common/python/gxana/paths.py", "packages/common/tests/python/test_paths.py",
           "packages/common/tests/python/test_mc_stage.py", "scripts/migrate_paths.py"}


def test_no_legacy_paths_in_tracked_and_new_files():
    files = subprocess.run(["git", "ls-files", "--cached", "--others", "--exclude-standard", *SCOPES],
                           cwd=ROOT, capture_output=True, text=True, check=True).stdout.split()
    hits = []
    for rel in files:
        if rel in ALLOWED:
            continue
        path = ROOT / rel
        try:
            text = path.read_text(errors="ignore")
        except (IsADirectoryError, FileNotFoundError):
            continue
        for n, line in enumerate(text.splitlines(), 1):
            if PATTERN.search(line):
                hits.append(f"{rel}:{n}: {line.strip()[:120]}")
    assert not hits, "\n".join(hits)
