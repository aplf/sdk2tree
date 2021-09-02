#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

"$DIR"/eval-dk2tree-dmgen.sh $1
"$DIR"/eval-dk2tree-webgraph.sh $1

"$DIR"/eval-k2sdsl-dmgen.sh $1
"$DIR"/eval-k2sdsl-webgraph.sh $1

"$DIR"/eval-k2tree-dmgen.sh $1
"$DIR"/eval-k2tree-webgraph.sh $1

"$DIR"/eval-k2trie-dmgen.sh $1
"$DIR"/eval-k2trie-webgraph.sh $1

"$DIR"/eval-sdk2tree-dmgen.sh $1
"$DIR"/eval-sdk2tree-webgraph.sh $1
