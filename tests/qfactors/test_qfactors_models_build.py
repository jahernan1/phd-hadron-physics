"""Every kpkpxim fit model compiles against the fork engine (syntax-only g++)."""
import shutil
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import qfactors

from qfactors_helpers import gx_env

CFG = config.load_channel("kpkpxim")
ROOT = Path(__file__).resolve().parents[2]
MODELS = sorted(p.name for p in (ROOT / "packages/qfactors").glob("configPDFs*.h"))
pytestmark = [
    pytest.mark.skipif(not (ROOT / "packages/qfactors/main.C").is_file(), reason="packages/qfactors not checked out"),
    pytest.mark.skipif(shutil.which("root-config") is None, reason="ROOT not on PATH"),
]


@pytest.mark.parametrize("model", MODELS)
def test_model_compiles_with_engine(model, tmp_path):
    job = qfactors.plan_qfactors(CFG, "2017-01", model=model, environ=gx_env(tmp_path))
    qfactors.stage(job)
    qfactors.compile_main(job, syntax_only=True)


def test_channel_model_path_compiles_with_engine(tmp_path):
    model = tmp_path / "qfactors_models" / "configPDFs_X.h"
    model.parent.mkdir()
    shutil.copyfile(ROOT / "packages/qfactors/configPDFs.h", model)
    job = qfactors.plan_qfactors(CFG, "2017-01", model=str(model), environ=gx_env(tmp_path))
    qfactors.stage(job)
    assert (job.work_dir / "configPDFs.h").read_bytes() == model.read_bytes()
    qfactors.compile_main(job, syntax_only=True)
