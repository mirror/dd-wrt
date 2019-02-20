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


# TODO: once code get fixed, add matching 'check' calls

SKIP_WITH_LVMPOLLD=1

. lib/inittest

test -f /proc/mdstat && grep -q raid1 /proc/mdstat || \
	modprobe raid1 || skip

aux lvmconf 'devices/md_component_detection = 1'
aux extend_filter_LVMTEST "a|/dev/md|"

aux prepare_devs 4

pvcreate "$dev2"
aux prepare_md_dev 0 64 2 "$dev1" "$dev2"
# Incorrectly shows  $dev2 as PV for 'raid0'
pvs -vvvv


vgcreate $SHARED $vg "$dev3" "$dev4"

# create 2 disk MD raid1 array
# by default using metadata format 1.0 with data at the end of device
aux prepare_md_dev 1 64 2 "$dev1" "$dev2"

mddev=$(< MD_DEV)
pvdev=$(< MD_DEV_PV)
sleep 3
mdadm --stop "$mddev"

# copy fake PV/VG header PV3 -> PV2 (which is however md raid1 leg)
dd if="$dev3"  of="$dev2" bs=64k count=1 conv=fdatasync

# remove VG on PV3 & PV4
vgremove -f $vg

sleep 3
aux udev_wait
# too bad  'dd' wakes up  md array reassembling
should not mdadm --detail "$mddev"
should not mdadm --stop "$mddev"
sleep 3

# print what  blkid thinks about each PV
for i in "$dev1" "$dev2" "$dev3" "$dev4"
do
	blkid -c /dev/null -w /dev/null "$i" || echo "Unknown signature"
done

# expect open count for each PV to be 0
dmsetup info -c

pvs -vvvv  "$dev2" "$dev3" || true

# still expect open count for each PV to be 0
dmsetup info -c

pvs -vvvv  "$dev3" "$dev2" || true

# and again we expect open count for each PV to be 0
dmsetup info -c
dmsetup table

# even after 3 second of possible hidden raid array assembling
sleep 3
dmsetup info -c

# if for any reason array went up - stop it again
if mdadm --detail "$mddev" ; then
	mdadm --stop "$mddev"
	aux udev_wait
	should not mdadm --detail "$mddev"
fi

# now reassemble array from  PV1 & PV2
mdadm --assemble --verbose "$mddev" "$dev1" "$dev2"
aux udev_wait
sleep 1

# and let 'fake hdr' to be fixed from master/primary leg
# (when mdadm supports repair)
if mdadm --action=repair "$mddev" ; then
	sleep 1
	pvscan -vvvv
	# should be showing correctly PV3 & PV4
	pvs -vvvv "$dev3" "$dev4"
fi
