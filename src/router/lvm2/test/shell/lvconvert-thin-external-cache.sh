#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test conversion cached LV to thin with cached external origin

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which mkfs.ext2 || skip
which fsck || skip

#
# Main
#
aux have_thin 1 5 0 || skip
aux have_cache 1 7 0 || skip

aux prepare_vg 2 64

# Test will use thin-pool
lvcreate -L10 -T $vg/tpool

lvcreate -aey -L20 -n $lv1 $vg


mkfs.ext2 "$DM_DEV_DIR/$vg/$lv1"
mkdir mnt
mount "$DM_DEV_DIR/$vg/$lv1" mnt
touch mnt/test

# Prepared cached LV - first in 'writeback' mode
lvcreate -H --cachemode writeback -L10 -n cpool $vg/$lv1

# Can't convert  'writeback' cache
not lvconvert --thin --thinpool $vg/tpool $vg/$lv1

# Switch to 'writethrough' - this should be supported
lvchange --cachemode writethrough $vg/$lv1

# Check $lv1 remains mounted (so it's not been unmounted by systemd)
not mount "$DM_DEV_DIR/$vg/$lv1" mnt

lvconvert --thin $vg/$lv1 --originname extorg --thinpool $vg/tpool

# check cache exist as extorg-real
check grep_dmsetup table  ${vg}-extorg-real "cache"


# Split cache from external origin (while in-use)
lvconvert --splitcache $vg/extorg

# check linear exist as extorg-real
check grep_dmsetup table  ${vg}-extorg-real "linear"
check lv_field $vg/extorg segtype linear

# Cache external origin in-use again
lvconvert -y -H $vg/extorg --cachepool $vg/cpool

get lv_field $vg/extorg attr | grep "^ori"

umount mnt

# Is filesystem still ok ?
fsck -n "$DM_DEV_DIR/$vg/$lv1"

lvchange -an $vg
lvchange -ay $vg

# Remove thin,   external origin remains
lvremove -f $vg/$lv1

#lvchange -prw  $vg/extorg
lvconvert --uncache $vg/extorg

lvremove -f $vg

#
# Check some more API variants
#

lvcreate -L10 -n pool $vg

lvcreate -aey -L2 -n $lv1 $vg
lvcreate -H -L2 $vg/$lv1

# Converts $vg/pool to thin-pool  AND  $vg/$lv1 to thin
lvconvert -y --type thin $vg/$lv1 --originname extorg --thinpool $vg/pool

check lv_field $vg/$lv1 segtype thin
check lv_field $vg/pool segtype thin-pool
check lv_field $vg/extorg segtype cache

lvconvert --uncache $vg/extorg

check lv_field $vg/extorg segtype linear

vgremove -ff $vg
