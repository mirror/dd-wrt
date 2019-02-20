#!/usr/bin/env bash

# Copyright (C) 2008-2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise fsadm filesystem resize on crypt devices'

SKIP_WITH_LVMPOLLD=1

# FIXME: cannot use brd (ramdisk)  - lsblk is NOT listing it
# so lsblk usage should be replaced
export LVM_TEST_PREFER_BRD=0

. lib/inittest

aux prepare_vg 1 300

# set to "skip" to avoid testing given fs and test warning result
# i.e. check_reiserfs=skip
check_ext2=
check_ext3=
check_xfs=
check_reiserfs=
check_cryptsetup=
DROP_SYMLINK=

CRYPT_NAME="$PREFIX-tcrypt"
CRYPT_DEV="$DM_DEV_DIR/mapper/$CRYPT_NAME"

CRYPT_NAME2="$PREFIX-tcrypt2"
CRYPT_DEV2="$DM_DEV_DIR/mapper/$CRYPT_NAME2"

CRYPT_NAME_PLAIN="$PREFIX-tcryptp"
CRYPT_DEV_PLAIN="$DM_DEV_DIR/mapper/$CRYPT_NAME_PLAIN"

FORMAT_PARAMS="-i1"
PWD1="93R4P4pIqAH8"
PWD2="mymJeD8ivEhE"
PWD3="ocMakf3fAcQO"
SKIP_DETACHED=

which cryptsetup || check_cryptsetup=${check_cryptsetup:-cryptsetup}

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
vg_lv3=$vg/${lv1}plain
dev_vg_lv="$DM_DEV_DIR/$vg_lv"
dev_vg_lv2="$DM_DEV_DIR/$vg_lv2"
dev_vg_lv3="$DM_DEV_DIR/$vg_lv3"
mount_dir="mnt"
# for recursive call
LVM_BINARY=$(which lvm)
export LVM_BINARY

test ! -d "$mount_dir" && mkdir "$mount_dir"

crypt_close() {
	cryptsetup remove "$1"
	if [ "$?" -eq 0 -a -n "$DROP_SYMLINK" ]; then
		rm -f "$DM_DEV_DIR/mapper/$1"
	fi
}

cleanup_mounted_and_teardown()
{
	umount "$mount_dir" || true
	crypt_close $CRYPT_NAME > /dev/null 2>&1 || true
	crypt_close $CRYPT_NAME2 > /dev/null 2>&1 || true
	crypt_close $CRYPT_NAME_PLAIN > /dev/null 2>&1 || true
	aux teardown
}

fscheck_ext3()
{
	fsck.ext3 -p -F -f "$1"
}

fscheck_xfs()
{
	if which xfs_repair ; then
		xfs_repair -n "$1"
	else
		xfs_check "$1"
	fi
}

