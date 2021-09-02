#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

# Create evaluation results directory if it does not exist.
#mkdir -p "$PROJ_ROOT/evaluated_dataset"

mkdir -p "$1"

"$DIR"/eval-dk2tree-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1
"$DIR"/eval-dk2tree-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1
"$DIR"/eval-dk2tree-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1
"$DIR"/eval-dk2tree-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1
"$DIR"/eval-dk2tree-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1
