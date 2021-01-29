#!/bin/sh
set -ue

prog="${0##*/}"

show_help() {
	echo "<INFO> Usage: ${prog} <partition image>"
}

image_write() {
	local device=$1
	local path=$2
	local image=$3

	echo "<INFO> umount -f $device ($path)"
	umount -f $device
	echo "<INFO> Restore image..."
	gunzip -c $image | dd of=$device bs=64K conv=sync,noerror
	echo "<INFO> mount -t antfs $device $path -o uft8"
	mount -t antfs $device $path -o uft8
}

partition_renew() {
	local devices=$(/sbin/blkid -o device)
	local image=$1
	local path=
	local label=

	for dev in ${devices} ; do
		label=`/sbin/blkid $dev -do value -s LABEL`
		if [ "$label" = "NTFS" ] ; then
			path=`mount | grep $dev | cut -d' ' -f3`
			image_write $dev $path $image
		fi
	done
}

OPTIND=1         # Reset in case getopts has been used previously in the shell.
while getopts ":h?" opt; do
	case "$opt" in
		h|\?)
			show_help
			exit 0
			;;
	esac
done
shift $((OPTIND-1))

if [ -z $@ ]; then
	echo "<ERROR> File not found: $1" >&2
	show_help
	exit 1
fi

if [ -e "$1" ]; then
	partition_renew $1
	echo "<INFO> Finished successfully"
fi
