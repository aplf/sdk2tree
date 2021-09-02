#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."
source "$DIR/util.sh"

cd "$PROJ_ROOT"

# Create dataset directory hierarchy and specific dataset directory.
mkdir -p "$PROJ_ROOT/prepared_datasets/dmgen/$1"

printf "####################\n"
printf "#################### Preparing dmgenerator: $1\n"
printf "####################\n\n"

# Compile Java code for transforming datasets if necessary.
printf "> Java CLASSPATH:\n\t$CLASSPATH\n\n"
printf "> Checking if we need to compile Java utility classes.\n"

if test -f "$(realpath $PROJ_ROOT/data_scripts/BinarizeAdjacencyTSV.class)"; then
    printf "\tAlready compiled: BinarizeAdjacencyTSV.class.\n\n"
else
    printf "\tjavac -cp "$CLASSPATH" $PROJ_ROOT/data_scripts/BinarizeAdjacencyTSV.java\n\n"
    printf "\t\t PROJ ROOT: $PROJ_ROOT"
    javac -cp "$CLASSPATH" $PROJ_ROOT/data_scripts/BinarizeAdjacencyTSV.java &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/java-compilation.log"
fi

printf "> Checking if we need to compile the static K2-tree (UPC) implementation.\n"
if test -f "$(realpath $PROJ_ROOT/implementations/k2tree/kt_buildTree)"; then
    printf "\tAlready compiled: $(realpath $PROJ_ROOT/implementations/k2tree/kt_buildTree).\n\n"
else
    cd "$PROJ_ROOT/implementations/k2tree"
    make all &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/make_k2tree_UPC_static.log"
fi

printf "> Checking if we need to compile the dmgenerator program.\n"
if test -f "$(realpath $PROJ_ROOT/data_scripts/dmgenerator/dmgen)"; then
    printf "\tAlready compiled: $(realpath $PROJ_ROOT/data_scripts/dmgenerator/dmgen).\n\n"
else
    printf "\tcd $PROJ_ROOT/data_scripts/dmgenerator\n\n"
    cd "$PROJ_ROOT/data_scripts/dmgenerator"
    
    printf "\tmake clean all\n\n"
    make clean all &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/dmgenerator-compilation.log"

    printf "\tcd $PROJ_ROOT\n\n"
    cd "$PROJ_ROOT"
fi


#############################################
############### Setup the dataset. ##########
#############################################

cd "$PROJ_ROOT/prepared_datasets/dmgen/$1"

# Generate (if necessary) $1.tsv file with addition commands (e.g. 'a 1 5') for our (INESC-ID) version and UPC's dynamic version.
if test -f "$1.tsv"; then
    printf "> $1.tsv exists.\n"
else
    printf "> Generating $1.tsv:\n"
    { time $PROJ_ROOT/data_scripts/dmgenerator/dmgen $1 0.5 1 1> $1.tsv ; } 2> $1.dmgen.time
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1.tsv)\n\n"
fi

# Generate binary $1.adj format for the static k2-tree in $PROJ_ROOT/implementations/k2-tree.
if test -f "$1.adj"; then
    printf "> $1.adj file exists.\n"
else
    printf "> Generating $1.adj:\n"
    pushd "$PROJ_ROOT/data_scripts"
    java -cp "$CLASSPATH" BinarizeAdjacencyTSV $PROJ_ROOT/prepared_datasets/dmgen/$1/$1.tsv &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/gen-adj.log"
    popd
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1.adj)\n\n"
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
    python3 "$PROJ_ROOT/python/make_op_streams.py" $STREAM_FLAGS -i $1.tsv &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/gen-removals-lists-checks.log"

    printf "\tpython3 "$PROJ_ROOT/python/make_op_streams.py" $STREAM_FLAGS -i $1.tsv &> "$PROJ_ROOT/prepared_datasets/dmgen/$1/gen-removals-lists-checks.log"\n\n"

    if [[ $STREAM_FLAGS == *"-r"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1-deletions.tsv)\n"
    fi
    if [[ $STREAM_FLAGS == *"-l"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1-lists.tsv)\n"
    fi
    if [[ $STREAM_FLAGS == *"-c"* ]]; then
        printf "\tNew: $(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1-checks.tsv)\n"
    fi
fi

# Generate $1.kt to test the static k2-tree version.
if test -f "$1.kt"; then
    printf "> $1.kt file exists.\n"
else
    printf "> Generating $1.kt:\n"
    { time "$PROJ_ROOT/implementations/k2tree/kt_buildTree" $1.adj $1 ; } 2> $1.kt.time
    printf "\t$(realpath $PROJ_ROOT/prepared_datasets/dmgen/$1/$1.kt)\n"
fi
