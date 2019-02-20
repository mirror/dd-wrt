#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise caching thin-pool's data LV


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 0 0 || skip
aux have_cache 1 3 0 || skip

which mkfs.ext4 || skip

aux prepare_pvs 2 64
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

lvcreate -L10M -V10M -T $vg/pool --name $lv1

lvcreate -H -L10 $vg/pool

mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"

lvconvert --uncache $vg/pool
fsck -n "$DM_DEV_DIR/$vg/$lv1"

lvcreate -H -L10 $vg/pool_tdata
fsck -n "$DM_DEV_DIR/$vg/$lv1"
lvconvert --uncache $vg/pool_tdata

vgremove -ff $vg
