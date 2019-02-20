#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise toollib process_each_lv'
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 1 256

lvcreate -an -n $lv1 -l4 $vg
lvcreate -an -n $lv2 -L4 $vg
lvcreate -an -n $lv3 -l+4 $vg
lvcreate -an -n $lv4 -L+4 $vg
not lvcreate -n $lv5 -l-4 $vg
not lvcreate -n $lv5 -L-4 $vg

lvremove $vg/$lv1
lvremove $vg/$lv2
lvremove $vg/$lv3
lvremove $vg/$lv4

lvcreate -an -n $lv1 -l4 $vg
lvresize -y -l8 $vg/$lv1
lvresize -y -L16 $vg/$lv1
lvresize -y -l+1 $vg/$lv1
lvresize -y -L+1 $vg/$lv1
lvresize -y -l-1 $vg/$lv1
lvresize -y -L-1 $vg/$lv1

lvcreate -an -n $lv2 -l4 $vg
lvextend -y -l8 $vg/$lv2
lvextend -y -L16 $vg/$lv2
lvextend -y -l+1 $vg/$lv2
lvextend -y -L+1 $vg/$lv2
not lvextend -y -l-1 $vg/$lv2
not lvextend -y -L-1 $vg/$lv2

lvcreate -an -n $lv3 -l64 $vg
lvreduce -y -l32 $vg/$lv3
lvreduce -y -L8 $vg/$lv3
lvreduce -y -l-1 $vg/$lv3
lvreduce -y -L-1 $vg/$lv3
not lvreduce -y -l+1 $vg/$lv3
not lvreduce -y -L+1 $vg/$lv3

# relative with percent extents

lvcreate -an -n $lv6 -l+100%FREE $vg
lvremove $vg/$lv6

lvcreate -an -n $lv6 -l1 $vg
lvextend -y -l+100%FREE $vg/$lv6
lvremove $vg/$lv6

lvcreate -an -n $lv6 -l1 $vg
lvresize -y -l+100%FREE $vg/$lv6
lvremove $vg/$lv6

if aux have_thin 1 0 0 ; then
# relative poolmetadatasize
lvcreate --type thin-pool -L64 --poolmetadatasize 32 -n $lv7 $vg
lvresize --poolmetadatasize 64 $vg/$lv7
lvresize --poolmetadatasize +8 $vg/$lv7
not lvresize -y --poolmetadatasize -8 $vg/$lv7

lvextend --poolmetadatasize +8 $vg/$lv7
not lvextend -y --poolmetadatasize -8 $vg/$lv7
fi

vgremove -y $vg
