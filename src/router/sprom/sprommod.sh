#!/bin/bash

if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
	echo "Usage: $0 DEVICE [OPTIONS]"
	echo ""
	echo "DEVICE is the wireless device (eth0, wlan0, wlan0.11, ...)"
	echo "OPTIONS are the options to bcm43xx-sprom"
	exit 1
fi

device="$1"
shift

args="$@"
if [ "$1" = "--all" ]; then
	shift
	args="`bcm43xx-sprom --help 2>&1| sed 's/  \(--[^ ]*\).*/\1 GET/;t;d' | tr '\n' ' '` $@"
fi

if [ -z "$device" ]; then
	echo "No DEVICE given"
	exit 1
fi

orig_data="$(iwpriv "$device" read_sprom)"
err=$?
if [ $err -ne 0 ]; then
	echo "Could not read SPROM ($err)"
	exit 1
fi
mod_data="$(echo "$orig_data" | bcm43xx-sprom $args)"
err=$?
if [ $err -ne 0 ]; then
	echo "Could not modify SPROM data ($err)"
	exit 1
fi
if [ -z "$mod_data" ]; then
	echo "No data. Not modified?"
	exit 1
fi
iwpriv "$device" write_sprom "$mod_data"
err=$?
if [ $err -ne 0 ]; then
	echo "Could not write SPROM data ($err)"
	exit 1
fi

exit 0
