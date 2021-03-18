#!/bin/bash

cd pi-dist
# dt=$(date +"%Y-%m-%d")
# sed -i -E "s/build_date=.+/build_date=\"$dt\"/" Dockerfile
# prerequisite:
# docker run --rm --privileged docker/binfmt:a7996909642ee92942dcd6cff44b9b95f08dad64
# To verify the qemu handlers are registered properly, run the following and make sure the first line of the output is “enabled”
# cat /proc/sys/fs/binfmt_misc/qemu-aarch64
# docker buildx ls
# docker buildx inspect --bootstrap

# https://www.docker.com/blog/getting-started-with-docker-for-arm-on-linux/

# raspberry pi install docker
# Steps
# 1. Install Docker
# curl -sSL https://get.docker.com | sh

# 2. Add permission to Pi User to run Docker Commands
# sudo usermod -aG docker pi

# Reboot here or run the next commands with a sudo

# 3. Test Docker installation
# docker run hello-world

# 4. IMPORTANT! Install proper dependencies
# sudo apt-get install -y libffi-dev libssl-dev
# sudo apt-get install -y python3 python3-pip
# sudo apt-get remove python-configparser

# 5. Install Docker Compose
# sudo pip3 -v install docker-compose
if [ -n $1 ]; then
    use_cache=" --no-cache"
fi
docker buildx build $use_cache \
--build-arg BUILD_DATE=$(date -u +'%Y-%m-%dT%H:%M:%SZ') \
-t novice/py-portal:latest \
--platform linux/arm/v7 \
--push .