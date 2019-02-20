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


SKIP_WITH_LVMPOLLD=1

LVM_SKIP_LARGE_TESTS=0

. lib/inittest

which mkfs.ext4 || skip
aux have_raid 1 14 0 || skip

if [ $LVM_SKIP_LARGE_TESTS -eq 0 ]
then
	aux prepare_pvs 65 9
else
	aux prepare_pvs 20 9
fi

get_devs

vgcreate $SHARED -s 1M "$vg" "${DEVICES[@]}"

function _lvcreate
{
	local level=$1
	local req_stripes=$2
	local stripes=$3
	local size=$4
	local vg=$5
	local lv=$6

	lvcreate -y -aey --type $level -i $req_stripes -L $size -n $lv $vg
	check lv_first_seg_field $vg/$lv segtype "$level"
	check lv_first_seg_field $vg/$lv datastripes $req_stripes
	check lv_first_seg_field $vg/$lv stripes $stripes
	mkfs.ext4 "$DM_DEV_DIR/$vg/$lv"
	fsck -fn "$DM_DEV_DIR/$vg/$lv"
}

function _lvconvert
{
	local req_level=$1
	local level=$2
	local data_stripes=$3
	local stripes=$4
	local vg=$5
	local lv=$6
	local region_size=${7-}
	local wait_and_check=1
	local R=""

	[ -n "$region_size" ] && R="-R $region_size"
	[ "${level:0:7}" = "striped" ] && wait_and_check=0
	[ "${level:0:5}" = "raid0" ] && wait_and_check=0

	lvconvert -y --ty $req_level $R "$DM_DEV_DIR/$vg/$lv" || return $?

	check lv_first_seg_field $vg/$lv segtype "$level"
	check lv_first_seg_field $vg/$lv data_stripes $data_stripes
	check lv_first_seg_field $vg/$lv stripes $stripes
	[ -n "$region_size" ] && check lv_field $vg/$lv regionsize $region_size
	if [ "$wait_and_check" -eq 1 ]
	then
		fsck -fn "$DM_DEV_DIR/$vg/$lv"
		aux wait_for_sync $vg $lv
	fi
	fsck -fn "$DM_DEV_DIR/$vg/$lv"
}

function _reshape_layout
{
	local type=$1
	shift
	local data_stripes=$1
	shift
	local stripes=$1
	shift
	local vg=$1
	shift
	local lv=$1
	shift
	local opts="$*"
	local ignore_a_chars=0

	[[ "$opts" =~ "--stripes" ]] && ignore_a_chars=1

	lvconvert -y --ty $type $opts "$DM_DEV_DIR/$vg/$lv"
	check lv_first_seg_field $vg/$lv segtype "$type"
	check lv_first_seg_field $vg/$lv data_stripes $data_stripes
	check lv_first_seg_field $vg/$lv stripes $stripes
	aux wait_for_sync $vg $lv $ignore_a_chars
	fsck -fn "$DM_DEV_DIR/$vg/$lv"
}

# Delay leg so that rebuilding status characters
#  can be read before resync finished too quick.
# aux delay_dev "$dev1" 1

#
# Start out with raid5(_ls)
#

# Create 3-way striped raid5 (4 legs total)
# _lvcreate raid5_ls 3 4 16M $vg $lv1
_lvcreate raid5_ls 3 4 16M $vg $lv1
check lv_first_seg_field $vg/$lv1 segtype "raid5_ls"
aux wait_for_sync $vg $lv1

# Reshape it to 256K stripe size
_reshape_layout raid5_ls 3 4 $vg $lv1 --stripesize 256K
check lv_first_seg_field $vg/$lv1 stripesize "256.00k"

# Convert raid5(_n) -> striped testing raid5_ls gets rejected
not _lvconvert striped striped 3 3 $vg $lv1 512k
_reshape_layout raid5_n 3 4 $vg $lv1
_lvconvert striped striped 3 3 $vg $lv1

# Convert striped -> raid5_n
_lvconvert raid5_n raid5_n 3 4 $vg $lv1 "" 1

# Convert raid5_n -> raid5_ls
_reshape_layout raid5_ls 3 4 $vg $lv1

# Convert raid5_ls to 5 stripes
_reshape_layout raid5_ls 5 6 $vg $lv1 --stripes 5

# Convert raid5_ls back to 3 stripes
_reshape_layout raid5_ls 3 6 $vg $lv1 --stripes 3 --force
_reshape_layout raid5_ls 3 4 $vg $lv1 --stripes 3

