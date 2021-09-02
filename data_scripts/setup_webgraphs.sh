#!/usr/bin/env bash

#######################################################
####################### Real-world WebGraph files. ####
#######################################################

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT=$DIR/..

cd $DIR


./setup-webgraph-param.sh uk-2007-05@100000
./setup-webgraph-param.sh in-2004
./setup-webgraph-param.sh eu-2015-host
./setup-webgraph-param.sh indochina-2004
./setup-webgraph-param.sh uk-2014-host

cd $PROJ_ROOT