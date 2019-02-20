#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
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

aux have_thin 1 0 0 || skip

# Test --poolmetadatasize range
# allocating large devices for testing
aux prepare_pvs 10 16500
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

# Size 0 is not valid
invalid lvcreate -L4M --chunksize 128 --poolmetadatasize 0 -T $vg/pool1 2>out
lvcreate -Zn -L4M --chunksize 128 --poolmetadatasize 16k -T $vg/pool1 2>out
grep "WARNING: Minimum" out
# FIXME: metadata allocation fails, if PV doesn't have at least 16GB
# i.e. pool metadata device cannot be multisegment
lvcreate -Zn -L4M --chunksize 64k --poolmetadatasize 17G -T $vg/pool2 2>out
grep "WARNING: Maximum" out
check lv_field $vg/pool1_tmeta size "2.00m"
check lv_field $vg/pool2_tmeta size "15.81g"

# Check we do report correct percent values.
lvcreate --type zero -L3G $vg -n pool3
lvconvert -y --thinpool $vg/pool3
lvchange --errorwhenfull y $vg/pool3
lvchange --zero n $vg/pool3
lvcreate -V10G $vg/pool3 -n $lv1
lvcreate -V2G $vg/pool3 -n $lv2
dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=512b count=1 conv=fdatasync
# ...excercise write speed to 'zero' device ;)
dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv2" bs=64K count=32767 conv=fdatasync
lvs -a $vg
# Check the percentage is not shown as 0.00
check lv_field $vg/$lv1 data_percent "0.01"
# Check the percentage is not shown as 100.00
check lv_field $vg/$lv2 data_percent "99.99"


# Check can start and see thinpool with metadata size above kernel limit
lvcreate -L4M --poolmetadatasize 16G -T $vg/poolM
check lv_field $vg/poolM data_percent "0.00"

lvremove -ff $vg

# Test automatic calculation of pool metadata size
lvcreate -L160G -T $vg/pool
check lv_field $vg/pool lv_metadata_size "80.00m"
check lv_field $vg/pool chunksize        "128.00k"
lvremove -ff $vg/pool

lvcreate -L10G --chunksize 256 -T $vg/pool1
lvcreate -L60G --chunksize 1024 -T $vg/pool2
check lv_field $vg/pool1_tmeta size "2.50m"
check lv_field $vg/pool2_tmeta size "3.75m"
lvremove -ff $vg

# Block size of multiple 64KB needs >= 1.4
if aux have_thin 1 4 0 ; then
# Test chunk size is rounded to 64KB boundary
lvcreate -L10G --poolmetadatasize 4M -T $vg/pool
check lv_field $vg/pool chunk_size "192.00k"
fi
# Old thinpool target required rounding to power of 2
aux lvmconf "global/thin_disabled_features = [ \"block_size\" ]"
lvcreate -L10G --poolmetadatasize 4M -T $vg/pool_old
check lv_field $vg/pool_old chunk_size "256.00k"
lvremove -ff $vg
# reset
#aux lvmconf "global/thin_disabled_features = []"

vgremove -ff $vg
