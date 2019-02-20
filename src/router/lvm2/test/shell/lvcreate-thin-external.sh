#!/usr/bin/env bash

# Copyright (C) 2013-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test creation of thin snapshots using external origin


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which mkfs.ext2 || skip
which fsck || skip

#
# Main
#
aux have_thin 1 3 0 || skip

aux prepare_pvs 2 64
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

# Newer thin-pool target (>= 1.13) supports unaligned external origin
# But this test is written to test and expect older behavior
aux lvmconf 'global/thin_disabled_features = [ "external_origin_extend" ]'

# Test validation for external origin being multiple of thin pool chunk size
lvcreate -L10M -T $vg/pool192 -c 192k
lvcreate -an -pr -Zn -l1 -n $lv1 $vg
not lvcreate -s $vg/$lv1 --thinpool $vg/pool192

lvcreate -an -pr -Zn -l5 -n $lv2 $vg
not lvcreate -s $vg/$lv2 --thinpool $vg/pool192
lvremove -f $vg

# Prepare pool and external origin with filesystem
lvcreate -L10M -V10M -T $vg/pool --name $lv1
mkfs.ext2 "$DM_DEV_DIR/$vg/$lv1"

lvcreate -L4M -n $lv2 $vg
mkfs.ext2 "$DM_DEV_DIR/$vg/$lv2"

# Fail to create external origin snapshot of rw LV
not lvcreate -s $vg/$lv2 --thinpool $vg/pool

lvchange -p r $vg/$lv2

# Fail to create snapshot of active r LV
# FIXME: kernel update needed
not lvcreate -s $vg/$lv2 --thinpool $vg/pool

# Deactivate LV we want to use as external origin
# once kernel will ensure read-only this condition may go away
lvchange -an $vg/$lv2

lvcreate -s $vg/$lv2 --thinpool $vg/pool

# Fail with --thin and --snapshot
not lvcreate -s $vg/$lv5 --name $vg/$lv7 -T $vg/newpool

# Cannot specify size and thin pool.
# TODO: maybe with --poolsize
invalid lvcreate -s $vg/$lv2 -L10 --thinpool $vg/pool
invalid lvcreate -s -K $vg/$lv2 --name $vg/$lv3 -L20 --chunksize 128 --thinpool $vg/newpool

not lvcreate -s $vg/$lv2 --chunksize 64 --thinpool $vg/pool
not lvcreate -s $vg/$lv2 --zero y --thinpool $vg/pool
not lvcreate -s $vg/$lv2 --poolmetadata $vg/$lv1 --thinpool $vg/pool

# Fail with nonexistent pool
not lvcreate -s $vg/$lv2 --thinpool $vg/newpool

# Create pool and snap
lvcreate -T --name $vg/$lv3 -V10 -L20 --chunksize 128 --thinpool $vg/newpool
lvcreate -s -K $vg/$lv3 --name $vg/$lv4
lvcreate -s -K $vg/$lv2 --name $vg/$lv5 --thinpool $vg/newpool
# Make normal thin snapshot
lvcreate -s -K $vg/$lv5 --name $vg/$lv6
# We do not need to specify thinpool when doing thin snap, but it should work
lvcreate -s -K $vg/$lv5 --name $vg/$lv7 --thinpool $vg/newpool

check inactive $vg $lv2
lvchange -ay $vg/$lv2
lvcreate -s -K $vg/$lv2 --name $vg/$lv8 --thinpool $vg/newpool

lvs -o+chunksize $vg

check active $vg $lv3
check active $vg $lv4
check active $vg $lv5
check active $vg $lv6
check active $vg $lv7

fsck -n "$DM_DEV_DIR/$vg/$lv1"
fsck -n "$DM_DEV_DIR/$vg/$lv7"

vgremove -ff $vg
