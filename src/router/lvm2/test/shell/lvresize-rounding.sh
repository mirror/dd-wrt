#!/usr/bin/env bash

# Copyright (C) 2007-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 3 22
get_devs

vgcreate $SHARED -s 32K "$vg" "${DEVICES[@]}"

lvcreate -an -Zn -l4 -i3 -I64 $vg

lvcreate -an -Zn -l8 -i2 -I64 $vg

lvcreate -an -Zn -l16 $vg

lvcreate -an -Zn -l32 -i3 -I64 -n $lv1 $vg

lvresize -l+64 -i3 -I64 $vg/$lv1

lvresize -l+64 -i3 -I128 $vg/$lv1

#lvcreate -l100%FREE -i3 -I64 --alloc anywhere $vg
vgremove -f $vg

# 15 extents
LVM_TEST_AUX_TRACE=yes
aux prepare_vg 3 22
unset LVM_TEST_AUX_TRACE

# Block some extents
lvcreate -an -Zn -l4 -i3 $vg
lvcreate -an -Zn -l1 $vg

lvcreate -an -Zn -l100%FREE -n $lv1 -i3 $vg
check vg_field $vg vg_free_count 2
lvremove -f $vg/$lv1

lvcreate -an -Zn -l1 -n $lv1 -i3 $vg
lvextend -l+100%FREE -i3 $vg/$lv1
check vg_field $vg vg_free_count 2

lvreduce -f -l50%LV $vg/$lv1
vgremove -f $vg

vgcreate $SHARED -s 4M $vg "$dev1" "$dev2" "$dev3"

# Expect to play with 15 extents
check vg_field $vg vg_free_count 15

# Should be rounded to 12 extents
lvcreate -an -Zn -l10 -n lv -i3 $vg
check vg_field $vg vg_free_count 3

# Should want 16 extents
not lvextend -l+4 $vg/lv

# Round up to whole free space
lvextend -l+100%FREE $vg/lv
check vg_field $vg vg_free_count 0

# Rounds up and should reduce just by 3 extents
lvreduce -f -l-4 $vg/lv
check vg_field $vg vg_free_count 3

# Should round up to 15 extents
lvextend -f -l+1 $vg/lv
check vg_field $vg vg_free_count 0

lvreduce -f -l-4 $vg/lv
check vg_field $vg vg_free_count 3

lvextend -l90%VG $vg/lv
check vg_field $vg vg_free_count 0

not lvreduce -f -l-10%LV $vg/lv
check vg_field $vg vg_free_count 0

lvreduce -f -l-20%LV $vg/lv
check vg_field $vg vg_free_count 3
