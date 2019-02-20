#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test foreing user of thin-pool


SKIP_WITH_LVMPOLLD=1

. lib/inittest

MOUNT_DIR=mnt

cleanup_mounted_and_teardown()
{
	umount "$MOUNT_DIR" || true
	dmsetup remove $THIN
	vgremove -ff $vg
	aux teardown
}

percent_() {
	get lv_field $vg/pool data_percent | cut -d. -f1
}

#
# Main
#
aux have_thin 1 0 0 || skip
which mkfs.ext4 || skip

# Use our mkfs config file to get approximately same results
# TODO: maybe use it for all test via some 'prepare' function
export MKE2FS_CONFIG="$TESTOLDPWD/lib/mke2fs.conf"

aux prepare_dmeventd
aux prepare_vg 2 64

# Create named pool only
lvcreate --errorwhenfull y -L2 -T $vg/pool

POOL="$vg-pool"
THIN="${PREFIX}_thin"

# Foreing user is using own ioctl command to create thin devices
dmsetup message $POOL 0 "create_thin 0"
dmsetup message $POOL 0 "set_transaction_id 0 1"

dmsetup status
# Once the transaction id has changed, lvm2 shall not be able to create thinLV
fail lvcreate -V10 $vg/pool

trap 'cleanup_mounted_and_teardown' EXIT

# 20M thin device
dmsetup create $THIN --table "0 40960 thin $DM_DEV_DIR/mapper/$POOL 0"

dmsetup table
dmsetup info -c

mkdir "$MOUNT_DIR"
# This mkfs should fill 2MB pool over 95%
# no autoresize is configured
mkfs.ext4 "$DM_DEV_DIR/mapper/$THIN"
test "$(percent_)" -gt 95
mount "$DM_DEV_DIR/mapper/$THIN" "$MOUNT_DIR"

pvchange -x n "$dev1" "$dev2"

test "$(percent_)" -gt 95
# Configure autoresize
aux lvmconf 'activation/thin_pool_autoextend_percent = 10' \
	    'activation/thin_pool_autoextend_threshold = 75'

# Give it some time to left dmeventd do some (failing to resize) work
sleep 20

# And check foreign thin device is still mounted
mount | grep "$MOUNT_DIR" | grep "$THIN"
test "$(percent_)" -gt 95

pvchange -x y "$dev1" "$dev2"

# FIXME: ATM tell dmeventd explicitely we've changed metadata
#        however dmeventd shall be aware of any metadata change
#        and automagically retry resize operation after that.
lvchange --refresh $vg/pool

# Give it some time and let dmeventd do some work
for i in $(seq 1 15) ; do
	test "$(percent_)" -ge 75 || break
	sleep 1
done

test "$(percent_)" -lt 75

# And check foreign thin device is still mounted
mount | grep "$MOUNT_DIR" | grep "$THIN"
