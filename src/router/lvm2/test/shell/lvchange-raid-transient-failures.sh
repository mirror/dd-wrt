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

aux have_raid 1 10 1 || skip
aux prepare_vg 6

#
# FIXME: add multi-segment leg tests
#

function _check_raid
{
	local vg=$1
	shift
	local lv=$1
	shift
	local fail=$1
	shift
	local good=$1
	shift
	local devs=( "$@" )

	aux wait_for_sync $vg $lv
	aux disable_dev --error --silent "${devs[@]}"
	mkfs.ext4 "$DM_DEV_DIR/$vg/$lv"
	fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv"
	check raid_leg_status $vg $lv "$fail"
	aux enable_dev --silent "${devs[@]}"
	lvs -a -o +devices $vg | tee out
	not grep unknown out
	lvchange --refresh $vg/$lv
	fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv"
	aux wait_for_sync $vg $lv
	fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv"
	check raid_leg_status $vg $lv "$good"
}

# raid1 with transiently failing devices
lv=4way
lvcreate -aey --type raid1 -m 3 --ignoremonitoring -L 1 -n $lv $vg
_check_raid $vg $lv "ADAD" "AAAA" "$dev2" "$dev4"
lvremove -y $vg/$lv

# raid6 with transiently failing devices
lv=6way
lvcreate -aey --type raid6 -i 4 --ignoremonitoring -L 1 -n $lv $vg
_check_raid $vg $lv "ADADAA" "AAAAAA" "$dev2" "$dev4"
lvremove -y $vg/$lv

# raid10 with transiently failing devices
lv=6way
lvcreate -aey --type raid10 -i 3 -m 1 --ignoremonitoring -L 1 -n $lv $vg
_check_raid $vg $lv "ADADDA" "AAAAAA" "$dev2" "$dev4" "$dev5"
lvremove -y $vg/$lv

vgremove -f $vg
