#!/bin/sh
set -e -x
IMAGE=radvd-autogen:latest
IMAGE_PRESENT=0
if docker images -q "${IMAGE}" |grep -sq . ; then
	IMAGE_PRESENT=1
fi
if [ "$IMAGE_PRESENT" -eq 0 ]; then
	docker build -f Docker.autogen -t "${IMAGE}" .
fi
CONTAINER_USER="$(id -u):$(id -g)"
docker run \
	--network=none \
	--user="${CONTAINER_USER}" \
	--mount=type=bind,src="${PWD}",dst=/workdir,ro=false \
	--rm=true \
	--privileged=false \
	--security-opt=no-new-privileges \
	--tty=false \
	--interactive=false \
	"${IMAGE}"
