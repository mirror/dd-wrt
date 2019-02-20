#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test various supported conversion of snapshot of cached volume

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 1


# Prepare cached LV
lvcreate -aey -L1 -n $lv1 $vg
lvcreate -H -L2 -n cpool $vg/$lv1

# Prepare snapshot 'cow' LV
lvcreate -L3 -n cow $vg

# Can't use  'cached'  cow volume
not lvconvert -s cow $vg/$lv1

# Use cached LV with 'striped' cow volume
lvconvert -y -s $vg/$lv1 cow
check lv_field $vg/cow segtype linear
check lv_field $vg/$lv1 segtype cache

# Drop cache while being in-use origin
lvconvert --splitcache $vg/$lv1
check lv_field $vg/$lv1 segtype linear

# Cache existing origin
lvconvert -y --cache $vg/$lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 segtype cache

# Cannot split from 'origin' (being cached LV)
not lvconvert -y --splitsnapshot $vg/$lv1

lvchange --cachemode writeback $vg/$lv1
check lv_field  $vg/$lv1 cache_mode "writeback"
check grep_dmsetup status  ${vg}-${lv1}-real "writeback"

lvchange --cachemode writethrough $vg/$lv1
check lv_field  $vg/$lv1 cache_mode "writethrough"
check grep_dmsetup status  ${vg}-${lv1}-real "writethrough"

# Split 'cow'  from cached origin
lvconvert -y --splitsnapshot $vg/cow
get lv_field $vg/cow attr | grep "^-wi"

vgremove -f $vg
