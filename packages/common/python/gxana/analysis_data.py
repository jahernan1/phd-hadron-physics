"""Preserved analysis data: golden inputs and reference outputs kept outside git.

GlueX practice: analysis code is published on GitHub and the data it analysed
is preserved on disk under /work/halld/gluex_analysis_data/ at JLab. Here that
directory is $GXANA_ANALYSIS_DATA (default <repo>/gluex_analysis_data,
gitignored); each channel documents its files, with sha256 and size, in
analyses/<channel>/analysis_data.yaml (docs/analysis_data.md).
"""
from __future__ import annotations

import hashlib
import os
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional

from gxana import config as gconfig
from gxana.config import ConfigError
from gxana.paths import analysis_data_root, repo_root

MANIFEST_NAME = "analysis_data.yaml"


@dataclass
class Manifest:
    path: Path
    header: str  # leading comment block, kept by write_manifest
    data: Dict[str, Any]

    @property
    def subdir(self) -> str:
        return self.data["subdir"]

    @property
    def files(self) -> Dict[str, Dict[str, Any]]:
        return self.data["files"]


@dataclass(frozen=True)
class FileStatus:
    path: str
    state: str  # ok | open (present, not locked) | miss | diff (sha256 changed) | new (not in manifest)
    detail: str = ""


def load_manifest(channel: str, root: Optional[Path] = None) -> Manifest:
    import yaml

    path = (root or repo_root()) / "analyses" / channel / MANIFEST_NAME
    if not path.is_file():
        raise ConfigError(f"no {MANIFEST_NAME} for channel {channel!r} ({path})")
    text = path.read_text()
    data = yaml.safe_load(text) or {}
    if not isinstance(data, dict) or not isinstance(data.get("subdir"), str):
        raise ConfigError(f"{path}: needs a top-level 'subdir' string")
    files = data.get("files") or {}
    if not isinstance(files, dict):
        raise ConfigError(f"{path}: 'files' must map relative path -> {{sha256, bytes}}")
    data["files"] = {rel: dict(entry or {}) for rel, entry in files.items()}
    lines = text.splitlines(keepends=True)
    count = 0
    while count < len(lines) and lines[count].startswith("#"):
        count += 1
    return Manifest(path, "".join(lines[:count]), data)


def data_dir(manifest: Manifest, environ: Optional[Mapping[str, str]] = None) -> Path:
    return analysis_data_root(environ) / manifest.subdir


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def scan(base: Path) -> List[str]:
    """Relative POSIX paths of every non-hidden file under base."""
    found = []
    for path in base.rglob("*"):
        rel = path.relative_to(base)
        if path.is_file() and not any(part.startswith(".") for part in rel.parts):
            found.append(rel.as_posix())
    return sorted(found)


def status(manifest: Manifest, base: Path) -> List[FileStatus]:
    on_disk = set(scan(base))
    results = []
    for rel in sorted(manifest.files):
        entry = manifest.files[rel]
        if rel not in on_disk:
            results.append(FileStatus(rel, "miss"))
        elif not entry.get("sha256"):
            results.append(FileStatus(rel, "open", "not locked (gxana data lock)"))
        elif sha256_file(base / rel) != entry["sha256"]:
            results.append(FileStatus(rel, "diff", "sha256 differs from manifest"))
        else:
            results.append(FileStatus(rel, "ok"))
    for rel in sorted(on_disk - set(manifest.files)):
        results.append(FileStatus(rel, "new", "not in manifest (gxana data lock)"))
    return results


def lock(manifest: Manifest, base: Path) -> int:
    """Record sha256 and bytes of every file under base; return how many.

    Entries whose file is absent stay in the manifest (status reports them as miss).
    """
    names = scan(base)
    for rel in names:
        path = base / rel
        manifest.files[rel] = {"sha256": sha256_file(path), "bytes": path.stat().st_size}
    manifest.data["files"] = dict(sorted(manifest.files.items()))
    write_manifest(manifest)
    return len(names)


def write_manifest(manifest: Manifest) -> None:
    import yaml

    body = yaml.safe_dump(manifest.data, sort_keys=False, default_flow_style=False, width=1000)
    manifest.path.write_text(manifest.header + body)


