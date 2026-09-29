import pytest

from gxana import config
from gxana.stages import systematics as sy

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}
SYS = "/o/kpkpxim/systematics"
EDGES = ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"]


def _cfg():
    return config.load_channel("kpkpxim")


def _plan(steps, cfg=None, environ=ENV):
    return sy.plan_systematics(cfg or _cfg(), steps, environ=environ)


def _values(argv, option):
    return [argv[i + 1] for i, a in enumerate(argv) if a == option]


def _touch_tables(tmp_path, suffixes, periods=("kpkpxim__M23_2017-01_ana56", "kpkpxim__B4_M23_2018-01_ana03")):
    d = tmp_path / "kpkpxim" / "systematics" / "xsection_data" / "johnson"
    d.mkdir(parents=True)
    for period in periods:
        for suffix in suffixes:
            (d / f"totxsec_flatTree_{period}_{suffix}.txt").write_text("x\n")
    return {**ENV, "GXANA_OUTPUT": str(tmp_path)}


def test_config_block():
    scfg = _cfg()["systematics"]
    assert scfg["label"] == "johnson"
    assert scfg["fit"]["model"] == "JohnsonMCShapeSyst"
    assert len(scfg["cuts"]) == 5 and len(scfg["barlow_macros"]) == 6


def test_bin_commands_cut_major_period_order():
    cmds = [c.argv for c in _plan(["bin"])]
    assert len(cmds) == 15
    assert cmds[0][1:3] == [
        "variation",
        f"{SYS}/variation_trees/flatTree_kpkpxim__M23_2017-01_ana56_chisqndf_gen_amp_V2_ac_YstarRest_variations.root"]
    assert cmds[0][3] == (
        f"{SYS}/variation_trees/binned_flatTree_kpkpxim__M23_2017-01_ana56_chisqndf_gen_amp_V2_ac_YstarRest_variations.root")
    assert "B4_M23_2018-01_ana03_chisqndf" in cmds[1][2]
    assert "B4_M23_2018-08_ana02_chisqndf" in cmds[2][2]
    assert "M23_2017-01_ana56_total_mm2_abs" in cmds[3][2]
    assert "kphigh_prap" in cmds[-1][2]
    assert cmds[0][cmds[0].index("--energy") + 1] == "6.4,7.4,7.86,8.19,8.45,8.68,9.26,10.18,11.4"
    assert cmds[0][cmds[0].index("--t") + 1] == "0.1,0.35,0.53,0.71,0.92,1.19,1.53,2.4"


def test_tables_commands():
    cmds = [c.argv for c in _plan(["tables"])]
    assert len(cmds) == 15
    a = cmds[0]
    assert a[a.index("--fit") + 1] == "JohnsonMCShapeSyst"
    assert _values(a, "--param") == [
        "mu=1.3217,1.32,1.33", "lambda=0.004,0.002,0.007", "gamma=-0.01,-0.5,0.5", "delta=1.2,0.2,1.5"]
    assert _values(a, "--out") == [f"{SYS}/xsection_data"]
    assert _values(a, "--plots") == [f"{SYS}/fits"]
    assert _values(a, "--weight") == ["hybrid_combo"] and _values(a, "--cheby") == ["2"]
    assert _values(a, "--label") == ["johnson"]
    binned = (f"{SYS}/variation_trees/binned_flatTree_kpkpxim__M23_2017-01_ana56_chisqndf_"
              "gen_amp_V2_ac_YstarRest_variations.root")
    assert a[-1] == (f"flatTree_kpkpxim__M23_2017-01_ana56:{binned}:{binned}:"
                     "/o/kpkpxim/xsection/binned_trees/binned_thrown_flatTree_kpkpxim__M23_2017-01_ana56_"
                     "gen_amp_V2_ac_YstarRest.root:/d/flux/flux_30274_31057_r4.root")
    assert cmds[2][-1].endswith(":/d/flux/flux_50685_51768.root")


def test_weight_discovers_variations_and_patterns(tmp_path):
    env = _touch_tables(tmp_path, ["vary_chisqndf_6", "vary_chisqndf_10", "vary_total_mm2_abs_0.01"])
    (tmp_path / "kpkpxim/systematics/xsection_data/johnson/diffxsec_flatTree_x_vary_chisqndf_6_emin_6.40_emax_7.40.txt").write_text("x\n")
    cmds = [c.argv for c in _plan(["weight"], environ=env)]
    assert len(cmds) == 3 * 9
    in_dir = str(tmp_path / "kpkpxim/systematics/xsection_data/johnson")
    assert all(c[-4] == in_dir and c[-3] == str(tmp_path / "kpkpxim/systematics/weighted_data/johnson") for c in cmds)
    pats = [c[-1] for c in cmds]
    assert pats[0] == "totxsec*_vary_chisqndf_10.txt"  # sorted suffixes
    assert pats[1:9] == [f"diffxsec*_vary_chisqndf_10_emin_{e}*.txt" for e in EDGES]
    assert pats[9] == "totxsec*_vary_chisqndf_6.txt"
    assert pats[18] == "totxsec*_vary_total_mm2_abs_0.01.txt"


def test_weight_without_tables_is_an_error(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    with pytest.raises(sy.SystematicsError, match="no variation tables"):
        _plan(["weight"], environ=env)
    assert sy.run_systematics(_cfg(), ["weight"], environ=env) == 1


def test_barlow_commands():
    cmds = _plan(["barlow"])
    assert len(cmds) == 6
    assert cmds[0].cwd == SYS
    assert cmds[0].argv[:5] == ["root", "-l", "-b", "-q", "/r/rootlogon.C"]
    assert cmds[0].argv[5] == '/r/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowChiSqNdf.C("johnson")'
    assert cmds[-1].argv[5].endswith('barlow/PlotXSecBarlowLambdaFlightSig.C("johnson")')


def test_unknown_step_rejected():
    with pytest.raises(config.ConfigError, match="unknown step"):
        _plan(["plot"])
    with pytest.raises(config.ConfigError, match="unknown step"):
        sy.run_systematics(_cfg(), ["plot"], environ=ENV)


def test_run_dry_run_prints_and_runs_nothing(capsys, tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = sy.run_systematics(_cfg(), ["bin", "tables", "weight"], dry_run=True, environ=env,
                            runner=lambda *a, **k: calls.append(a))
    out = capsys.readouterr().out
    assert rc == 0 and not calls and not (tmp_path / "kpkpxim").exists()
    assert out.count("gxana_xsec_bin") == 15 and out.count("gxana_xsec_tables") == 15
    assert "weight: commands depend on the tables step output" in out


def test_run_executes_in_order_and_stops_on_failure(tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    seen = []

    class Result:
        returncode = 3

    def runner(argv, check=False, **kwargs):
        seen.append(argv)
        return Result()

    assert sy.run_systematics(_cfg(), ["bin"], environ=env, runner=runner) == 3
    assert len(seen) == 1
    for sub in ("variation_trees", "xsection_data/johnson", "fits/johnson", "weighted_data/johnson", "plots"):
        assert (tmp_path / "kpkpxim/systematics" / sub).is_dir()


def test_barlow_needs_nominal_tables(tmp_path, capsys):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    assert sy.run_systematics(_cfg(), ["barlow"], environ=env, runner=lambda *a, **k: None) == 1
    assert "xsection" in capsys.readouterr().out
