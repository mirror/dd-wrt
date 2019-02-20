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

# Exercise creation of cache and cache pool volumes

# Full CLI uses  --type
# Shorthand CLI uses -H | --cache


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

# FIXME: parallel cache metadata allocator is crashing when used value 8000!
aux prepare_vg 5 80000

aux lvmconf 'global/cache_disabled_features = [ "policy_smq" ]'


#######################
# Cache_Pool creation #
#######################
# TODO: Unsupported yet creation of cache pool and cached volume at once
# TODO: Introduce  --pooldatasize
# TODO: Policy to determine cache pool size and cache pool name
invalid lvcreate -H -l 1 $vg
invalid lvcreate -H -l 1 --name $lv1 $vg
invalid lvcreate -l 1 --cache $vg
# Only cached volume could be created
invalid lvcreate -l 1 --type cache $vg
# Striping is not supported with cache-pool creation
invalid lvcreate -l 1 -i 2 --type cache-pool $vg
# Fails as it needs to see VG content
fail lvcreate -l 1 --type cache --cachepool $vg/pool1
fail lvcreate -l 1 --type cache --cachepool pool2 $vg
fail lvcreate -l 1 --cache $vg/pool3
fail lvcreate -l 1 -H --cachepool pool4 $vg
fail lvcreate -l 1 -H --name $lv2 $vg/pool5
fail lvcreate -l 1 -H --name $lv3 --cachepool $vg/pool6
fail lvcreate -l 1 -H --name $vg/$lv4 --cachepool pool7

# Unlike in thin pool case - cache pool and cache volume both need size arg.
# So we require cache pool to exist and need to fail when it's missing.
#
# --cachepool gives implicit --cache
fail lvcreate -l 1 --cachepool pool8 $vg

# no size specified
invalid lvcreate --cachepool pool $vg 2>&1 | tee err
#grep "specify either size or extents" err
grep "No command with matching syntax recognised" err

# Check nothing has been created yet
check vg_field $vg lv_count 0

# Checks that argument passed with --cachepool is really a cache-pool
lvcreate -an -l 1 -n $lv1 $vg
# Hint: nice way to 'tee' only stderr.log so we can check it's log_error()
fail lvcreate -L10 --cachepool $vg/$lv1 2> >(tee -a stderr.log >&2)
grep "not a cache pool" stderr.log

# With --type cache-pool we are clear which segtype has to be created
lvcreate -l 1 --type cache-pool $vg/pool1
check lv_field $vg/pool1 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --name $vg/pool2 $vg
check lv_field $vg/pool2 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --cachepool $vg/pool3 $vg
check lv_field $vg/pool3 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --cachepool $vg/pool4
check lv_field $vg/pool4 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --cachepool pool5 $vg
check lv_field $vg/pool5 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --name pool6 $vg
check lv_field $vg/pool6 segtype "cache-pool"
lvcreate -l 1 --type cache-pool --name $vg/pool7
check lv_field $vg/pool7 segtype "cache-pool"

lvremove -f $vg


# Check the percentage values are reported for both cache and cache-pool
lvcreate --type cache-pool  -L1 $vg/cpool
lvcreate -H -L4 -n $lv1 $vg/cpool

check lv_field $vg/$lv1 origin "[${lv1}_corig]"
check lv_field $vg/$lv1 copy_percent "0.00"
# there should be something present (value differs per policy version)
test -n "$(get lv_field $vg/$lv1 data_percent)"
test -n "$(get lv_field $vg/$lv1 metadata_percent)"
check lv_field $vg/cpool copy_percent "0.00"
test -n "$(get lv_field $vg/cpool data_percent)"
test -n "$(get lv_field $vg/cpool metadata_percent)"
# check we also display percent value for segmented output (-o+devices)
lvs -a -o+devices $vg/cpool | tee out
grep "0.00" out
lvremove -f $vg


# Validate ambiguous pool name is detected
invalid lvcreate -l 1 --type cache-pool --cachepool pool1 $vg/pool2
invalid lvcreate -l 1 --type cache-pool --name pool3 --cachepool pool4 $vg
invalid lvcreate -l 1 --type cache-pool --name pool6 --cachepool pool6 $vg/pool7
invalid lvcreate -l 1 --type cache-pool --name pool8 $vg/pool9

# Unsupported with cache & cache pool
invalid lvcreate --type cache-pool --discards passdown -l1 $vg
invalid lvcreate -H --discards passdown -l1 $vg
invalid lvcreate --type cache-pool --virtualsize 1T -l1 $vg
invalid lvcreate -H --virtualsize 1T -l1 $vg

check vg_field $vg lv_count 0


for mode in "" "--cachemode writethrough"
do

################
# Cache creation
# Creating a cache is a two phase process
# - first, cache_pool (or origin)
# - then, the cache LV (lvcreate distinguishes supplied origin vs cache_pool)
################

lvcreate --type cache-pool -l 1 -n pool $vg $mode
# Select automatic name for cached LV
lvcreate --type cache -l1 $vg/pool

lvcreate --type cache-pool -l 1 -n pool1 $vg $mode
lvcreate --cache -l1 -n $lv1 --cachepool $vg/pool1
dmsetup table ${vg}-$lv1 | grep cache  # ensure it is loaded in kernel

lvcreate --type cache-pool -l 1 -n pool2 $vg $mode
lvcreate -H -l1 -n $lv2 --cachepool pool2 $vg

#
# Now check removals
#

