#!/usr/bin/env bash

# Copyright (C) 2014-2015 Red Hat, Inc. All rights reserved.
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

aux prepare_pvs 6 16

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# MAKE SURE ALL PV, VG AND LV NAMES CREATED IN
# THIS TEST ARE UNIQUE - THIS SIMPLIFIES TESTING
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# create $VGS with assorted tags
vgcreate $SHARED $vg1 --vgmetadatacopies 2 --addtag "vg_tag3" --addtag "vg_tag2" -s 4m "$dev1" "$dev2" "$dev3"
vgcreate $SHARED $vg2 --addtag "vg_tag2" -s 4m "$dev4" "$dev5"
vgcreate $SHARED $vg3 --addtag "vg_tag1" -s 4m "$dev6"

# add PV assorted tags
pvchange --addtag "pv_tag3" --addtag "pv_tag1" --addtag "pv_tag2" "$dev1"
pvchange --addtag "pv_tag1" --addtag "pv_tag4" "$dev6"

# create $LVS with assorted tags and various sizes
lvcreate --addtag 'lv_tag2.-+/=!:&#' --addtag "lv_tag1" -L8m -n "vol1" $vg1
lvcreate --addtag "lv_tag1" -L4m -n "vol2" $vg1
lvcreate --readahead 512 --addtag "lv_tag1" -L16m -n "abc" $vg2
lvcreate --readahead 512 -My --minor 254 -L4m -n "xyz" $vg3
lvcreate -L4m -aey -n "orig" $vg3
lvcreate -L4m -s "$vg3/orig" -n "snap"

OUT_LOG_FILE="out"
ERR_LOG_FILE="err"

sel() {
	local items_found

	${1}s --noheadings -o ${1}_name --select "$2" 2>"$ERR_LOG_FILE" | tee "$OUT_LOG_FILE"
        shift 2

	test -f "$OUT_LOG_FILE" || {
		echo "  >>> Missing log file to check!"
		return 1
	}

	# there shouldn't be any selection syntax error
	grep "Selection syntax error at" "$ERR_LOG_FILE" >/dev/null && {
		echo "  >>> Selection syntax error hit!"
		return 1
	}

	items_found=$(wc -l "$OUT_LOG_FILE" | cut -f 1 -d ' ')

	# the number of lines on output must match
	test "$items_found" -eq $# || {
		echo "  >>> NUMBER OF ITEMS EXPECTED: $#" "$@"
		echo "  >>> NUMBER OF ITEMS FOUND: $items_found ($(< $OUT_LOG_FILE))"
		return 1
	}

	# the names selected must be correct
	# each pv, vg and lv name is unique so just check
	# the presence of the names given as arg
	for name in "$@" ; do
		grep "$name" "$OUT_LOG_FILE" >/dev/null || {
			echo "  >>> $name not found in the output log"
			return 1
		}
	done

	rm -f "$OUT_LOG_FILE" "$ERR_LOG_FILE"
}

##########################
# STRING FIELD SELECTION #
##########################
#$LVS 'lv_name="vol1"' && result vol1
sel lv 'lv_name="vol1"' vol1
#$LVS 'lv_name!="vol1"' && result vol2 abc xyz
sel lv 'lv_name!="vol1"' vol2 abc xyz orig snap
# check string values are accepted without quotes too
sel lv 'lv_name=vol1' vol1
# check single quotes are also accepted instead of double quotes
sel lv "lv_name='vol1'" vol1

###############################
# STRING LIST FIELD SELECTION #
###############################
sel pv 'tags=["pv_tag1"]'
# for one item, no need to use []
sel pv 'tags="pv_tag1"' "$dev1" "$dev6"
# no match
sel pv 'tags=["pv_tag1" && "pv_tag2"]'
sel pv 'tags=["pv_tag1" && "pv_tag2" && "pv_tag3"]' "$dev1"
# check the order has no effect on selection result
sel pv 'tags=["pv_tag3" && "pv_tag2" && "pv_tag1"]' "$dev1"
sel pv 'tags=["pv_tag4" || "pv_tag3"]' "$dev1" "$dev6"
sel pv 'tags!=["pv_tag1"]' "$dev1" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6"
# check mixture of && and || - this is not allowed
not sel pv 'tags=["pv_tag1" && "pv_tag2" || "pv_tag3"]'
# check selection with blank value
sel lv 'tags=""' xyz orig snap
sel lv 'tags={}' xyz orig snap
sel lv 'tags=[]' xyz orig snap
# check subset selection
sel pv 'tags={"pv_tag1"}' "$dev1" "$dev6"
sel pv 'tags={"pv_tag1" && "pv_tag2"}' "$dev1"

