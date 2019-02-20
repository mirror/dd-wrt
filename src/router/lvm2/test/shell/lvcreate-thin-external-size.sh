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

# Test unaligned size of external origin and thin pool chunk size


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which cmp || skip

#
# Main
#

# Test needs thin-pool target with unaligned ext-orig size support
aux have_thin 1 13 0 || skip

aux prepare_pvs 2 640
get_devs

# Use 8K extent size
vgcreate $SHARED -s 8K "$vg" "${DEVICES[@]}"

# Prepare some numeric pattern with ~64K size
seq -s ' ' -w 0 10922 > 64K

d1="$DM_DEV_DIR/$vg/$lv1"
d2="$DM_DEV_DIR/$vg/$lv2"

# Prepare external origin LV with size not being a multiple of thin pool chunk size
lvcreate -l47 -n $lv1 $vg

# Fill end with pattern
dd if=64K of="$d1" bs=8192 seek=45 count=2 conv=fdatasync

# Switch to read-only volume
lvchange -an $vg/$lv1
lvchange -pr $vg/$lv1

lvcreate -L2M -T $vg/pool -c 192K
lvcreate -s $vg/$lv1 --name $lv2 --thinpool $vg/pool

# Check the tail of $lv2 matches $lv1
dd if="$d2" of=16K bs=8192 skip=45 count=2
cmp -n 16384 -l 64K 16K

# Now extend and rewrite
lvextend -l+2 $vg/$lv2

dd if=64K of="$d2" bs=8192 seek=46 count=3 conv=fdatasync
dd if="$d2" of=24K bs=8192 skip=46 count=3 iflag=direct
cmp -n 24576 -l 64K 24K

# Consumes 2 192K chunks -> 66.67%
check lv_field $vg/$lv2 data_percent "66.67"

lvreduce -f -l-24 $vg/$lv2

dd if=64K of="$d2" bs=8192 seek=24 count=1 conv=fdatasync
dd if="$d2" of=8K bs=8192 skip=24 count=1 iflag=direct
cmp -n 8192 -l 64K 8K

# Check extension still works
lvextend -l+2 $vg/$lv2

lvremove -f $vg/pool

lvcreate -L256M -T $vg/pool -c 64M
lvcreate -s $vg/$lv1 --name $lv2 --thinpool $vg/pool
lvextend -l+2 $vg/$lv2

dd if=64K of="$d2" bs=8192 seek=45 count=4 conv=fdatasync
dd if="$d2" of=32K bs=8192 skip=45 count=4 iflag=direct
cmp -n 32768 -l 64K 32K

lvextend -L+64M $vg/$lv2

# Consumes 64M chunk -> 50%
check lv_field $vg/$lv2 data_percent "50.00"

vgremove -ff $vg
