#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
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

aux lvmconf 'devices/md_component_detection = 1'

aux prepare_devs 4

if test -n "$LVM_TEST_LVM1" ; then
mdatypes='1 2'
else
mdatypes='2'
fi

for mdatype in $mdatypes
do
# pvcreate (lvm$mdatype) refuses to overwrite an mounted filesystem (bz168330)
	test ! -d mnt && mkdir mnt
	if mke2fs "$dev1"; then
		mount "$dev1" mnt
		not pvcreate -M$mdatype "$dev1" 2>err
		grep "Can't open $dev1 exclusively.  Mounted filesystem?" err
		umount "$dev1"
		# wipe the filesystem signature for next
		# pvcreate to not issue any prompts
		dd if=/dev/zero of="$dev1" bs=1K count=2
	fi

# pvcreate (lvm$mdatype) succeeds when run repeatedly (pv not in a vg) (bz178216)
    pvcreate -M$mdatype "$dev1"
    pvcreate -M$mdatype "$dev1"
    pvremove -f "$dev1"

# pvcreate (lvm$mdatype) fails when PV belongs to VG
#   pvcreate -M$mdatype "$dev1"
    vgcreate $SHARED -M$mdatype $vg1 "$dev1"
    not pvcreate -M$mdatype "$dev1"

    vgremove -f $vg1
    pvremove -f "$dev1"

# pvcreate (lvm$mdatype) fails when PV1 does and PV2 does not belong to VG
    pvcreate -M$mdatype "$dev1"
    pvcreate -M$mdatype "$dev2"
    vgcreate $SHARED -M$mdatype $vg1 "$dev1"

# pvcreate a second time on $dev2 and $dev1
    not pvcreate -M$mdatype "$dev2" "$dev1"

    vgremove -f $vg1
    pvremove -f "$dev2" "$dev1"

# NOTE: Force pvcreate after test completion to ensure clean device
#test_expect_success
#  "pvcreate (lvm$mdatype) fails on md component device"
#  'mdadm -C -l raid0 -n 2 /dev/md0 "$dev1" "$dev2" &&
#   pvcreate -M$mdatype "$dev1";
#   status=$?; echo status=$status; test $status != 0 &&
#   mdadm --stop /dev/md0 &&
#   pvcreate -ff -y -M$mdatype "$dev1" "$dev2" &&
#   pvremove -f "$dev1" "$dev2"'
done

# pvcreate (lvm2) fails without -ff when PV with metadatacopies=0 belongs to VG
pvcreate --metadatacopies 0 "$dev1"
pvcreate --metadatacopies 1 "$dev2"
vgcreate $SHARED $vg1 "$dev1" "$dev2"
not pvcreate "$dev1"
vgremove -f $vg1
pvremove -f "$dev2" "$dev1"

# pvcreate (lvm2) succeeds with -ff when PV with metadatacopies=0 belongs to VG
pvcreate --metadatacopies 0 "$dev1"
pvcreate --metadatacopies 1 "$dev2"
vgcreate $SHARED $vg1 "$dev1" "$dev2"
pvcreate -ff -y "$dev1"
vgreduce --removemissing $vg1
vgremove -ff $vg1
pvremove -f "$dev2" "$dev1"

for i in 0 1 2 3
do
# pvcreate (lvm2) succeeds writing LVM label at sector $i
    pvcreate --labelsector $i "$dev1"
    dd if="$dev1" bs=512 skip=$i count=1 2>/dev/null | strings | grep LABELONE >/dev/null
    pvremove -f "$dev1"
done

# pvcreate (lvm2) fails writing LVM label at sector 4
not pvcreate --labelsector 4 "$dev1"

backupfile="$PREFIX.mybackupfile"
uuid1=freddy-fred-fred-fred-fred-fred-freddy
uuid2=freddy-fred-fred-fred-fred-fred-fredie
bogusuuid=fred

# pvcreate rejects uuid option with less than 32 characters
not pvcreate --norestorefile --uuid $bogusuuid "$dev1"

# pvcreate rejects uuid option without restorefile
not pvcreate --uuid $uuid1 "$dev1"

