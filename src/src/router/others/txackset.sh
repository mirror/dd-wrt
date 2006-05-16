#!/bin/sh
case "$1" in
	1) byte="\\x01";;
	0) byte="\\x00";;
	*)
		echo "Usage: $0 <0|1>"
		exit 1
	;;
esac

# Use the first symbol associated with the wl driver as base address
base="0x$(sort /proc/ksyms | grep \\\[wl\\\] | head -n1 | awk '{ print $1 }')"

# wlc_sendpkt should be near to the code that we want to patch. Use it for verification
ofs1="$((0x$(grep wlc_sendpkt /proc/ksyms | awk '{ print $1 }') - $base))"
case "$ofs1" in
	200252) offset="0x33c9c";;
	75388) offset="0x154dc";;
esac

[ -z "$offset" ] && {
	echo Offset for this driver not found.
	exit 1
}

# The opcode to patch here is 'ori s3,s3,1' in the function that prepares the headers
# for sending a packet to the driver.
# Target the first byte of this opcode, which is the lower part of the immediate value
# Setting it to 0 will disable the driver's request for Tx ACK in the hardware

echo -ne "$byte" | dd of=/dev/kmem seek="$(($base + $offset))" bs=1 count=1 2>&- >&-
