#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_CLVMD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext3 || skip

aux prepare_vg 2

# Note: inittest.sh  sets   LVM_SYSTEM_DIR to 'just' etc
etc_lv="$DM_DEV_DIR/$vg/$lv1"

cleanup_mounted_and_teardown()
{
	umount "$mount_dir" || true
	aux teardown
}

vgreduce $vg "$dev2"

lvcreate -n $lv1 -l 20%FREE $vg
mkfs.ext3 -b4096 -j "$etc_lv"

#
# check  read-only  archive dir
#
mount_dir="etc/archive"
trap 'cleanup_mounted_and_teardown' EXIT
mkdir -p "$mount_dir"
mount -n -r "$etc_lv" "$mount_dir"

aux lvmconf "backup/archive = 1" "backup/backup = 1"

# cannot archive to read-only - requires user to specify -An
not lvcreate -n $lv2 -l 10%FREE $vg
lvcreate -An -n $lv2 -l 10%FREE $vg

not vgextend  $vg "$dev2"
vgextend -An $vg "$dev2"

umount "$mount_dir" || true

vgreduce $vg "$dev2"

#
# check  read-only  backup dir
#
mount_dir="etc/backup"
mount -n -r "$etc_lv" "$mount_dir"

# Must not fail on making backup
vgscan

lvcreate -An -n $lv3 -l 10%FREE $vg

vgextend $vg "$dev2"

#
# Now check both archive & backup read-only
#
rm -rf etc/archive
ln -s backup etc/archive

# Must not fail on making backup
vgscan
lvcreate -An -n $lv4 -l 10%FREE $vg

umount "$mount_dir" || true

# TODO maybe also support --ignorelockingfailure ??
vgremove -ff $vg
