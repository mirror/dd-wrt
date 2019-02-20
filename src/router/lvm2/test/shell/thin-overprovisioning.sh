#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test warns when thin pool is overprovisiong


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 3 0 || skip

# 2PVs by 32M
aux prepare_vg 2 33

lvcreate -L32 -T $vg/pool
# check there is link node for UNUSED thin-pool
test -e "$DM_DEV_DIR/$vg/pool"

# leave 12M free space
lvcreate -an -n $lv1 -L16 $vg 2>&1 | tee out
vgs $vg

lvcreate -n thin1 -V30 $vg/pool 2>&1 | tee out
not grep "WARNING: Sum" out
# check again link node is now gone for a USED thin-pool
test ! -e "$DM_DEV_DIR/$vg/pool"

# Pool gets overprovisioned
lvcreate -an -n thin2 -V4 $vg/pool 2>&1 | tee out
grep "WARNING: Sum" out
grep "amount of free space in volume group (12.00 MiB)" out

# Eat all space in VG
lvcreate -an -n $lv2 -L12 $vg 2>&1 | tee out
grep "WARNING: Sum" out
grep "no free space in volume group" out

lvcreate -an -n thin3 -V1G $vg/pool 2>&1 | tee out
grep "WARNING: Sum" out
grep "the size of whole volume group" out

lvremove -ff $vg/thin2 $vg/thin3 $vg/$lv2

# Create 2nd thin pool in a VG

lvcreate -L4 -T $vg/pool2
lvcreate -V4 -n thin2 $vg/pool2 2>&1 | tee out
not grep "WARNING: Sum" out

lvcreate -an -V4 -n thin3 $vg/pool2 2>&1 | tee out
grep "WARNING: Sum of all thin volume sizes (38.00 MiB)" out
grep "free space in volume group (6.00 MiB)" out

lvcreate -an -L6 -n $lv3 $vg 2>&1 | tee out
grep "no free space in volume group" out

lvremove -ff $vg/thin2 $vg/thin3

lvcreate -an -V4 -n thin2 $vg/pool2 2>&1 | tee out
not grep "WARNING: Sum" out

# Check if resize notices problem
lvextend -L+8 $vg/thin2

vgs $vg

vgremove -ff $vg
