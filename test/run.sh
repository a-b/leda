#!/bin/bash

shopt -s nullglob
for f in test/*.lua
do
    
    export LEDA_PATH=`pwd`/lib
    
    if [ "$f" != "test/luaunit.lua"  ]
    then
        echo "running $f"
        ./leda "$f"
    fi
    
    if [ $? -ne 0 ]; then
        exit -1
    fi



done
 