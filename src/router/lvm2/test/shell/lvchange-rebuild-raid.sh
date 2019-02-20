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

aux have_raid 1 3 2 || skip
v1_9_0=0
aux have_raid 1 9 0 && v1_9_0=1

aux prepare_vg 8
get_devs

_sync() {
	aux enable_dev "${DEVICES[@]}"

	aux wait_for_sync $vg $lv1
	test "$#" -eq 0 || check raid_leg_status $vg $lv1 "$@"

	# restore to delay_dev tables for all devices
	aux restore_from_devtable "${DEVICES[@]}"
}

# Delay legs so that rebuilding status characters can be read
for d in "${DEVICES[@]}"
do
	aux delay_dev "$d" 0 50 "$(get first_extent_sector "$d")"
done

# rhbz 1064592

##############################################
# Create an 8-way striped raid10 with 4 mirror
# groups and rebuild selected PVs.
lvcreate --type raid10 -m 1 -i 4 -l 2 -n $lv1 $vg
_sync

# Rebuild 1st and 2nd device would rebuild a
# whole mirror group and needs to be rejected.
not lvchange --yes --rebuild "$dev1" --rebuild "$dev2" $vg/$lv1
not check raid_leg_status $vg $lv1 "aaAAAAA"
_sync "AAAAAAAA"

# Rebuild 1st and 3rd device from different mirror groups is fine.
lvchange --yes --rebuild "$dev1" --rebuild "$dev3" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "aAaAAAAA"
_sync "AAAAAAAA"

# Rebuild devices 1, 3, 6 from different mirror groups is fine.
lvchange --yes --rebuild "$dev1" --rebuild "$dev3" --rebuild "$dev6" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "aAaAAaAA"
_sync "AAAAAAAA"

# Rebuild devices 1, 3, 5 and 6 with 5+6 being
# being a whole mirror group needs to be rejected.
not lvchange --yes --rebuild "$dev1" --rebuild "$dev3" --rebuild "$dev6" --rebuild "$dev5" $vg/$lv1
not check raid_leg_status $vg $lv1 "aAaAaaAA"
_sync "AAAAAAAA"

# Rebuild devices 1, 3, 5 and 7 from different mirror groups is fine.
lvchange --yes --rebuild "$dev1" --rebuild "$dev3" --rebuild "$dev5" --rebuild "$dev7" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "aAaAaAaA"
_sync

# Rebuild devices 2, 4, 6 and 8 from different mirror groups is fine.
lvchange --yes --rebuild "$dev2" --rebuild "$dev4" --rebuild "$dev6" --rebuild "$dev8" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "AaAaAaAa"
_sync "AAAAAAAA"

##############################################
# Create an 8-legged raid1 and rebuild selected PVs
lvremove --yes $vg/$lv1
lvcreate --yes --type raid1 -m 7 -l 2 -n $lv1 $vg
_sync "AAAAAAAA"

# Rebuilding all raid1 legs needs to be rejected.
not lvchange --yes --rebuild "$dev1" --rebuild "$dev2" --rebuild "$dev3" --rebuild "$dev4" \
		   --rebuild "$dev5" --rebuild "$dev6" --rebuild "$dev7" --rebuild "$dev8" $vg/$lv1
not check raid_leg_status $vg $lv1 "aaaaaaaa"
_sync "AAAAAAAA"

# Rebuilding all but the raid1 master leg is fine.
lvchange --yes --rebuild "$dev2" --rebuild "$dev3" --rebuild "$dev4" \
	       --rebuild "$dev5" --rebuild "$dev6" --rebuild "$dev7" --rebuild "$dev8" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "Aaaaaaaa"
_sync "AAAAAAAA"

# Rebuilding the raid1 master leg is fine.
lvchange --yes --rebuild "$dev1" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "aAAAAAAA"
_sync "AAAAAAAA"

# Rebuild legs on devices 2, 4, 6 and 8 is fine.
lvchange --yes --rebuild "$dev2" --rebuild "$dev4" --rebuild "$dev6" --rebuild "$dev8" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "AaAaAaAa"
_sync "AAAAAAAA"

##############################################
# Create an 6-legged raid6 and rebuild selected PVs
lvremove --yes $vg/$lv1
lvcreate --yes --type raid6 -i 4 -l 2 -n $lv1 $vg
_sync "AAAAAA"

# Rebuilding all raid6 stripes needs to be rejected.
not lvchange --yes --rebuild "$dev1" --rebuild "$dev2" --rebuild "$dev3" \
		   --rebuild "$dev4" --rebuild "$dev5" --rebuild "$dev6"  $vg/$lv1
not check raid_leg_status $vg $lv1 "aaaaaa"
_sync "AAAAAA"

# Rebuilding more than 2 raid6 stripes needs to be rejected.
not lvchange --yes --rebuild "$dev2" --rebuild "$dev4" --rebuild "$dev6" $vg/$lv1
not check raid_leg_status $vg $lv1 "AaAaAa"
_sync "AAAAAA"

# Rebuilding any 1 raid6 stripe is fine.
lvchange --yes --rebuild "$dev2" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "AaAAAA"
_sync

lvchange --yes --rebuild "$dev5"  $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "AAAAaA"
_sync "AAAAAA"

# Rebuilding any 2 raid6 stripes is fine.
lvchange --yes --rebuild "$dev2" --rebuild "$dev4" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "AaAaAA"
_sync "AAAAAA"

lvchange --yes --rebuild "$dev1" --rebuild "$dev5" $vg/$lv1
[ $v1_9_0 -eq 1 ] && check raid_leg_status $vg $lv1 "aAAAaA"
_sync "AAAAAA"

vgremove -ff $vg
