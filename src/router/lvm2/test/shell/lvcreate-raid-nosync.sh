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

aux have_raid 1 7 0 || skip

segtypes="raid5"
aux have_raid4 && segtypes="raid4 $segtypes"

aux prepare_vg 6

_sync() {
	aux enable_dev "$dev1"

	aux wait_for_sync $vg $lv1
	test -z "$1" || check raid_leg_status $vg $lv1 $1
	lvremove --yes $vg/$lv1

	# restore to delay_dev tables for all devices
	aux restore_from_devtable "$dev1"
}


# Delay 1st leg so that rebuilding status characters
#  can be read before resync finished too quick.
aux delay_dev "$dev1" 0 100 "$(get first_extent_sector "$dev1")"

# raid0/raid0_meta don't support resynchronization
for r in raid0 raid0_meta
do
	lvcreate --type $r -Zn -i 3 -l 1 -n $lv1 $vg
	check raid_leg_status $vg $lv1 "AAA"
	lvremove --yes $vg/$lv1
done

# raid1 supports resynchronization
lvcreate --type raid1 -m 2 -Zn -l 4 -n $lv1 $vg
check raid_leg_status $vg $lv1 "aaa"
_sync "AAA"

# raid1 supports --nosync
lvcreate --type raid1 --nosync -Zn -m 2 -l 1 -n $lv1 $vg
check raid_leg_status $vg $lv1 "AAA"
lvremove --yes $vg/$lv1

for r in $segtypes
do
	# raid4/5 support resynchronization
	lvcreate --type $r -Zn -i 3 -L10 -n $lv1 $vg
	check raid_leg_status $vg $lv1 "aaaa"
	_sync "AAAA"

	# raid4/5 support --nosync
	lvcreate --type $r -Zn --nosync -i 3 -l 1 -n $lv2 $vg
	check raid_leg_status $vg $lv2 "AAAA"
	lvremove --yes $vg
done

# raid6 supports resynchronization
lvcreate --type raid6 -Zn -i 3 -l 4 -n $lv1 $vg
check raid_leg_status $vg $lv1 "aaaaa"
_sync "AAAAA"

# raid6 rejects --nosync; it has to initialize P- and Q-Syndromes
not lvcreate --type raid6 --nosync -Zn -i 3 -l 1 -n $lv1 $vg

# raid10 supports resynchronization
lvcreate --type raid10 -m 1 -Zn -i 3 -L10 -n $lv1 $vg
check raid_leg_status $vg $lv1 "aaaaaa"
_sync "AAAAAA"

# raid10 supports --nosync
lvcreate --type raid10 --nosync -m 1 -Zn -i 3 -l 1 -n $lv1 $vg
check raid_leg_status $vg $lv1 "AAAAAA"

vgremove -ff $vg
