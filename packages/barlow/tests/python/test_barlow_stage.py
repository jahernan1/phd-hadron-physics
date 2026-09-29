import copy
from pathlib import Path

import pytest

from gxana import config
from gxana_barlow import manifest
from gxana_barlow import stage as st
from gxana_barlow.variations import expand

ENV = {"GXANA_ROOT": "/r", "GXANA_DATA": "/d", "GXANA_OUTPUT": "/o"}
OUT = "/o/kpkpxim/barlow"
STEMS = ["kpkpxim__M23_2017-01_ana56", "kpkpxim__B4_M23_2018-01_ana03", "kpkpxim__B4_M23_2018-08_ana02"]
MC = "gen_amp_V2_ac_YstarRest"
EDGES = ["6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18", "11.40"]


def _cfg():
    return copy.deepcopy(config.load_channel("kpkpxim"))


def _plan(steps, cfg=None, environ=ENV):
    cfg = cfg or _cfg()
    return st.plan(cfg, steps, expand(cfg["barlow"]), environ=environ)


def _values(argv, option):
    return [argv[i + 1] for i, a in enumerate(argv) if a == option]


def _vfile(stem, family, out=OUT):
    return f"{out}/variation_trees/flatTree_{stem}_{family}_{MC}_variations.root"


class Ok:
    returncode = 0


def test_trees_commands_family_major():
    cmds = [c.argv for c in _plan(["trees"])]
    assert len(cmds) == 15
    a = cmds[0]
    assert a[0] == "gxana_barlow_trees"
    assert _values(a, "--tree") == ["flatTree_kpkpxim"]
    assert _values(a, "--input") == [f"/d/Trees/flatTree/rawTrees/flatTree_{STEMS[0]}.root"]
    assert _values(a, "--input-mc") == [f"/d/Trees/flatTree/rawTrees/flatTree_{STEMS[0]}_{MC}.root"]
    assert _values(a, "--define")[0] == "hybrid_combo=best_combo_rf*acc_weight"
    assert _values(a, "--define")[1] == "t_dist_truth=-(beam_p4_truth - kphigh_p4 ).M2()"
    assert _values(a, "--filter-data") == ["beam_E>6.4&&beam_E<11.4", "beam_vertexZ>50.4&&beam_vertexZ<79.1"]
    assert _values(a, "--filter-mc")[0] == "beam_E_Truth>6.4&&beam_E_Truth<11.4"
    assert len(_values(a, "--branch")) == 26
    assert _values(a, "--variation") == [
        f"vary_chisqndf_{v}=chisqndf<{v}&&total_mm2_abs<0.02&&xim_pathlensig>2&&lambda_pathlensig>0&&kphigh_prap>2&&t_dist<2.4"
        for v in ("6", "7", "9", "10")]
    assert _values(a, "--out") == [_vfile(STEMS[0], "chisqndf")]
    assert _values(a, "--threads") == ["0"]
    assert _values(cmds[1], "--out") == [_vfile(STEMS[1], "chisqndf")]
    assert len(_values(cmds[9], "--variation")) == 2  # lambda_pathlensig, first period
    assert _values(cmds[-1], "--out") == [_vfile(STEMS[2], "kphigh_prap")]


def test_check_commands():
    cmds = [c.argv for c in _plan(["check"])]
    assert len(cmds) == 15
    a = cmds[0]
    assert a[:2] == ["gxana_barlow_trees", "--check"]
    assert _values(a, "--out") == [_vfile(STEMS[0], "chisqndf")]
    assert _values(a, "--nominal") == [f"/d/flatTrees/flatTree_{STEMS[0]}_nominal_kphighrap.root"]
    assert _values(a, "--nominal-mc") == [f"/d/flatTrees/flatTree_{STEMS[0]}_{MC}_nominal_kphighrap.root"]
    assert _values(a, "--name") == [f"flatTree_{STEMS[0]}"]
    assert _values(a, "--weight") == ["hybrid_combo"]
    assert _values(a, "--yields") == [f"{OUT}/output_yields.txt"]
    assert _values(a, "--fit-dir") == [f"{OUT}/fits"]
    assert len(_values(a, "--variation")) == 4


