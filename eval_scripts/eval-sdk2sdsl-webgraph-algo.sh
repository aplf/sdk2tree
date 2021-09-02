#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

# Create evaluation results directory if it does not exist.
#mkdir -p "$PROJ_ROOT/evaluated_dataset"
mkdir -p "$1"

"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1 PR
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1 BFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1 DFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1 CC
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2007-05@100000 uk-2007-05@100000 100000 $1 CT_dummy_hash

"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1 PR
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1 BFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1 DFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1 CC
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/in-2004 in-2004 1382908 $1 CT_dummy_hash

"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1 PR
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1 BFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1 DFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1 CC
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/uk-2014-host uk-2014-host 4769354 $1 CT_dummy_hash

"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1 PR
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1 BFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1 DFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1 CC
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/indochina-2004 indochina-2004 7414866 $1 CT_dummy_hash

"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1 PR
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1 BFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1 DFS
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1 CC
"$DIR"/eval-sdk2sdsl-param.sh prepared_datasets/webgraph/eu-2015-host eu-2015-host 11264052 $1 CT_dummy_hash
