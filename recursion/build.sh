#!/usr/bin/env bash
mkdir bin > /dev/null 2>&1

cd bin

cmake -DCMAKE_BUILD_TYPE=Release ..
make 
echo ">"
echo Compilation has been finished. Check bin/ directory
echo Start tests...
./RecursiveFunction && echo Done || echo Tests failed