# Convert raid5_ls to 7 stripes
_reshape_layout raid5_ls 7 8 $vg $lv1 --stripes 7

# Convert raid5_ls to 9 stripes
_reshape_layout raid5_ls 9 10 $vg $lv1 --stripes 9

# Convert raid5_ls to 14 stripes
_reshape_layout raid5_ls 14 15 $vg $lv1 --stripes 14

if [ $LVM_SKIP_LARGE_TESTS -eq 0 ]
then
	# Convert raid5_ls to 63 stripes
	_reshape_layout raid5_ls 63 64 $vg $lv1 --stripes 63

	# Convert raid5_ls back to 27 stripes
	_reshape_layout raid5_ls 27 64 $vg $lv1 --stripes 27 --force
	_reshape_layout raid5_ls 27 28 $vg $lv1 --stripes 27

	# Convert raid5_ls back to 4 stripes checking
	# conversion to striped/raid* gets rejected
	# with existing LVs to be removed afer reshape
	_reshape_layout raid5_ls 4 28 $vg $lv1 --stripes 4 --force
else
	# Convert raid5_ls back to 4 stripes checking
	# conversion to striped/raid* gets rejected
	# with existing LVs to be removed afer reshape
	_reshape_layout raid5_ls 4 15 $vg $lv1 --stripes 4 --force
fi

# No we got the data reshaped and the freed SubLVs still present
# -> check takeover request gets rejected
not lvconvert --yes --type striped "$DM_DEV_DIR/$vg/$lv1"
not lvconvert --yes --type raid0 "$DM_DEV_DIR/$vg/$lv1"
not lvconvert --yes --type "$DM_DEV_DIR/raid0_meta $vg/$lv1"
not lvconvert --yes --type "$DM_DEV_DIR/raid6 $vg/$lv1"
# Remove the freed SubLVs
_reshape_layout raid5_ls 4 5 $vg $lv1 --stripes 4

# Convert raid5_ls back to 3 stripes
_reshape_layout raid5_ls 3 5 $vg $lv1 --stripes 3 --force
_reshape_layout raid5_ls 3 4 $vg $lv1 --stripes 3

# Convert raid5_ls -> raid5_rs
_reshape_layout raid5_rs 3 4 $vg $lv1

# Convert raid5_rs -> raid5_la
_reshape_layout raid5_la 3 4 $vg $lv1

# Convert raid5_la -> raid5_ra
_reshape_layout raid5_ra 3 4 $vg $lv1

# Convert raid5_ra -> raid6_ra_6
_lvconvert raid6_ra_6 raid6_ra_6 3 5 $vg $lv1 "4.00m" 1

# Convert raid5_la -> raid6(_zr)
_reshape_layout raid6 3 5 $vg $lv1

# Convert raid6(_zr) -> raid6_nc
_reshape_layout raid6_nc 3 5 $vg $lv1

# Convert raid6(_nc) -> raid6_nr
_reshape_layout raid6_nr 3 5 $vg $lv1

# Convert raid6_nr) -> raid6_rs_6
_reshape_layout raid6_rs_6 3 5 $vg $lv1

# Convert raid6_rs_6 to 5 stripes
_reshape_layout raid6_rs_6 5 7 $vg $lv1 --stripes 5

# Convert raid6_rs_6 to 4 stripes
_reshape_layout raid6_rs_6 4 7 $vg $lv1 --stripes 4 --force
_reshape_layout raid6_rs_6 4 6 $vg $lv1 --stripes 4
check lv_first_seg_field $vg/$lv1 stripesize "256.00k"

# Convert raid6_rs_6 to raid6_n_6
_reshape_layout raid6_n_6 4 6 $vg $lv1

# Convert raid6_n_6 -> striped
_lvconvert striped striped 4 4 $vg $lv1
check lv_first_seg_field $vg/$lv1 stripesize "256.00k"

# Convert striped -> raid10(_near)
_lvconvert raid10 raid10 4 8 $vg $lv1

# Convert raid10 to 10 stripes and 64K stripesize
# FIXME: change once we support odd numbers of raid10 stripes
not _reshape_layout raid10 4 9 $vg $lv1 --stripes 9 --stripesize 64K
_reshape_layout raid10 10 20 $vg $lv1 --stripes 10 --stripesize 64K
check lv_first_seg_field $vg/$lv1 stripesize "64.00k"

# Convert raid6_n_6 -> striped
_lvconvert striped striped 10 10 $vg $lv1
check lv_first_seg_field $vg/$lv1 stripesize "64.00k"

vgremove -ff $vg
