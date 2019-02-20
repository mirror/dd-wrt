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

. lib/inittest

# Test reshaping under io load

# FIXME: This test requires 3GB in /dev/shm!
test $(aux total_mem) -gt $((4096*1024)) || skip

which mkfs.ext4 || skip
aux have_raid 1 13 2 || skip

mount_dir="mnt"

cleanup_mounted_and_teardown()
{
	umount "$mount_dir" || true
	aux teardown
}

aux prepare_pvs 16 32

get_devs

vgcreate $SHARED -s 1M "$vg" "${DEVICES[@]}"

trap 'cleanup_mounted_and_teardown' EXIT

# Create 13-way striped raid5 (14 legs total)
lvcreate --yes --type raid5_ls --stripes 13 -L190M -n$lv1 $vg
check lv_first_seg_field $vg/$lv1 segtype "raid5_ls"
check lv_first_seg_field $vg/$lv1 data_stripes 13
check lv_first_seg_field $vg/$lv1 stripes 14
echo y|mkfs -t ext4 /dev/$vg/$lv1
aux wait_for_sync $vg $lv1

mkdir -p $mount_dir
mount "$DM_DEV_DIR/$vg/$lv1" $mount_dir
mkdir -p $mount_dir/1 $mount_dir/2

aux delay_dev "$dev2" 0 100

echo 3 >/proc/sys/vm/drop_caches
cp -r /usr/bin $mount_dir/1 >/dev/null 2>/dev/null &
cp -r /usr/bin $mount_dir/2 >/dev/null 2>/dev/null &
sync &

# Reshape it to 256K stripe size
lvconvert --yes --stripesize 256 $vg/$lv1
aux delay_dev "$dev2" 0 0
check lv_first_seg_field $vg/$lv1 stripesize "256.00k"

kill -9 %%
wait

umount $mount_dir

fsck -fn "$DM_DEV_DIR/$vg/$lv1"

vgremove -ff $vg