def test_bin_commands():
    cmds = [c.argv for c in _plan(["bin"])]
    assert len(cmds) == 15
    assert cmds[0][:4] == ["gxana_xsec_bin", "variation", _vfile(STEMS[0], "chisqndf"),
                           f"{OUT}/variation_trees/binned_flatTree_{STEMS[0]}_chisqndf_{MC}_variations.root"]
    assert cmds[0][cmds[0].index("--energy") + 1] == "6.4,7.4,7.86,8.19,8.45,8.68,9.26,10.18,11.4"
    assert cmds[0][cmds[0].index("--t") + 1] == "0.1,0.35,0.53,0.71,0.92,1.19,1.53,2.4"


def test_tables_commands():
    cmds = [c.argv for c in _plan(["tables"])]
    assert len(cmds) == 15
    a = cmds[0]
    assert a[a.index("--fit") + 1] == "JohnsonMCShapeSyst"
    assert _values(a, "--param") == [
        "mu=1.3217,1.32,1.33", "lambda=0.004,0.002,0.007", "gamma=-0.01,-0.5,0.5", "delta=1.2,0.2,1.5"]
    assert _values(a, "--out") == [f"{OUT}/xsection_data"] and _values(a, "--plots") == [f"{OUT}/fits"]
    assert _values(a, "--weight") == ["hybrid_combo"] and _values(a, "--cheby") == ["2"]
    assert _values(a, "--label") == ["johnson"]
    binned = f"{OUT}/variation_trees/binned_flatTree_{STEMS[0]}_chisqndf_{MC}_variations.root"
    assert a[-1] == (f"flatTree_{STEMS[0]}:{binned}:{binned}:"
                     f"/o/kpkpxim/xsection/binned_trees/binned_thrown_flatTree_{STEMS[0]}_{MC}.root:"
                     "/d/flux/flux_30274_31057_r4.root")
    assert cmds[2][-1].endswith(":/d/flux/flux_50685_51768.root")


def test_weight_commands_follow_the_manifest():
    cmds = [c.argv for c in _plan(["weight"])]
    assert len(cmds) == 18 * 9
    assert all(c[-4] == f"{OUT}/xsection_data/johnson" and c[-3] == f"{OUT}/weighted_data/johnson" for c in cmds)
    pats = [c[-1] for c in cmds]
    assert pats[0] == "totxsec*_vary_chisqndf_6.txt"
    assert pats[1:9] == [f"diffxsec*_vary_chisqndf_6_emin_{e}*.txt" for e in EDGES[:-1]]
    assert pats[9] == "totxsec*_vary_chisqndf_7.txt"
    assert pats[-9] == "totxsec*_vary_kphigh_prap_2.2.txt"


def test_plot_commands():
    cmds = [c.argv for c in _plan(["plot"])]
    assert len(cmds) == 5
    a = cmds[0]
    assert a[0] == "gxana_barlow_plot"
    assert _values(a, "--nominal-dir") == ["/o/kpkpxim/xsection/weighted_data/johnson"]
    assert _values(a, "--var-dir") == [f"{OUT}/weighted_data/johnson"]
    assert _values(a, "--out-dir") == [f"{OUT}/plots"]
    assert _values(a, "--family") == ["chisqndf"] and _values(a, "--label") == ["#chi^{2}_{#nu} < "]
    assert _values(a, "--variation") == ["chisqndf_6=6", "chisqndf_7=7", "chisqndf_9=9", "chisqndf_10=10"]
    assert _values(a, "--energy") == [f"{lo}:{hi}" for lo, hi in zip(EDGES, EDGES[1:])]
    assert _values(a, "--canvas") == ["800,800"]
    assert _values(a, "--legend-diff") == ["0.72,0.5,0.93,0.9"]
    assert _values(a, "--y-floor") == ["8"] and _values(a, "--y-pad-diff") == ["0"]
    assert _values(a, "--canvas-def-w") == ["600"] and _values(a, "--title-offset-y") == ["0.8"]
    assert _values(a, "--title-offsets-tot") == ["0.9,0.3"] and _values(a, "--tot-y-ndiv") == ["1"]
    assert _values(a, "--threshold") == ["4.0"]
    xim, lam, kph = cmds[2], cmds[3], cmds[4]
    assert _values(xim, "--canvas") == ["default"] and _values(xim, "--title-offsets-tot") == ["1.1,0.35"]
    assert _values(lam, "--tot-y-ndiv") == ["0"] and len(_values(lam, "--variation")) == 2
    assert _values(kph, "--legend-tot") == ["0.61,0.51,0.89,0.9"]


