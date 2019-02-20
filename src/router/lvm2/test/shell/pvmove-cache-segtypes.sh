#!/usr/bin/env bash

# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description="ensure pvmove works with the cache segment types"
SKIP_WITH_LVMLOCKD=1

. lib/inittest

# pvmove fails when a RAID LV is the origin of a cache LV
# pvmoving cache types is currently disabled in tools/pvmove.c
# So, for now we set everything up and make sure pvmove /isn't/ allowed.
# This allows us to ensure that it is disallowed even when there are
# stacking complications to consider.

which md5sum || skip

aux have_cache 1 3 0 || skip
# for stacking
aux have_thin 1 8 0 || skip
aux have_raid 1 4 2 || skip

aux prepare_vg 5 80

for mode in "--atomic" ""
do
# Each of the following tests does:
# 1) Create two LVs - one linear and one other segment type
#    The two LVs will share a PV.
# 2) Move both LVs together
# 3) Move only the second LV by name

# Testing pvmove of cache-pool LV (can't check contents though)
###############################################################
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate --type cache-pool -n ${lv1}_pool -l 4 $vg "$dev1"
check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg ${lv1}_pool "$dev1"

pvmove $mode "$dev1" "$dev5" 2>&1 | tee out
lvs -a -o+devices $vg
check lv_tree_on $vg ${lv1}_pool "$dev5"
check lv_tree_on $vg ${lv1}_foo "$dev5"

lvremove -ff $vg
dmsetup info -c | not grep $vg

# Testing pvmove of origin LV
#############################
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate --type cache-pool -n ${lv1}_pool -l 4 $vg "$dev5"
lvcreate --type cache -n $lv1 -l 8 $vg/${lv1}_pool "$dev1"

check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg ${lv1}_pool "$dev5"
check lv_tree_on $vg ${lv1} "$dev1"

aux mkdev_md5sum $vg $lv1
pvmove $mode "$dev1" "$dev3" 2>&1 | tee out
check lv_tree_on $vg ${lv1}_foo "$dev3"
#check lv_tree_on $vg ${lv1}_pool "$dev5"
lvs -a -o name,attr,devices $vg
check lv_tree_on $vg ${lv1} "$dev3"
#check dev_md5sum $vg $lv1

#pvmove $mode -n $lv1 "$dev3" "$dev1"
#check lv_tree_on $vg ${lv1}_foo "$dev3"
#check lv_tree_on $vg ${lv1}_pool "$dev5"
#check lv_tree_on $vg ${lv1} "$dev1"
#check dev_md5sum $vg $lv1
lvremove -ff $vg
dmsetup info -c | not grep $vg

# Testing pvmove of a RAID origin LV
####################################
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate --type raid1 -m 1 -l 8 -n $lv1 $vg "$dev1" "$dev2"
lvcreate --type cache -l 4 -n ${lv1}_pool $vg/$lv1 "$dev5"
check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg ${lv1} "$dev1" "$dev2"
check lv_tree_on $vg ${lv1}_pool "$dev5"

aux mkdev_md5sum $vg $lv1
pvmove $mode "$dev1" "$dev3" 2>&1 | tee out
check lv_tree_on $vg ${lv1}_foo "$dev3"
lvs -a -o+devices $vg
not check lv_tree_on $vg ${lv1} "$dev1"
#check lv_tree_on $vg ${lv1}_pool "$dev5"
#check dev_md5sum $vg $lv1 -- THIS IS WHERE THINGS FAIL IF PVMOVE NOT DISALLOWED

#pvmove $mode -n $lv1 "$dev3" "$dev1"
#check lv_tree_on $vg ${lv1}_foo "$dev3"
#check lv_tree_on $vg ${lv1} "$dev1" "$dev2"
#check lv_tree_on $vg ${lv1}_pool "$dev5"
#check dev_md5sum $vg $lv1
lvremove -ff $vg
dmsetup info -c | not grep $vg

