#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Set up things to run tests with sanlock'

. lib/inittest

[ -z "$LVM_TEST_LOCK_TYPE_SANLOCK" ] && skip;

# Create a device and a VG that are both outside the scope of
# the standard lvm test suite so that they will not be removed
# and will remain in place while all the tests are run.
#
# Use this VG to hold the sanlock global lock which will be used
# by lvmlockd during other tests.
#
# This script will be run before any standard tests are run.
# After all the tests are run, another script will be run
# to remove this VG and device.

GL_DEV="/dev/mapper/GL_DEV"
GL_FILE="$PWD/gl_file.img"
dmsetup remove GL_DEV || true
rm -f "$GL_FILE"
dd if=/dev/zero of="$GL_FILE" bs=$((1024*1024)) count=1024 2> /dev/null
GL_LOOP=$(losetup -f "$GL_FILE" --show)
echo "0 $(blockdev --getsize $GL_LOOP linear $GL_LOOP 0)" | dmsetup create GL_DEV

aux prepare_sanlock
aux prepare_lvmlockd

vgcreate --config 'devices { global_filter=["a|GL_DEV|", "r|.*|"] filter=["a|GL_DEV|", "r|.*|"]}' --lock-type sanlock glvg $GL_DEV

vgs --config 'devices { global_filter=["a|GL_DEV|", "r|.*|"] filter=["a|GL_DEV|", "r|.*|"]}' -o+locktype,lockargs glvg

