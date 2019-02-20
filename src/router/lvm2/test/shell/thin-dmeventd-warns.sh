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

# test if dmeventd produces multiple warnings when pools runs above 80%


SKIP_WITH_LVMPOLLD=1

. lib/inittest

which blkdiscard || skip

percent_() {
	get lv_field $vg/pool data_percent | cut -d. -f1
}

wait_warn_() {

	for i in $(seq 1 7)
	do
		test "$(grep -E -c "WARNING: Thin pool.*is now" debug.log_DMEVENTD_out)" -eq "$1" && return 0
		sleep 2
	done

	die "Waiting too log for dmeventd log warning"
}
#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_dmeventd
aux prepare_vg

lvcreate -L8 -V8 -T $vg/pool -n $lv1


dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=256K count=26
test "$(percent_)" -gt 80

# Give it some time to dmeventd to log WARNING
wait_warn_ 1

dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=256K count=30
test "$(percent_)" -gt 90

# Give it some time to dmeventd to log WARNING
wait_warn_ 2

dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=1M count=8
test "$(percent_)" -eq 100

wait_warn_ 3

blkdiscard "$DM_DEV_DIR/$vg/$lv1"

# FIXME: Enforce thin-pool metadata commit with flushing status
dmsetup status ${vg}-pool-tpool
# Wait for thin-pool monitoring to notice lower values
sleep 11
# ATM dmeventd is not logging event for thin-pool getting
# below 'WARNED' threshold.


dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv1" bs=256K count=30
test "$(percent_)" -gt 90

lvs -a $vg
dmsetup status ${vg}-pool-tpool

# Check pool again Warns
wait_warn_ 4

vgremove -f $vg
