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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext4 || skip
aux have_raid 1 3 5 || skip

aux prepare_vg 4
get_devs

for d in "$dev1" "$dev2" "$dev3" "$dev4"
do
	aux delay_dev "$d" 0 20 "$(get first_extent_sector "$d")"
done

#
# Test writemostly prohibited on resynchronizing raid1
#

# Create 4-way raid1 LV
lvcreate -aey --ty raid1 -m 3 -Zn -L16M -n $lv1 $vg
not lvchange -y --writemostly "$dev1" "$vg/$lv1"
check lv_field $vg/$lv1 segtype "raid1"
check lv_field $vg/$lv1 stripes 4
check lv_attr_bit health $vg/${lv1}_rimage_0 "-"
aux enable_dev "${DEVICES[@]}"
aux wait_for_sync $vg $lv1
lvchange -y --writemostly "$dev1" "$vg/$lv1"
check lv_attr_bit health $vg/${lv1}_rimage_0 "w"

vgremove -ff $vg
