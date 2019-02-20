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

#######################################################################
# This series of tests is meant to validate the correctness of
# 'dmsetup status' for RAID LVs - especially during various sync action
# transitions, like: recover, resync, check, repair, idle, reshape, etc
#######################################################################

SKIP_WITH_LVMPOLLD=1

. lib/inittest

# check for version 1.9.0
# - it is the point at which linear->raid1 uses "recover"
# check for version 1.13.0 instead
# - it is the point at which a finishing "recover" doesn't print all 'a's
aux have_raid 1 13 0 || skip



aux prepare_pvs 9
get_devs

vgcreate $SHARED -s 2m "$vg" "${DEVICES[@]}"

###########################################
# Upconverted RAID1 should never have all 'a's in status output
###########################################
aux delay_dev "$dev2" 0 50
lvcreate -aey -l 2 -n $lv1 $vg "$dev1"
lvconvert --type raid1 -y -m 1 $vg/$lv1 "$dev2"
while ! check in_sync $vg $lv1; do
        a=( $(dmsetup status $vg-$lv1) ) || die "Unable to get status of $vg/$lv1"
	b=( $(echo "${a[6]}" | sed s:/:' ':) )
	if [ "${b[0]}" -ne "${b[1]}" ]; then
		# First, 'check in_sync' should only need to check the ratio
		#  If we are here, it is probably doing more than that.
		# If not in-sync, then we should only ever see "Aa"
		[ "${a[5]}" == "Aa" ]
	else
		[ "${a[5]}" != "aa" ]
		should [ "${a[5]}" == "AA" ] # RHBZ 1507719
	fi
        sleep .1
done
aux enable_dev "$dev2"
lvremove -ff $vg

###########################################
# Upconverted RAID1 should not be at 100% right after upconvert
###########################################
aux delay_dev "$dev2" 0 50
lvcreate -aey -l 2 -n $lv1 $vg "$dev1"
lvconvert --type raid1 -y -m 1 $vg/$lv1 "$dev2"
a=( $(dmsetup status $vg-$lv1) ) || die "Unable to get status of $vg/$lv1"
b=( $(echo "${a[6]}" | sed s:/:' ':) )
should [ "${b[0]}" -ne "${b[1]}" ] # RHBZ 1507729
aux enable_dev "$dev2"
lvremove -ff $vg

###########################################
# Catch anything suspicious with linear -> RAID1 upconvert
###########################################
aux delay_dev "$dev2" 0 50
lvcreate -aey -l 2 -n $lv1 $vg "$dev1"
lvconvert --type raid1 -y -m 1 $vg/$lv1 "$dev2"
while true; do
        a=( $(dmsetup status $vg-$lv1) ) || die "Unable to get status of $vg/$lv1"
	b=( $(echo "${a[6]}" | sed s:/:' ':) )
	if [ "${b[0]}" -ne "${b[1]}" ]; then
		# If the sync operation ("recover" in this case) is not
		# finished, then it better be as follows:
		[ "${a[5]}" = "Aa" ]

		# Might be transitioning from "idle" to "recover".
		# Kernel could check mddev->recovery for the intent to
		# begin a "recover" and report that... probably would be
		# better.  RHBZ 1507719
		should [ "${a[7]}" = "recover" ]
	else
		# Tough to tell the INVALID case,
		#   Before starting sync thread: "Aa X/X recover"
		# from the valid case,
		#   Just finished sync thread: "Aa X/X recover"
		should [ "${a[5]}" = "AA" ] # RHBZ 1507719
		should [ "${a[7]}" = "idle" ] # RHBZ 1507719
		break
	fi
        sleep .1
done
aux enable_dev "$dev2"
lvremove -ff $vg

###########################################
# Catch anything suspicious with RAID1 2-way -> 3-way upconvert
###########################################
aux delay_dev "$dev3" 0 50
lvcreate --type raid1 -m 1 -aey -l 2 -n $lv1 $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv1
lvconvert -y -m +1 $vg/$lv1 "$dev3"
while true; do
        a=( $(dmsetup status $vg-$lv1) ) || die "Unable to get status of $vg/$lv1"
	b=( $(echo "${a[6]}" | sed s:/:' ':) )
	if [ "${b[0]}" -ne "${b[1]}" ]; then
		# If the sync operation ("recover" in this case) is not
		# finished, then it better be as follows:
		[ "${a[5]}" = "AAa" ]
		[ "${a[7]}" = "recover" ]
	else
		# Tough to tell the INVALID case,
		#   Before starting sync thread: "AAa X/X recover"
		# from the valid case,
		#   Just finished sync thread: "AAa X/X recover"
		should [ "${a[5]}" = "AAA" ] # RHBZ 1507719
		should [ "${a[7]}" = "idle" ] # RHBZ 1507719
		break
	fi
        sleep .1
done
aux enable_dev "$dev3"
lvremove -ff $vg

###########################################
# Catch anything suspicious with RAID1 initial resync
###########################################
aux delay_dev "$dev2" 0 50
lvcreate --type raid1 -m 1 -aey -l 2 -n $lv1 $vg "$dev1" "$dev2"
while true; do
        a=( $(dmsetup status $vg-$lv1) ) || die "Unable to get status of $vg/$lv1"
	b=( $(echo "${a[6]}" | sed s:/:' ':) )
	if [ "${b[0]}" -ne "${b[1]}" ]; then
		# If the sync operation ("resync" in this case) is not
		# finished, then it better be as follows:
		[ "${a[5]}" = "aa" ]

		# Should be in "resync", but it is possible things are only
		# just getting going - in which case, it could be "idle"
		# with 0% sync ratio
		[ "${a[7]}" = "resync" ] || \
		  [[ "${a[7]}" = "idle" && "${b[0]}" -eq "0" ]]
	else
		should [ "${a[5]}" = "AA" ] # RHBZ 1507719
		should [ "${a[7]}" = "idle" ] # RHBZ 1507719
		break
	fi
        sleep .1
done
aux enable_dev "$dev2"
lvremove -ff $vg

vgremove -ff $vg
