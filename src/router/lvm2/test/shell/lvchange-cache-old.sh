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

# Exercise usage of older metadata which are missing some new settings


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

# FIXME: parallel cache metadata allocator is crashing when used value 8000!
aux prepare_vg 5 80


lvcreate -l 10 --type cache-pool $vg/cpool
lvcreate -l 20 -H -n $lv1 --cachepool $vg/cpool $vg

vgcfgbackup -f backup $vg

# check metadata without cache policy
lvchange -an $vg
grep -v "policy =" backup >backup_1
vgcfgrestore -f backup_1 $vg
lvchange -ay $vg

# check metadata without cache mode
lvchange -an $vg
grep -v "cache_mode =" backup >backup_2
vgcfgrestore -f backup_2 $vg
lvchange -ay $vg

vgremove -ff $vg
