#!/usr/bin/env bash

# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

which mkfs.ext2 || skip

extend() {
	lvextend --use-policies --config "activation { snapshot_autoextend_threshold = $1 }" $vg/snap
}

write() {
	mount "$DM_DEV_DIR/$vg/snap" mnt
	dd if=/dev/zero of="mnt/file$1" bs=1k count=$2
	umount mnt
}

percent() {
	get lv_field $vg/snap snap_percent | cut -d. -f1
}

# no dmeventd running in this test, testing --use-policies
aux prepare_vg 2

lvcreate -aey -L24 -n base $vg
mkfs.ext2 "$DM_DEV_DIR/$vg/base"

lvcreate -s -L16 -n snap $vg/base
mkdir mnt

write 1 4096
pre=$(percent)
extend 50
test "$pre" -eq "$(percent)"

write 2 4096
pre=$(percent)
extend 50
test "$pre" -gt "$(percent)"

vgremove -f $vg
