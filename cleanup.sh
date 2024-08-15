#!/bin/bash +x

# remove docker container
tag="aislab-platform"
if [[ "$(docker ps -a -q  --filter ancestor=$tag)" != "" ]]; then
    docker rm $(docker ps -a -q  --filter ancestor=$tag) -f 
fi

# remove docker image
tag="aislab-platform"
if [[ "$(docker images -q $tag)" != "" ]]; then
    docker rmi $tag
fi
