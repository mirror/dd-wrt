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

aux have_raid 1 3 0 || skip

aux prepare_pvs 5
get_devs

vgcreate $SHARED -s 256k "$vg" "${DEVICES[@]}"

# Start with linear on 2 PV and ensure that converting to
# RAID is not allowed to reuse PVs for different images.  (Bug 1113180)
lvcreate -aey -l 4 -n $lv1 $vg "$dev1:0-1" "$dev2:0-1"
not lvconvert -y --type raid1 -m 1 $vg/$lv1 "$dev1" "$dev2"
not lvconvert -y --type raid1 -m 1 $vg/$lv1 "$dev1" "$dev3:0-2"
lvconvert -y --type raid1 -m 1 $vg/$lv1 "$dev3"
not lvconvert -m 0 $vg/$lv1
lvconvert -y -m 0 $vg/$lv1
# RAID conversions are not honoring allocation policy!
# lvconvert -y --type raid1 -m 1 --alloc anywhere $vg/$lv1 "$dev1" "$dev2"
lvremove -ff $vg


# Setup 2-way RAID1 LV, spread across 4 devices.
# For each image:
#  - metadata LV + 1 image extent (2 total extents) on one PV
#  - 2 image extents on the other PV
# Then attempt allocation of another image from 2 extents on
# a 5th PV and the remainder of the rest of already used PVs.
#
# This should fail because there is insufficient space on the
# non-parallel PV (i.e. there is not enough space for the image
# if it doesn't share a PV with another image).
lvcreate --type raid1 -m 1 -l 3 -n $lv1 $vg \
    "$dev1:0-1" "$dev2:0-1" "$dev3:0-1" "$dev4:0-1"
aux wait_for_sync $vg $lv1
# Should not be enough non-overlapping space.
not lvconvert -m +1 $vg/$lv1 \
    "$dev5:0-1" "$dev1" "$dev2" "$dev3" "$dev4"
lvconvert -y -m +1 $vg/$lv1 "$dev5"
not lvconvert -m 0 $vg/$lv1
lvconvert -y -m 0 $vg/$lv1
# Should work due to '--alloc anywhere'
# RAID conversion not honoring allocation policy!
#lvconvert -y -m +1 --alloc anywhere $vg/$lv1 \
#    "$dev5:0-1" "$dev1" "$dev2" "$dev3" "$dev4"
lvremove -ff $vg


# Setup 2-way RAID1 LV, spread across 4 devices
#  - metadata LV + 1 image extent (2 total extents) on one PV
#  - 2 image extents on the other PV
# Kill one PV.  There should be enough space on the remaining
# PV for that image to reallocate the entire image there and
# still maintain redundancy.
lvcreate --type raid1 -m 1 -l 3 -n $lv1 $vg \
    "$dev1:0-1" "$dev2:0-1" "$dev3:0-1" "$dev4:0-1"
aux wait_for_sync $vg $lv1
aux disable_dev "$dev1"
lvconvert -y --repair $vg/$lv1 "$dev2" "$dev3" "$dev4"
#FIXME: ensure non-overlapping images (they should not share PVs)
aux enable_dev "$dev1"
lvremove -ff $vg

vgremove -ff $vg
