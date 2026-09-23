"""Preserved analysis data: golden inputs and reference outputs kept outside git.

GlueX practice: analysis code is published on GitHub and the data it analysed
is preserved on disk under /work/halld/gluex_analysis_data/ at JLab. Here that
directory is $GXANA_ANALYSIS_DATA (default <repo>/gluex_analysis_data,
gitignored); each channel documents its files, with sha256 and size, in
analyses/<channel>/analysis_data.yaml (docs/analysis_data.md).
"""
from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional

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
