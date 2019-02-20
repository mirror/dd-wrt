#!/usr/bin/env bash

# Copyright (C) 2011-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test currently needs to drop
# 'return NULL' in _lv_create_an_lv after log_error("Can't create %s without using "


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

check_lv_field_modules_()
{
	mod=$1
	shift

	for d in "$@"; do
		check lv_field $vg/$d modules $mod
	done
}

#
# Main
#
aux have_thin 1 0 0 || skip
which mkfs.ext4 || skip

aux prepare_pvs 2 64
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

# Create named pool only
lvcreate -l1 -T $vg/pool1
lvcreate -l1 -T --thinpool $vg/pool2
lvcreate -l1 -T --thinpool pool3 $vg
invalid lvcreate -l1 --type thin $vg/pool4
invalid lvcreate -l1 --type thin --thinpool $vg/pool5
invalid lvcreate -l1 --type thin --thinpool pool6 $vg
lvcreate -l1 --type thin-pool $vg/pool7
lvcreate -l1 --type thin-pool --thinpool $vg/pool8
lvcreate -l1 --type thin-pool --thinpool pool9 $vg

lvremove -ff $vg/pool1 $vg/pool2 $vg/pool3 $vg/pool7 $vg/pool8 $vg/pool9
check vg_field $vg lv_count 0


# Let's pretend pool is like normal LV when using --type thin-pool support --name
# Reject ambiguous thin pool names
invalid lvcreate --type thin-pool -l1 --name pool1 $vg/pool2
invalid lvcreate --type thin-pool -l1 --name pool3 --thinpool pool4 $vg
invalid lvcreate --type thin-pool -l1 --name pool5 --thinpool pool6 $vg/pool7
invalid lvcreate --type thin-pool -l1 --name pool8 --thinpool pool8 $vg/pool9

# no size specified and no origin name give for snapshot
invalid lvcreate --thinpool pool $vg

check vg_field $vg lv_count 0

lvcreate --type thin-pool -l1 --name pool1 $vg
lvcreate --type thin-pool -l1 --name $vg/pool2
# If the thin pool name is unambiguous let it proceed
lvcreate --type thin-pool -l1 --name pool3 $vg/pool3
lvcreate --type thin-pool -l1 --name pool4 --thinpool $vg/pool4
lvcreate --type thin-pool -l1 --name pool5 --thinpool $vg/pool5 $vg/pool5

check lv_field $vg/pool1 segtype "thin-pool"
check lv_field $vg/pool2 segtype "thin-pool"
check lv_field $vg/pool3 segtype "thin-pool"
check lv_field $vg/pool4 segtype "thin-pool"
check lv_field $vg/pool5 segtype "thin-pool"

lvremove -ff $vg


# Create default pool name
lvcreate -l1 -T $vg
invalid lvcreate -l1 --type thin $vg
lvcreate -l1 --type thin-pool $vg

lvremove -ff $vg
check vg_field $vg lv_count 0


# Create default pool and default thin LV
lvcreate -l1 -V2G -T $vg
lvcreate -l1 -V2G --type thin $vg

lvremove -ff $vg


# Create named pool and default thin LV
lvcreate -L4M -V2G --name lvo1 -T $vg/pool1
lvcreate -L4M -V2G --name lvo2 -T --thinpool $vg/pool2
lvcreate -L4M -V2G --name lvo3 -T --thinpool pool3 $vg
lvcreate -L4M -V2G --name lvo4 --type thin $vg/pool4
lvcreate -L4M -V2G --name lvo5 --type thin --thinpool $vg/pool5
lvcreate -L4M -V2G --name lvo6 --type thin --thinpool pool6 $vg

check lv_exists $vg lvo1 lvo2 lvo3
lvremove -ff $vg


# Create named pool and named thin LV
lvcreate -L4M -V2G -T $vg/pool1 --name lv1
lvcreate -L4M -V2G -T $vg/pool2 --name $vg/lv2
lvcreate -L4M -V2G -T --thinpool $vg/pool3 --name lv3
lvcreate -L4M -V2G -T --thinpool $vg/pool4 --name $vg/lv4
lvcreate -L4M -V2G -T --thinpool pool5 --name lv5 $vg
lvcreate -L4M -V2G -T --thinpool pool6 --name $vg/lv6 $vg

check lv_exists $vg lv1 lv2 lv3 lv4 lv5 lv6
lvremove -ff $vg


lvcreate -L4M -V2G --type thin $vg/pool1 --name lv1
lvcreate -L4M -V2G --type thin $vg/pool2 --name $vg/lv2
lvcreate -L4M -V2G --type thin --thinpool $vg/pool3 --name lv3
lvcreate -L4M -V2G --type thin --thinpool $vg/pool4 --name $vg/lv4
lvcreate -L4M -V2G --type thin --thinpool pool5 --name lv5 $vg
lvcreate -L4M -V2G --type thin --thinpool pool6 --name $vg/lv6 $vg

check lv_exists $vg lv1 lv2 lv3 lv4 lv5 lv6
lvremove -ff $vg


# Create default thin LV in existing pool
lvcreate -L4M -T $vg/pool
lvcreate -V2G --name lvo0 -T $vg/pool
lvcreate -V2G --name lvo1 -T --thinpool $vg/pool
lvcreate -V2G --name lvo2 -T --thinpool pool $vg
lvcreate -V2G --name lvo3 --type thin $vg/pool
lvcreate -V2G --name lvo4 --type thin --thinpool $vg/pool
lvcreate -V2G --name lvo5 --type thin --thinpool pool $vg

