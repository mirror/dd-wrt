#!/usr/bin/env bash
# Copyright (C) 2011-2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# get.sh: get various values from volumes
#
# USAGE:
#  get pv_field PV field [pvs params]
#  get vg_field VG field [vgs params]
#  get lv_field LV field [lvs params]
#
#  get lv_devices LV     [lvs params]

test -z "$BASH" || set -e -o pipefail

# trims only leading prefix and suffix
trim_() {
	rm -f debug.log             # drop log, command was ok
	local var=${1%${1##*[! ]}}  # remove trailing space characters
	echo "${var#${var%%[! ]*}}" # remove leading space characters
}

pv_field() {
	local r
	r=$(pvs --config 'log{prefix=""}' --noheadings -o "$2" "${@:3}" "$1")
	trim_ "$r"
}

vg_field() {
	local r
	r=$(vgs --config 'log{prefix=""}' --noheadings -o "$2" "${@:3}" "$1")
	trim_ "$r"
}

lv_field() {
	local r
	r=$(lvs --config 'log{prefix=""}' --noheadings -o "$2" "${@:3}" "$1")
	trim_ "$r"
}

lv_first_seg_field() {
	local r
	r=$(head -1 < <(lvs --config 'log{prefix=""}' --unbuffered --noheadings -o "$2" "${@:3}" "$1"))
	trim_ "$r"
}

lvh_field() {
	local r
	r=$(lvs -H --config 'log{prefix=""}' --noheadings -o "$2" "${@:3}" "$1")
	trim_ "$r"
}

lva_field() {
	local r
	r=$(lvs -a --config 'log{prefix=""}' --noheadings -o "$2" "${@:3}" "$1")
	trim_ "$r"
}

lv_devices() {
	lv_field "$1" devices -a "${@:2}" | sed 's/([^)]*)//g; s/,/\n/g'
}

lv_field_lv_() {
	lv_field "$1" "$2" -a --unbuffered | tr -d []
}

lv_tree_devices_() {
	local lv="$1/$2"
	local type
	type=$(lv_first_seg_field "$lv" segtype -a)
	#local orig
	#orig=$(lv_field_lv_ "$lv" origin)
	# FIXME: should we count in also origins ?
	#test -z "$orig" || lv_tree_devices_ $1 $orig
	case "$type" in
	linear|striped)
		lv_devices "$lv"
		;;
	mirror|raid*)
		local log
		log=$(lv_field_lv_ "$lv" mirror_log)
		test -z "$log" || lv_tree_devices_ "$1" "$log"
		for i in $(lv_devices "$lv")
			do lv_tree_devices_ "$1" "$i"; done
		;;
	thin)
		lv_tree_devices_ "$1" "$(lv_field_lv_ "$lv" pool_lv)"
		;;
	thin-pool)
		lv_tree_devices_ "$1" "$(lv_field_lv_ "$lv" data_lv)"
		lv_tree_devices_ "$1" "$(lv_field_lv_ "$lv" metadata_lv)"
		;;
	cache)
		lv_tree_devices_ "$1" "$(lv_devices "$lv")"
		;;
	cache-pool)
		lv_tree_devices_ "$1" "$(lv_field_lv_ "$lv" data_lv)"
		lv_tree_devices_ "$1" "$(lv_field_lv_ "$lv" metadata_lv)"
		;;
	esac
}

lv_tree_devices() {
	lv_tree_devices_ "$@" | sort | uniq
}

first_extent_sector() {
	pv_field "$@" pe_start --units s --nosuffix
}

#set -x
unset LVM_VALGRIND
unset LVM_LOG_FILE_EPOCH
"$@"
