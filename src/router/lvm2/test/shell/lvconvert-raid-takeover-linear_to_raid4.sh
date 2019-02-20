#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA2110-1301 USA


SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext4 || skip
aux have_raid 1 14 0 || skip

aux prepare_vg 4 32

# FIXME: lvconvert leaks  'error' devices
detect_error_leak_()
{
	dmsetup table -S "name=~^$vg-" | not grep "error" || \
		die "Device(s) with error target should not be here."
}

# Create linear LV
lvcreate -y -L 9M -n $lv $vg
check lv_field $vg/$lv segtype "linear"
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripes 1
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv"
fsck -fn "$DM_DEV_DIR/$vg/$lv"

# Step 1: convert linear -> raid4 (convert to 2-legged raid1)
lvconvert -y --stripes 3 --ty raid4 $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid1"
check lv_field $vg/$lv data_stripes 2
check lv_field $vg/$lv stripes 2
aux wait_for_sync $vg $lv

# Step 2: convert linear ->raid4 (convert to raid4)
lvconvert -y --stripes 3 --ty raid4 $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid4"
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripes 2

# Step 3: convert linear ->raid4 (reshape to add stripes)
lvconvert -y --stripes 3 --ty raid4 $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid4"
check lv_field $vg/$lv data_stripes 3
check lv_field $vg/$lv stripes 4

vgremove -ff $vg
