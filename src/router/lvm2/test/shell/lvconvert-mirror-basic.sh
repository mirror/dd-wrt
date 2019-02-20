#!/usr/bin/env bash

# Copyright (C) 2010-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA



. lib/inittest

log_name_to_count() {
	case "$1"  in
	mirrored) echo 2 ;;
	disk)	  echo 1 ;;
	*)	  echo 0 ;;
	esac
}

# FIXME: For test_[up|down]convert, I'd still like to be able
# to specifiy devices - especially if I can do partial PV
# specification for down-converts.  It may even be wise to
# do one round through these tests without specifying the PVs
# to use and one round where we do.

# test_lvconvert
#   start_mirror_count:  The '-m' argument to create with
#   start_log_type: core|disk|mirrored
#   final_mirror_count: The '-m' argument to convert to
#   final_log_type: core|disk|mirrored
#   active: Whether the LV should be active when the convert happens
#
# Exmaple: Convert 3-way disk-log mirror to
#          2-way disk-log mirror while not active
# -> test_lvconvert 2 disk 3 disk 0

test_lvconvert() {
	local start_count=$1
	local start_count_p1=$(( start_count + 1 ))
	local start_log_type=$2
	local finish_count=$3
	local finish_count_p1=$(( finish_count + 1 ))
	local finish_log_type=$4
	local start_log_count
	local finish_log_count
	local max_log_count
	local alloc=""
	local active="-aey"
	local i

	test "$5" = "active" && active="-an"
	#test $finish_count -gt $start_count && up=true

	# Do we have enough devices for the mirror images?
	test $start_count_p1 -gt ${#DEVICES[@]} && \
		die "Action requires too many devices"

	# Do we have enough devices for the mirror images?
	test $finish_count_p1 -gt ${#DEVICES[@]} && \
		die "Action requires too many devices"

	start_log_count=$(log_name_to_count $start_log_type)
	finish_log_count=$(log_name_to_count $finish_log_type)
	if [ $finish_log_count -gt $start_log_count ]; then
		max_log_count=$finish_log_count
	else
		max_log_count=$start_log_count
	fi

	if [ $start_count -gt 0 ]; then
		# Are there extra devices for the log or do we overlap
		if [ $(( start_count_p1 + start_log_count )) -gt ${#DEVICES[@]} ]; then
			alloc="--alloc anywhere"
		fi

		lvcreate "$active" -Zn -l2 --type mirror -m $start_count --mirrorlog $start_log_type \
			-n $lv1 $vg $alloc
		check mirror_legs $vg $lv1 $start_count_p1
		# FIXME: check mirror log
	else
		lvcreate "$active" -Zn -l2 -n $lv1 $vg
	fi

	lvs -a -o name,copy_percent,devices $vg

	# Are there extra devices for the log or do we overlap
	if [ $(( finish_count_p1 +  finish_log_count )) -gt ${#DEVICES[@]} ]; then
		alloc="--alloc anywhere"
	fi

	# --mirrorlog is invalid with -m0
	if [ "$finish_count" -eq 0 ]; then
		mirrorlog=""
		finish_log_type=""
	else
		mirrorlog="--mirrorlog"
	fi

	lvconvert --type mirror -m $finish_count $mirrorlog $finish_log_type \
		$vg/$lv1 $alloc

	test "$active" = "-an" || lvchange "$active" $vg/$lv1

	check mirror_no_temporaries $vg $lv1
	if [ "$finish_count_p1" -eq 1 ]; then
		check linear $vg $lv1
	else
		if test -n "$alloc"; then
			check mirror_nonredundant $vg $lv1
		else
			check mirror $vg $lv1
		fi
		check mirror_legs $vg $lv1 $finish_count_p1
	fi
}

aux prepare_vg 5 5
get_devs

test_many() {
	i=$1
	for j in $(seq 0 3); do
		for k in core disk; do
			for l in core disk; do
				if test "$i" -eq "$j" && test "$k" = "$l"; then continue; fi
				: ----------------------------------------------------
				: "Testing mirror conversion -m$i/$k -> -m$j/$l"
				: ----------------------------------------------------
				test_lvconvert $i $k $j $l 0
				lvremove -ff $vg
				test_lvconvert $i $k $j $l 1
				lvremove -ff $vg
			done
		done
	done
}
