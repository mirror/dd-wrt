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

# 'Exercise some lvcreate diagnostics'


SKIP_WITH_LVMPOLLD=1

. lib/inittest

cleanup_lvs() {
	lvremove -ff $vg
	(dm_table | not grep $vg) || \
		die "ERROR: lvremove did leave some some mappings in DM behind!"
}

aux prepare_pvs 2
get_devs

aux pvcreate --metadatacopies 0 "$dev1"
aux vgcreate $SHARED "$vg" "${DEVICES[@]}"

# ---
# Create snapshots of LVs on --metadatacopies 0 PV (bz450651)
lvcreate -aey -n$lv1 -l4 $vg "$dev1"
lvcreate -n$lv2 -l4 -s $vg/$lv1
lvcreate -n$lv3 -l4 --permission r -s $vg/$lv1
cleanup_lvs

vgremove -ff $vg
