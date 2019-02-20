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

# ensure there is no data loss during thin-pool resize


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which md5sum || skip

aux have_thin 1 0 0 || skip

aux prepare_vg 2 20

lvcreate -L1M -V2M -n $lv1 -T $vg/pool

# just ensure we check what we need to check
check lv_field $vg/pool size "1.00m"
check lv_field $vg/$lv1 size "2.00m"

# prepare 2097152  file content
seq 0 315465 > 2M
md5sum 2M | cut -f 1 -d ' ' | tee MD5
dd if=2M of="$DM_DEV_DIR/mapper/$vg-$lv1" bs=512K conv=fdatasync >log 2>&1 &
#dd if=2M of="$DM_DEV_DIR/mapper/$vg-$lv1" bs=2M oflag=direct &

# give it some time to fill thin-volume
# eventually loop to wait for 100% full pool...
sleep .1
lvs -a $vg

# this must not 'block & wait' on suspending flush
# if it waits on thin-pool's target timeout
# it will harm queued data
lvextend -L+512k $vg/pool
lvextend -L+512k $vg/pool

# collect 'dd' result
wait
cat log

lvs -a $vg

dd if="$DM_DEV_DIR/mapper/$vg-$lv1" of=2M-2 iflag=direct
md5sum 2M-2 | cut -f 1 -d ' '  | tee MD5-2

# these 2 are supposed to match
diff MD5  MD5-2


# Do not want to see Live & Inactive table entry
( dm_info attr,name | not grep "LI-.*${PREFIX}" ) || {
        dmsetup table --inactive | grep ${PREFIX}
	die "Found device with Inactive table"
}

# Check wrapping active thin-pool linear mapping has matching size
POOLSZ=$(dmsetup table ${vg}-pool-tpool | cut -d ' ' -f 2)
WRAPSZ=$(dmsetup table ${vg}-pool | cut -d ' ' -f 2)

#
# FIXME: currently requires to update 2 dependent targets in one 'preload'
#        lvm2 cannot handle this and would need one extra --refresh pass.
#        Once resolved - enabled this test.
#        Maybe other solution without fake linear mapping could be found.
#        Eventually strictly map just single sector as it has no real use?
#
#should test "${POOLSZ}" = "${WRAPSZ}" || \
#        die "Wrapping pool device size does not match real pool size"

vgremove -f $vg
