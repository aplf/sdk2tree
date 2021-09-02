#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

"$DIR"/eval-k2sdsl-dmgen-algo.sh $1
"$DIR"/eval-k2sdsl-webgraph-algo.sh $1