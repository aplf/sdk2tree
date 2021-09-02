#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

IMPLEMENTATION="k2trie"





K2TREE_BIN="$PROJ_ROOT/implementations/$IMPLEMENTATION/project"

STREAMER_BIN="$DIR/streamer.sh"

printf "####################\n"
printf "#################### $IMPLEMENTATION: dynamic k2-trie evaluation: $1/$2\n"
printf "####################\n\n"

printf "###############\n"
printf "############### SCENARIO 1: 96 96 256 12 16 0.99 $3\n"
printf "###############\n\n"

# Create evaluation results directory if it does not exist.

OUT_DIR="$4/$2/$IMPLEMENTATION-1"
mkdir -p "$OUT_DIR"
cd $OUT_DIR

printf "#########\n"
printf "##### Scenario: additions only (A).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-${IMPLEMENTATION}_1-A-time.tsv"; then
    printf "> $OUT_DIR/$2-${IMPLEMENTATION}_1-A-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-${IMPLEMENTATION}_1-A-time.tsv:\n"

    printf "/usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_1-A-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-A.err ;"

    mkdir -p "kt-1-A"
    cd "kt-1-A"

	/usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_1-A-memory.txt $K2TREE_BIN 96 96 1024 8 10 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2>> $OUT_DIR/$2-${IMPLEMENTATION}_1-A.err ;

    mv ${IMPLEMENTATION}_times.tsv $OUT_DIR/$2-${IMPLEMENTATION}_1-A-time.tsv
fi

cd $OUT_DIR


printf "\n#########\n"
printf "##### Scenario: additions + checks (AC).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-AC-time.tsv"; then
    printf "> $OUT_DIR/$2-${IMPLEMENTATION}_1-AC-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-${IMPLEMENTATION}_1-AC-time.tsv:\n"

    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-AC-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-AC.err\n"

    mkdir -p "kt-1-AC"
    cd "kt-1-AC"

    /usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_1-AC-memory.txt $K2TREE_BIN 96 96 1024 8 10 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2>> $OUT_DIR/$2-${IMPLEMENTATION}_1-AC.err ;

    mv ${IMPLEMENTATION}_times.tsv $OUT_DIR/$2-${IMPLEMENTATION}_1-AC-time.tsv    
fi

printf "\n###############\n"
printf "############### SCENARIO 2: 96 96 256 12 16 0.99 $3\n"
printf "###############\n\n"

# Create evaluation results directory if it does not exist.
OUT_DIR="$4/$2/$IMPLEMENTATION-2"
mkdir -p "$OUT_DIR"
cd $OUT_DIR



printf "#########\n"
printf "##### Scenario: additions only (A).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-${IMPLEMENTATION}_2-A-time.tsv"; then
    printf "> $OUT_DIR/$2-${IMPLEMENTATION}_2-A-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-${IMPLEMENTATION}_2-A-time.tsv:\n"

    printf "/usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_2-A-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-${IMPLEMENTATION}_2-A.err ;"

    mkdir -p "kt-2-A"
    cd "kt-2-A"

    /usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_2-A-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2>> $OUT_DIR/$2-${IMPLEMENTATION}_2-A.err ;

    mv ${IMPLEMENTATION}_times.tsv $OUT_DIR/$2-${IMPLEMENTATION}_2-A-time.tsv

    
fi

cd $OUT_DIR

printf "\n#########\n"
printf "##### Scenario: additions + checks (AC).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-${IMPLEMENTATION}_2-AC-time.tsv"; then
    printf "> $OUT_DIR/$2-${IMPLEMENTATION}_2-AC-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-${IMPLEMENTATION}_2-AC-time.tsv:\n"

    printf "/usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_2-AC-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2> $OUT_DIR/$2-${IMPLEMENTATION}_2-AC.err\n"

    mkdir -p "kt-2-AC"
    cd "kt-2-AC"

	/usr/bin/time -v -o $OUT_DIR/$2-${IMPLEMENTATION}_2-AC-memory.txt $K2TREE_BIN 96 96 256 12 16 0.99 $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $PROJ_ROOT/$1/$2-checks.tsv ) > /dev/null 2>> $OUT_DIR/$2-${IMPLEMENTATION}_2-AC.err ;

    mv ${IMPLEMENTATION}_times.tsv $OUT_DIR/$2-${IMPLEMENTATION}_2-AC-time.tsv

    
fi
