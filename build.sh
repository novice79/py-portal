#!/bin/bash
# set -x

dir="_build_$1"
[[ $HOSTNAME == "y700" ]] && export VCPKG_ROOT=/data/vcpkg
cmake -H. -B$dir \
-DCMAKE_BUILD_TYPE=Release \
-G Ninja 

cmake --build $dir
mkdir -p dist
[[ -f "$dir/py" ]] && mv $dir/py dist/
# [ ! -d dist/prefab ] && cp -r prefab dist/