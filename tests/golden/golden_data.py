"""Constants shared by the golden tests (legacy kpkpxim naming)."""

# Tree stems of the three thesis run periods, in the order the legacy
# MakeXSecFitVariations.C processed them (fit parameters carry over).
PERIOD_TREES = (
    "kpkpxim__M23_2017-01_ana56",
    "kpkpxim__B4_M23_2018-01_ana03",
    "kpkpxim__B4_M23_2018-08_ana02",
)

# Legacy FitFunctions.cpp GetFluxHist: period -> flux file.
FLUX_FILES = {
    "kpkpxim__M23_2017-01_ana56": "flux/flux_30274_31057_r4.root",
    "kpkpxim__B4_M23_2018-01_ana03": "flux/flux_40856_42559.root",
    "kpkpxim__B4_M23_2018-08_ana02": "flux/flux_50685_51768.root",
}

# (label, period) of the legacy components/ layout.
PERIOD_LABELS = (("sp17", "2017-01"), ("sp18", "2018-01"), ("fa18", "2018-08"))

# Lower edges of the eight beam-energy bins, as they appear in file names.
ENERGY_BIN_LOWS = ("6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18")
