#!/usr/bin/env bash

#######################################################
##################### Synthetic dmgenerator files. ####
#######################################################

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT=$DIR/..

cd $DIR


./setup-dmgen-param.sh 50000
./setup-dmgen-param.sh 100000
./setup-dmgen-param.sh 500000
./setup-dmgen-param.sh 1000000
#./setup-dmgen-param.sh 10000000

cd $PROJ_ROOT