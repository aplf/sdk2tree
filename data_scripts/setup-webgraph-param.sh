#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."
source "$DIR/util.sh"

cd "$PROJ_ROOT"

# Create dataset directory hierarchy and specific dataset directory.
mkdir -p "$PROJ_ROOT/prepared_datasets/webgraph/$1"

printf "####################\n"
printf "#################### Preparing webgraph: $1\n"
printf "####################\n\n"

# Compile Java code for transforming datasets if necessary.
printf "> Java CLASSPATH:\n\t$CLASSPATH\n\n"
printf "> Checking if we need to compile Java utility classes.\n"

if test -f "$(realpath $PROJ_ROOT/Transforma.class)"; then
    printf "\tAlready compiled: Transforma.class.\n"
else
    printf "\tjavac -cp "$CLASSPATH" Transforma.java\n"
    javac -cp "$CLASSPATH" Transforma.java &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/java-compilation.log"
fi

if test -f "$(realpath $PROJ_ROOT/Transforma2.class)"; then
    printf "\tAlready compiled: Transforma2.class.\n\n"
else
    printf "\tjavac -cp "$CLASSPATH" Transforma2.java\n\n"
    javac -cp "$CLASSPATH" Transforma2.java &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/java-compilation.log"
fi

printf "> Checking if we need to compile the static K2-tree (UPC) implementation.\n"
if test -f "$(realpath $PROJ_ROOT/implementations/k2tree/kt_buildTree)"; then
    printf "\tAlready compiled: $(realpath $PROJ_ROOT/implementations/k2tree/kt_buildTree).\n\n"
else
    cd "$PROJ_ROOT/implementations/k2tree"
    make all &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/make_k2tree_UPC_static.log"
fi

#############################################
############### Setup the dataset. ##########
#############################################

cd "$PROJ_ROOT/prepared_datasets/webgraph/$1"

# Download $1 if necessary.
if test -f "$1.graph"; then
    printf "> $1.graph exists.\n"
else
    printf "> Downloading $1.graph:\n"
    wget http://data.law.di.unimi.it/webdata/$1/$1.graph &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/$1.graph.log"
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.graph)\n\n"
fi

if test -f "$1.properties"; then
    printf "> $1.properties exists.\n"
else
    printf "> Downloading $1.properties:\n"
    wget http://data.law.di.unimi.it/webdata/$1/$1.properties &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/$1.properties.log"
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.properties)\n\n"
fi


# Generate .offsets and .obl file from WebGraph if necessary.
if test -f "$1.offsets" && test -f "$1.obl"; then
    printf "> $1.offsets and $1.obl files exist.\n"
else
    printf "> Generating $1.offsets and $1.obl files:\n"
    java -cp "$CLASSPATH" it.unimi.dsi.webgraph.BVGraph -o -O -L $1 &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/gen-offsets-obl.log"
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.offsets)\n"
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.obl)\n\n"
    printf "\n"
fi


# Generate binary $1.adj format for the static k2-tree in $PROJ_ROOT/implementations/k2-tree.
if test -f "$1.adj"; then
    printf "> $1.adj file exists.\n"
else
    printf "> Generating $1.adj:\n"
    java -cp "$CLASSPATH" Transforma $1 &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/gen-adj.log"
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.adj)\n\n"
fi

# Generate $1.tsv file with addition commands (e.g. 'a 1 5') for our (INESC-ID) version and UPC's dynamic version.
if test -f "$1.tsv"; then
    printf "> $1.tsv file exists.\n"
else
    printf "> Generating $1.tsv:\n"
    java -cp "$CLASSPATH" Transforma2 $1 > $1.tsv
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.tsv)\n\n"
fi


# Check which suffix stream files are necessary.
STREAM_FLAGS=""
if ! test -f "$1-deletions.tsv"; then
    STREAM_FLAGS="$STREAM_FLAGS -r"
    printf "> Generating $1-deletions.tsv file.\n"
fi
if ! test -f "$1-lists.tsv"; then
    STREAM_FLAGS="$STREAM_FLAGS -l"
    printf "> Generating $1-lists.tsv file.\n"
fi
if ! test -f "$1-checks.tsv"; then
    STREAM_FLAGS="$STREAM_FLAGS -c"
    printf "> Generating $1-checks.tsv file.\n"
fi

# Generate if any is missing.
if [[ $STREAM_FLAGS ]]; then
    python3 "$PROJ_ROOT/python/make_op_streams.py" $STREAM_FLAGS -i $1.tsv &> "$PROJ_ROOT/prepared_datasets/webgraph/$1/gen-removals-lists-checks.log"

    if [[ $STREAM_FLAGS == *"-r"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1-deletions.tsv)\n"
    fi
    if [[ $STREAM_FLAGS == *"-l"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1-lists.tsv)\n\n"
    fi
    if [[ $STREAM_FLAGS == *"-c"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1-checks.tsv)\n\n"
    fi
fi

# Generate $1.kt to test the static k2-tree version.
if test -f "$1.kt"; then
    printf "> $1.kt file exists.\n"
else
    printf "> Generating $1.kt:\n"
    { time "$PROJ_ROOT/implementations/k2tree/kt_buildTree" $1.adj $1 ; } 2> $1.kt.time
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/webgraph/$1/$1.kt)\n"
fi
