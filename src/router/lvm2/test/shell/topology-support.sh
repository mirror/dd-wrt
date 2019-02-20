#!/usr/bin/env bash

# Copyright (C) 2010-2015 Red Hat, Inc. All rights reserved.
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

which mkfs.ext3 || skip

lvdev_() {
    echo "$DM_DEV_DIR/$1/$2"
}

test_snapshot_mount() {
    lvcreate -aey -L4M -n $lv1 $vg "$dev1"
    mkfs.ext3 -b4096 "$(lvdev_ $vg $lv1)"
    mkdir test_mnt
    mount "$(lvdev_ $vg $lv1)" test_mnt
    lvcreate -L4M -n $lv2 -s $vg/$lv1
    umount test_mnt
    aux udev_wait
    # mount the origin
    mount "$(lvdev_ $vg $lv1)" test_mnt
    umount test_mnt
    aux udev_wait
    # mount the snapshot
    mount "$(lvdev_ $vg $lv2)" test_mnt
    umount test_mnt
    rm -r test_mnt
    vgchange -an $vg
    lvremove -f $vg/$lv1
}

# FIXME add more topology-specific tests and validation (striped LVs, etc)

NUM_DEVS=1
PER_DEV_SIZE=34
DEV_SIZE=$(( NUM_DEVS * PER_DEV_SIZE ))

# ---------------------------------------------
# Create "desktop-class" 4K drive
# (logical_block_size=512, physical_block_size=4096, alignment_offset=0):
LOGICAL_BLOCK_SIZE=512
aux prepare_scsi_debug_dev $DEV_SIZE \
    sector_size=$LOGICAL_BLOCK_SIZE physblk_exp=3
# Test that kernel supports topology
if [ ! -e "/sys/block/$(basename "$(< SCSI_DEBUG_DEV)")/alignment_offset" ] ; then
	aux cleanup_scsi_debug_dev
	skip
fi
check sysfs "$(< SCSI_DEBUG_DEV)" queue/logical_block_size "$LOGICAL_BLOCK_SIZE"
aux prepare_pvs $NUM_DEVS $PER_DEV_SIZE
get_devs

vgcreate $SHARED $vg "${DEVICES[@]}"
test_snapshot_mount
vgremove $vg

aux cleanup_scsi_debug_dev

# ---------------------------------------------
# Create "desktop-class" 4K drive w/ 63-sector DOS partition compensation
# (logical_block_size=512, physical_block_size=4096, alignment_offset=3584):
LOGICAL_BLOCK_SIZE=512
aux prepare_scsi_debug_dev $DEV_SIZE \
    sector_size=$LOGICAL_BLOCK_SIZE physblk_exp=3 lowest_aligned=7
check sysfs "$(< SCSI_DEBUG_DEV)" queue/logical_block_size $LOGICAL_BLOCK_SIZE

aux prepare_pvs $NUM_DEVS $PER_DEV_SIZE
vgcreate $SHARED $vg "${DEVICES[@]}"
test_snapshot_mount
vgremove $vg

aux cleanup_scsi_debug_dev

# ---------------------------------------------
# Create "enterprise-class" 4K drive
# (logical_block_size=4096, physical_block_size=4096, alignment_offset=0):
LOGICAL_BLOCK_SIZE=4096
aux prepare_scsi_debug_dev $DEV_SIZE \
    sector_size=$LOGICAL_BLOCK_SIZE
check sysfs "$(< SCSI_DEBUG_DEV)" queue/logical_block_size $LOGICAL_BLOCK_SIZE

aux prepare_pvs $NUM_DEVS $PER_DEV_SIZE
vgcreate $SHARED $vg "${DEVICES[@]}"
test_snapshot_mount
vgremove $vg

aux cleanup_scsi_debug_dev

# scsi_debug option opt_blks appeared in Oct 2010
aux kernel_at_least 2 6 37 || exit 0

# ---------------------------------------------
# Create "enterprise-class" 512 drive w/ HW raid stripe_size = 768K
# (logical_block_size=512, physical_block_size=512, alignment_offset=0):
# - tests case where optimal_io_size=768k < default PE alignment=1MB
LOGICAL_BLOCK_SIZE=512
aux prepare_scsi_debug_dev $DEV_SIZE \
    sector_size=$LOGICAL_BLOCK_SIZE opt_blks=1536

check sysfs "$(< SCSI_DEBUG_DEV)" queue/logical_block_size $LOGICAL_BLOCK_SIZE
check sysfs "$(< SCSI_DEBUG_DEV)" queue/optimal_io_size 786432

aux prepare_pvs 1 $PER_DEV_SIZE

# Kernel (3.19) could provide wrong results - in this case skip
# test with incorrect result - lvm2 can't figure out good values.
SHOULD=""
check sysfs "$dev1" queue/optimal_io_size 786432 || SHOULD=should
$SHOULD check pv_field "${DEVICES[@]}" pe_start 768.00k

aux cleanup_scsi_debug_dev
