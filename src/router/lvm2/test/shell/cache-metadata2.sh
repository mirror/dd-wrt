#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise usage of metadata2 cache metadata format


SKIP_WITH_LVMPOLLD=1

# Until new version of cache_check tools - no integrity validation
LVM_TEST_CACHE_CHECK_CMD=""

. lib/inittest

META2=
aux have_cache 1 10 0 || {
	META2=not
	aux have_cache 1 3 0 || skip
}

aux prepare_vg 5 80

lvcreate -L2 -n $lv1 $vg

lvcreate --type cache-pool -L1 $vg/cpool1
# no parameter - no format is stored
check lv_field $vg/cpool1 cachemetadataformat ""

lvcreate --type cache-pool -L1 --config 'allocation/cache_metadata_format=1' $vg/cpool
# format is in configuration - would be applied during actual caching
# so not stored in this moment
check lv_field $vg/cpool cachemetadataformat ""


lvcreate --type cache-pool -L1 --cachemetadataformat 1 $vg/cpool2
# format was specified on cmdline  - preserve it metadata
check lv_field $vg/cpool2 cachemetadataformat "1"

lvconvert --yes -H --cachepool $vg/cpool --config 'allocation/cache_metadata_format=1' $vg/$lv1
check lv_field $vg/cpool2 cachemetadataformat "1"

lvs -a -o+cachemetadataformat $vg

lvremove -f $vg

lvcreate --type cache-pool --cachepolicy cleaner --cachemetadataformat 1 -L1 $vg/cpool
lvcreate -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 cachemetadataformat "1"
lvremove -f $vg

if [ -z "$META2" ]; then
# for these test we need kernel with metadata2 support

lvcreate --type cache-pool -L1 $vg/cpool
lvcreate -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 cachemetadataformat "2"
lvremove -f $vg

lvcreate -L10 -n $lv1 $vg
lvcreate --type cache-pool -L1 $vg/cpool
lvconvert -y -H --cachepool $vg/cpool $vg/$lv1
check lv_field $vg/$lv1 cachemetadataformat "2"
lvremove -f $vg


lvcreate -L10 -n $lv1 $vg
lvcreate --type cache-pool -L1 $vg/cpool
lvconvert --cachemetadataformat 1 -y -H --cachepool $vg/cpool $vg/$lv1
check lv_field $vg/$lv1 cachemetadataformat "1"
lvremove -f $vg

lvcreate -L10 -n $lv1 $vg
lvcreate --type cache-pool -L1 $vg/cpool
lvconvert --config 'allocation/cache_metadata_format=1' -y -H --cachepool $vg/cpool $vg/$lv1
check lv_field $vg/$lv1 cachemetadataformat "1"
lvremove -f $vg

lvcreate --type cache-pool --cachepolicy cleaner -L1 $vg/cpool
lvcreate -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 cachemetadataformat "2"
lvremove -f $vg

lvcreate --type cache-pool --cachepolicy mq --cachemetadataformat 1 -L1 $vg/cpool
check lv_field $vg/cpool cachemetadataformat "1"
lvcreate -H -L10 -n $lv1 --cachemetadataformat 2 --cachepool $vg/cpool
check lv_field $vg/$lv1 cachemetadataformat "2"
lvremove -f $vg

fi
#lvs -a -o name,cachemetadataformat,kernelmetadataformat,chunksize,cachepolicy,cachemode $vg

vgremove -f $vg
