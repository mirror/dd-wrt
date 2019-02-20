#!/bin/sh
# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
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

aux have_raid 1 3 0 || skip

# Test for MD raid1 kernel bug causing read
# errors on failing first leg sectors.

# Create VG with 2 PVs
aux prepare_vg 2 2

# Create 2-legged raid1 LV
lvcreate --yes --type raid1 --mirrors 1 --extents 1 --name $lv $vg
aux wait_for_sync $vg $lv

aux error_dev "$dev1" 20:500

dd if=$DM_DEV_DIR/$vg/$lv iflag=direct,fullblock of=/dev/zero bs=128K count=1

vgremove --force $vg
