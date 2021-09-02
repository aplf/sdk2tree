#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

# Create evaluation results directory if it does not exist.
#mkdir -p "$PROJ_ROOT/evaluated_dataset"
mkdir -p "$1"

"$DIR"/eval-k2trie-param.sh prepared_datasets/dmgen/50000 50000 50000 $1
"$DIR"/eval-k2trie-param.sh prepared_datasets/dmgen/100000 100000 100000 $1
"$DIR"/eval-k2trie-param.sh prepared_datasets/dmgen/500000 500000 500000 $1
"$DIR"/eval-k2trie-param.sh prepared_datasets/dmgen/1000000 1000000 1000000 $1
