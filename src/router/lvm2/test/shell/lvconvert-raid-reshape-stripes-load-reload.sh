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

which md5sum || skip
which mkfs.ext4 || skip
aux have_raid 1 13 2 || skip

mount_dir="mnt"

cleanup_mounted_and_teardown()
{
	umount "$mount_dir" || true
	aux teardown
}

checksum_()
{
	md5sum "$1" | cut -f1 -d' '
}

aux prepare_pvs 16 32

get_devs

vgcreate $SHARED -s 1M "$vg" "${DEVICES[@]}"

trap 'cleanup_mounted_and_teardown' EXIT

# Create 10-way striped raid5 (11 legs total)
lvcreate --yes --type raid5_ls --stripesize 64K --stripes 10 -L64M -n$lv1 $vg
check lv_first_seg_field $vg/$lv1 segtype "raid5_ls"
check lv_first_seg_field $vg/$lv1 stripesize "64.00k"
check lv_first_seg_field $vg/$lv1 data_stripes 10
check lv_first_seg_field $vg/$lv1 stripes 11
echo y|mkfs -t ext4 /dev/$vg/$lv1

mkdir -p "$mount_dir"
mount "$DM_DEV_DIR/$vg/$lv1" "$mount_dir"

echo 3 >/proc/sys/vm/drop_caches
# FIXME: This is filling up ram disk. Use sane amount of data please! Rate limit the data written!
dd if=/dev/urandom of="$mount_dir/random" bs=1M count=50 conv=fdatasync
checksum_ "$mount_dir/random" >MD5

# FIXME: wait_for_sync - is this really testing anything under load?
aux wait_for_sync $vg $lv1
aux delay_dev "$dev2" 0 200

# Reshape it to 15 data stripes
lvconvert --yes --stripes 15 $vg/$lv1
check lv_first_seg_field $vg/$lv1 segtype "raid5_ls"
check lv_first_seg_field $vg/$lv1 stripesize "64.00k"
check lv_first_seg_field $vg/$lv1 data_stripes 15
check lv_first_seg_field $vg/$lv1 stripes 16

# Reload table during reshape to test for data corruption
for i in {0..5}
do
	dmsetup table $vg-$lv1|dmsetup load $vg-$lv1
	dmsetup suspend --noflush $vg-$lv1
	dmsetup resume $vg-$lv1
	sleep 0.3
done

aux delay_dev "$dev2" 0

kill -9 %% || true
wait

checksum_ "$mount_dir/random" >MD5_new

umount $mount_dir

fsck -fn "$DM_DEV_DIR/$vg/$lv1"

# Compare checksum is matching
cat MD5 MD5_new
diff MD5 MD5_new

vgremove -ff $vg
