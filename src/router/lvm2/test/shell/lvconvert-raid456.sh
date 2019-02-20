#!/usr/bin/env bash

# Copyright (C) 2013-2014 Red Hat, Inc. All rights reserved.
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

get_image_pvs() {
	local d
	local images

	images=$(dmsetup ls | grep "${1}-${2}_.image_.*" | cut -f1 | sed -e s:-:/:)
	lvs --noheadings -a -o devices $images | sed s/\(.\)//
}

########################################################
# MAIN
########################################################
aux raid456_replace_works || skip
aux have_raid 1 3 0 || skip

aux prepare_vg 7  # 7 devices for 2 dev replacement of 5-dev RAID6

levels="5 6"
aux have_raid4 && levels="4 5 6"

# RAID 4/5/6 (can replace up to 'parity' devices)
for i in $levels; do
	lvcreate --type raid$i -i 3 -l 3 -n $lv1 $vg

	if [ $i -eq 6 ]; then
		dev_cnt=5
		limit=2
	else
		dev_cnt=4
		limit=1
	fi

	for j in {1..3}; do
	for o in $(seq 0 $i); do
		replace=""

		devices=( $(get_image_pvs $vg $lv1) )

		for k in $(seq $j); do
			index=$(( ( k + o ) % dev_cnt ))
			replace="$replace --replace ${devices[$index]}"
		done
		aux wait_for_sync $vg $lv1

		if [ $j -gt $limit ]; then
			not lvconvert $replace $vg/$lv1
		else
			lvconvert $replace $vg/$lv1
		fi
	done
	done

	lvremove -ff $vg
done

vgremove -ff $vg
