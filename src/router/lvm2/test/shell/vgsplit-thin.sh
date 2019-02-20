#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test vgsplit command options for validity

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip

aux prepare_devs 5
get_devs

vgcreate "$vg1" "${DEVICES[@]}"
lvcreate -T -L8M $vg1/pool1 -V10M -n $lv1 "$dev1" "$dev2"
lvcreate -T -L8M $vg1/pool2 -V10M -n $lv2 "$dev3" "$dev4"

# Test with external origin if available
lvcreate -l1 -an -pr --zero n -n eorigin $vg1 "$dev5"
aux have_thin 1 5 0 && lvcreate -an -s $vg1/eorigin -n $lv3 --thinpool $vg1/pool1

# Cannot move active thin
not vgsplit $vg1 $vg2 "$dev1" "$dev2" "$dev5"

vgchange -an $vg1
not vgsplit $vg1 $vg2 "$dev1"
not vgsplit $vg1 $vg2 "$dev2" "$dev3"
vgsplit $vg1 $vg2 "$dev1" "$dev2" "$dev5"
lvs -a -o+devices $vg1 $vg2

vgmerge $vg1 $vg2

vgremove -ff $vg1

# Test vgsplit with ext.origin:
if aux have_thin 1 5 0; then
vgcreate "$vg1" "${DEVICES[@]}"
lvcreate -T -L8M $vg1/pool1 -V10M -n $lv1 "$dev1" "$dev2"
lvcreate -l1 -an -pr -n $lv2 $vg1 "$dev3"
lvcreate -s $vg1/$lv2 -n $lv3 --thinpool $vg1/pool1
lvcreate -l1 -n $lv4 $vg1 "$dev4"
vgchange -an $vg1

# Can not split ext.origin from thin-data:
not vgsplit $vg1 $vg2 "$dev1" "$dev2"
not vgsplit $vg1 $vg2 "$dev3"

vgsplit $vg1 $vg2 "$dev1" "$dev2" "$dev3"

vgmerge $vg1 $vg2

vgremove -ff $vg1
fi
