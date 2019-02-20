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

# Check very large device size (upto 15Exa bytes)
# this needs 64bit arch

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux can_use_16T || skip

aux prepare_pvs 1
get_devs

# Prepare large enough backend device
vgcreate -s 4M "$vg" "${DEVICES[@]}"
lvcreate --type snapshot -s -l 100%FREE -n $lv $vg --virtualsize 15P
aux extend_filter_LVMTEST

# Check usability with largest extent size
pvcreate "$DM_DEV_DIR/$vg/$lv"
vgcreate -s 4G $vg1 "$DM_DEV_DIR/$vg/$lv"

lvcreate -an -Zn -l50%FREE -n $lv1 $vg1
lvcreate -s -l100%FREE -n $lv2 $vg1/$lv1
check lv_field $vg1/$lv2 size "7.50p"
lvremove -ff $vg1

lvcreate --type snapshot -V15E -l1 -n $lv1 -s $vg1
check lv_field $vg1/$lv1 origin_size "15.00e"

vgremove -ff $vg1
vgremove -ff $vg
