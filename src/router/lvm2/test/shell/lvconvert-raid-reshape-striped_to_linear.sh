#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
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

aux lvmconf 'activation/raid_region_size = 512'

which mkfs.ext4 || skip
aux have_raid 1 14 0 || skip

aux prepare_vg 5 20

#
# Test single step linear -> striped conversion
#

# Create 4-way striped  LV
lvcreate -aey -i 4 -I 32k -L 16M -n $lv1 $vg
check lv_field $vg/$lv1 segtype "striped"
check lv_field $vg/$lv1 data_stripes 4
check lv_field $vg/$lv1 stripes 4
check lv_field $vg/$lv1 stripesize "32.00k"
check lv_field $vg/$lv1 reshape_len_le ""
echo y|mkfs -t ext4 $DM_DEV_DIR/$vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert striped -> raid5(_n)
lvconvert -y --ty raid5 -R 128k $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_field $vg/$lv1 segtype "raid5_n"
check lv_field $vg/$lv1 data_stripes 4
check lv_field $vg/$lv1 stripes 5
check lv_field $vg/$lv1 stripesize "32.00k"
check lv_field $vg/$lv1 regionsize "128.00k"
check lv_field $vg/$lv1 reshape_len_le 0
aux wait_for_sync $vg $lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Extend raid5_n LV by factor 4 to keep size once linear
lvresize -y -L 64M $vg/$lv1
aux wait_for_sync $vg $lv1

check lv_field $vg/$lv1 segtype "raid5_n"
check lv_field $vg/$lv1 data_stripes 4
check lv_field $vg/$lv1 stripes 5
check lv_field $vg/$lv1 stripesize "32.00k"
check lv_field $vg/$lv1 regionsize "128.00k"
check lv_field $vg/$lv1 reshape_len_le "0"
aux wait_for_sync $vg $lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid5_n LV to 1 stripe (2 legs total),
# 64k stripesize and 1024k regionsize
# FIXME: "--type" superfluous (cli fix needed)
lvconvert -y -f --ty raid5_n --stripes 1 -I 64k -R 1024k $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_first_seg_field $vg/$lv1 segtype "raid5_n"
check lv_first_seg_field $vg/$lv1 data_stripes 1
check lv_first_seg_field $vg/$lv1 stripes 5
check lv_first_seg_field $vg/$lv1 stripesize "32.00k"
check lv_first_seg_field $vg/$lv1 regionsize "1.00m"
check lv_first_seg_field $vg/$lv1 reshape_len_le 10
# for slv in {0..4}
# do
#	check lv_first_seg_field $vg/${lv1}_rimage_${slv} reshape_len_le 2
# done
aux wait_for_sync $vg $lv1 1
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Remove the now freed legs
lvconvert -y --stripes 1 $vg/$lv1
check lv_first_seg_field $vg/$lv1 segtype "raid5_n"
check lv_first_seg_field $vg/$lv1 data_stripes 1
check lv_first_seg_field $vg/$lv1 stripes 2
check lv_first_seg_field $vg/$lv1 stripesize "32.00k"
check lv_first_seg_field $vg/$lv1 regionsize "1.00m"
check lv_first_seg_field $vg/$lv1 reshape_len_le 4
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid5_n to raid1
lvconvert -y --type raid1 $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_first_seg_field $vg/$lv1 segtype "raid1"
check lv_first_seg_field $vg/$lv1 data_stripes 2
check lv_first_seg_field $vg/$lv1 stripes 2
check lv_first_seg_field $vg/$lv1 stripesize "0"
check lv_first_seg_field $vg/$lv1 regionsize "1.00m"
check lv_first_seg_field $vg/$lv1 reshape_len_le ""
fsck -fn $DM_DEV_DIR/$vg/$lv1

# Convert raid1 -> linear
lvconvert -y --type linear $vg/$lv1
fsck -fn $DM_DEV_DIR/$vg/$lv1
check lv_first_seg_field $vg/$lv1 segtype "linear"
check lv_first_seg_field $vg/$lv1 data_stripes 1
check lv_first_seg_field $vg/$lv1 stripes 1
check lv_first_seg_field $vg/$lv1 stripesize "0"
check lv_first_seg_field $vg/$lv1 regionsize "0"
check lv_first_seg_field $vg/$lv1 reshape_len_le ""
fsck -fn $DM_DEV_DIR/$vg/$lv1

vgremove -ff $vg
