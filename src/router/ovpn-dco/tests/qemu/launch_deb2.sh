#!/bin/bash

DISK=emulation/debian2.img
BZIMAGE=linux-kernel/arch/x86/boot/bzImage
SHARE=/home/user/share

sudo qemu-system-x86_64 \
    -device virtio-balloon \
    -enable-kvm \
    -accel kvm \
    -kernel ${BZIMAGE} \
    -append 'console=ttyS0 root=/dev/vda1 pci=noacpi' \
    -drive file=${DISK},if=virtio,cache=writeback \
    -virtfs local,path=${SHARE},security_model=passthrough,mount_tag=k1 \
    -nic tap,ifname=tap0,script=no,downscript=no,model=virtio-net-pci \
    -display none \
    -nographic \
    -serial mon:stdio \
    -m 2048 -boot c \
    -snapshot -s
