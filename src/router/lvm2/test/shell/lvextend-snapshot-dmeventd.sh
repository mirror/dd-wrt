#!/usr/bin/env bash

# Copyright (C) 2010-2012 Red Hat, Inc. All rights reserved.
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

extend() {
	lvextend --use-policies --config "activation { snapshot_autoextend_threshold = $1 }" $vg/snap
}

write_() {
	dd if=/dev/zero of="$DM_DEV_DIR/$vg/snap" bs=1k count=$2 seek=$1 oflag=direct
}

percent_() {
	get lv_field $vg/snap snap_percent | cut -d. -f1
}

wait_for_change_() {
	# dmeventd only checks every 10 seconds :(
	for i in $(seq 1 25) ; do
		test "$(percent_)" != "$1" && return
		sleep 1
	done

	return 1  # timeout
}

aux prepare_dmeventd
aux prepare_vg 2

lvcreate -aey -L16M -n base $vg
lvcreate -s -L4M -n snap $vg/base

write_ 0 1000
test 24 -eq "$(percent_)"

lvchange --monitor y $vg/snap

write_ 1000 1700
pre=$(percent_)
# Normally the usage should be ~66% here, however on slower systems
# dmeventd could be actually 'fast' enough to have COW already resized now
# so mark test skipped if we are below 50% by now
test "$pre" -gt 50 || skip
wait_for_change_ $pre
test "$pre" -gt "$(percent_)"

# check that a second extension happens; we used to fail to extend when the
# utilisation ended up between THRESH and (THRESH + 10)... see RHBZ 754198
# (the utilisation after the write should be 57 %)

write_ 2700 2000
pre=$(percent_)
# Mark test as skipped if already resized...
test "$pre" -gt 70 || skip
wait_for_change_ $pre
test "$pre" -gt "$(percent_)"

vgremove -f $vg
