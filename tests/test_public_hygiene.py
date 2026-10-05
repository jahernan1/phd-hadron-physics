"""Public hygiene: tracked text files carry no laptop home paths, no other
people's home directories and no citations of local-only planning documents."""
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

AUTHOR_USERS = {"hjesse", "jahernan", "jessehernandez"}
LAPTOP = re.compile(r"/Users/[A-Za-z0-9_.-]+")
HOME = re.compile(r"/home/([A-Za-z0-9_.-]+)")
# Upstream MCwrapper template example (`evtgen:/u/home/evtgen.cfg`), not a person.
HOME_ALLOWED = {"evtgen.cfg"}

# The public refactor record may be cited by file name (`docs/history/REFACTOR_SPEC.md D18`,
# `REFACTOR_SPEC.md §8`); anything else that looks like a planning-document
# reference points at notes that are not in the repository.
SPEC_REF = re.compile(r"REFACTOR_SPEC\.md`?\)?\s*(?:§\s*[\d.]+|D\d+)(?:\s*(?:,|and)\s*(?:§\s*[\d.]+|D\d+))*")
# Bare planning ids (S1, C2, D20) are flagged wherever they appear; a future legitimate
# token (e.g. "D0 meson") is handled by rewording or a narrow allowlist entry.
CITATION = re.compile(
    r"\bspec\s*(?:§|[Ss]ec(?:tion\b|\.)|D\d|S\d)"  # spec §8, spec section 5, spec Sec.12, spec D20, spec S7
    r"|(?<![\w/.$-])(?:S(?:10|\d[ab]?)|C\d|D\d{1,2})(?!\w)"  # planning ids: S1, S4a, C2, D20
    r"|\bPlan\s+\d"                              # Plan 4
    r"|\bTask\s+\d"                              # Task 2
    r"|superpowers"
)
# Files exempt from CITATION: the public refactor record numbers its own
# decisions and plan rows; .gitignore keeps the ignore pattern for local plans;
# this test spells the patterns out.
CITATION_EXEMPT = {"docs/history/REFACTOR_SPEC.md", ".gitignore", "tests/test_public_hygiene.py"}


def _tracked_text_files():
    out = subprocess.run(["git", "ls-files", "-z"], cwd=ROOT, capture_output=True, check=True).stdout
    for rel in out.decode().split("\0"):
        path = ROOT / rel
        if not rel or not path.is_file():   # submodule gitlinks, deleted files
            continue
        data = path.read_bytes()
        if b"\0" in data[:8192]:
            continue
        yield rel, data.decode("utf-8", errors="ignore")


def _scan():
    hits = []
    for rel, text in _tracked_text_files():
        legacy = rel.startswith("archive/") and rel != "archive/README.md"
        for n, line in enumerate(text.splitlines(), 1):
            where = f"{rel}:{n}: {line.strip()[:120]}"
            if LAPTOP.search(line):
                hits.append(("laptop", where))
            for m in HOME.finditer(line):
                if m.group(1) not in AUTHOR_USERS | HOME_ALLOWED:
                    hits.append(("home", where))
            if not legacy and rel not in CITATION_EXEMPT and CITATION.search(SPEC_REF.sub("", line)):
                hits.append(("citation", where))
    return hits


def test_no_laptop_home_paths():
    hits = [w for k, w in _scan() if k == "laptop"]
    assert not hits, "\n".join(hits)


def test_no_other_users_home_directories():
    hits = [w for k, w in _scan() if k == "home"]
    assert not hits, "\n".join(hits)


def test_no_planning_document_citations():
    hits = [w for k, w in _scan() if k == "citation"]
    assert not hits, "\n".join(hits)
