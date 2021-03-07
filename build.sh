#!/bin/bash
BT=Release
if [[ $# > 0 ]]; then BT=Debug; fi
# echo $BT
dir="_build_$1"
[[ $1 == "pi" ]] || export VCPKG_ROOT=/data/vcpkg
cmake -H. -B$dir \
-DCMAKE_BUILD_TYPE=$BT \
-G Ninja 

cmake --build $dir
mkdir -p dist
mv $dir/py dist/
[ ! -d dist/prefab ] && cp -r prefab dist/