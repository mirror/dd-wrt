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

test_description="ensure pvmove works with raid segment types"
SKIP_WITH_LVMLOCKD=1

. lib/inittest

which md5sum || skip

aux have_raid 1 3 5 || skip

aux prepare_pvs 5 20
get_devs

vgcreate -s 128k "$vg" "${DEVICES[@]}"

for mode in "--atomic" ""
do
# Each of the following tests does:
# 1) Create two LVs - one linear and one other segment type
#    The two LVs will share a PV.
# 2) Move both LVs together
# 3) Move only the second LV by name

# Testing pvmove of RAID1 LV
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate -aey --regionsize 16K -l 2 --type raid1 -m 1 -n $lv1 $vg "$dev1" "$dev2"
check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg $lv1 "$dev1" "$dev2"
aux mkdev_md5sum $vg $lv1
pvmove $mode "$dev1" "$dev5"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check lv_tree_on $vg $lv1 "$dev2" "$dev5"
check dev_md5sum $vg $lv1
pvmove $mode -n $lv1 "$dev5" "$dev4"
check lv_tree_on $vg $lv1 "$dev2" "$dev4"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check dev_md5sum $vg $lv1
lvremove -ff $vg

# Testing pvmove of RAID10 LV
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate -aey -l 4 --type raid10 -i 2 -m 1 -n $lv1 $vg \
                "$dev1" "$dev2" "$dev3" "$dev4"
check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
aux mkdev_md5sum $vg $lv1

# Check collocation of SubLVs is prohibited
not pvmove $mode -n ${lv1}_rimage_0 "$dev1" "$dev2"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
not pvmove $mode -n ${lv1}_rimage_1 "$dev2" "$dev1"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
not pvmove $mode -n ${lv1}_rmeta_0 "$dev1" "$dev3"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"

pvmove $mode "$dev1" "$dev5"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check lv_tree_on $vg $lv1 "$dev2" "$dev3" "$dev4" "$dev5"
check dev_md5sum $vg $lv1
pvmove $mode -n $lv1 "$dev5" "$dev1"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check dev_md5sum $vg $lv1
lvremove -ff $vg

# Testing pvmove of RAID5 LV
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate -aey -l 4 --type raid5 -i 2 -n $lv1 $vg \
                "$dev1" "$dev2" "$dev3"
check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg $lv1 "$dev1" "$dev2" "$dev3"
aux mkdev_md5sum $vg $lv1
pvmove $mode "$dev1" "$dev5"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check lv_tree_on $vg $lv1 "$dev2" "$dev3" "$dev5"
check dev_md5sum $vg $lv1
pvmove $mode -n $lv1 "$dev5" "$dev4"
check lv_tree_on $vg $lv1 "$dev2" "$dev3" "$dev4"
check lv_tree_on $vg ${lv1}_foo "$dev5"
check dev_md5sum $vg $lv1

lvremove -ff $vg
done

vgremove -ff $vg