# Testing pvmove of a RAID cachepool (metadata and data)
########################################################
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
lvcreate --type raid1 -L 6M -n meta $vg "$dev1" "$dev2"
lvcreate --type raid1 -L 4M -n ${lv1}_pool $vg "$dev1" "$dev2"
lvconvert --yes --type cache-pool $vg/${lv1}_pool --poolmetadata $vg/meta
lvcreate --type cache -n $lv1 -L 8M $vg/${lv1}_pool "$dev5"

check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg ${lv1}_pool "$dev1" "$dev2"
check lv_tree_on $vg ${lv1} "$dev5"

aux mkdev_md5sum $vg $lv1
# This will move ${lv1}_foo and the cache-pool data & meta
#  LVs, both of which contain a RAID1 _rimage & _rmeta LV - 5 total LVs
pvmove $mode "$dev1" "$dev3" 2>&1 | tee out
check lv_tree_on $vg ${lv1}_foo "$dev3"
not check lv_tree_on $vg ${lv1}_pool "$dev1"
#check lv_tree_on $vg ${lv1} "$dev5"
#check dev_md5sum $vg $lv1

#pvmove $mode -n ${lv1}_pool "$dev3" "$dev1"
#check lv_tree_on $vg ${lv1}_foo "$dev3"
#check lv_tree_on $vg ${lv1}_pool "$dev1" "$dev2"
#check lv_tree_on $vg ${lv1} "$dev5"
#check dev_md5sum $vg $lv1
lvremove -ff $vg
dmsetup info -c | not grep $vg

# Testing pvmove of Thin-pool on cache LV on RAID
#################################################
lvcreate -aey -l 2 -n ${lv1}_foo $vg "$dev1"
# RAID for cachepool
lvcreate --type raid1 -m 1 -L 6M -n meta $vg "$dev1" "$dev2"
lvcreate --type raid1 -m 1 -L 4M -n cachepool $vg "$dev1" "$dev2"
lvconvert --yes --type cache-pool $vg/cachepool --poolmetadata $vg/meta
# RAID for thin pool data LV
lvcreate --type raid1 -m 1 -L 8 -n thinpool $vg "$dev3" "$dev4"
# Convert thin pool data to a cached LV
lvconvert --type cache -Zy $vg/thinpool --cachepool $vg/cachepool
# Create simple thin pool meta
lvcreate -aey -L 2M -n meta $vg "$dev1"
# Use thin pool data LV to build a thin pool
lvconvert --yes --thinpool $vg/thinpool --poolmetadata $vg/meta
# Create a thin lv for fun
lvcreate -T $vg/thinpool -V 20 -n thin_lv

check lv_tree_on $vg ${lv1}_foo "$dev1"
check lv_tree_on $vg cachepool "$dev1" "$dev2"
check lv_tree_on $vg thinpool "$dev1" "$dev3" "$dev4"

aux mkdev_md5sum $vg thin_lv
lvs -a -o name,attr,devices $vg
# Should move ${lv1}_foo and thinpool_tmeta from dev1 to dev5
pvmove $mode "$dev1" "$dev5" 2>&1 | tee out
lvs -a -o name,attr,devices $vg
check lv_tree_on $vg ${lv1}_foo "$dev5"
not check lv_tree_on $vg cachepool "$dev1"
check lv_tree_on $vg thinpool "$dev3" "$dev4" "$dev5" # Move non-cache tmeta
#check dev_md5sum $vg/thin_lv

#pvmove $mode -n $vg/cachepool "$dev5" "$dev1"
#check lv_tree_on $vg ${lv1}_foo "$dev5"
#check lv_tree_on $vg $vg/cachepool "$dev1" "$dev2"
#check lv_tree_on $vg $vg/thinpool "$dev3" "$dev4"
#check dev_md5sum $vg/thin_lv

lvremove -ff $vg
dmsetup info -c | not grep $vg

done
