#!/bin/bash
cd $(dirname "$0")


# build docker image
tag="aislab-platform"
echo docker images -q $tag > /dev/null 2>&1
if [[ "$(docker images -q $tag)" == "" ]]; then
    docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) --build-arg NAME="$(id -un)" --build-arg ARCH="$(uname -m)" -t $tag .
fi

# $OSTYPE
#   windows => msys
#   mac => darwin

# run a docker container

if [[ $OSTYPE == "msys" ]]; then
    winpty docker run --privileged -v "/$PWD":/workspace --name aislab-platform -it --rm $tag bash
else
    docker run --privileged -v "$PWD":/workspace --name aislab-platform -it --rm $tag bash
fi
