#!/usr/bin/env bash
# Copyright (C) 2006 OpenWrt.org

[ $# == 6 ] || {
	echo "SYNTAX: $0 <file> <kernel size> <kernel directory> <rootfs size> <rootfs image>"
	exit 1
}

file="$1"
part1s="$2"
part1d="$3"
part2s="$4"
part2f="$5"
part3s="$6"
part4s=1
head=16
sect=63
cyl=$(( ($part1s + $part2s + $part3s + $part4s) * 1024 * 1024 / ($head * $sect * 512)))

#dd if=/dev/zero of="$file" bs=1M count=$(($part1s + $part2s - 1))  2>/dev/null || exit
# create partition table
rm -f $file
which ptgen
set `ptgen -o "$file" -h $head -s $sect -p ${part1s}m -p ${part2s}m -p ${part3s}m -p ${part4s}m`

KERNELOFFSET="$(($1 / 512))"
KERNELSIZE="$(($2 / 512))"
ROOTFSOFFSET="$(($3 / 512))"
ROOTFSSIZE="$(($4 / 512))"
BLOCKS="$((($KERNELSIZE / 2) - 1))"

[ $# == 8 ] || {
	echo "gen_image.sh: wrong/old ptgen version ABORT"
	exit 1
}

genext2fs -d "$part1d" -b "$BLOCKS" "$file.kernel"
dd if="$file.kernel" of="$file" bs=512 seek="${KERNELOFFSET}" conv=notrunc
rm -f "$file.kernel"

dd if="$part2f" of="$file" bs=512 seek="${ROOTFSOFFSET}" conv=notrunc

which chpax >/dev/null && chpax -zp $(which grub)
grub --device-map=/dev/null <<EOF
device (hd0) $file
geometry (hd0) $cyl $head $sect
root (hd0,0)
setup (hd0)
EOF

