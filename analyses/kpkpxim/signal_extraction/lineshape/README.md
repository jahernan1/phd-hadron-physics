# Single-bin lineshape fits

Single-bin examples of the cross-section yield fits (M(Λπ⁻), Johnson signal
shape fixed from MC, data fitted with Johnson + Chebychev), and lineshape-width
checks. They document the fit model behind the cross-section yields (the
production fits run in `gxana run xsection`, `packages/xsection`).

## OneUMLFit.C (dissertation chapter 6 example fits, chapter 7 fit-model figures)

Entry `OneUMLFit()`. `GetXim1320_IM(stem, bin)` opens the binned trees
`$GXANA_DATA/flatTrees/binned_<stem>_nominal_kphighrap.root` and
`binned_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`, fits the MC
Johnson shape and then the data, weighted by `hybrid_combo`. As preserved it
fits one bin of Spring 2017 (`flatTree_kpkpxim__M23_2017-01_ana56`,
`emin_9.26_emax_10.18_tmin_1.19_tmax_1.53`); edit the call in `OneUMLFit()`
to choose another period or bin.

Input: the `bin` step of `gxana run xsection` writes the binned trees to
`$GXANA_OUTPUT/kpkpxim/xsection/binned_trees/`; the macro reads
`$GXANA_DATA/flatTrees/`, so copy or link them first (as for the thrown
trees in the channel README).

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/xsection/fits
cd $GXANA_OUTPUT/kpkpxim/xsection/fits
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/signal_extraction/lineshape/OneUMLFit.C
```

Output: the fit results are printed to the terminal. The PDF writes to
`$GXANA_OUTPUT/kpkpxim/xsection/fits/<weight>/` (`recon_*.pdf`, `data_*.pdf`)
are commented out in the macro (`fitCan->Print`), so uncomment them to get the
figures (the directory is created by the macro).

## SingleGaussianFit.C, DoubleGaussianFit.C (chapter 7, low priority)

Entries `SingleGaussianFit()` and `DoubleGaussianFit()`: the same binned
inputs fitted with a single or double Gaussian signal to check the lineshape
width. One bin of Spring 2017 (`emin_7.40_emax_7.86_tmin_0.10_tmax_0.35`) is
hard-coded and the `SaveAs` calls are commented out, so they only print fit
results. Run as `OneUMLFit.C` with the file name replaced.
