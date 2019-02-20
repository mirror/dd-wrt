#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
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

aux have_raid 1 3 0 || skip

aux lvmconf 'activation/raid_fault_policy = "warn"'

aux prepare_vg 3 32
get_devs

# Create 2-legged RAID1 and wait for it to complete initial resync
lvcreate --type raid1 -m 1 -l 4 -n $lv $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv

# Disable first PV thus erroring first leg
aux disable_dev "$dev1"

# Reduce VG by missing PV
vgreduce --force --removemissing $vg
check raid_leg_status $vg $lv "DA"

# Conversion to 2 legs must fail on degraded 2-legged raid1 LV
not lvconvert -y -m1 $vg/$lv
check raid_leg_status $vg $lv "DA"

# Repair has to succeed
lvconvert -y --repair $vg/$lv
aux wait_for_sync $vg $lv
check raid_leg_status $vg $lv "AA"

lvremove -ff $vg/$lv

vgremove -ff $vg
