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

# Exercise conversion of cache and cache pool


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 8 0 || skip

aux prepare_vg 5 80

lvcreate --type cache-pool -an -v -L 2 -n cpool $vg

lvcreate -H --cachepolicy smq -L 4 -n corigin --cachepool $vg/cpool

check lv_field $vg/corigin cache_policy "smq"

lvconvert --splitcache $vg/corigin

lvs -o+cache_policy -a $vg

vgremove -f $vg
