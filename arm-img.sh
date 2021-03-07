#!/bin/bash
# docker run --rm --privileged docker/binfmt:a7996909642ee92942dcd6cff44b9b95f08dad64
# docker buildx inspect --bootstrap
docker run -h arm-ub20 -v $PWD:/workplace \
--name aub \
-u="$(id -u):$(id -g)" \
-w /workplace --rm -it arm32v7-ub20 bash

# docker run -h arm-ub20 --rm -it arm32v7-ub20 bash
# docker exec -u 0 -it aub bash