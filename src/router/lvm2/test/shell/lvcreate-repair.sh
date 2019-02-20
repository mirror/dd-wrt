#!/usr/bin/env bash

# Copyright (C) 2011-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 3

# fail multiple devices
for i in "$dev1" "$dev2" "$dev3" ; do
	for j in "$dev2" "$dev3" ; do
		if test "$i" = "$j" ; then continue ; fi

		vgremove -ff $vg
		vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3"
		# exit 1

		lvcreate -l1 -n $lv1 $vg "$dev1"

		aux disable_dev "$i" "$j"

		vgreduce --removemissing --force $vg

		# check if reduced device was removed
		test "$i" = "$dev1" && dm_table | not grep -E "$vg-$lv1: *[^ ]+"

		lvcreate -l1 -n $lv2 $vg

		test "$i" != "$dev1" && check lv_exists $vg $lv1
		check lv_exists $vg $lv2

		aux enable_dev "$i" "$j"
		vgscan

		test "$i" != "$dev1" && check lv_exists $vg $lv1
		check lv_exists $vg $lv2
	done
done

vgremove -ff $vg
vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3"

# use tricky 'dd'
for i in "$dev1" "$dev2" "$dev3" ; do
	for j in "$dev2" "$dev3" ; do

		if test "$i" = "$j" ; then continue ; fi

		dd if="$i" of=backup_i bs=256K count=1
		dd if="$j" of=backup_j bs=256K count=1

		lvcreate -l1 -n $lv1 $vg "$dev1"

		dd if=backup_j of="$j" bs=256K count=1
		dd if=backup_i of="$i" bs=256K count=1

		check lv_exists $vg $lv1
		# mda should be now consistent
		lvremove -f $vg/$lv1
	done
done


# confuse lvm with active LV left behind
dd if="$dev1" of=backup_i bs=256K count=1
dd if="$dev2" of=backup_j bs=256K count=1

lvcreate -l1 $vg "$dev1"

dd if=backup_j of="$dev2" bs=256K count=1
dd if=backup_i of="$dev1" bs=256K count=1

# CHECKME: following command writes here:
# vgreduce --removemissing --force $vg
#
# WARNING: Inconsistent metadata found for VG LVMTESTvg - updating to use version 2
# Volume group "LVMTESTvg" is already consistent

# dirty game
dd if=/dev/zero of="$dev3" bs=256K count=1

vgreduce --removemissing --force $vg

# FIXME: here is LV1 left active - but metadata does not know about it
# and lvcreate does not check whether such device exists in the table
# so it ends with: 
#
# device-mapper: create ioctl failed: Device or resource busy
# Failed to activate new LV.

should lvcreate -l1 $vg "$dev1"
should not dmsetup remove ${vg}-lvol0

vgremove -ff $vg
