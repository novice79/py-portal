#!/bin/bash

docker run -it --rm --name be \
-h ub-build \
-u novice \
-v $PWD:/data/workspace \
-w /data/workspace \
novice/build