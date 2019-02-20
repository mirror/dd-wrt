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
aux have_raid 1 13 2 || skip

aux prepare_vg 6

#
# Test multi step raid5 -> raid10 conversion
#

# Create raid5(_ls) LV
lvcreate -y --type raid5 -i 3 -L 16M -R 256K -n $lv1 $vg
check lv_field $vg/$lv1 segtype "raid5"
check lv_field $vg/$lv1 stripes 4
check lv_field $vg/$lv1 data_stripes 3
check lv_field $vg/$lv1 region_size "256.00k"
echo y|mkfs -t ext4 $DM_DEV_DIR/$vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
aux wait_for_sync $vg $lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid5 -> raid10 (first step raid5 -> raid5_n)
lvconvert -y --ty raid10 $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_field $vg/$lv1 segtype "raid5_n"
check lv_field $vg/$lv1 stripes 4
check lv_field $vg/$lv1 data_stripes 3
check lv_field $vg/$lv1 region_size "256.00k"
aux wait_for_sync $vg $lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid5 -> raid10 (second step raid5_n -> raid0_meta)
lvconvert -y --ty raid10 $vg/$lv1
check lv_field $vg/$lv1 segtype "raid0_meta"
check lv_field $vg/$lv1 stripes 3
check lv_field $vg/$lv1 data_stripes 3
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid5 -> raid10 (third + last step raid0_meta -> raid10)
lvconvert -y --ty raid10 -R 256K $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_field $vg/$lv1 segtype "raid10"
check lv_field $vg/$lv1 stripes 6
check lv_field $vg/$lv1 data_stripes 3
check lv_field $vg/$lv1 region_size "256.00k"

vgremove -ff $vg
