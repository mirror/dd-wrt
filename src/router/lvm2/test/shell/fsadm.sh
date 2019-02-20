#!/usr/bin/env bash

# Copyright (C) 2008-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise fsadm filesystem resize'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 1 100

# set to "skip" to avoid testing given fs and test warning result
# i.e. check_reiserfs=skip
check_ext2=
check_ext3=
check_xfs=
check_reiserfs=

which mkfs.ext2 || check_ext2=${check_ext2:-mkfs.ext2}
which mkfs.ext3 || check_ext3=${check_ext3:-mkfs.ext3}
which fsck.ext3 || check_ext3=${check_ext3:-fsck.ext3}
which mkfs.xfs || check_xfs=${check_xfs:-mkfs.xfs}
which xfs_check || {
	which xfs_repair || check_xfs=${check_xfs:-xfs_repair}
}
grep xfs /proc/filesystems || check_xfs=${check_xfs:-no_xfs}

which mkfs.reiserfs || check_reiserfs=${check_reiserfs:-mkfs.reiserfs}
which reiserfsck || check_reiserfs=${check_reiserfs:-reiserfsck}
modprobe reiserfs || true
grep reiserfs /proc/filesystems || check_reiserfs=${check_reiserfs:-no_reiserfs}

vg_lv=$vg/$lv1
vg_lv2=$vg/${lv1}bar
dev_vg_lv="$DM_DEV_DIR/$vg_lv"
dev_vg_lv2="$DM_DEV_DIR/$vg_lv2"
mount_dir="mnt"
mount_space_dir="mnt space dir"
# for recursive call
LVM_BINARY=$(which lvm)
export LVM_BINARY

test ! -d "$mount_dir" && mkdir "$mount_dir"
test ! -d "$mount_space_dir" && mkdir "$mount_space_dir"

cleanup_mounted_and_teardown()
{
	umount "$mount_dir" || true
	umount "$mount_space_dir" || true
	aux teardown
}

fscheck_ext3()
{
	fsck.ext3 -p -F -f "$dev_vg_lv"
}

fscheck_xfs()
{
	if which xfs_repair ; then
		xfs_repair -n "$dev_vg_lv"
	else
		xfs_check "$dev_vg_lv"
	fi
}

fscheck_reiserfs()
{
	reiserfsck --check -p -f "$dev_vg_lv" </dev/null
}

check_missing()
{
	local t
	eval "t=\$check_$1"
	test -z "$t" && return 0
	test "$t" = skip && return 1
	echo "WARNING: fsadm test skipped $1 tests, $t tool is missing."
	# trick to get test listed with warning
	# should false;
	return 1
}

# Test for block sizes != 1024 (rhbz #480022)
lvcreate -n $lv1 -L20M $vg
lvcreate -n ${lv1}bar -L10M $vg
trap 'cleanup_mounted_and_teardown' EXIT

if check_missing ext2; then
	mkfs.ext2 -b4096 -j "$dev_vg_lv"

	fsadm --lvresize resize $vg_lv 30M
	# Fails - not enough space for 4M fs
	not fsadm -y --lvresize resize "$dev_vg_lv" 4M
	lvresize -L+10M -r $vg_lv
	lvreduce -L10M -r $vg_lv

	fscheck_ext3
	mount "$dev_vg_lv" "$mount_dir"
	not fsadm -y --lvresize resize $vg_lv 4M
	echo n | not lvresize -L4M -r -n $vg_lv
	lvresize -L+20M -r -n $vg_lv
	umount "$mount_dir"
	fscheck_ext3

	lvresize -f -L20M $vg_lv
fi

if check_missing ext3; then
	mkfs.ext3 -b4096 -j "$dev_vg_lv"
	mkfs.ext3 -b4096 -j "$dev_vg_lv2"

	fsadm --lvresize resize $vg_lv 30M
	# Fails - not enough space for 4M fs
	not fsadm -y --lvresize resize "$dev_vg_lv" 4M
	lvresize -L+10M -r $vg_lv
	lvreduce -L10M -r $vg_lv

	fscheck_ext3
	mount "$dev_vg_lv" "$mount_dir"
	lvresize -L+10M -r $vg_lv
	mount "$dev_vg_lv2" "$mount_space_dir"
	fsadm --lvresize -e -y resize $vg_lv2 25M

	not fsadm -y --lvresize resize $vg_lv 4M
	echo n | not lvresize -L4M -r -n $vg_lv
	lvresize -L+20M -r -n $vg_lv
	lvresize -L-10M -r -y $vg_lv
	umount "$mount_dir"
	umount "$mount_space_dir"
	fscheck_ext3

	lvresize -f -L20M $vg_lv
fi

if check_missing xfs; then
	mkfs.xfs -l internal,size=2000b -f "$dev_vg_lv"

	fsadm --lvresize resize $vg_lv 30M
	# Fails - not enough space for 4M fs
	lvresize -L+10M -r $vg_lv
	not lvreduce -L10M -r $vg_lv

	fscheck_xfs
	mount "$dev_vg_lv" "$mount_dir"
	lvresize -L+10M -r -n $vg_lv
	umount "$mount_dir"
	fscheck_xfs

	lvresize -f -L20M $vg_lv
fi

if check_missing reiserfs; then
	mkfs.reiserfs -s 513 -f "$dev_vg_lv"

	fsadm --lvresize resize $vg_lv 30M
	lvresize -L+10M -r $vg_lv
	fsadm --lvresize -y resize $vg_lv 10M

	fscheck_reiserfs
	mount "$dev_vg_lv" "$mount_dir"

	fsadm -y --lvresize resize $vg_lv 30M
	umount "$mount_dir"
	fscheck_reiserfs

	lvresize -f -L20M $vg_lv
fi

vgremove -ff $vg
