#!/bin/bash
#
# Copyright (C) 2023 Corellium LLC
#
# Author: Ernesto A. Fernández <ernesto@corellium.com>
#
# Call as ./test.sh to test mkfs on various container sizes. Some will be very
# big so this should always be done inside a filesystem that supports sparse
# files.

success=0

set -e
cleanup() {
	rm -f /tmp/sizetest.img
	[ $success -eq 1 ] && echo "TEST PASSED" || echo "TEST FAILED"
}
trap cleanup exit

test_size() {
	truncate -s $1 /tmp/sizetest.img
	./mkapfs /tmp/sizetest.img
	../apfsck/apfsck -cuw /tmp/sizetest.img
}

touch /tmp/sizetest.img

# Single block ip bitmap, single block spaceman, no CABs
test_size 512K # Minimum size
test_size 15G
test_size 1454383300608	# Maximum size

# Multiblock ip bitmap, single block spaceman, no CABs
test_size 1454383304704	# Minimum size
test_size 3T
test_size 7390296539136	# Maximum size

# Multiblock ip bitmap, multiblock spaceman, no CABs
test_size 7390296543232	# Minimum size
test_size 7T
test_size 8574096900096	# Maximum size

# Multiblock ip bitmap, single block spaceman, has CABs
test_size 8574096904192	# Minimum size
test_size 15T

# Filesystems > ~113 TiB not yet supported

# Regression tests for sizes that caused problems in the past
test_size 3G

success=1
