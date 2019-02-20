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

# Create 3-way striped raid4 (4 legs total)
lvcreate -y --ty raid4 --stripes 3 -L 9M -n $lv $vg
check lv_field $vg/$lv segtype "raid4"
check lv_field $vg/$lv data_stripes 3
check lv_field $vg/$lv stripes 4
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv"
fsck -fn "$DM_DEV_DIR/$vg/$lv"

# Step 1: grow before removing stripes
lvextend -y -L27M $vg/$lv
aux wait_for_sync $vg $lv

# Step 2: convert raid4 -> linear (reshape to remove stripes)
lvconvert -y -f --ty linear $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid4"
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripes 4
aux wait_for_sync $vg $lv 1

# Step 2: convert raid4 -> linear (remove freed stripes)
lvconvert -y --ty linear $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid4"
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripes 2

# Step 3: convert raid4 -> linear (convert to raid1)
lvconvert -y --ty linear $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "raid1"
check lv_field $vg/$lv data_stripes 2
check lv_field $vg/$lv stripes 2

# Step 4: convert raid4 -> linear (convert to linear)
lvconvert -y --ty linear $vg/$lv
detect_error_leak_
check lv_field $vg/$lv segtype "linear"
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripes 1

vgremove -ff $vg
