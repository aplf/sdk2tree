#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

IMPLEMENTATION="sdk2tree"

# Create evaluation results directory if it does not exist.
#OUT_DIR="$PROJ_ROOT/evaluated_dataset/$2/$IMPLEMENTATION"
OUT_DIR="$4/$2/$IMPLEMENTATION"
mkdir -p $OUT_DIR

cd $OUT_DIR

K2TREE_BIN="$PROJ_ROOT/implementations/$IMPLEMENTATION/project"

STREAMER_BIN="$DIR/streamer.sh"

printf "####################\n"
printf "#################### $IMPLEMENTATION: dynamic (INESC-ID) collection dynamization k2-tree evaluation: $1/$2\n"
printf "####################\n\n"

printf "> Results will be written to:\t$OUT_DIR\n\n"

printf "#########\n"
printf "##### Scenario: additions only (A).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv"; then
    printf "> $OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv:\n"
    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-A-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-A.err\n"
    /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-A-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-A.err ;

    echo $(pwd)

    mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-A-time.tsv
    mkdir -p kt-A
    mv *.kt kt-A/
fi

printf "\n#########\n"
printf "##### Scenario: additions + deletions (AD).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-AD-time.tsv"; then
    printf "> $OUT_DIR/$2-$IMPLEMENTATION-AD-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-AD-time.tsv:\n"
    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AD-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-deletions.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AD.err\n"
    /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AD-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-deletions.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AD.err ;

    mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-AD-time.tsv
    mkdir -p  kt-AD
    mv *.kt kt-AD/
fi

printf "\n#########\n"
printf "##### Scenario: additions + checks (AC).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-AC-time.tsv"; then
    printf "> $OUT_DIR/$2-$IMPLEMENTATION-AC-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-AC-time.tsv:\n"
    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AC-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AC.err\n"
    /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AC-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AC.err ;

    mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-AC-time.tsv
    mkdir -p  kt-AC
    mv *.kt kt-AC/
fi

printf "\n#########\n"
printf "##### Scenario: additions + lists (AL).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-AL-time.tsv"; then
    printf "> $OUT_DIR/$2-$IMPLEMENTATION-AL-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-AL-time.tsv:\n"
    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AL-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-lists.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AL.err\n"
    /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AL-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-lists.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AL.err ;

    mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-AL-time.tsv
    mkdir -p kt-AL
    mv *.kt kt-AL/
fi
