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

# Exercise creation of snapshot of cached LV

SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext2 || skip
which fsck || skip

aux have_cache 1 5 0 || skip

aux prepare_vg 2

lvcreate --type cache-pool -L1 $vg/cpool
lvcreate -H -L4 -n $lv1 --cachepool $vg/cpool $vg

lvcreate -s -L2 -n $lv2 $vg/$lv1
check lv_field $vg/$lv1 segtype cache


# Make some 'fs' data in snapshot
mkfs.ext2 "$DM_DEV_DIR/$vg/$lv2"
mkdir mnt
mount "$DM_DEV_DIR/$vg/$lv2" mnt
touch mnt/test
umount mnt

sync
aux udev_wait

# Merge snap to origin
lvconvert --merge $vg/$lv2

# Check cached origin has no valid fs.
fsck -n "$DM_DEV_DIR/$vg/$lv1"

# Check deactivation
lvchange -an $vg

# Check activation
lvchange -ay $vg


lvconvert --uncache $vg/$lv1
check lv_field $vg/$lv1 segtype linear

# Uncached origin is fine as well
fsck -n "$DM_DEV_DIR/$vg/$lv1"


vgremove -ff $vg