# Removal of cached LV removes every related LV
check lv_field $vg/$lv1 segtype "cache"
lvremove -f $vg/$lv1
check lv_not_exists $vg $lv1 pool1 pool1_cdata pool1_cmeta
# to preserve cachepool use  lvconvert --splitcache $vg/$lv1

# Removal of cache pool leaves origin uncached
check lv_field $vg/$lv2 segtype "cache"
lvremove -f $vg/pool2
check lv_not_exists $vg pool2 pool2_cdata pool2_cmeta
check lv_field $vg/$lv2 segtype "linear"

lvremove -f $vg

done


# Conversion through lvcreate case
# Bug 1110026
# Create origin, then cache pool and cache the origin
lvcreate -aey -l 2 -n $lv1 $vg
lvcreate --type cache -l 1 $vg/$lv1
dmsetup table ${vg}-$lv1 | grep cache  # ensure it is loaded in kernel

lvremove -f $vg


# Check minimum cache pool metadata size
lvcreate -l 1 --type cache-pool --poolmetadatasize 1 $vg 2>out
grep "WARNING: Minimum" out

# FIXME: This test is failing in allocator with smaller VG sizes
lvcreate -l 1 --type cache-pool --poolmetadatasize 17G $vg 2>out
grep "WARNING: Maximum" out

lvremove -f $vg

########################################
# Cache conversion and r/w permissions #
########################################

# writeable origin and 'default' => writable cache + origin
lvcreate -an -l1 -n $vg/$lv1
# do not allow stripping for cache-pool
fail lvcreate -H -i 2 -l1 -n cpool1 $vg/$lv1
lvcreate -H -l1 -n cpool1 $vg/$lv1
check lv_attr_bit perm $vg/cpool1 "w"
check lv_attr_bit perm $vg/${lv1}_corig "w"
check lv_attr_bit perm $vg/$lv1 "w"

# writeable origin and -pr => conversion is not supported
lvcreate -an -l1 -n $vg/$lv2
fail lvcreate -H -l1 -pr -n cpool2 $vg/$lv2

# read-only origin and -pr => read-only cache + origin
lvcreate -an -pr -l1 -n $vg/$lv3
lvcreate -an -H -l1 -pr -n cpool3 $vg/$lv3
check lv_attr_bit perm $vg/cpool3 "w"
check lv_attr_bit perm $vg/${lv3}_corig "r"
check lv_attr_bit perm $vg/$lv3 "r"
check inactive $vg $lv3
check inactive $vg cpool3

# read-only origin and 'default' => read-only cache + origin
lvcreate -an -pr -l1 -n $vg/$lv4
lvcreate -H -l1 -n cpool4 $vg/$lv4
check lv_attr_bit perm $vg/cpool4 "w"
check lv_attr_bit perm $vg/${lv4}_corig "r"
check lv_attr_bit perm $vg/$lv4 "r"

# read-only origin and -prw => conversion unsupported
lvcreate -an -pr -l1 -n $vg/$lv5
fail lvcreate -H -l1 -prw -n cpool5 $vg/$lv5

# cached volume respects permissions
lvcreate --type cache-pool -l1 -n $vg/cpool
lvcreate -H -l1 -pr -n $lv6 $vg/cpool
check lv_attr_bit perm $vg/cpool "w"
check lv_attr_bit perm $vg/$lv6 "r"

lvremove -f $vg

########################################
# Validate args are properly preserved #
########################################
lvcreate --type cache-pool -L10 --chunksize 256 --cachemode writeback $vg/cpool1
check lv_field $vg/cpool1 chunksize "256.00k"
check lv_field $vg/cpool1 cachemode "writeback"
# check striping is supported when creating a cached LV
lvcreate -H -L10 -i 2 -n $lv1 $vg/cpool1
check lv_field $vg/${lv1}_corig stripes "2" -a
check lv_field $vg/$lv1 chunksize "256.00k"
check lv_field $vg/$lv1 cachemode "writeback"

lvcreate --type cache-pool -L10 --chunksize 256 --cachemode writethrough $vg/cpool2
lvcreate -H -L10 --chunksize 512 --cachemode writeback -n $lv2 $vg/cpool2
check lv_field $vg/$lv2 chunksize "512.00k"
check lv_field $vg/$lv2 cachemode "writeback"

# Chunk bigger then pool size
fail lvcreate --type cache-pool -l1 --chunksize 1G $vg/cpool3

lvcreate --type cache-pool -L10 $vg/cpool4
fail lvcreate -H -L10 --chunksize 16M $vg/cpool4

lvdisplay --maps $vg

lvremove -f $vg

lvcreate --type cache-pool -L10 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg --cachesettings migration_threshold=233
dmsetup status | grep $vg
dmsetup status | grep $vg-corigin | grep 'migration_threshold 233'
lvchange -an $vg
lvchange -ay $vg
dmsetup status | grep $vg-corigin | grep 'migration_threshold 233'

lvremove -f $vg

lvcreate --type cache-pool -L10 --cachepolicy mq --cachesettings migration_threshold=233 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg
dmsetup status | grep $vg
dmsetup status | grep $vg-corigin | grep 'migration_threshold 233'

lvremove -f $vg


##############################
# Test things that should fail
##############################

# Creation of read-only cache pool is not supported
invalid lvcreate -pr --type cache-pool -l1 -n $vg/cpool

# Atempt to use bigger chunk size then cache pool data size
fail lvcreate -l 1 --type cache-pool --chunksize 16M $vg 2>out
grep "chunk size" out

# Option testing
# --chunksize
# --cachepolicy
# --poolmetadatasize
# --poolmetadataspare

vgremove -ff $vg
