#!/usr/bin/env bash

# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

check_lv_field_modules_()
{
	mod=$1
	shift

	for d in "$@"; do
		check lv_field "$vg/$d" modules "$mod"
	done
}


#
# Main
#
aux have_thin 1 0 0 || skip
which mkfs.ext4 || skip

aux prepare_pvs 2 64
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

lvcreate -L10M -V10M -T $vg/pool --name $lv1
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
# create read-only thin snapshot of thin LV
lvcreate -K -s $vg/$lv1 -pr --name snap
# check snapshot filesystem was properly frozen before snapping
fsck -n "$DM_DEV_DIR/$vg/snap"
lvcreate -K -s $vg/$lv1 --name $lv2
lvcreate -K -s $vg/$lv1 --name $vg/$lv3
# old-snapshot without known size is invalid
invalid lvcreate --type snapshot $vg/$lv1
invalid lvcreate --type snapshot $vg/$lv1 --name $lv4
invalid lvcreate --type snapshot $vg/$lv1 --name $vg/$lv5
# some other ways how to take a thin snapshot
lvcreate -T $vg/$lv1
lvcreate --thin $vg/$lv1 --name $lv4
lvcreate --type thin $vg/$lv1 --name $vg/$lv5
# virtual size needs thin pool
fail lvcreate --type thin $vg/$lv1 -V20

# create old-style snapshot
lvcreate -s -L10M --name oldsnap1 $vg/$lv2
lvcreate -s -L10M --name oldsnap2 $vg/$lv2

# thin snap of snap of snap...
lvcreate -K -s --name sn1 $vg/$lv2
lvcreate -K -s --name sn2 $vg/sn1
lvcreate -K -s --name sn3 $vg/sn2
lvcreate -K -s --name sn4 $vg/sn3

lvremove -ff $vg

lvcreate -L10M --zero n -T $vg/pool -V10M --name $lv1
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
lvcreate -K -s $vg/$lv1 --name snap
fsck -n "$DM_DEV_DIR/$vg/snap"

vgremove -ff $vg
