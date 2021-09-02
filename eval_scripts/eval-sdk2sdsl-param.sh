#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PROJ_ROOT="$DIR/.."

cd "$PROJ_ROOT"

IMPLEMENTATION="sdk2sdsl"

# Create evaluation results directory if it does not exist.
#OUT_DIR="$PROJ_ROOT/evaluated_dataset/$2/$IMPLEMENTATION"
OUT_DIR="$4/$2/$IMPLEMENTATION"

#echo $OUT_DIR
#exit 0

mkdir -p $OUT_DIR

cd $OUT_DIR

K2TREE_BIN="$PROJ_ROOT/implementations/$IMPLEMENTATION/project"

STREAMER_BIN="$DIR/streamer.sh"

printf "\n####################\n"
printf "#################### $IMPLEMENTATION: dynamic (sdsl-lite) k2-tree evaluation: $1/$2\n"
printf "####################\n\n"

printf "> Results will be written to:\t$OUT_DIR\n\n"


if [ "$#" -eq 5 ]; then
    printf "> Checking if we need to run $IMPLEMENTATION graph algorithm: $5.\n\n"

    if test -f "$OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5-time.tsv"; then
        printf "> $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5-time.tsv file exists - no need to run.\n"
    else 
        set -x
        
        rm -rf dktree_serialize/
        
        printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5-time.tsv:\n"
        printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $5 ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5.err\n"
        /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv $5 ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-ALGO-$5.err ;

        if ! test -f "$2-$IMPLEMENTATION-ALGO-$5-output.txt"; then
            mv $3-$IMPLEMENTATION-ALGO-$5-output.txt $2-$IMPLEMENTATION-ALGO-$5-output.txt
        fi
        
        mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-ALGO-$5-time.tsv
        rm -rf kt-ALGO-$5/
        mv dktree_serialize/ kt-ALGO-$5/
    fi

    exit 0
fi


printf "#########\n"
printf "##### Scenario: additions only (A).\n"
printf "#########\n\n"

if test -f "$OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv"; then
    printf "> $OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv file exists.\n"
else 
    printf "> Generating $OUT_DIR/$2-$IMPLEMENTATION-A-time.tsv:\n"
    printf "/usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-A-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-A.err\n"
    /usr/bin/time -v -o $OUT_DIR/$2-$IMPLEMENTATION-A-memory.txt $K2TREE_BIN $3 < <( $STREAMER_BIN $PROJ_ROOT/$1/$2.tsv ) > /dev/null 2> $OUT_DIR/$2-$IMPLEMENTATION-A.err ;

    mv ${IMPLEMENTATION}_times.tsv $2-$IMPLEMENTATION-A-time.tsv
    #mkdir -p kt-A
    mv dktree_serialize/ kt-A/
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
    #mkdir -p  kt-AD
    mv dktree_serialize/ kt-AD/
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
    #mkdir -p kt-AC
    mv dktree_serialize/ kt-AC/
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
    #mkdir -p kt-AL
    mv dktree_serialize/ kt-AL/
fi
