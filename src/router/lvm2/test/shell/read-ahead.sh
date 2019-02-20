#!/usr/bin/env bash

# Copyright (C) 2008 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#
# tests basic functionality of read-ahead and ra regressions
#

test_description='Test read-ahead functionality'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 5

#COMM "test various read ahead settings (bz450922)"
lvcreate -l 100%FREE -i5 -I512 -n $lv $vg
ra=$(get lv_field $vg/$lv lv_kernel_read_ahead --units s --nosuffix)
test $(( ( ra / 5 ) * 5 )) -le $ra
not lvchange -r auto $vg/$lv 2>&1 | grep auto
check lv_field $vg/$lv lv_read_ahead auto
lvchange -r 640 $vg/$lv
check lv_field $vg/$lv lv_read_ahead 640 --units s --nosuffix
lvremove -ff $vg

#COMM "read ahead is properly inherited from underlying PV"
blockdev --setra 768 "$dev1"
vgscan
lvcreate -n $lv -L4m $vg "$dev1"
test "$(blockdev --getra "$DM_DEV_DIR/$vg/$lv")" -eq 768
lvremove -ff $vg

# Check default, active/inactive values for read_ahead / kernel_read_ahead
lvcreate -n $lv -l 50%FREE $vg
lvchange -an $vg/$lv
check lv_field $vg/$lv lv_read_ahead auto
check lv_field $vg/$lv lv_kernel_read_ahead -1
lvchange -r 512 $vg/$lv
lvchange -ay $vg/$lv
check lv_field $vg/$lv lv_read_ahead 256.00k
check lv_field $vg/$lv lv_kernel_read_ahead 256.00k

vgremove -ff $vg
