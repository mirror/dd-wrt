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
aux prepare_vg 5 8000

# Use 10M origin size
lvcreate -aey -L10 -n $lv1 $vg
lvcreate -H -L5 $vg/$lv1

# replace 10M size with 5M size of cache device
NEWCLINE=$(dmsetup table $vg-$lv1 | sed 's/20480/10240/')
dmsetup reload $vg-$lv1 --table "$NEWCLINE"
dmsetup resume $vg-$lv1

# Check that mismatching cache target is shown by lvs
lvs -a $vg 2>&1 | grep "WARNING"
check lv_attr_bit state $vg/$lv1 "X"

lvs -o+lv_active $vg

lvremove -f $vg



lvcreate --type cache-pool -L10 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg
lvs -o lv_name,cache_policy
lvs -o lv_name,cache_settings

lvremove -f $vg

lvcreate --type cache-pool -L10 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg --cachepolicy mq \
	 --cachesettings migration_threshold=233
lvs -o lv_name,cache_policy   | grep mq
lvs -o lv_name,cache_settings | grep migration_threshold=233

lvremove -f $vg

lvcreate --type cache-pool -L10 --cachepolicy mq --cachesettings migration_threshold=233 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg
lvs -o lv_name,cache_policy   | grep mq
lvs -o lv_name,cache_settings | grep migration_threshold=233

lvremove -f $vg

lvcreate --type cache-pool -L10 --cachepolicy mq --cachesettings migration_threshold=233 --cachesettings sequential_threshold=13 $vg/cpool
lvcreate --type cache -l 1 --cachepool $vg/cpool -n corigin $vg
lvs -o lv_name,cache_policy   | grep mq
lvs -a -o lv_name,cache_policy -S 'cache_policy=mq' | grep corigin
lvs -o lv_name,cache_settings | grep migration_threshold=233
lvs -o lv_name,cache_settings | grep sequential_threshold=13

lvcreate -n foo -l 1 $vg
lvs -S 'cache_policy=mq' | grep corigin
lvs -S 'cache_policy=mq' | not grep foo
lvs -S 'cache_policy=undefined' | not grep corigin
lvs -S 'cache_policy=undefined' | grep foo
lvs -o +cache_policy -S 'cache_policy=mq' | grep corigin
lvs -o +cache_policy -S 'cache_policy=mq' | not grep foo
lvs -o +cache_policy -S 'cache_policy=undefined' | not grep corigin
lvs -o +cache_policy -S 'cache_policy=undefined' | grep foo
lvs -o +cache_policy -O cache_policy
lvs -o +cache_settings -S 'cache_settings={migration_threshold=233}' | grep corigin
lvs -o +cache_settings -S 'cache_settings!={migration_threshold=233}' | grep foo
lvs -o +cache_policy -O cache_settings

lvremove -f $vg

lvcreate -n foo -l 1 $vg
lvs -a -S 'cache_policy=undefined' | grep foo

vgremove -ff $vg
