#!/usr/bin/env bash

# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
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

which mkfs.ext3 || skip
aux have_raid 1 3 0 || skip

aux lvmconf \
	'activation/raid_fault_policy = "allocate"'

aux prepare_dmeventd
aux prepare_vg 5

lvcreate -aey --type raid1 -m 3 --ignoremonitoring -L 1 -n 4way $vg
lvchange --monitor y $vg/4way
lvs -a -o all,lv_modules $vg
lvdisplay --maps $vg
aux wait_for_sync $vg 4way
aux disable_dev "$dev2" "$dev4"
mkfs.ext3 "$DM_DEV_DIR/$vg/4way"
sleep 5 # FIXME: need a "poll" utility, akin to "check"
aux enable_dev "$dev2" "$dev4"

dmsetup table
dmsetup status
dmsetup info -c
vgremove -vvvv -ff $vg
