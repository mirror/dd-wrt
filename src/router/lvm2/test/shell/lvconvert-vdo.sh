#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

#
# Main
#
aux have_vdo 6 2 0 || skip

aux prepare_vg 2 6400

# Conversion to vdo-pool
lvcreate -L5G -n $lv1 $vg
# Check there is big prompting warning
not lvconvert --type vdo-pool $vg/$lv1 |& tee out
grep "WARNING" out


lvconvert -y --type vdo-pool $vg/$lv1
lvremove -f $vg


# 
lvcreate -L5G -n $lv1 $vg
lvconvert -y --vdopool $vg/$lv1
lvremove -f $vg


lvcreate -L5G -n $lv1 $vg
lvconvert -y --vdopool $vg/$lv1 -n $lv2
check lv_field $vg/$lv1 segtype vdo-pool
check lv_field $vg/${lv1}_vdata segtype linear -a
check lv_field $vg/$lv2 segtype vdo
lvremove -f $vg


lvcreate -L5G -n $lv1 $vg
lvconvert -y --type vdo-pool $vg/$lv1 -n $lv2 -V10G
lvremove -f $vg


lvcreate -L5G -n $lv1 $vg
lvconvert -y --vdopool $vg/$lv1 -n $lv2 -V10G --compression n --deduplication n
check lv_field $vg/$lv1 size "5.00g"
check lv_field $vg/${lv1}_vdata size "5.00g" -a
check lv_field $vg/$lv2 size "10.00g"
lvremove -f $vg


# Simple stacking works...
# Just be sure test do not try to synchronize 5G of mirror!!
lvcreate -L5G --type mirror --nosync -n $lv1 $vg
lvconvert -y --vdopool $vg/$lv1 -n $lv2
lvs -a $vg
check lv_field $vg/${lv1}_vdata segtype mirror -a
lvremove -f $vg


vgremove -ff $vg
