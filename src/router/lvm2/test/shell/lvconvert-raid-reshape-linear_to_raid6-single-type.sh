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

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

# Ensure expected default region size
aux lvmconf 'activation/raid_region_size = 512'

which mkfs.ext4 || skip
aux have_raid 1 14 0 || skip

aux prepare_vg 5

#
# Test multi step linear -> striped conversion
#

# Create linear LV
lvcreate -aey -L 16M -n $lv $vg
check lv_field $vg/$lv segtype "linear"
check lv_field $vg/$lv stripes 1
check lv_field $vg/$lv data_stripes 1
echo y|mkfs -t ext4 $DM_DEV_DIR/$vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv

# Convert linear -> raid1 (takeover)
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_field $vg/$lv segtype "raid1"
check lv_field $vg/$lv stripes 2
check lv_field $vg/$lv data_stripes 2
check lv_field $vg/$lv regionsize "128.00k"
aux wait_for_sync $vg $lv
fsck -fn $DM_DEV_DIR/$vg/$lv

# Convert raid1 -> raid5_ls (takeover)
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_field $vg/$lv segtype "raid5_ls"
check lv_field $vg/$lv stripes 2
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripesize "64.00k"
check lv_field $vg/$lv regionsize "128.00k"

# Convert raid5_ls adding stripes (reshape)
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_first_seg_field $vg/$lv segtype "raid5_ls"
check lv_first_seg_field $vg/$lv stripes 4
check lv_first_seg_field $vg/$lv data_stripes 3
check lv_first_seg_field $vg/$lv stripesize "64.00k"
check lv_first_seg_field $vg/$lv regionsize "128.00k"
check lv_first_seg_field $vg/$lv reshape_len_le 8
aux wait_for_sync $vg $lv
fsck -fn $DM_DEV_DIR/$vg/$lv

# Convert raid5_ls -> raid6_ls_6 (takeover)
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_first_seg_field $vg/$lv segtype "raid6_ls_6"
check lv_first_seg_field $vg/$lv stripes 5
check lv_first_seg_field $vg/$lv data_stripes 3
check lv_first_seg_field $vg/$lv stripesize "64.00k"
check lv_first_seg_field $vg/$lv regionsize "128.00k"
check lv_first_seg_field $vg/$lv reshape_len_le 0
aux wait_for_sync $vg $lv

# Convert raid6_ls_6 -> raid6(_zr) (reshape)
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_first_seg_field $vg/$lv segtype "raid6"
check lv_first_seg_field $vg/$lv stripes 5
check lv_first_seg_field $vg/$lv data_stripes 3
check lv_first_seg_field $vg/$lv stripesize "64.00k"
check lv_first_seg_field $vg/$lv regionsize "128.00k"
check lv_first_seg_field $vg/$lv reshape_len_le 10
aux wait_for_sync $vg $lv

# Remove reshape space
lvconvert -y --type raid6 --stripes 3 --stripesize 64K --regionsize 128K $vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
check lv_first_seg_field $vg/$lv segtype "raid6"
check lv_first_seg_field $vg/$lv stripes 5
check lv_first_seg_field $vg/$lv data_stripes 3
check lv_first_seg_field $vg/$lv stripesize "64.00k"
check lv_first_seg_field $vg/$lv regionsize "128.00k"
check lv_first_seg_field $vg/$lv reshape_len_le 0

vgremove -ff $vg
