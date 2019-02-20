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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

MOUNT_DIR=mnt
MKFS=$(which mkfs.ext3) || skip

cleanup_mounted_and_teardown()
{
	umount "$MOUNT_DIR" || true
	aux teardown
}

aux lvmconf 'allocation/mirror_logs_require_separate_pvs = 1'

aux prepare_vg 5

################### Check lost mirror leg #################
#
# NOTE: using  --regionsize 1M  has  major impact on my box
# on read performance while mirror is synchronized
# with the default 512K - my C2D T61 reads just couple MB/s!
#
lvcreate -aey --type mirror -L10 --regionsize 1M -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3"
"$MKFS" "$DM_DEV_DIR/$vg/$lv1"
mkdir "$MOUNT_DIR"

aux delay_dev "$dev2" 0 500 "$(get first_extent_sector "$dev2"):"
aux delay_dev "$dev4" 0 500 "$(get first_extent_sector "$dev4"):"
#
# Enforce syncronization
# ATM requires unmounted/unused LV??
#
lvchange --yes --resync $vg/$lv1
trap 'cleanup_mounted_and_teardown' EXIT
mount "$DM_DEV_DIR/$vg/$lv1" "$MOUNT_DIR"

# run 'dd' operation during failure of 'mlog/mimage' device

dd if=/dev/zero of=mnt/zero bs=4K count=100 conv=fdatasync 2>err &

PERCENT=$(get lv_field $vg/$lv1 copy_percent)
PERCENT=${PERCENT%%\.*}  # cut decimal
# and check less than 50% mirror is in sync (could be unusable delay_dev ?)
test "$PERCENT" -lt 50 || skip
#lvs -a -o+devices $vg

#aux disable_dev "$dev3"
aux disable_dev "$dev2"

lvconvert --yes --repair $vg/$lv1
lvs -a $vg

aux enable_dev "$dev2"

wait
# dd MAY NOT HAVE produced any error message
not grep error err

lvs -a -o+devices $vg
umount "$MOUNT_DIR"
fsck -n "$DM_DEV_DIR/$vg/$lv1"

aux enable_dev "$dev4"
lvremove -ff $vg