##########################
# NUMBER FIELD SELECTION #
##########################
sel vg 'pv_count=3' $vg1
sel vg 'pv_count!=3' $vg3 $vg2
sel vg 'pv_count<2' $vg3
sel vg 'pv_count<=2' $vg3 $vg2
sel vg 'pv_count>2' $vg1
sel vg 'pv_count>=2' $vg1 $vg2

########################
# SIZE FIELD SELECTION #
########################
# check size units are accepted as well as floating point numbers for sizes
sel lv 'size=8388608b' vol1
sel lv 'size=8192k' vol1
sel lv 'size=8m' vol1
sel lv 'size=8.00m' vol1
sel lv 'size=0.0078125g' vol1
sel lv 'size=0.00000762939453125t' vol1
sel lv 'size=0.000000007450580596923828125p' vol1
sel lv 'size=0.0000000000072759576141834259033203125e' vol1

sel lv 'size>8m' abc
sel lv 'size>=8m' abc vol1
sel lv 'size<8m' vol2 xyz orig snap
sel lv 'size<=8m' vol2 xyz vol1 orig snap

###########################
# PERCENT FIELD SELECTION #
###########################
if aux target_at_least dm-snapshot 1 10 0; then
	# Test zero percent only if snapshot can be zero.
	# Before 1.10.0, the snap percent included metadata size.
	sel lv 'snap_percent=0' snap
fi
dd if=/dev/zero of="$DM_DEV_DIR/$vg3/snap" bs=1M count=1 conv=fdatasync
sel lv 'snap_percent<50' snap
sel lv 'snap_percent>50'
# overflow snapshot -> invalidated, but still showing 100%
not dd if=/dev/zero of="$DM_DEV_DIR/$vg3/snap" bs=1M count=4 conv=fdatasync
sel lv 'snap_percent=100' snap
# % char is accepted as suffix for percent values
sel lv 'snap_percent=100%' snap
# percent values over 100% are not accepted
not sel lv 'snap_percent=101%'

#########################
# REGEX FIELD SELECTION #
#########################
sel lv 'lv_name=~"^vol[12]"' vol1 vol2
sel lv 'lv_name!~"^vol[12]"' abc xyz orig snap
# check regex is accepted without quotes too
sel lv 'lv_name=~^vol[12]' vol1 vol2

###########
# GENERIC #
###########
# check prefix works for selection too
sel lv 'lv_name="vol1"' vol1
sel lv 'name="vol1"' vol1

# check reserved values are accepted for certain fields as well as usual values
sel vg 'vg_mda_copies=unmanaged' $vg2 $vg3
sel vg 'vg_mda_copies=2' $vg1
# also, we must match only vg1, not including vg2 and vg3
# when comparing ranges - unamanged is mapped onto 2^64 - 1 internally,
# so we need to skip this internal value if it matches with selection criteria!
sel vg 'vg_mda_copies>=2' $vg1
not sel vg 'vg_mda_copies=18446744073709551615'

sel lv 'lv_read_ahead=auto' vol1 vol2 orig snap
sel lv 'lv_read_ahead=256k' abc xyz

sel lv 'lv_minor=-1' vol1 vol2 abc orig snap
sel lv 'lv_minor=undefined' vol1 vol2 abc orig snap
sel lv 'lv_minor=undef' vol1 vol2 abc orig snap
sel lv 'lv_minor=unknown' vol1 vol2 abc orig snap
sel lv 'lv_minor=254' xyz
# also test synonym for string field type
sel lv 'seg_monitor=undefined' vol1 vol2 abc abc orig snap xyz

# if size unit not spefied, the 'm' (MiB) unit is used by default
sel lv 'lv_size=8' vol1

# no need to use quotes for the whole selection string if it does not clash with shell
sel lv name=vol1 vol1

##########################################
# FORMING MORE COMPLEX SELECTION CLAUSES #
##########################################
# AND clause
sel lv 'lv_tags=lv_tag1 && lv_size=4m' vol2
# OR clause
sel lv 'lv_name=vol1 || lv_name=vol2' vol1 vol2
# grouping by using ( )
sel lv '(lv_name=vol1 || lv_name=vol2) || vg_tags=vg_tag1' vol1 vol2 orig snap xyz
sel lv '(lv_name=vol1 && lv_size=100m) || vg_tags=vg_tag1' xyz orig snap
sel lv '(lv_name=vol1 || lv_name=vol2) && vg_tags=vg_tag1'
sel lv '(lv_name=vol1 || lv_name=vol2) && lv_size < 8m' vol2
sel lv '(lv_name=vol1 && lv_size=8m) && vg_tags=vg_tag2' vol1
# negation of clause grouped by ( )
sel lv '!(lv_name=vol1 || lv_name=vol2)' abc xyz orig snap

vgremove -ff $vg1 $vg2 $vg3
