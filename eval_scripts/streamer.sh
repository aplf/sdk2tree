#!/usr/bin/env bash


run_algo="false"

algo="none"

ctr=0
for algo in "$@"; do 
    let "ctr+=1"
done

case $algo in
   PR|BFS|DFS|CC|CT_dummy|CT_dummy_hash)
     run_algo="true" ;;
   *)
     true ;;
esac

#echo "$ctr"

#exit 0

for var in "$@"; do
    case $var in
        PR|BFS|DFS|CC|CT_dummy|CT_dummy_hash)
            echo "e $var" ;;
        *)
            cat "$var" ;;
    esac
    #if [[ "$var" == "$algo" ]]; then
    #    echo "e $algo"
    #else
    #    cat "$var"
    #fi
    echo "s"
    echo "saving_file_$ctr"
    ctr=$((ctr+1))
done

echo "x"

