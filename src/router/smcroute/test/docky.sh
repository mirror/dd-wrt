#!/bin/sh

running_in_docker() {
  awk -F/ '$2 == "docker"' /proc/self/cgroup | grep -q "docker"
}

export srcdir=$(realpath $1)
shift

if running_in_docker; then
    "$*"
else
    dir=$(pwd)
    docker pull ghcr.io/troglobit/misc:latest
    docker run --cap-add=NET_ADMIN -esrcdir -v"$srcdir":"$srcdir" -w "$dir" -it ghcr.io/troglobit/misc:latest "$*"
fi
