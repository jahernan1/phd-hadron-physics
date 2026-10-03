"""Pinned upstream sources + author patches (packages/montecarlo/external.lock).

fetch = clone at the locked sha, `git am` the patches, then prove the tree
reproduces the author's files (sha256 per touched file). An existing
checkout is reused only if it already matches; it is never overwritten.
"""
from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Callable, Dict, List, Mapping, NamedTuple, Optional

from gxana.analysis_data import sha256_file
from gxana.paths import env_path, repo_root

LOCK_REL = Path("packages/montecarlo/external.lock")
PATCH_DIR_REL = Path("packages/montecarlo/patches")
# git am needs a committer identity; each patch keeps its own From: author.
GIT_IDENTITY = ["-c", "user.name=gxana", "-c", "user.email=gxana@localhost"]
Runner = Callable[..., subprocess.CompletedProcess]


class ExternalsError(RuntimeError):
    """external.lock is invalid, or a checkout does not match it."""


class External(NamedTuple):
    name: str
    url: str
    ref: str
    sha: str
    fetch: bool
    patches: List[Path]
    files: Dict[str, str]


def load_lock(root: Optional[Path] = None) -> Dict[str, External]:
    import yaml

    base = root or repo_root()
    data = yaml.safe_load((base / LOCK_REL).read_text()) or {}
    out: Dict[str, External] = {}
    for name, entry in (data.get("externals") or {}).items():
        missing = [k for k in ("url", "ref", "sha", "fetch") if k not in entry]
        if missing:
            raise ExternalsError(f"{name}: external.lock entry missing {missing}")
        sha = str(entry["sha"])
        if len(sha) != 40 or any(c not in "0123456789abcdef" for c in sha):
            raise ExternalsError(f"{name}: sha must be 40 lowercase hex, got {sha!r}")
        patches = [base / PATCH_DIR_REL / rel for rel in entry.get("patches") or []]
        for patch in patches:
            if not patch.is_file():
                raise ExternalsError(f"{name}: patch not found: {patch}")
        out[name] = External(name, str(entry["url"]), str(entry["ref"]), sha, bool(entry["fetch"]),
                             patches, {str(k): str(v) for k, v in (entry.get("files") or {}).items()})
    return out


def default_dest(ext: External, environ: Optional[Mapping[str, str]] = None) -> Path:
    return env_path("GXANA_EXTERNALS", ext.name, environ=environ)


def version_set_names(root: Optional[Path] = None) -> List[str]:
    """Sim version set names from env/version_sets/*.xml.in (docs/REFACTOR_SPEC.md)."""
    base = root or repo_root()
    return sorted(p.name[: -len(".xml.in")] for p in (base / "env" / "version_sets").glob("*.xml.in"))


def versioned_dest(ext: External, version_set: str, environ: Optional[Mapping[str, str]] = None) -> Path:
    """$GXANA_EXTERNALS/<name>-<version_set>: halld_sim keeps one checkout per
    sim version set (env/version_sets/<set>.xml.in home=), unlike the other
    externals which have a single default_dest()."""
    return env_path("GXANA_EXTERNALS", f"{ext.name}-{version_set}", environ=environ)


def _git(runner: Runner, *args: str, cwd: Optional[Path] = None) -> str:
    proc = runner(["git", *args], cwd=cwd, capture_output=True, text=True)
    if proc.returncode != 0:
        raise ExternalsError(f"git {' '.join(args)} failed: {proc.stderr.strip()}")
    return proc.stdout.strip()


def check(ext: External, dest: Path, runner: Runner = subprocess.run) -> List[str]:
    """Differences between the checkout at dest and the lock ([] = matches)."""
    if not (dest / ".git").exists():
        return [f"{dest} is not a git checkout"]
    problems: List[str] = []
    try:
        base = _git(runner, "rev-parse", f"HEAD~{len(ext.patches)}", cwd=dest)
    except ExternalsError as err:
        base = ""
        problems.append(str(err))
    if base and base != ext.sha:
        problems.append(f"base commit {base[:12]} != locked {ext.sha[:12]} ({ext.ref}) + {len(ext.patches)} patch(es)")
    dirty = _git(runner, "status", "--porcelain", "--untracked-files=no", cwd=dest)
    if dirty:
        problems.append("tracked files modified: " + ", ".join(line[3:] for line in dirty.splitlines()))
    for rel, want in sorted(ext.files.items()):
        path = dest / rel
        if not path.is_file():
            problems.append(f"{rel}: missing")
            continue
        got = sha256_file(path)
        if got != want:
            problems.append(f"{rel}: sha256 {got[:12]} != locked {want[:12]}")
    return problems


def fetch(ext: External, dest: Path, runner: Runner = subprocess.run,
          log: Callable[[str], None] = print) -> None:
    if not ext.fetch:
        raise ExternalsError(f"{ext.name} is pinned only (fetch: false); the GlueX version set provides it")
    if dest.exists():
        problems = check(ext, dest, runner)
        if problems:
            raise ExternalsError(f"{dest} exists and does not match the lock:\n  " + "\n  ".join(problems)
                                 + "\nremove it or choose another --dest")
        log(f"{ext.name}: {dest} already matches the lock")
        return
    dest.parent.mkdir(parents=True, exist_ok=True)
    try:
        _git(runner, "clone", "--quiet", "--no-checkout", ext.url, str(dest))
        _git(runner, "-c", "advice.detachedHead=false", "checkout", "--quiet", ext.sha, cwd=dest)
        if ext.patches:
            _git(runner, *GIT_IDENTITY, "am", "--quiet", *(str(p) for p in ext.patches), cwd=dest)
        problems = check(ext, dest, runner)
        if problems:
            raise ExternalsError(f"{ext.name}: patched tree at {dest} does not reproduce the locked files:\n  "
                                 + "\n  ".join(problems))
    except ExternalsError as err:
        raise ExternalsError(f"{err}; remove {dest} and retry") from err
    log(f"{ext.name}: {ext.ref} ({ext.sha[:12]}) + {len(ext.patches)} patch(es) -> {dest}")
