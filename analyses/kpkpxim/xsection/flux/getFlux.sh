#!/bin/sh

#Get flux for all data sets
python2.7 /group/halld/Software/hd_utilities/psflux/plot_flux_ccdb.py -b 30274 -e 31057 -n 500 -m 6.4 -x 11.4 --rcdb-query='@is_production and @status_approved' -r 4

python2.7 /group/halld/Software/hd_utilities/psflux/plot_flux_ccdb.py -b 40856 -e 42559 -n 500 -m 6.4 -x 11.4 --rcdb-query='@is_2018production and @status_approved' -r 2 

python2.7 /group/halld/Software/hd_utilities/psflux/plot_flux_ccdb.py -b 50685 -e 51768 -n 500 -m 6.4 -x 11.4 --rcdb-query='@is_2018production and @status_approved and beam_on_current>49' -r 2 

python2.7 /group/halld/Software/hd_utilities/psflux/plot_flux_ccdb.py -b 51384 -e 51457 -n 500 -m 3.0 -x 6.0 --rcdb-query='@is_2018production and @status_approved and beam_on_current<49' -r 2 

python2.7 /group/halld/Software/hd_utilities/psflux/plot_flux_ccdb.py -b 71943 -e 73266 -n 500 -m 6.4 -x 11.4 --rcdb-query='@is_dir_production and @status_approved' -r 1 