def test_unknown_step_and_bad_config_stop_before_any_command():
    calls = []
    with pytest.raises(config.ConfigError, match="unknown step 'barlow'"):
        st.run_barlow(_cfg(), ["barlow"], environ=ENV, runner=lambda *a, **k: calls.append(a))
    cfg = _cfg()
    cfg["barlow"]["families"]["chisqndf"]["op"] = "=="
    with pytest.raises(config.ConfigError, match="op"):
        st.run_barlow(cfg, ["plot"], dry_run=True, environ=ENV, runner=lambda *a, **k: calls.append(a))
    assert not calls


def test_dry_run_prints_every_step_and_writes_nothing(capsys, tmp_path):
    env = {**ENV, "GXANA_OUTPUT": str(tmp_path)}
    calls = []
    rc = st.run_barlow(_cfg(), list(st.STEPS), dry_run=True, environ=env, runner=lambda *a, **k: calls.append(a))
    out = capsys.readouterr().out
    assert rc == 0 and not calls and not any(tmp_path.iterdir())
    assert out.count("gxana_barlow_trees --check") == 15
    assert out.count("gxana_barlow_trees") == 30 and out.count("gxana_xsec_bin") == 15
    assert out.count("gxana_xsec_tables") == 15 and out.count("weighted_average") == 162
    assert out.count("gxana_barlow_plot") == 5


def _touch(paths):
    for p in paths:
        Path(p).parent.mkdir(parents=True, exist_ok=True)
        Path(p).write_text("x")


def _tmp_env(tmp_path):
    return {**ENV, "GXANA_DATA": str(tmp_path / "d"), "GXANA_OUTPUT": str(tmp_path / "o")}


def test_trees_preflight_lists_every_missing_input(tmp_path, capsys):
    env = _tmp_env(tmp_path)
    raw = tmp_path / "d/Trees/flatTree/rawTrees"
    _touch([raw / f"flatTree_{s}.root" for s in STEMS] + [raw / f"flatTree_{STEMS[0]}_{MC}.root"])
    calls = []
    assert st.run_barlow(_cfg(), ["trees"], environ=env, runner=lambda *a, **k: calls.append(a)) == 1
    err = capsys.readouterr().err
    assert not calls
    assert "trees: missing inputs" in err
    assert f"flatTree_{STEMS[1]}_{MC}.root" in err and f"flatTree_{STEMS[2]}_{MC}.root" in err
    assert f"flatTree_{STEMS[0]}_{MC}.root" not in err


def test_trees_writes_the_manifest_then_runs(tmp_path):
    env = _tmp_env(tmp_path)
    raw = tmp_path / "d/Trees/flatTree/rawTrees"
    _touch([raw / f"flatTree_{s}.root" for s in STEMS] + [raw / f"flatTree_{s}_{MC}.root" for s in STEMS])
    seen = []
    rc = st.run_barlow(_cfg(), ["trees"], environ=env, runner=lambda argv, **k: seen.append(argv) or Ok())
    assert rc == 0 and len(seen) == 15
    out = tmp_path / "o/kpkpxim/barlow"
    assert len(manifest.load_checked(out, _cfg()["barlow"])) == 18
    for sub in ("variation_trees", "xsection_data/johnson", "fits/johnson", "weighted_data/johnson", "plots"):
        assert (out / sub).is_dir()