# pvcreate rejects uuid already in use
pvcreate --norestorefile --uuid $uuid1 "$dev1"
not pvcreate --norestorefile --uuid $uuid1 "$dev2"

# pvcreate rejects non-existent file given with restorefile
not pvcreate --uuid $uuid1 --restorefile "$backupfile" "$dev1"

# pvcreate rejects restorefile with uuid not found in file
pvcreate --norestorefile --uuid $uuid1 "$dev1"
vgcfgbackup -f "$backupfile"
not pvcreate --uuid $uuid2 --restorefile "$backupfile" "$dev2"

# vgcfgrestore of a VG containing a PV with zero PEs (bz #820116)
# (use case: one PV in a VG used solely to keep metadata)
size_mb=$(($(blockdev --getsz "$dev1") / 2048))
pvcreate --metadatasize $size_mb "$dev1"
vgcreate $SHARED $vg1 "$dev1"
vgcfgbackup -f "$backupfile"
vgcfgrestore -f "$backupfile" "$vg1"
vgremove -f $vg1
pvremove -f "$dev1"

# pvcreate --restorefile should handle --dataalignment and --dataalignmentoffset
# and check it's compatible with pe_start value being restored
# X * dataalignment + dataalignmentoffset == pe_start
pvcreate --norestorefile --uuid "$uuid1" --dataalignment 600k --dataalignmentoffset 32k "$dev1"
vgcreate $SHARED $vg1 "$dev1"
vgcfgbackup -f "$backupfile" "$vg1"
vgremove -ff $vg1
pvremove -ff "$dev1"
# the dataalignment and dataalignmentoffset is ignored here since they're incompatible with pe_start
pvcreate --restorefile "$backupfile" --uuid "$uuid1" --dataalignment 500k --dataalignmentoffset 10k "$dev1" 2> err
grep "incompatible with restored pe_start value" err
# 300k is multiple of 600k so this should pass
pvcreate --restorefile "$backupfile" --uui "$uuid1" --dataalignment 300k --dataalignmentoffset 32k "$dev1" 2> err
not grep "incompatible with restored pe_start value" err

# pvcreate rejects non-existent uuid given with restorefile
not pvcreate --uuid "$uuid2" --restorefile "$backupfile" "$dev1" 2> err
grep "Can't find uuid $uuid2 in backup file $backupfile" err

# pvcreate rejects restorefile without uuid
not pvcreate --restorefile "$backupfile" "$dev1" 2>err
grep -- "--uuid is required with --restorefile" err

# pvcreate rejects uuid restore with multiple volumes specified
not pvcreate --uuid "$uuid1" --restorefile "$backupfile" "$dev1" "$dev2" 2>err
grep "Can only set uuid on one volume at once" err

# --bootloaderareasize not allowed with pvcreate --restorefile
not pvcreate --uuid "$uuid1" --restorefile "$backupfile" --bootloaderareasize 1m "$dev1" "$dev2" 2>err
grep -- "Command does not accept option combination: --bootloaderareasize  with --restorefile" err

rm -f "$backupfile"

pvcreate --norestorefile --uuid $uuid1 "$dev1"
vgcreate $SHARED --physicalextentsize 1m $vg1 "$dev1"
vgcfgbackup -f "$backupfile" "$vg1"
vgremove -ff "$vg1"
pvremove -ff "$dev1"

# when 2nd mda requested on pvcreate --restorefile and not enough space for it, pvcreate fails
not pvcreate --restorefile "$backupfile" --uuid $uuid1 --metadatacopies 2 "$dev1" 2>err
grep "Not enough space available for metadata area with index 1 on PV $dev1" err

rm -f "$backupfile"

# pvcreate wipes swap signature when forced
dd if=/dev/zero of="$dev1" bs=1024 count=64
mkswap "$dev1"
blkid -c /dev/null "$dev1" | grep "swap"
pvcreate -f "$dev1"
# blkid cannot make up its mind whether not finding anything it knows is a failure or not
(blkid -c /dev/null "$dev1" || true) | not grep "swap"
