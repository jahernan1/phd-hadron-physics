"""Nominal terms + family values -> one Variation per cut value (spec §6)."""
from __future__ import annotations

from dataclasses import asdict, dataclass
from typing import Any, Dict, List


@dataclass(frozen=True)
class Variation:
    id: str
    family: str
    value: str
    cut: str
    tree: str
    label: str

    @property
    def mc_tree(self) -> str:
        return f"{self.tree}_mc"

    def to_dict(self) -> Dict[str, str]:
        return asdict(self)


def expand(bcfg: Dict[str, Any]) -> List[Variation]:
    """Nominal terms in config order with the family's term replaced by
    `family op value`, then the fixed terms, joined with '&&' -- the form the legacy
    replaceValueInDelimitedString produced (GetVariationTreesUML.C:199-227)."""
    nominal = bcfg["nominal"]
    fixed = list(bcfg.get("fixed") or [])
    out = []
    for family, fam in bcfg["families"].items():
        for value in fam["values"]:
            terms = [f"{family}{fam['op']}{value}" if key == family else term for key, term in nominal.items()]
            vid = f"{family}_{value}"
            out.append(Variation(vid, family, value, "&&".join(terms + fixed), f"vary_{vid}",
                                 f"{fam['label']}{value}"))
    return out
