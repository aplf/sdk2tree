#!/usr/bin/env bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

# https://stackoverflow.com/questions/3601515/how-to-check-if-a-variable-is-set-in-bash
if [ "$machine" == "Cygwin" ]; then
    export CLASSPATH="$(cygpath -w "${PROJ_ROOT}")"";$(cygpath -w "${PROJ_ROOT}")data_scripts\lib\*"
    #TODO: neet to test if this compiles in Cygwin (probably not)
else
    if [ -z ${PROJ_ROOT+x} ]; then 
        CLASSPATH_PREFIX="lib/*"
    else
        CLASSPATH_PREFIX="$PROJ_ROOT:$PROJ_ROOT/data_scripts/*:$PROJ_ROOT/data_scripts/lib/*"
    fi
    export CLASSPATH="$CLASSPATH:$CLASSPATH_PREFIX"
    #export CLASSPATH="$PROJ_ROOT:$PROJ_ROOT/data_scripts/lib/*"
fi

echo "$CLASSPATH"