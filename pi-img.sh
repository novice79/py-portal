#!/bin/bash
cd pi-dist
# prerequisite:
# docker run --rm --privileged docker/binfmt:a7996909642ee92942dcd6cff44b9b95f08dad64
# docker buildx ls
# docker buildx inspect --bootstrap

# https://www.docker.com/blog/getting-started-with-docker-for-arm-on-linux/

# docker container ls
# CONTAINER ID   IMAGE                           COMMAND       CREATED       STATUS       PORTS     NAMES
# 09e15ff4f0a2   moby/buildkit:buildx-stable-1   "buildkitd"   10 days ago   Up 6 hours             buildx_buildkit_mb0

docker buildx build -t novice/py-portal:latest \
--platform linux/arm/v7 --push .