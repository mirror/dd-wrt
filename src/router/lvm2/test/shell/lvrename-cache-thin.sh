#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Check rename of stacked  thin over cached LV


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip
aux have_thin 1 0 0 || skip

aux prepare_vg 1 80

lvcreate -L10 -n cpool $vg
lvcreate -L10 -n tpool $vg
lvcreate -L10 -n $lv1 $vg

lvconvert --yes --cache --cachepool cpool $vg/tpool

# currently the only allowed stacking is cache thin data volume
lvconvert --yes --type thin-pool $vg/tpool

lvcreate -V10 $vg/tpool

# check cache pool remains same after thin-pool rename
lvrename $vg/tpool  $vg/newpool

check lv_exists $vg newpool cpool
check lv_not_exists $vg tpool

# allowing rename of internal cache pool
lvrename $vg/cpool  $vg/cachepool

check lv_exists $vg cachepool
check lv_not_exists $vg cpool

lvs -a $vg

vgremove -f $vg
