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

aux prepare_pvs 3
get_devs

aux lvmconf 'allocation/maximise_cling = 0' \
	    'allocation/mirror_logs_require_separate_pvs = 1'

# not required, just testing
aux pvcreate --metadatacopies 0 "$dev1"

vgcreate $SHARED "$vg" "${DEVICES[@]}"
pvchange --addtag fast "${DEVICES[@]}"

# 3 stripes with 3 PVs (selected by tag, @fast) is fine
lvcreate -l3 -i3 $vg @fast

# too many stripes(4) for 3 PVs
not lvcreate -l4 -i4 $vg @fast

# 2 stripes is too many with just one PV
not lvcreate -l2 -i2 $vg "$DM_DEV_DIR/mapper/pv1"

# lvcreate mirror
lvcreate -aey -l1 --type mirror -m1 --nosync $vg @fast

# lvcreate mirror w/corelog
lvcreate -aey -l1 --type mirror -m2 --corelog --nosync $vg @fast

# lvcreate mirror w/no free PVs
not lvcreate -aey -l1 --type mirror -m2 $vg @fast

# lvcreate mirror (corelog, w/no free PVs)
not lvcreate -aey -l1 --type mirror -m3 --corelog $vg @fast

# lvcreate mirror with a single PV arg
not lvcreate -aey -l1 --type mirror -m1 --corelog $vg "$dev1"

vgremove -ff $vg
