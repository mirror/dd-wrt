#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise cache flushing is abortable


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 2

# Data device on later delayed dev1
lvcreate -L4 -n cpool $vg "$dev1"
lvconvert -y --type cache-pool $vg/cpool "$dev2"
lvcreate -H -L 4 -n $lv1 --chunksize 32k --cachemode writeback --cachepool $vg/cpool $vg "$dev2"

#
# Ensure cache gets promoted blocks
#
for i in $(seq 1 10) ; do
echo 3 >/proc/sys/vm/drop_caches
dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=64K count=20 conv=fdatasync || true
echo 3 >/proc/sys/vm/drop_caches
dd if="$DM_DEV_DIR/$vg/$lv1" of=/dev/null bs=64K count=20 || true
done


# Delay dev to ensure we have some time to 'capture' interrupt in flush
aux delay_dev "$dev1" 100 0 "$(get first_extent_sector "$dev1"):"

# TODO, how to make writeback cache dirty
test "$(get lv_field $vg/$lv1 cache_dirty_blocks)" -gt 0 || {
	lvdisplay --maps $vg
	skip "Cannot make a dirty writeback cache LV."
}

sync
dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=4k count=100 conv=fdatasync

LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -v --splitcache $vg/$lv1 >logconvert 2>&1 &
PID_CONVERT=$!
for i in {1..50}; do
	dmsetup table "$vg-$lv1" | grep cleaner && break
	test "$i" -ge 100 && die "Waited for cleaner policy on $vg/$lv1 too long!"
	echo "Waiting for cleaner policy on $vg/$lv1"
	sleep .05
done
kill -INT $PID_CONVERT
aux enable_dev "$dev1"
wait

grep -E "Flushing.*aborted" logconvert || {
	cat logconvert || true
	vgremove -f $vg
	die "Flushing of $vg/$lv1 not aborted ?"
}

# check the table got restored
check grep_dmsetup table $vg-$lv1 "writeback"

vgremove -f $vg