fscheck_reiserfs()
{
	reiserfsck --check -p -f "$1" </dev/null
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

get_crypt_kname() {
	lsblk -r -n -o KNAME,NAME | grep "$1" | cut -d ' ' -f 1
}


# $1 device
# $2 pass
crypt_format() {
	echo "$2" | cryptsetup luksFormat "$FORMAT_PARAMS" "$1"
}


# $1 device
# $2 pass
# $3 name
crypt_open() {
	local kname=
	echo "$2" | cryptsetup luksOpen "$1" "$3"
	test -L "$DM_DEV_DIR/mapper/$3" || {
		kname=$(get_crypt_kname $3)
		ln -s "/dev/$kname" "$DM_DEV_DIR/mapper/$3"
		DROP_SYMLINK=1
	}
}

# $1 data device
# $2 pass
# $3 name
# $4 header
crypt_open_detached() {
	local kname=
	echo "$2" | cryptsetup luksOpen --header "$4" "$1" "$3" || return $?
	test -L "$DM_DEV_DIR/mapper/$3" || {
		kname=$(get_crypt_kname $3)
		ln -s "/dev/$kname" "$DM_DEV_DIR/mapper/$3"
		DROP_SYMLINK=1
	}
}

# $1 device
# $2 pass
# $3 name
crypt_open_plain() {
	local kname=
	echo "$2" | cryptsetup create "$3" "$1"
	test -L "$DM_DEV_DIR/mapper/$3" || {
		kname=$(get_crypt_kname $3)
		ln -s "/dev/$kname" "$DM_DEV_DIR/mapper/$3"
		DROP_SYMLINK=1
	}
}

# $1 device
# $2 type
create_crypt_device()
{
	crypt_format "$dev_vg_lv" $PWD1
	crypt_open "$dev_vg_lv" $PWD1 "$CRYPT_NAME"

	crypt_format "$dev_vg_lv2" $PWD2

	if crypt_open_detached "$dev_vg_lv3" "$PWD2" "$PREFIX-test" "$dev_vg_lv2"; then
		crypt_close "$PREFIX-test"
	else
		SKIP_DETACHED=1
	fi
}

which lsblk > /dev/null || skip
check_missing cryptsetup || skip

vgchange -s 128k
lvcreate -n $lv1 -L25M $vg
lvcreate -n ${lv1}bar -L35M $vg
lvcreate -n ${lv1}plain -L35M $vg
create_crypt_device
trap 'cleanup_mounted_and_teardown' EXIT


# $1 LVM backend (vg/lv name)
# $2 LVM backend device (/dev/vg/lv)
# $3 active dm-crypt device (/dev/mapper/some_name )
test_ext2_resize() {
	mkfs.ext2 -b4096 -j "$3"

	fsadm --lvresize resize $1 30M
	# Fails - not enough space for 4M fs
	not fsadm -y --lvresize resize "$2" 4M
	lvresize -L+10M -r $1
	lvreduce -L10M -r $1

	fscheck_ext3 "$3"
	mount "$3" "$mount_dir"
	not fsadm -y --lvresize resize $1 4M
	echo n | not lvresize -L4M -r -n $1
	lvresize -L+20M -r -n $1
	umount "$mount_dir"
	fscheck_ext3 "$3"
}

test_ext2_small_shrink() {
	mkfs.ext2 "$3"

	lvresize -L-1 -r $1
	lvresize -L-1 -r $1

	fscheck_ext3 "$3"
}

test_ext3_resize() {
	mkfs.ext3 -b4096 -j "$3"

	fsadm --lvresize resize $1 30M
	# Fails - not enough space for 4M fs
	not fsadm -y --lvresize resize "$2" 4M
	lvresize -L+10M -r $1
	lvreduce -L10M -r $1

	fscheck_ext3 "$3"
	mount "$3" "$mount_dir"
	lvresize -L+10M -r $1

	not fsadm -y --lvresize resize $1 4M
	echo n | not lvresize -L4M -r -n $1
	lvresize -L+20M -r -n $1
	lvresize -L-10M -r -y $1
	umount "$mount_dir"
}

test_ext3_small_shrink() {
	mkfs.ext3 "$3"

	lvresize -L-1 -r $1
	lvresize -L-1 -r $1

	fscheck_ext3 "$3"
}

test_xfs_resize() {
	mkfs.xfs -l internal,size=1000b -f "$3"

	fsadm --lvresize resize $1 30M
	# Fails - not enough space for 4M fs
	lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	fscheck_xfs "$3"
	mount "$3" "$mount_dir"
	lvresize -L+10M -r -n $1
	umount "$mount_dir"
	fscheck_xfs "$3"
}

test_xfs_small_shrink() {
	mkfs.xfs -l internal,size=1000b -f "$3"

	not lvresize -L-1 -r $1
	fscheck_xfs "$3"
}

test_reiserfs_resize() {
	mkfs.reiserfs -s 513 -f "$3"

	fsadm --lvresize resize $1 30M
	lvresize -L+10M -r $1
	fsadm --lvresize -y resize $1 10M

	fscheck_reiserfs "$3"
	mount "$3" "$mount_dir"

	fsadm -y --lvresize resize $1 30M
	umount "$mount_dir"
	fscheck_reiserfs "$3"
}

test_reiserfs_small_shrink() {
	mkfs.reiserfs -s 513 -f "$3"

	lvresize -y -L-1 -r $1
	lvresize -y -L-1 -r $1

	fscheck_reiserfs "$3"
}

# $1 LVM backend (vg/lv name)
# $2 LVM backend device (/dev/vg/lv)
# $3 active dm-crypt device (/dev/mapper/some_name )
# $4 active dm-crypt name ( some_name )
test_ext2_inactive() {
	crypt_open "$2" $PWD2 "$4"
	mkfs.ext2 -b4096 -j "$3"
	crypt_close "$4"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	crypt_open "$2" $PWD2 "$4"
	fscheck_ext3 "$3"
	crypt_close "$4"
}

test_ext3_inactive() {
	crypt_open "$2" $PWD2 "$4"
	mkfs.ext3 -b4096 -j "$3"
	crypt_close "$4"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	crypt_open "$2" $PWD2 "$4"
	fscheck_ext3 "$3"
	crypt_close "$4"
}

test_xfs_inactive() {
	crypt_open "$2" $PWD2 "$4"
	mkfs.xfs -l internal,size=1000b -f "$3"
	crypt_close "$4"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	crypt_open "$2" $PWD2 "$4"
	fscheck_xfs "$3"
	crypt_close "$4"
}

test_reiserfs_inactive() {
	crypt_open "$2" $PWD2 "$4"
	mkfs.reiserfs -s 513 -f "$3"
	crypt_close "$4"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	crypt_open "$2" $PWD2 "$4"
	fscheck_reiserfs "$3"
	crypt_close "$4"
}

# $1 LVM backend (vg/lv name)
# $2 LVM backend device (/dev/vg/lv)
# $3 active dm-crypt device (/dev/mapper/some_name )
# $4 active dm-crypt name ( some_name )
test_ext2_plain() {
	mkfs.ext2 -b4096 -j "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	fscheck_ext3 "$3"

	fsadm --cryptresize resize $3 30M
	fsadm --cryptresize resize $3 35M
	fscheck_ext3 "$3"

	mount "$3" "$mount_dir"
	not fsadm -y --cryptresize resize $3 4M
	umount "$mount_dir"
	fscheck_ext3 "$3"

	crypt_close "$4"
	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	crypt_open_plain "$2" $PWD3 "$4"
	fscheck_ext3 "$3"
}

test_ext3_plain() {
	mkfs.ext3 -b4096 -j "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	fscheck_ext3 "$3"

	fsadm --cryptresize resize $3 30M
	fsadm --cryptresize resize $3 35M
	fscheck_ext3 "$3"

	mount "$3" "$mount_dir"
	not fsadm -y --cryptresize resize $3 4M
	umount "$mount_dir"
	fscheck_ext3 "$3"

	crypt_close "$4"
	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	crypt_open_plain "$2" $PWD3 "$4"
	fscheck_ext3 "$3"
}

test_xfs_plain() {
	mkfs.xfs -l internal,size=1000b -f "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	fscheck_xfs "$3"

	lvresize -f -L+10M $1
	fsadm --cryptresize resize $3 40M
	# no shrink support in xfs
	not fsadm --cryptresize resize $3 35M
	fscheck_xfs "$3"

	crypt_close "$4"
	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	crypt_open_plain "$2" $PWD3 "$4"
	fscheck_xfs "$3"

	lvresize -f -L35M $1
}

test_reiserfs_plain() {
	mkfs.reiserfs -s 513 -f "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L-10M -r $1
	fscheck_reiserfs "$3"

	fsadm -y --cryptresize resize $3 30M
	fsadm -y --cryptresize resize $3 35M
	fscheck_reiserfs "$3"

	crypt_close "$4"
	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	crypt_open_plain "$2" $PWD3 "$4"
	fscheck_reiserfs "$3"
}

# $1 LVM header backend (vg/lv name)
# $2 LVM hedaer backend device (/dev/vg/lv)
# $3 active dm-crypt device (/dev/mapper/some_name )
# $4 active dm-crypt name ( some_name )a
test_ext2_detached() {
	mkfs.ext2 -b4096 -j "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	fscheck_ext3 "$3"
}

test_ext3_detached() {
	mkfs.ext3 -b4096 -j "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1
	fscheck_ext3 "$3"
}

test_xfs_detached() {
	mkfs.xfs -l internal,size=1000b -f "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	fscheck_xfs "$3"
}

test_reiserfs_detached() {
	mkfs.reiserfs -s 513 -f "$3"

	not fsadm --lvresize resize $1 30M
	not lvresize -L+10M -r $1
	not lvreduce -L10M -r $1

	fscheck_reiserfs "$3"
}

if check_missing ext2; then
	test_ext2_resize "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	test_ext2_inactive "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"

	crypt_open_plain "$dev_vg_lv3" $PWD3 "$CRYPT_NAME_PLAIN"
	test_ext2_plain "$vg_lv3" "$dev_vg_lv3" "$CRYPT_DEV_PLAIN" "$CRYPT_NAME_PLAIN"
	crypt_close "$CRYPT_NAME_PLAIN"

	lvresize -f -L100M $vg_lv
	cryptsetup resize $CRYPT_NAME
	test_ext2_small_shrink "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	if [ -z "$SKIP_DETACHED" ]; then
		crypt_open_detached "$dev_vg_lv3" $PWD2 "$CRYPT_NAME2" "$dev_vg_lv2"
		test_ext2_detached "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"
		crypt_close "$CRYPT_NAME2"
	fi
fi

if check_missing ext3; then
	test_ext3_resize "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	test_ext3_inactive "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"

	crypt_open_plain "$dev_vg_lv3" $PWD3 "$CRYPT_NAME_PLAIN"
	test_ext3_plain "$vg_lv3" "$dev_vg_lv3" "$CRYPT_DEV_PLAIN" "$CRYPT_NAME_PLAIN"
	crypt_close "$CRYPT_NAME_PLAIN"

	lvresize -f -L100M $vg_lv
	cryptsetup resize $CRYPT_NAME
	test_ext3_small_shrink "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	if [ -z "$SKIP_DETACHED" ]; then
		crypt_open_detached "$dev_vg_lv3" $PWD2 "$CRYPT_NAME2" "$dev_vg_lv2"
		test_ext3_detached "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"
		crypt_close "$CRYPT_NAME2"
	fi
fi

if check_missing xfs; then
	test_xfs_resize "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	test_xfs_inactive "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"

	crypt_open_plain "$dev_vg_lv3" $PWD3 "$CRYPT_NAME_PLAIN"
	test_xfs_plain "$vg_lv3" "$dev_vg_lv3" "$CRYPT_DEV_PLAIN" "$CRYPT_NAME_PLAIN"
	crypt_close "$CRYPT_NAME_PLAIN"

	lvresize -f -L100M $vg_lv
	cryptsetup resize $CRYPT_NAME
	test_xfs_small_shrink "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	if [ -z "$SKIP_DETACHED" ]; then
		crypt_open_detached "$dev_vg_lv3" $PWD2 "$CRYPT_NAME2" "$dev_vg_lv2"
		test_xfs_detached "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"
		crypt_close "$CRYPT_NAME2"
	fi
fi

if check_missing reiserfs; then
	test_reiserfs_resize "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	test_reiserfs_inactive "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"

	crypt_open_plain "$dev_vg_lv3" $PWD3 "$CRYPT_NAME_PLAIN"
	test_reiserfs_plain "$vg_lv3" "$dev_vg_lv3" "$CRYPT_DEV_PLAIN" "$CRYPT_NAME_PLAIN"
	crypt_close "$CRYPT_NAME_PLAIN"

	lvresize -f -L100M $vg_lv
	cryptsetup resize $CRYPT_NAME
	test_reiserfs_small_shrink "$vg_lv" "$dev_vg_lv" "$CRYPT_DEV"
	lvresize -f -L25M $vg_lv
	cryptsetup resize $CRYPT_NAME

	if [ -z "$SKIP_DETACHED" ]; then
		crypt_open_detached "$dev_vg_lv3" $PWD2 "$CRYPT_NAME2" "$dev_vg_lv2"
		test_reiserfs_detached "$vg_lv2" "$dev_vg_lv2" "$CRYPT_DEV2" "$CRYPT_NAME2"
		crypt_close "$CRYPT_NAME2"
	fi
fi

crypt_close "$CRYPT_NAME"

vgremove -ff $vg
