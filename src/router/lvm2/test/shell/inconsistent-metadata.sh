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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 3 12
get_devs

lvcreate -aye --type mirror -m 1 -l 1 -n mirror $vg
lvcreate -l 1 -n resized $vg
lvchange -a n $vg/mirror

aux backup_dev "${DEVICES[@]}"

init() {
	aux restore_dev "${DEVICES[@]}"
	not check lv_field $vg/resized lv_size "8.00m"
	lvresize -L 8192K $vg/resized
	aux restore_dev "$dev1"
}

init
vgscan 2>&1 | tee cmd.out
grep "Inconsistent metadata found for VG $vg" cmd.out
vgscan 2>&1 | tee cmd.out
not grep "Inconsistent metadata found for VG $vg" cmd.out
check lv_field $vg/resized lv_size "8.00m"

# vgdisplay fixes
init
vgdisplay $vg 2>&1 | tee cmd.out
grep "Inconsistent metadata found for VG $vg" cmd.out
vgdisplay $vg 2>&1 | tee cmd.out
not grep "Inconsistent metadata found for VG $vg" cmd.out
check lv_field $vg/resized lv_size "8.00m"

# lvs fixes up
init
lvs $vg 2>&1 | tee cmd.out
grep "Inconsistent metadata found for VG $vg" cmd.out
vgdisplay $vg 2>&1 | tee cmd.out
not grep "Inconsistent metadata found for VG $vg" cmd.out
check lv_field $vg/resized lv_size "8.00m"

# vgs fixes up as well
init
vgs $vg 2>&1 | tee cmd.out
grep "Inconsistent metadata found for VG $vg" cmd.out
vgs $vg 2>&1 | tee cmd.out
not grep "Inconsistent metadata found for VG $vg" cmd.out
check lv_field $vg/resized lv_size "8.00m"

echo Check auto-repair of failed vgextend - metadata written to original pv but not new pv
vgremove -f $vg
pvremove -ff "${DEVICES[@]}"
pvcreate "${DEVICES[@]}"
aux backup_dev "$dev2"
vgcreate $SHARED $vg "$dev1"
vgextend $vg "$dev2"
aux restore_dev "$dev2"
vgscan
should check compare_fields vgs $vg vg_mda_count pvs "$dev2" vg_mda_count

vgremove -ff $vg
