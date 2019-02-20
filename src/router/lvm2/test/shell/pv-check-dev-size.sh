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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 1 8

aux lvmconf 'metadata/check_pv_device_sizes = 1'

CHECK_MSG="smaller than corresponding PV size"

vgcreate $SHARED "$vg" "$dev1" 2>err
not grep "$CHECK_MSG" err
pvs 2>err
not grep "$CHECK_MSG" err
vgremove -ff $vg

# set PV size to 2x dev size
pvcreate --yes --setphysicalvolumesize 16m "$dev1"
vgcreate $SHARED "$vg" "$dev1" 2>err
grep "$CHECK_MSG" err
pvs 2>err
grep "$CHECK_MSG" err
vgremove -ff $vg

# should be quiet if requested
aux lvmconf 'metadata/check_pv_device_sizes = 0'
pvcreate --yes --setphysicalvolumesize 16m "$dev1"
vgcreate $SHARED "$vg" "$dev1" 2>err
not grep "$CHECK_MSG" err
pvs 2>err
not grep "$CHECK_MSG" err

vgremove -ff $vg
