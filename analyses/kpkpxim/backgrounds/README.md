# Background channels

Reactions that feed down into, or share final-state particles with,
γp → K⁺K⁺Ξ⁻, studied to understand the background under the Ξ⁻ peak.

| Channel | Selectors (`selectors/`) | `dOutputFileName` |
|---|---|---|
| γp → π⁰K⁺K⁺Ξ⁻ | `DSelector_pi0kpkpxim.C/.h` | `pi0kpkpxim.root` (flat tree `flatTree_pi0kpkpxim.root`) |
| γp → π⁺π⁻K⁺Λ | `DSelector_pippimkplamb.C/.h`, `DSelector_thrown_pippimkplamb.C/.h` | `pippimkplamb.root`, `thrown_pippimkplamb.root` |

Fit macros: `KstarFit.C` (K* contribution), `YstarBWFitsData.C`
(Breit–Wigner fits to Y* → K⁺Ξ⁻ structures in data).

Like the main selectors, these need the GlueX environment to load.
