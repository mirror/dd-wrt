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

aux lvmconf 'activation/raid_region_size = 512'

which mkfs.ext4 || skip
aux have_raid 1 14 0 || skip

aux prepare_vg 5

#
# Test multi step striped -> linear conversion
#

# Create 4-way striped LV
lvcreate -aey --type striped -L 16M --stripes 4 --stripesize 64K -n $lv $vg
check lv_first_seg_field $vg/$lv segtype "striped"
check lv_first_seg_field $vg/$lv stripes 4
check lv_first_seg_field $vg/$lv data_stripes 4
check lv_first_seg_field $vg/$lv stripesize "64.00k"
echo y|mkfs -t ext4 $DM_DEV_DIR/$vg/$lv
fsck -fn $DM_DEV_DIR/$vg/$lv
lvextend -y -L64M $DM_DEV_DIR/$vg/$lv

# Convert striped -> raid5_n
lvconvert -y --type linear $vg/$lv
check lv_field $vg/$lv segtype "raid5_n"
check lv_field $vg/$lv data_stripes 4
check lv_field $vg/$lv stripes 5
check lv_field $vg/$lv data_stripes 4
check lv_field $vg/$lv stripesize "64.00k"
check lv_field $vg/$lv regionsize "512.00k"
check lv_field $vg/$lv reshape_len_le 0
aux wait_for_sync $vg $lv
fsck -fn $DM_DEV_DIR/$vg/$lv

# Restripe raid5_n LV to single data stripe
#
# Need --force in order to remove stripes thus shrinking LV size!
lvconvert -y --force --type linear $vg/$lv
aux wait_for_sync $vg $lv 1
fsck -fn $DM_DEV_DIR/$vg/$lv
# Remove the now freed stripes
lvconvert -y --type linear $vg/$lv
check lv_field $vg/$lv segtype "raid5_n"
check lv_field $vg/$lv stripes 2
check lv_field $vg/$lv data_stripes 1
check lv_field $vg/$lv stripesize "64.00k"
check lv_field $vg/$lv regionsize "512.00k"
check lv_field $vg/$lv reshape_len_le 4

# Convert raid5_n -> raid1
lvconvert -y --type linear $vg/$lv
check lv_field $vg/$lv segtype "raid1"
check lv_field $vg/$lv stripes 2
check lv_field $vg/$lv data_stripes 2
check lv_field $vg/$lv stripesize 0
check lv_field $vg/$lv regionsize "512.00k"
check lv_field $vg/$lv reshape_len_le ""
fsck -fn $DM_DEV_DIR/$vg/$lv

# Convert raid1 -> linear
lvconvert -y --type linear $vg/$lv
check lv_first_seg_field $vg/$lv segtype "linear"
check lv_first_seg_field $vg/$lv stripes 1
check lv_first_seg_field $vg/$lv data_stripes 1
check lv_first_seg_field $vg/$lv stripesize 0
check lv_first_seg_field $vg/$lv regionsize 0
fsck -fn $DM_DEV_DIR/$vg/$lv

vgremove -ff $vg
