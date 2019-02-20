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

# Exercise snapshot merge also when stacked


export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which mkfs.ext3 || skip

aux target_at_least dm-snapshot-merge 1 0 0 || skip

aux prepare_vg 2 100

snap_and_merge() {
	lvcreate -s -n $lv2 -L20 $vg/$lv1 "$dev2"
	#dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=1M count=10 conv=fdatasync
	aux udev_wait
	mkfs.ext3 "$DM_DEV_DIR/$vg/$lv2"
	sync
	lvs -a $vg

	SLEEP_PID=$(aux hold_device_open $vg $lv1 20)

	# initiate background merge
	lvconvert -b --merge $vg/$lv2

	lvs -a -o+lv_merging,lv_merge_failed $vg
	get lv_field $vg/$lv1 lv_attr | grep "Owi-ao"
	get lv_field $vg/$lv2 lv_attr | grep "Swi-a-s---"
	kill $SLEEP_PID

	aux delay_dev "$dev1"  0 200 "$(get first_extent_sector "$dev1"):"
	lvchange --poll n --refresh $vg/$lv1
	dmsetup table
	lvs -av -o+lv_merging,lv_merge_failed $vg
	# Origin is closed and snapshot merge could run
	get lv_field $vg/$lv1 lv_attr | grep "Owi-a-"
	sleep 1
	check lv_attr_bit state $vg/$lv2 "a"
	aux error_dev "$dev2" "$(get first_extent_sector "$dev2"):"
	aux enable_dev "$dev1"
	# delay to let snapshot merge 'discover' failing COW device
	sleep 1
	sync
	dmsetup status
	lvs -a -o+lv_merging,lv_merge_failed $vg
	check lv_attr_bit state $vg/$lv1 "m"
	check lv_attr_bit state $vg/$lv2 "m"

	# device OK and running in full speed
	aux enable_dev "$dev2"

	# reactivate so merge can finish
	lvchange -an $vg
	lvchange -ay $vg
	sleep 1
	lvs -a -o+lv_merging,lv_merge_failed $vg
	check lv_not_exists $vg $lv2
	fsck -n "$DM_DEV_DIR/$vg/$lv1"

	lvremove -f $vg
}


# First check merge on plain linear LV
lvcreate -aey -L50 -n $lv1 $vg "$dev1"
snap_and_merge

# When available check merge of old snapshot with Thin LV being origin
if aux have_thin 1 0 0 ; then
	lvcreate -T -L10 -V50 -n $lv1 $vg/pool "$dev1"
	snap_and_merge
fi

# TODO snapshot merge with Mirror, Raid, Cache...

vgremove -f $vg
