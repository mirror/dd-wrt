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

aux prepare_devs 5
get_devs

pvcreate "$dev1"
pvcreate --metadatacopies 0 "$dev2"
pvcreate --metadatacopies 0 "$dev3"
pvcreate "$dev4"
pvcreate --metadatacopies 0 "$dev5"

vgcreate $SHARED "$vg" "${DEVICES[@]}"
lvcreate -n $lv -l 1 -i5 -I256 $vg

pvchange -x n "$dev1"
pvchange -x y "$dev1"
vgchange -a n $vg
pvchange --uuid "$dev1"
pvchange --uuid "$dev2"
vgremove -f $vg

# check that PVs without metadata don't cause too many full device rescans (bz452606)
for mdacp in 1 0; do
	pvcreate --metadatacopies "$mdacp" "${DEVICES[@]}"
	pvcreate "$dev1"
	vgcreate $SHARED "$vg" "${DEVICES[@]}"
	lvcreate -n $lv1 -l 2 -i5 -I256 $vg
	lvcreate -aey -n $lv2 --type mirror -m2 -l 2  $vg
	lvchange -an $vg/$lv1 $vg/$lv2
	vgchange -aey $vg
	lvchange -an $vg/$lv1 $vg/$lv2
	vgremove -f $vg
done
not grep "Cached VG .* incorrect PV list" out0
