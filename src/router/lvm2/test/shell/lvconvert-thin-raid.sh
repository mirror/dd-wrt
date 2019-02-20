#!/usr/bin/env bash

# Copyright (C) 2014-2017 Red Hat, Inc. All rights reserved.
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

aux have_thin 1 0 0 || skip
aux have_raid 1 4 0 || skip

aux prepare_vg 4

# create RAID LVs for data and metadata volumes
lvcreate -aey -L10M --type raid1 -m3 -n $lv1 $vg
lvcreate -aey -L8M --type raid1 -m3 -n $lv2 $vg
aux wait_for_sync $vg $lv1
aux wait_for_sync $vg $lv2
lvchange -an $vg/$lv1

# FIXME: temporarily we return error code 5
INVALID=not
# conversion fails for internal volumes
$INVALID lvconvert --thinpool $vg/${lv1}_rimage_0
$INVALID lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $vg/${lv2}_rimage_0

lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $vg/$lv2

lvchange -ay $vg

lvconvert --splitmirrors 1 --name data2 $vg/${lv1}_tdata "$dev2"
lvconvert --splitmirrors 1 --name data3 $vg/${lv1}_tdata "$dev3"
# Check split and track gets rejected on 2-legged raid1
not lvconvert --splitmirrors 1 --trackchanges $vg/${lv1}_tdata "$dev4"
lvconvert -y --splitmirrors 1 --trackchanges $vg/${lv1}_tdata "$dev4"

lvconvert --splitmirrors 1 --name meta1 $vg/${lv1}_tmeta "$dev1"
lvconvert --splitmirrors 1 --name meta2 $vg/${lv1}_tmeta "$dev2"
# Check split and track gets rejected on 2-legged raid1
not lvconvert --splitmirrors 1 --trackchanges $vg/${lv1}_tmeta "$dev4"
lvconvert -y --splitmirrors 1 --trackchanges $vg/${lv1}_tmeta "$dev4"

lvremove -ff $vg/data2 $vg/data3 $vg/meta1 $vg/meta2

lvconvert --merge $vg/${lv1}_tdata_rimage_1
lvconvert --merge $vg/${lv1}_tmeta_rimage_1

lvconvert -y -m +1 $vg/${lv1}_tdata "$dev2"
lvconvert -y -m +1 $vg/${lv1}_tmeta "$dev1"

vgremove -ff $vg