STAGE_MODES = ("copy", "link")


@dataclass(frozen=True)
class StageAction:
    source: Path
    dest: Path
    mode: str   # copy (a later step writes this path) | link (its writer replaces a linked destination, never writes through it)
    state: str  # new | ok (already staged) | conflict (dest exists, differs) | missing (no source)


def _stage_state(source: Path, dest: Path, mode: str, sha256: Optional[str]) -> str:
    if not source.is_file():
        return "missing"
    if not dest.exists() and not dest.is_symlink():
        return "new"
    if mode == "link":
        return "ok" if dest.is_symlink() and dest.resolve() == source.resolve() else "conflict"
    if dest.is_symlink() or not dest.is_file() or dest.stat().st_size != source.stat().st_size:
        return "conflict"
    return "ok" if sha256_file(dest) == (sha256 or sha256_file(source)) else "conflict"


def stage_plan(manifest: Manifest, base: Path, cfg: Dict[str, Any],
               environ: Optional[Mapping[str, str]] = None) -> List[StageAction]:
    """The manifest's `stage:` block expanded per run period: {stem} is the data tree stem,
    {mc_stem} the stem of stage.mc_sample; a `to` ending in '/' is a directory."""
    block = manifest.data.get("stage")
    if not isinstance(block, dict) or not block.get("files"):
        raise ConfigError(f"{manifest.path}: no 'stage' block; nothing to stage for this channel")
    mc_sample = block.get("mc_sample")
    actions = []
    for period in gconfig.require(cfg, "periods"):
        fields = {"stem": gconfig.tree_stem(cfg, period, "data")}
        if mc_sample:
            fields["mc_stem"] = gconfig.tree_stem(cfg, period, mc_sample)
        for entry in block["files"]:
            if not isinstance(entry, dict):
                raise ConfigError(f"{manifest.path}: stage entry {entry!r} must be a mapping with from, to, mode")
            mode = entry.get("mode")
            if mode not in STAGE_MODES or not entry.get("from") or not entry.get("to"):
                raise ConfigError(f"{manifest.path}: stage entry {entry!r}: need from, to and mode in {STAGE_MODES}")
            rel = entry["from"].format(**fields)
            source = base / rel
            to = gconfig.expand_env(entry["to"], environ).format(**fields)
            dest = Path(to) / source.name if to.endswith("/") else Path(to)
            sha = manifest.files.get(rel, {}).get("sha256")
            actions.append(StageAction(source, dest, mode, _stage_state(source, dest, mode, sha)))
    return actions


class StageError(OSError):
    """Staging stopped part way; `placed` are the destinations already in place."""

    def __init__(self, message: str, placed: List[Path]):
        super().__init__(message)
        self.placed = placed


def apply_stage(actions: List[StageAction]) -> None:
    """Copy or link every `new` action (callers refuse plans with missing/conflict first).

    A copy goes to `<dest>.part` and is renamed, so an interrupted copy leaves no partial file;
    a destination that appeared since the plan is a conflict, never overwritten."""
    placed: List[Path] = []
    for action in actions:
        if action.state != "new":
            continue
        part = action.dest.with_name(action.dest.name + ".part")
        try:
            action.dest.parent.mkdir(parents=True, exist_ok=True)
            if action.dest.exists() or action.dest.is_symlink():
                raise FileExistsError(f"{action.dest} appeared since the plan (conflict); not overwritten")
            if action.mode == "copy":
                if part.exists() or part.is_symlink():
                    part.unlink()
                shutil.copy2(action.source, part)
                if action.dest.exists() or action.dest.is_symlink():
                    raise FileExistsError(f"{action.dest} appeared since the plan (conflict); not overwritten")
                os.rename(part, action.dest)
            else:
                os.symlink(action.source.resolve(), action.dest)  # fails if dest exists
        except OSError as exc:
            if part.exists() or part.is_symlink():
                part.unlink()
            raise StageError(f"{action.dest}: {exc}", placed) from exc
        placed.append(action.dest)
