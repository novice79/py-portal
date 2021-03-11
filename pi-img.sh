#!/bin/bash
cd pi-dist
docker buildx build -t novice/py-portal:latest \
--platform linux/arm/v7 --push .