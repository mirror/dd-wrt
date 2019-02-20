#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA2110-1301 USA

# check we may convert thin-pool to raid1/raid10 and back
# RHBZ#1365286


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_thin 1 0 0 || skip
aux have_raid 1 9 0 || skip

aux prepare_vg 6

lvcreate -L4 -i3 -T $vg/pool -V10

for i in 1 2 ; do
lvconvert --type raid10 -y $vg/pool_tdata
check grep_dmsetup table $vg-pool_tdata "raid10"
aux wait_for_sync $vg pool_tdata

lvconvert --type striped -y $vg/pool_tdata
check grep_dmsetup table $vg-pool_tdata "striped"
done

lvremove -f $vg

lvcreate -L4  -T $vg/pool -V10 -n $lv1

for j in data meta ; do
  LV=pool_t${j}
  for i in 1 2 ; do
    lvconvert --type raid1 -m1 -y  $vg/$LV
    check grep_dmsetup table $vg-${LV} "raid1"
    aux wait_for_sync $vg $LV

    lvconvert --type raid1 -m0 -y  $vg/$LV
    check grep_dmsetup table ${vg}-${LV} "linear"
  done
done


#
# Now same test again, when lock holding LV is not a thin-poll
# but thinLV $lv1
#
lvchange -an $vg
lvchange -ay $vg/$lv1

for j in data meta ; do
  LV=pool_t${j}
  for i in 1 2 ; do
    lvconvert --type raid1 -m1 -y  $vg/$LV
    check grep_dmsetup table $vg-${LV} "raid1"
    aux wait_for_sync $vg $LV

    lvconvert --type raid1 -m0 -y  $vg/$LV
    check grep_dmsetup table ${vg}-${LV} "linear"
  done
done

vgremove -ff $vg
