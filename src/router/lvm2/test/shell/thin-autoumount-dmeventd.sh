#!/usr/bin/env bash

# Copyright (C) 2012-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# no automatic extensions, just umount


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

mntdir="${PREFIX}mnt with space"
mntusedir="${PREFIX}mntuse"

cleanup_mounted_and_teardown()
{
	umount "$mntdir" 2>/dev/null || true
	umount "$mntusedir" 2>/dev/null || true
	vgremove -ff $vg
	aux teardown
}

is_lv_opened_()
{
	test "$(get lv_field "$1" lv_device_open --binary)" = 1
}

#
# Main
#
which mkfs.ext4 || skip
export MKE2FS_CONFIG="$TESTDIR/lib/mke2fs.conf"

aux have_thin 1 0 0 || skip

# Simple implementation of umount when lvextend fails
cat <<- EOF >testcmd.sh
#!/bin/sh

echo "Data: \$DMEVENTD_THIN_POOL_DATA"
echo "Metadata: \$DMEVENTD_THIN_POOL_METADATA"

"$TESTDIR/lib/lvextend" --use-policies \$1 || {
	umount "$mntdir"  || true
	umount "$mntusedir" || true
	return 0
}
test "\$($TESTDIR/lib/lvs -o selected -S "data_percent>95||metadata_percent>95" --noheadings \$1)" -eq 0 || {
	umount "$mntdir"  || true
	umount "$mntusedir" || true
	return 0
}
EOF
chmod +x testcmd.sh
# Show prepared script
cat testcmd.sh

# Use autoextend percent 0 - so extension fails and triggers umount...
aux lvmconf "activation/thin_pool_autoextend_percent = 0" \
	    "activation/thin_pool_autoextend_threshold = 70" \
	    "dmeventd/thin_command = \"/$PWD/testcmd.sh\""

aux prepare_dmeventd

aux prepare_vg 2

lvcreate -L8M -V8M -n $lv1 -T $vg/pool
lvcreate -V8M -n $lv2 -T $vg/pool

mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv2"

lvchange --monitor y $vg/pool

mkdir "$mntdir" "$mntusedir"
trap 'cleanup_mounted_and_teardown' EXIT
mount "$DM_DEV_DIR/$vg/$lv1" "$mntdir"
mount "$DM_DEV_DIR/$vg/$lv2" "$mntusedir"

# Check both LVs are opened (~mounted)
is_lv_opened_ "$vg/$lv1"
is_lv_opened_ "$vg/$lv2"

touch "$mntusedir/file$$"
sync

# Running 'keeper' process sleep holds the block device still in use
sleep 60 < "$mntusedir/file$$" >/dev/null 2>&1 &
PID_SLEEP=$!

lvs -a $vg
# Fill pool above 95%  (to cause 'forced lazy umount)
dd if=/dev/zero of="$mntdir/file$$" bs=256K count=20 conv=fdatasync

lvs -a $vg

# Could loop here for a few secs so dmeventd can do some work
# In the worst case check only happens every 10 seconds :(
# With low water mark it quickly discovers overflow and umounts $vg/$lv1
for i in $(seq 1 12) ; do
	is_lv_opened_ "$vg/$lv1" || break
	test $i -lt 12 || die "$mntdir should have been unmounted by dmeventd!"
	sleep 1
done

lvs -a $vg

is_lv_opened_ "$vg/$lv2" || \
	die "$mntusedir is not mounted here (sleep already expired??)"

# Kill device holding process
kill $PID_SLEEP
wait

not is_lv_opened_ "$vg/$lv2" || {
	mount
	die "$mntusedir should have been unmounted by dmeventd!"
}
