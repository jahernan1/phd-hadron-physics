#!/bin/bash

# preliminaries
export GLUEX_TOP=/d/grid17/gluex/gluex_top
export BUILD_SCRIPTS=$GLUEX_TOP/build_scripts
export BMS_OSNAME=`$BUILD_SCRIPTS/osrelease.pl`
# gluex software versions
#source $BUILD_SCRIPTS/gluex_env_jlab.csh  /group/halld/www/halldweb/html/dist/version_recon-2017_01-ver03_3.xml
source $BUILD_SCRIPTS/gluex_env_version.sh  $GLUEX_TOP/halld_versions/analysis-2017_01-ver30.xml
export MCWRAPPER_CENTRAL=/d/grid17/hjesse/MC/gluex_MCwrapper
#setenv CCDB_CONNECTION mysql://ccdb_user@hallddb.jlab.org/ccdb
#setenv JANA_CALIB_URL mysql://ccdb_user@hallddb.jlab.org/ccdb
#setenv JANA_RESOURCE_DIR /group/halld/www/halldweb/html/resources
#if ( ! $?JANA_PLUGIN_PATH ) then
#    setenv JANA_PLUGIN_PATH 
#endif

export PATH=/d/grid17/gluex/gluex_top/rcdb/rcdb_0.03.01/:$PATH

# for eventstore
export PYTHONPATH=$HALLD_HOME/$BMS_OSNAME/lib/python2:$PYTHONPATH

unset HALLD_MY
