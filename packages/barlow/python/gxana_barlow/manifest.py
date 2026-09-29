"""<output_dir>/variations.json: the variation list every step after `trees` reads."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List

from gxana_barlow.config import config_hash
from gxana_barlow.variations import Variation, expand

MANIFEST = "variations.json"


class ManifestError(RuntimeError):
    """variations.json is missing or was written from a different config."""


def build(bcfg: Dict[str, Any]) -> Dict[str, Any]:
    return {"config_hash": config_hash(bcfg), "variations": [v.to_dict() for v in expand(bcfg)]}


def write(output_dir: Path, manifest: Dict[str, Any]) -> Path:
    path = Path(output_dir) / MANIFEST
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(manifest, indent=2) + "\n")
    return path


def read(output_dir: Path) -> Dict[str, Any]:
    path = Path(output_dir) / MANIFEST
    if not path.is_file():
        raise ManifestError(f"no {MANIFEST} in {output_dir}; run --steps trees first")
    return json.loads(path.read_text())


def load_checked(output_dir: Path, bcfg: Dict[str, Any]) -> List[Variation]:
    data = read(output_dir)
    if data.get("config_hash") != config_hash(bcfg):
        raise ManifestError("config changed since trees; rerun --steps trees")
    return [Variation(**entry) for entry in data["variations"]]
