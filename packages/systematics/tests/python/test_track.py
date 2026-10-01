import pytest

from gxana_systematics import track

COUNTS = {  # MC counts of the 2026-09-30 run (data columns abbreviated)
    "kp1": (10483.7, 0.0, 127564.0, 0.0),
    "kp2": (337.243, 10146.4, 3836.44, 123726.0),
    "pim1": (6394.36, 4085.63, 74361.6, 53165.9),
    "pim2": (7725.93, 2754.39, 90216.3, 37341.6),
    "proton": (9514.81, 968.851, 109214.0, 18349.4),
}


def test_raw_and_override():
    eff = track.efficiencies(COUNTS, 0.03, 0.05, {"proton": 0.05})
    assert eff["kp1"]["mc"] == pytest.approx(0.03)
    assert eff["kp2"]["mc_raw"] == pytest.approx(0.0493985, abs=1e-7)
    assert eff["proton"]["mc_raw"] == pytest.approx(0.0328769, abs=1e-7)
    assert eff["proton"]["mc"] == 0.05 and eff["proton"]["data"] == 0.05


def test_cli_totals(tmp_path):
    counts = tmp_path / "track_counts.txt"
    counts.write_text("particle nlow_data nhigh_data nlow_mc nhigh_mc\n" +
                      "".join(f"{k} {' '.join(str(v) for v in c)}\n" for k, c in COUNTS.items()))
    out = tmp_path / "track_efficiency.txt"
    assert track.main(["--counts", str(counts), "--out", str(out), "--low", "0.03", "--high", "0.05",
                       "--override", "proton=0.05", "--report", "mc"]) == 0
    lines = out.read_text().splitlines()
    assert lines[0] == "particle data mc data_raw mc_raw"
    total = lines[-2].split()
    assert total[0] == "total" and float(total[4]) == pytest.approx(0.186468, abs=1e-6)
    assert float(total[2]) == pytest.approx(0.186468 - 0.0328769 + 0.05, abs=1e-6)
    assert lines[-1].split()[:2] == ["report", "mc"]


def test_zero_counts_particle_is_an_error():
    with pytest.raises(ValueError, match="kp1"):
        track.efficiencies({"kp1": (0.0, 0.0, 1.0, 1.0)}, 0.03, 0.05, {})