def test_later_steps_need_a_current_manifest(tmp_path, capsys):
    env = _tmp_env(tmp_path)
    runner = lambda *a, **k: Ok()  # noqa: E731
    assert st.run_barlow(_cfg(), ["bin"], environ=env, runner=runner) == 1
    assert "run --steps trees first" in capsys.readouterr().err
    out = tmp_path / "o/kpkpxim/barlow"
    manifest.write(out, manifest.build(_cfg()["barlow"]))
    cfg = _cfg()
    cfg["barlow"]["families"]["chisqndf"]["values"][0] = "5"
    assert st.run_barlow(cfg, ["bin"], environ=env, runner=runner) == 1
    assert "config changed since trees; rerun --steps trees" in capsys.readouterr().err


def test_bin_preflight_and_stop_on_first_failure(tmp_path, capsys):
    env = _tmp_env(tmp_path)
    out = tmp_path / "o/kpkpxim/barlow"
    manifest.write(out, manifest.build(_cfg()["barlow"]))
    files = [_vfile(s, f, str(out)) for f in ("chisqndf", "total_mm2_abs", "xim_pathlensig",
                                              "lambda_pathlensig", "kphigh_prap") for s in STEMS]
    _touch(files[:-1])
    assert st.run_barlow(_cfg(), ["bin"], environ=env, runner=lambda *a, **k: Ok()) == 1
    assert files[-1] in capsys.readouterr().err
    _touch(files[-1:])
    seen = []

    class Fail:
        returncode = 3

    assert st.run_barlow(_cfg(), ["bin"], environ=env, runner=lambda argv, **k: seen.append(argv) or Fail()) == 3
    assert len(seen) == 1


def test_check_truncates_output_yields(tmp_path):
    env = _tmp_env(tmp_path)
    out = tmp_path / "o/kpkpxim/barlow"
    manifest.write(out, manifest.build(_cfg()["barlow"]))
    fams = ("chisqndf", "total_mm2_abs", "xim_pathlensig", "lambda_pathlensig", "kphigh_prap")
    _touch([_vfile(s, f, str(out)) for f in fams for s in STEMS])
    _touch([tmp_path / f"d/flatTrees/flatTree_{s}_nominal_kphighrap.root" for s in STEMS])
    _touch([tmp_path / f"d/flatTrees/flatTree_{s}_{MC}_nominal_kphighrap.root" for s in STEMS])
    (out / "output_yields.txt").write_text("old row\n")
    assert st.run_barlow(_cfg(), ["check"], environ=env, runner=lambda *a, **k: Ok()) == 0
    assert (out / "output_yields.txt").read_text() == ""


def test_plot_preflight_needs_the_nominal_tables(tmp_path, capsys):
    env = _tmp_env(tmp_path)
    out = tmp_path / "o/kpkpxim/barlow"
    manifest.write(out, manifest.build(_cfg()["barlow"]))
    assert st.run_barlow(_cfg(), ["plot"], environ=env, runner=lambda *a, **k: Ok()) == 1
    err = capsys.readouterr().err
    assert "xsection/weighted_data/johnson/totxsec_weighted_output.txt" in err
    assert "barlow/weighted_data/johnson/weighted_totxsec_vary_chisqndf_6.txt" in err


def test_weight_commands_helper():
    cmds = st.weight_commands("/i", "/w", ["a_1"], [6.4, 7.4, 11.4])
    assert [c.argv[-1] for c in cmds] == ["totxsec*_vary_a_1.txt", "diffxsec*_vary_a_1_emin_6.40*.txt",
                                          "diffxsec*_vary_a_1_emin_7.40*.txt"]
    assert all(c.step == "weight" and c.argv[-4:-2] == ["/i", "/w"] for c in cmds)
