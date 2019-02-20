#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
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

test -f /proc/mdstat && grep -q raid1 /proc/mdstat || \
	modprobe raid1 || skip

aux lvmconf 'devices/md_component_detection = 1'
aux extend_filter_LVMTEST "a|/dev/md|"

aux prepare_devs 2

# create 2 disk MD raid1 array
# by default using metadata format 1.0 with data at the end of device
aux prepare_md_dev 1 64 2 "$dev1" "$dev2"

mddev=$(< MD_DEV)
pvdev=$(< MD_DEV_PV)

vgcreate $vg "$mddev"

lvs $vg

lvcreate -n $lv1 -l 2 $vg
lvcreate -n $lv2 -l 2 -an $vg

lvchange -ay $vg/$lv2

lvs $vg

pvs -vvvv 2>&1|tee pvs.out

vgchange -an $vg

vgchange -ay -vvvv $vg 2>&1| tee vgchange.out

lvs $vg
pvs

vgchange -an $vg

mdadm --stop "$mddev"
aux udev_wait

# with md superblock 1.0 this pvs will report duplicates
# for the two md legs since the md device itself is not
# started
pvs 2>&1 |tee out
cat out
grep "prefers device" out

pvs -vvvv 2>&1| tee pvs2.out

# should not activate from the md legs
not vgchange -ay -vvvv $vg 2>&1|tee vgchange-fail.out

# should not show an active lv
lvs $vg

# start the md dev
mdadm --assemble "$mddev" "$dev1" "$dev2"
aux udev_wait

# Now that the md dev is online, pvs can see it and
# ignore the two legs, so there's no duplicate warning

pvs 2>&1 |tee out
cat out
not grep "prefers device" out

vgchange -ay $vg 2>&1 |tee out
cat out
not grep "prefers device" out

vgchange -an $vg

vgremove -f $vg
