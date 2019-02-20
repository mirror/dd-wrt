#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 5
get_devs

vgcreate $vg1 "$dev1"
vgcreate $vg2 "$dev3" "$dev4" "$dev5"

UUID1=$(get vg_field $vg1 uuid)

aux disable_dev "$dev1"
pvscan
# dev1 is missing
fail pvs "${DEVICES[@]}"

# create a new vg1 on dev2,
# so dev1 and dev2 have different VGs with the same name
vgcreate $vg1 "$dev2"

UUID2=$(get vg_field $vg1 uuid)

# Once dev1 is visible again, both VGs named "vg1" are visible.
aux enable_dev "$dev1"

pvs "$dev1"

# reappearing device (rhbz 995440)
lvcreate -aey -m2 --type mirror -l4 --alloc anywhere --corelog -n $lv1 $vg2

aux disable_dev "$dev3"
lvconvert --yes --repair $vg2/$lv1
aux enable_dev "$dev3"

# here it should fix any reappeared devices
lvs

lvs -a $vg2 -o+devices 2>&1 | tee out
not grep reappeared out

# This removes the first "vg1" using its uuid
vgremove -ff -S vg_uuid=$UUID1

# This removes the second "vg1" using its name,
# now that there is only one VG with that name.
vgremove -ff $vg1 $vg2

