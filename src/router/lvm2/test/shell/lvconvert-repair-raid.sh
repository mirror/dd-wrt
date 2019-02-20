#!/usr/bin/env bash

# Copyright (C) 2013-2017 Red Hat, Inc. All rights reserved.
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

aux have_raid 1 3 0 || skip
aux raid456_replace_works || skip

aux lvmconf 'allocation/maximise_cling = 0' \
	    'allocation/mirror_logs_require_separate_pvs = 1' \
	    'activation/raid_fault_policy = "allocate"'

aux prepare_vg 8 80
get_devs

offset=$(get first_extent_sector $dev1)

function delay
{
	for d in "${DEVICES[@]}"; do
		aux delay_dev "$d" 0 $1 "$offset"
	done
}

# It's possible small raid arrays do have problems with reporting in-sync.
# So try bigger size
RAID_SIZE=32

# Fast sync and repair afterwards
delay 0

# RAID1 transient failure check
lvcreate --type raid1 -m 1 -L $RAID_SIZE -n $lv1 $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv1
# enforce replacing live rimage leg with error target
dmsetup remove -f $vg-${lv1}_rimage_1 || true
# let it notice there is problem
echo a > "$DM_DEV_DIR/$vg/$lv1"
check grep_dmsetup status $vg-$lv1 AD
lvconvert -y --repair $vg/$lv1 "$dev3"
lvs -a -o+devices $vg
aux wait_for_sync $vg $lv1
# Raid should have fixed device
check grep_dmsetup status $vg-$lv1 AA
check lv_on $vg ${lv1}_rimage_1 "$dev3"
lvremove -ff $vg/$lv1


# RAID1 dual-leg single replace after initial sync
lvcreate --type raid1 -m 1 -L $RAID_SIZE -n $lv1 $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv1
aux disable_dev "$dev2"
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev2"
vgextend $vg "$dev2"
lvremove -ff $vg/$lv1

# Delayed sync to allow for repair during rebuild
delay 50

# RAID1 triple-leg single replace during initial sync
lvcreate --type raid1 -m 2 -L $RAID_SIZE -n $lv1 $vg "$dev1" "$dev2" "$dev3"
aux disable_dev "$dev2" "$dev3"
# FIXME 2016/11/04 AGK: Disabled next line as it fails to guarantee it is not already in sync.
#not lvconvert -y --repair $vg/$lv1
aux wait_for_sync $vg $lv1
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev2" "$dev3"
vgextend $vg "$dev2" "$dev3"
lvremove -ff $vg/$lv1


# Larger RAID size possible for striped RAID
RAID_SIZE=64

# Fast sync and repair afterwards
delay 0
# RAID5 single replace after initial sync
lvcreate --type raid5 -i 2 -L $RAID_SIZE -n $lv1 $vg "$dev1" "$dev2" "$dev3"
aux wait_for_sync $vg $lv1
aux disable_dev "$dev3"
vgreduce --removemissing -f $vg
lvconvert -y --repair $vg/$lv1
aux enable_dev "$dev3"
pvcreate -yff "$dev3"
vgextend $vg "$dev3"
lvremove -ff $vg/$lv1

# Delayed sync to allow for repair during rebuild
delay 60

# RAID5 single replace during initial sync
lvcreate --type raid5 -i 2 -L $RAID_SIZE -n $lv1 $vg "$dev1" "$dev2" "$dev3"
aux disable_dev "$dev3"
# FIXME: there is quite big sleep on several 'status' read retries
# so over 3sec - it may actually finish full sync
# Use 'should' for this test result.
should not lvconvert -y --repair $vg/$lv1
aux wait_for_sync $vg $lv1
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev3"
vgextend $vg "$dev3"
lvremove -ff $vg/$lv1

# Fast sync and repair afterwards
delay 0

# RAID6 double replace after initial sync
lvcreate --type raid6 -i 3 -L $RAID_SIZE -n $lv1 $vg \
    "$dev1" "$dev2" "$dev3" "$dev4" "$dev5"
aux wait_for_sync $vg $lv1
aux disable_dev "$dev4" "$dev5"
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev4" "$dev5"
vgextend $vg "$dev4" "$dev5"
lvremove -ff $vg/$lv1

# Delayed sync to allow for repair during rebuild
delay 50

# RAID6 single replace after initial sync
lvcreate --type raid6 -i 3 -L $RAID_SIZE -n $lv1 $vg \
    "$dev1" "$dev2" "$dev3" "$dev4" "$dev5"
aux disable_dev "$dev4"
not lvconvert -y --repair $vg/$lv1
delay 0 # Fast sync and repair afterwards
aux disable_dev "$dev4" # Need to disable again after changing delay
aux wait_for_sync $vg $lv1
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev4"
vgextend $vg "$dev4"
lvremove -ff $vg/$lv1

# Delayed sync to allow for repair during rebuild
delay 50

# RAID10 single replace after initial sync
lvcreate --type raid10 -m 1 -i 2 -L $RAID_SIZE -n $lv1 $vg \
    "$dev1" "$dev2" "$dev3" "$dev4"
aux disable_dev "$dev4"
not lvconvert -y --repair $vg/$lv1
delay 0 # Fast sync and repair afterwards
aux disable_dev "$dev4" # Need to disable again after changing delay
aux disable_dev "$dev1"
aux wait_for_sync $vg $lv1
lvconvert -y --repair $vg/$lv1
vgreduce --removemissing $vg
aux enable_dev "$dev4"
vgextend $vg "$dev4"
lvremove -ff $vg/$lv1

vgremove -ff $vg
