#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test merge of thin snapshot



export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

MKFS=mkfs.ext2
which $MKFS  || skip
which fsck || skip

MKFS="$MKFS -b4096"
#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_vg 2

lvcreate -T -L8M $vg/pool -V10M -n $lv1
lvcreate -s -K -n snap $vg/$lv1
# check exclusive lock is preserved after merge
check lv_field "$vg/$lv1" lv_active_exclusively "active exclusively"
lvconvert --merge $vg/snap
check lv_field "$vg/$lv1" lv_active_exclusively "active exclusively"
lvremove -ff $vg


lvcreate -T -L8M $vg/pool -V10M -n $lv1
lvchange --addtag tagL $vg/$lv1

mkdir mnt
$MKFS "$DM_DEV_DIR/$vg/$lv1"
mount "$DM_DEV_DIR/$vg/$lv1" mnt
touch mnt/test

lvcreate -K -s -n snap --addtag tagS $vg/$lv1
mkdir mntsnap
$MKFS "$DM_DEV_DIR/$vg/snap"
mount "$DM_DEV_DIR/$vg/snap" mntsnap
touch mntsnap/test_snap

lvs -o+tags,thin_id $vg

lvcreate -s -n snap1 $vg/$lv1

lvconvert --merge $vg/snap &>out
grep "Merging of thin snapshot $vg/snap will occur on next activation of $vg/${lv1}." out

# Can't merge another snapshot while "snap" is still being 'merged'.
not lvconvert --merge $vg/snap1 &>out
grep "Cannot merge snapshot" out

umount mnt

# Merge cannot happen
lvchange --refresh $vg/$lv1
check lv_field  $vg/$lv1 thin_id "1"

# Fails since it cannot deactivate both
not lvchange -an $vg/$lv1

# But test $lv1 is not active
check inactive $vg $lv1

# Also still cannot reactivate $lv1
not lvchange -ay $vg/$lv1

umount mntsnap

lvdisplay -a $vg | tee out
grep "merged with" out
grep "merging to" out

# Check there is no support for manipulation with hidden 'snap'
not lvchange --refresh $vg/snap
not lvchange -an $vg/snap
not lvremove $vg/snap


# Finally deactivate 'snap' again via $lv1
lvchange -an $vg/$lv1

# Still must not be activable
not lvchange -K -ay $vg/snap

lvs -a -o +tags,thin_id $vg

# Test if merge happens
lvchange -ay $vg/$lv1
check lv_exists $vg $lv1
check lv_field  $vg/$lv1 thin_id "2"
check lv_field $vg/$lv1 tags "tagL"
check lv_not_exists $vg snap

fsck -n "$DM_DEV_DIR/$vg/$lv1"
mount "$DM_DEV_DIR/$vg/$lv1" mnt
test -e mnt/test_snap
umount mnt


# test if thin snapshot has also 'old-snapshot'

lvcreate -s -n snap $vg/$lv1

# Also add old snapshot to thin origin
lvcreate -s -L10 -n oldsnapof_${lv1} $vg/$lv1
not lvconvert --merge $vg/snap
$MKFS "$DM_DEV_DIR/$vg/oldsnapof_${lv1}"
lvconvert --merge $vg/oldsnapof_${lv1}
fsck -n "$DM_DEV_DIR/$vg/$lv1"
check lv_not_exists $vg oldsnapof_${lv1}
# Add old snapshot to thin snapshot
lvcreate -s -L10 -n oldsnapof_snap $vg/snap
lvconvert --merge $vg/snap
lvremove -f $vg/oldsnapof_snap
check lv_field  $vg/$lv1 thin_id "4"

# Check --mergethin
lvcreate -s -n snap $vg/$lv1
check lv_field  $vg/snap thin_id "5"
lvconvert --mergethin $vg/snap  &>out
grep "Volume $vg/snap replaced origin $vg/${lv1}." out
check lv_field  $vg/$lv1 thin_id "5"

vgremove -ff $vg
