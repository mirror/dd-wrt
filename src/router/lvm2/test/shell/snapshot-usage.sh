#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# no automatic extensions please

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

MKFS=mkfs.ext2
which $MKFS || skip

fill() {
	dd if=/dev/zero of="$DM_DEV_DIR/${2:-$vg1/lvol0}" bs=$1 count=1 oflag=direct || \
		die "Snapshot does not fit $1"
}

cleanup_tail()
{
	test -z "${SLEEP_PID-}" || kill $SLEEP_PID || true
	wait
	vgremove -ff $vg1 || true
	vgremove -ff $vg
	aux teardown
}

TSIZE=15P
aux can_use_16T || TSIZE=15T

# With different snapshot target driver we may obtain different results.
# Older targets have metadata leak bug which needs extra compenstion.
# Ancient targets do not even provide separate info for metadata.
EXPECT1="16.00k"
EXPECT2="512.00k"
EXPECT3="32.00k"
EXPECT4="66.67"
if aux target_at_least dm-snapshot 1 10 0 ; then
	# Extra metadata size
	EXPECT4="0.00"

	if aux target_at_least dm-snapshot 1 12 0 ; then
		# When fixed leak, expect smaller sizes
		EXPECT1="12.00k"
		EXPECT2="384.00k"
		EXPECT3="28.00k"
	fi
fi

aux prepare_pvs 1
get_devs

vgcreate -s 4M "$vg" "${DEVICES[@]}"

# Play with 1 extent
lvcreate -aey -l1 -n $lv $vg
# 100%LV is not supported for snapshot
fail lvcreate -s -l 100%LV -n snap $vg/$lv 2>&1 | tee out
grep 'Please express size as %FREE, %ORIGIN, %PVS or %VG' out
# 100%ORIGIN needs to have enough space for all data and needs to round-up
lvcreate -s -l 100%ORIGIN -n $lv1 $vg/$lv
# everything needs to fit
fill 4M $vg/$lv1
lvremove -f $vg


# Automatically activates exclusively in cluster
lvcreate --type snapshot -s -l 100%FREE -n $lv $vg --virtualsize $TSIZE

aux extend_filter_LVMTEST
aux lvmconf "activation/snapshot_autoextend_percent = 20" \
            "activation/snapshot_autoextend_threshold = 50"

# Check usability with smallest (1k) extent size ($lv has 15P)
pvcreate --yes --setphysicalvolumesize 4T "$DM_DEV_DIR/$vg/$lv"
trap 'cleanup_tail' EXIT
vgcreate -s 4K $vg1 "$DM_DEV_DIR/$vg/$lv"

# Play with small 1k 128 extents
lvcreate -aey -L128K -n $lv $vg1
# 100%ORIGIN needs to have enough space for all data
lvcreate -s -l 100%ORIGIN -n snap100 $vg1/$lv
# everything needs to fit
fill 128k $vg1/snap100

# 50%ORIGIN needs to have enough space for 50% of data
lvcreate -s -l 50%ORIGIN -n snap50 $vg1/$lv
fill 64k $vg1/snap50

lvcreate -s -l 25%ORIGIN -n snap25 $vg1/$lv
fill 32k $vg1/snap25

# Check we do not provide too much extra space
not fill 33k $vg1/snap25

lvs -a $vg1
lvremove -f $vg1

# Test virtual snapshot over /dev/zero
lvcreate --type snapshot -V50 -L10 -n $lv1 -s $vg1
CHECK_ACTIVE="active"
test ! -e LOCAL_CLVMD || CHECK_ACTIVE="local exclusive"
check lv_field $vg1/$lv1 lv_active "$CHECK_ACTIVE"
lvchange -an $vg1

# On cluster snapshot gets exclusive activation
lvchange -ay $vg1
check lv_field $vg1/$lv1 lv_active "$CHECK_ACTIVE"

# Test removal of opened (but unmounted) snapshot (device busy) for a while
SLEEP_PID=$(aux hold_device_open $vg1 $lv1 60)

# Opened virtual snapshot device is not removable
# it should retry device removal for a few seconds
not lvremove -f $vg1/$lv1

kill $SLEEP_PID
SLEEP_PID=
# Wait for killed task, so there is no device holder
wait

lvremove -f $vg1/$lv1
check lv_not_exists $vg1 $lv1

# Check border size
lvcreate -aey -L4095G $vg1
lvcreate -s -L100K $vg1/lvol0
fill 4K
check lv_field $vg1/lvol1 data_percent "12.00"

lvremove -ff $vg1

# Create 4KB snapshot, does not need to be active here
lvcreate -an -Zn -l1 -n $lv1 $vg1
not lvcreate -s -l1 $vg1/$lv1
# snapshot cannot be smaller then 3 chunks (12K)
not lvcreate -s -l2 $vg1/$lv1
lvcreate -s -l30 -n $lv2 $vg1/$lv1
check lv_field $vg1/$lv2 size "$EXPECT1"

not lvcreate -s -c512 -l128 $vg1/$lv1
lvcreate -s -c128 -l1700 -n $lv3 $vg1/$lv1
# 3 * 128
check lv_field $vg1/$lv3 size "$EXPECT2"
lvremove -ff $vg1

lvcreate -aey -l5 $vg1
lvcreate -s -l3 $vg1/lvol0

# Fill 4KB -> 100% snapshot (1x 4KB chunk)
fill 4K
check lv_field $vg1/lvol1 data_percent "100.00"

# Check it resizes 100% full valid snapshot to fit threshold
lvextend --use-policies $vg1/lvol1
check lv_field $vg1/lvol1 data_percent "50.00"

fill 4K
lvextend --use-policies $vg1/lvol1
check lv_field $vg1/lvol1 size "24.00k"

lvextend -l+8 $vg1/lvol1
check lv_field $vg1/lvol1 size "$EXPECT3"

fill 20K

lvremove -f $vg1

# Check snapshot really deletes COW header for read-only snapshot
# Test needs special relation between chunk size and extent size
# This test expects extent size 1K
aux lvmconf "allocation/wipe_signatures_when_zeroing_new_lvs = 1"
lvcreate -aey -L4 -n $lv $vg1
lvcreate -c 8 -s -L1 -n snap $vg1/$lv
# Populate snapshot
#dd if=/dev/urandom of="$DM_DEV_DIR/$vg1/$lv" bs=4096 count=10
$MKFS "$DM_DEV_DIR/$vg1/$lv"
lvremove -f $vg1/snap

# Undeleted header would trigger attempt to access
# beyond end of COW device
# Fails to create when chunk size is different
lvcreate -s -pr -l3 -n snap $vg1/$lv

# When header is undelete, fails to read snapshot without read errors
#dd if="$DM_DEV_DIR/$vg1/snap" of=/dev/null bs=1M count=2
fsck -n "$DM_DEV_DIR/$vg1/snap"

# This test would trigger read of weird percentage for undeleted header
# And since older snapshot target counts with metadata sectors
# we have 2 valid results  (unsure about correct version number)
check lv_field $vg1/snap data_percent "$EXPECT4"

vgremove -ff $vg1