check lv_exists $vg lvo0 lvo1 lvo2 lvo3 lvo4 lvo5


# Create named thin LV in existing pool
lvcreate -V2G -T $vg/pool --name lv1
lvcreate -V2G -T $vg/pool --name $vg/lv2
lvcreate -V2G -T --thinpool $vg/pool --name lv3
lvcreate -V2G -T --thinpool $vg/pool --name $vg/lv4
lvcreate -V2G -T --thinpool pool --name lv5 $vg
lvcreate -V2G -T --thinpool pool --name $vg/lv6 $vg
lvcreate -V2G --type thin $vg/pool --name lv7
lvcreate -V2G --type thin $vg/pool --name $vg/lv8
lvcreate -V2G --type thin --thinpool $vg/pool --name lv9
lvcreate -V2G --type thin --thinpool $vg/pool --name $vg/lv10
lvcreate -V2G --type thin --thinpool pool --name lv11 $vg
lvcreate -V2G --type thin --thinpool pool --name $vg/lv12 $vg

check lv_exists $vg lv1 lv2 lv3 lv4 lv5 lv6 lv7 lv8 lv9 lv10 lv11 lv12
check vg_field $vg lv_count 19
check lv_field $vg/lv1 thin_id 7

lvremove -ff $vg
check vg_field $vg lv_count 0

# Create thin snapshot of thinLV
lvcreate -L10M -I4 -i2 -V10M -T $vg/pool --name lv1
mkfs.ext4 "$DM_DEV_DIR/$vg/lv1"
lvcreate -K -s $vg/lv1 --name snap_lv1
fsck -n "$DM_DEV_DIR/$vg/snap_lv1"
lvcreate -s $vg/lv1 --name lv2
lvcreate -s $vg/lv1 --name $vg/lv3
invalid lvcreate --type snapshot $vg/lv1 --name lv6
invalid lvcreate --type snapshot $vg/lv1 --name lv4
invalid lvcreate --type snapshot $vg/lv1 --name $vg/lv5

lvdisplay --maps $vg
check_lv_field_modules_ thin,thin-pool lv1 snap_lv1 lv2 lv3
check vg_field $vg lv_count 5

lvremove -ff $vg


# Normal Snapshots of thinLV
lvcreate -L4M -V2G -T $vg/pool --name lv1
lvcreate -s $vg/lv1 -l1 --name snap_lv1
lvcreate -s $vg/lv1 -l1 --name lv2
lvcreate -s $vg/lv1 -l1 --name $vg/lv3
lvcreate -s lv1 -L4M --name $vg/lv4

check_lv_field_modules_ snapshot snap_lv1 lv2 lv3 lv4
check vg_field $vg lv_count 6

lvremove -ff $vg
check vg_field $vg lv_count 0


# Check how allocator works with 2PVs where one is nearly full
lvcreate -l99%PV $vg "$dev1"
lvs -a $vg
# Check when separate metadata is required, allocation needs to fail
fail lvcreate -L10 -T --poolmetadataspare n --config 'allocation/thin_pool_metadata_require_separate_pvs=1' $vg
# Check when data and metadata may share the same PV, it shall pass
lvcreate -L10 -T --poolmetadataspare n --config 'allocation/thin_pool_metadata_require_separate_pvs=0' $vg
lvremove -f $vg


# Fail cases
# Too small pool size (1 extent 64KB) for given chunk size
not lvcreate --chunksize 256 -l1 -T $vg/pool1
# Too small chunk size (min is 64KB -  128 sectors)
not lvcreate --chunksize 32 -l1 -T $vg/pool1
# Too large chunk size (max is 1GB)
not lvcreate -L4M --chunksize 2G -T $vg/pool1
# Cannot specify --minor with pool
fail lvcreate -L10M --minor 100 -T $vg/pool_minor

# FIXME: Currently ambigous - is it for thin, thin-pool, both ?
fail lvcreate -L4M -Mn -m0 -T --readahead 32 -V20 -n $lv $vg/pool_normal

# Check read-ahead setting will also pass with -Mn -m0
lvcreate -L4M -Mn -m0 -T --readahead 64k $vg/pool_readahead
lvcreate -V20M -Mn -m0 -T --readahead 128k -n thin_readahead $vg/pool_readahead
check lv_field $vg/pool_readahead lv_read_ahead "64.00k"
check lv_field $vg/thin_readahead lv_read_ahead "128.00k"

if test ! -d /sys/block/dm-2345; then
# Check some unused minor and support for --minor with thins
	lvcreate --minor 2345 -T -V20M -n thin_minor $vg/pool_readahead
	check lv_field $vg/thin_minor lv_minor "2345"
fi

# Test creation of inactive pool
lvcreate -an -L4M -T $vg/pool1
lvcreate -V2G --name lv1 -T $vg/pool1
# Check we are able remove spare volume if we want to
lvremove -f $vg/lvol0_pmspare

# Origin name is not accepted
not lvcreate -s $vg/lv1 -L4M -V2G --name $vg/lv4

# Check we cannot create mirror/raid1 and thin or thinpool together
not lvcreate -T mirpool -L4M --alloc anywhere -m1 $vg
not lvcreate --thinpool mirpool -L4M --alloc anywhere -m1 $vg

vgremove -ff $vg
