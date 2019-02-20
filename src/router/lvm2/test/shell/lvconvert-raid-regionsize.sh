#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
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

which mkfs.ext4 || skip
aux have_raid 1 9 0 || skip

aux prepare_vg 6

function _test_regionsize
{
	local type=$1
	local regionsize=$2
	local regionsize_str=$3
	local vg=$4
	local lv=$5

	lvconvert --type "$type" --yes -R "$regionsize" "$vg/$lv"
	check lv_field $vg/$lv regionsize "$regionsize_str"

	not lvconvert --regionsize "$regionsize" "$vg/$lv" 2>err
	grep "is already" err

	fsck -fn "$DM_DEV_DIR/$vg/$lv"
}

function _test_regionsizes
{
	# FIXME: have to provide raid type or region size ain't set until cli validation merged
	local type=$1

	# Test RAID regionsize changes
	_test_regionsize "$type" 128K "128.00k" $vg $lv1
	_test_regionsize "$type" 256K "256.00k" $vg $lv1
	not _test_regionsize "$type" 1K "1.00k" $vg $lv1
	_test_regionsize "$type" 1m "1.00m" $vg $lv1
	not _test_regionsize "$type" 1G "1.00g" $vg $lv1
	not _test_regionsize "$type" 16K "16.00k" $vg $lv1
}

# Create 3-way raid1
lvcreate --yes -aey --type raid1 -m 2 -R64K -L8M -n $lv1 $vg
check lv_field $vg/$lv1 segtype "raid1"
check lv_field $vg/$lv1 stripes 3
check lv_field $vg/$lv1 regionsize "64.00k"
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
aux wait_for_sync $vg $lv1
fsck -fn "$DM_DEV_DIR/$vg/$lv1"

_test_regionsizes raid1

# Clean up
lvremove --yes $vg

# Needs reshaping kernel for raid6 conversion
if aux have_raid 1 14 0; then
# Create 5-way raid6
lvcreate --yes -aey --type raid6 -i 3 --stripesize 128K -R 256K -L8M -n $lv1 $vg
check lv_field $vg/$lv1 segtype "raid6"
check lv_field $vg/$lv1 stripes 5
check lv_field $vg/$lv1 stripesize "128.00k"
check lv_field $vg/$lv1 regionsize "256.00k"
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
aux wait_for_sync $vg $lv1
fsck -fn "$DM_DEV_DIR/$vg/$lv1"

_test_regionsizes raid6

# Clean up
lvremove --yes $vg
else
  echo "Skipping RAID6 tests"
fi

if aux have_raid 1 10 1; then
# Create 6-way raid01
lvcreate --yes -aey --type raid10 -i 3 -m 1 --stripesize 128K -R 256K -L8M -n $lv1 $vg
check lv_field $vg/$lv1 segtype "raid10"
check lv_field $vg/$lv1 stripes 6
check lv_field $vg/$lv1 stripesize "128.00k"
check lv_field $vg/$lv1 regionsize "256.00k"
mkfs.ext4 -t ext4 "$DM_DEV_DIR/$vg/$lv1"
aux wait_for_sync $vg $lv1
fsck -fn "$DM_DEV_DIR/$vg/$lv1"

_test_regionsizes raid10
else
  echo "Skipping RAID10 tests"
fi

vgremove -ff $vg
