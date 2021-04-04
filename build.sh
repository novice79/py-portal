#!/bin/bash
# set -x

dir="_build_$1"
[[ $HOSTNAME == "y700" ]] && export VCPKG_ROOT=/data/vcpkg
cmake -H. -B$dir \
-DCMAKE_BUILD_TYPE=Release \
-G Ninja 

cmake --build $dir
mkdir -p dist
[[ -f "$dir/py_striped" ]] && mv $dir/py_striped dist/py
# [ ! -d dist/prefab ] && cp -r prefab dist/