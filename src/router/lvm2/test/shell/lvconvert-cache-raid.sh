#!/usr/bin/env bash

# Copyright (C) 2014-2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise usage of stacked cache volume using raid volume

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip
aux have_raid 1 0 0 || skip

aux lvmconf 'global/cache_disabled_features = [ "policy_smq" ]'

aux prepare_vg 5 80

# Bug 1095843
# lvcreate RAID1 origin, lvcreate cache-pool, and lvconvert to cache
lvcreate --type raid1 -m 1 --nosync -l 2 -n $lv1 $vg
lvcreate --type cache-pool -l 1 -n ${lv1}_cachepool $vg
lvconvert --cache -Zy --cachepool $vg/${lv1}_cachepool $vg/$lv1
check lv_exists $vg/${lv1}_corig_rimage_0 # ensure images are properly renamed
dmsetup table ${vg}-$lv1 | grep cache   # ensure it is loaded in kernel
lvremove -f $vg


# lvcreate RAID1 origin, lvcreate RAID1 cache-pool, and lvconvert to cache
lvcreate --type raid1 -m 1 --nosync -l 2 -n $lv1 $vg
lvcreate --type raid1 -m 1 --nosync -l 2 -n ${lv1}_cachepool $vg
#should lvs -a $vg/${lv1}_cdata_rimage_0  # ensure images are properly renamed
lvconvert --yes --type cache --cachemode writeback --cachepool $vg/${lv1}_cachepool $vg/$lv1 2>&1 | tee out
grep "WARNING: Data redundancy could be lost" out
check lv_exists $vg/${lv1}_corig_rimage_0        # ensure images are properly renamed
dmsetup table ${vg}-$lv1 | grep cache   # ensure it is loaded in kernel
lvremove -f $vg


lvcreate -n corigin -m 1 --type raid1 --nosync -l 10 $vg
lvcreate -n cpool --type cache $vg/corigin --cachemode writeback -l 10 2>&1 | tee out
grep "WARNING: Data redundancy could be lost" out
not lvconvert --splitmirrors 1 --name split $vg/corigin "$dev1"
lvconvert --yes --splitmirrors 1 --name split $vg/corigin "$dev1"

lvremove -f $vg

lvcreate -n cpool_meta -m 1 --type raid1 -l 10 $vg
lvcreate -n cpool -m 1 --type raid1 -l 10 $vg
aux wait_for_sync $vg cpool_meta
aux wait_for_sync $vg cpool
lvs -a -o+seg_pe_ranges $vg
lvconvert --yes --type cache-pool --poolmetadata $vg/cpool_meta $vg/cpool
lvcreate -n corigin --type cache --cachepool $vg/cpool -l 10

lvchange --syncaction repair $vg/cpool_cmeta
aux wait_for_sync $vg cpool_cmeta

lvchange --syncaction repair $vg/cpool_cdata
aux wait_for_sync $vg cpool_cdata

lvconvert -y --repair $vg/cpool_cmeta
lvconvert -y --repair $vg/cpool_cdata

# do not allow reserved names for *new* LVs
not lvconvert --splitmirrors 1 --name split_cmeta $vg/cpool_cmeta "$dev1"
not lvconvert --splitmirrors 1 --name split_cdata $vg/cpool_cdata "$dev1"

# but allow manipulating existing LVs with reserved names
aux wait_for_sync $vg cpool_cmeta
aux wait_for_sync $vg cpool_cdata
lvconvert --yes --splitmirrors 1 --name split_meta $vg/cpool_cmeta "$dev1"
lvconvert --yes --splitmirrors 1 --name split_data $vg/cpool_cdata "$dev1"
not lvconvert --splitmirrors 1 --name split_data $vg/cpool_cdata "$dev1"

lvremove -f $vg


# Test up/down raid conversion of cache pool data and metadata
lvcreate --type cache-pool $vg/cpool -l 10
lvcreate -H -n corigin --cachepool $vg/cpool -l 20 $vg

lvconvert -y -m +1 --type raid1 $vg/cpool_cmeta
check lv_field $vg/cpool_cmeta layout "raid,raid1"
check lv_field $vg/cpool_cmeta role "private,cache,pool,metadata"

lvconvert -y -m +1 --type raid1 $vg/cpool_cdata
check lv_field $vg/cpool_cdata layout "raid,raid1"
check lv_field $vg/cpool_cdata role "private,cache,pool,data"

not lvconvert -m -1  $vg/cpool_cmeta
lvconvert -y -m -1  $vg/cpool_cmeta
check lv_field $vg/cpool_cmeta layout "linear"
lvconvert -y -m -1  $vg/cpool_cdata
check lv_field $vg/cpool_cdata layout "linear"

lvremove -f $vg

vgremove -f $vg
