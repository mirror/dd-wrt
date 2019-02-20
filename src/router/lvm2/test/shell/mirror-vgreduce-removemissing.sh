#!/usr/bin/env bash

# Copyright (C) 2008-2017 Red Hat, Inc. All rights reserved.
# Copyright (C) 2007 NEC Corporation
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description="ensure that 'vgreduce --removemissing' works on mirrored LV"

SKIP_WITH_LVMPOLLD=1

. lib/inittest

list_pvs=()

lv_is_on_ ()
{
	local lv=$vg/$1
	shift
	local list_pvs=( "$@" )

	echo "Check if $lv is exactly on PVs" "${list_pvs[@]}"
	rm -f out1 out2
	printf "%s\n" "${list_pvs[@]}" | sort | uniq > out1

	lvs -a -o+devices $lv
	get lv_devices "$lv" | sort | uniq > out2

	diff --ignore-blank-lines out1 out2
}

mimages_are_on_ ()
{
	local lv=$1
	shift
	local list_pvs=( "$@" )
	local mimages=()
	local i

	echo "Check if mirror images of $lv are on PVs" "${list_pvs[@]}"
	printf "%s\n" "${list_pvs[@]}" | sort | uniq > out1

	get lv_field_lv_ "$vg" lv_name -a | grep "${lv}_mimage_" | tee lvs_log
	test -s lvs_log || return 1

	while IFS= read -r i ; do
		mimages+=( "$i" )
	done < lvs_log

	for i in "${mimages[@]}"; do
		echo "Checking $vg/$i"
		lvs -a -o+devices "$vg/$i"
	done

	for i in "${mimages[@]}"; do
		get lv_devices "$vg/$i"
	done | sort | uniq > out2

	diff --ignore-blank-lines out1 out2
}

mirrorlog_is_on_()
{
	local lv=${1}_mlog
	shift
	lv_is_on_ $lv "$@"
}

lv_is_linear_()
{
	echo "Check if $1 is linear LV (i.e. not a mirror)"
	get lv_field $vg/$1 "stripes,attr" | tee out
	grep "^1 -" out >/dev/null
}

rest_pvs_()
{
	local index=$1
	local num=$2
	local rem=()
	local n
	local dev

	for n in $(seq 1 $(( index - 1 )) ) $(seq $(( index + 1 )) $num); do
		eval "dev=\$dev$n"
		rem+=( "$dev" )
	done

	printf "%s\n" "${rem[@]}"
}

# ---------------------------------------------------------------------
# Initialize PVs and VGs

aux prepare_pvs 5 80
get_devs

vgcreate $SHARED -s 64k "$vg" "${DEVICES[@]}"
BLOCKS=0-7
BLOCKS1=8-15
# ---------------------------------------------------------------------
# Common environment setup/cleanup for each sub testcases

prepare_lvs_()
{
	lvremove -ff $vg
	(dm_table | not grep $vg) || \
		die "ERROR: lvremove did leave some some mappings in DM behind!"
}

check_and_cleanup_lvs_()
{
	lvs -a -o+devices $vg
	prepare_lvs_
}

recover_vg_()
{
	aux enable_dev "$@"
	pvcreate -ff "$@"
	vgextend $vg "$@"
	check_and_cleanup_lvs_
}

#COMM "check environment setup/cleanup"
prepare_lvs_
check_and_cleanup_lvs_

# ---------------------------------------------------------------------
# one of mirror images has failed

#COMM "basic: fail the 2nd mirror image of 2-way mirrored LV"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev3":$BLOCKS
mimages_are_on_ $lv1 "$dev1" "$dev2"
mirrorlog_is_on_ $lv1 "$dev3"
aux disable_dev "$dev2"
vgreduce --removemissing --force $vg
lv_is_linear_ $lv1
lv_is_on_ $lv1 "$dev1"

# "cleanup"
recover_vg_ "$dev2"

# ---------------------------------------------------------------------
# LV has 3 images in flat,
# 1 out of 3 images fails

#COMM test_3way_mirror_fail_1_ <PV# to fail>
test_3way_mirror_fail_1_()
{
	local index=$1

	lvcreate -an -Zn -l2 --type mirror -m2 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev3" "$dev4":$BLOCKS
	mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3"
	mirrorlog_is_on_ $lv1 "$dev4"
	eval aux disable_dev "\$dev$index"
	vgreduce --removemissing --force $vg

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 3 )

	mimages_are_on_ "$lv1" "${list_pvs[@]}"
	mirrorlog_is_on_ $lv1 "$dev4"
}

for n in $(seq 1 3); do
	#COMM fail mirror image $(($n - 1)) of 3-way mirrored LV"
	prepare_lvs_
	test_3way_mirror_fail_1_ $n
	eval recover_vg_ "\$dev$n"
done

# ---------------------------------------------------------------------
# LV has 3 images in flat,
# 2 out of 3 images fail

#COMM test_3way_mirror_fail_2_ <PV# NOT to fail>
test_3way_mirror_fail_2_()
{
	local index=$1

	lvcreate -an -Zn -l2 --type mirror -m2 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev3" "$dev4":$BLOCKS
	mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3"
	mirrorlog_is_on_ $lv1 "$dev4"

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 3 )

	aux disable_dev "${list_pvs[@]}"
	vgreduce --force --removemissing $vg
	lv_is_linear_ $lv1
	eval lv_is_on_ $lv1 "\$dev$n"
}

for n in $(seq 1 3); do
	#COMM fail mirror images other than mirror image $(($n - 1)) of 3-way mirrored LV
	prepare_lvs_
	test_3way_mirror_fail_2_ $n

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$n" 3 )

	recover_vg_ "${list_pvs[@]}"
done

# ---------------------------------------------------------------------
# LV has 4 images, 1 of them is in the temporary mirror for syncing.
# 1 out of 4 images fails

#COMM test_3way_mirror_plus_1_fail_1_ <PV# to fail>
test_3way_mirror_plus_1_fail_1_()
{
	local index=$1

	lvcreate -an -Zn -l2 --type mirror -m2 -n $lv1 $vg "$dev1" "$dev2" "$dev3" "$dev5":$BLOCKS
	lvconvert -m+1 $vg/$lv1 "$dev4"
	check mirror_images_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
	check mirror_log_on $vg $lv1 "$dev5"
	eval aux disable_dev \$dev$index
	vgreduce --removemissing --force $vg

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 4 )

	check mirror_images_on $vg $lv1 "${list_pvs[@]}"
	check mirror_log_on $vg $lv1 "$dev5"
}

for n in $(seq 1 4); do
	#COMM "fail mirror image $(($n - 1)) of 4-way (1 converting) mirrored LV"
	prepare_lvs_
	test_3way_mirror_plus_1_fail_1_ $n
	eval recover_vg_ \$dev$n
done

# ---------------------------------------------------------------------
# LV has 4 images, 1 of them is in the temporary mirror for syncing.
# 3 out of 4 images fail

#COMM test_3way_mirror_plus_1_fail_3_ <PV# NOT to fail>
test_3way_mirror_plus_1_fail_3_()
{
	local index=$1
	local dev

	lvcreate -an -Zn -l2 --type mirror -m2 -n $lv1 $vg "$dev1" "$dev2" "$dev3" "$dev5":$BLOCKS
	lvconvert -m+1 $vg/$lv1 "$dev4"
	check mirror_images_on $vg $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
	check mirror_log_on $vg $lv1 "$dev5"

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 4 )

	aux disable_dev "${list_pvs[@]}"
	vgreduce --removemissing --force $vg
	lvs -a -o+devices $vg
	eval dev=\$dev$n
	check linear $vg $lv1
	check lv_on $vg $lv1 "$dev"
}

for n in $(seq 1 4); do
	#COMM "fail mirror images other than mirror image $(($n - 1)) of 4-way (1 converting) mirrored LV"
	prepare_lvs_
	test_3way_mirror_plus_1_fail_3_ $n

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$n" 4 )

	recover_vg_ "${list_pvs[@]}"
done

# ---------------------------------------------------------------------
# LV has 4 images, 2 of them are in the temporary mirror for syncing.
# 1 out of 4 images fail

# test_2way_mirror_plus_2_fail_1_ <PV# to fail>
test_2way_mirror_plus_2_fail_1_()
{
	local index=$1

	lvcreate -an -Zn -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
	lvconvert -m+2 $vg/$lv1 "$dev3" "$dev4"
	mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
	mirrorlog_is_on_ $lv1 "$dev5"
	eval aux disable_dev \$dev$n
	vgreduce --removemissing --force $vg

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 4 )

	mimages_are_on_ "$lv1" "${list_pvs[@]}"
	mirrorlog_is_on_ $lv1 "$dev5"
}

for n in $(seq 1 4); do
	#COMM "fail mirror image $(($n - 1)) of 4-way (2 converting) mirrored LV"
	prepare_lvs_
	test_2way_mirror_plus_2_fail_1_ $n
	eval recover_vg_ "\$dev$n"
done

# ---------------------------------------------------------------------
# LV has 4 images, 2 of them are in the temporary mirror for syncing.
# 3 out of 4 images fail

# test_2way_mirror_plus_2_fail_3_ <PV# NOT to fail>
test_2way_mirror_plus_2_fail_3_()
{
	local index=$1
	local dev

	lvcreate -an -Zn -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
	lvconvert -m+2 $vg/$lv1 "$dev3" "$dev4"
	mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3" "$dev4"
	mirrorlog_is_on_ $lv1 "$dev5"

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$index" 4 )

	aux disable_dev "${list_pvs[@]}"
	vgreduce --removemissing --force $vg
	lvs -a -o+devices $vg
	eval dev=\$dev$n
	not mimages_are_on_ $lv1 "$dev"
	lv_is_on_ $lv1 "$dev"
	not mirrorlog_is_on_ $lv1 "$dev5"
}

for n in $(seq 1 4); do
	#COMM "fail mirror images other than mirror image $(($n - 1)) of 4-way (2 converting) mirrored LV"
	prepare_lvs_
	test_2way_mirror_plus_2_fail_3_ $n

	list_pvs=(); while IFS= read -r line ; do
		list_pvs+=( "$line" )
	done < <( rest_pvs_ "$n" 4 )

	recover_vg_ "${list_pvs[@]}"
done

# ---------------------------------------------------------------------
# log device is gone (flat mirror and stacked mirror)

#COMM "fail mirror log of 2-way mirrored LV"
prepare_lvs_
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
mimages_are_on_ $lv1 "$dev1" "$dev2"
mirrorlog_is_on_ $lv1 "$dev5"
aux disable_dev "$dev5"
vgreduce --removemissing --force $vg
mimages_are_on_ $lv1 "$dev1" "$dev2"
not mirrorlog_is_on_ $lv1 "$dev5"
recover_vg_ "$dev5"

#COMM "fail mirror log of 3-way (1 converting) mirrored LV"
prepare_lvs_
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
lvconvert -m+1 $vg/$lv1 "$dev3"
mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3"
mirrorlog_is_on_ $lv1 "$dev5"
aux disable_dev "$dev5"
vgreduce --removemissing --force $vg
mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3"
not mirrorlog_is_on_ $lv1 "$dev5"
recover_vg_ "$dev5"

# ---------------------------------------------------------------------
# all images are gone (flat mirror and stacked mirror)

#COMM "fail all mirror images of 2-way mirrored LV"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
mimages_are_on_ $lv1 "$dev1" "$dev2"
mirrorlog_is_on_ $lv1 "$dev5"
aux disable_dev "$dev1" "$dev2"
vgreduce --removemissing --force $vg
not lvs $vg/$lv1
recover_vg_ "$dev1" "$dev2"

#COMM "fail all mirror images of 3-way (1 converting) mirrored LV"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
lvconvert -m+1 $vg/$lv1 "$dev3"
mimages_are_on_ $lv1 "$dev1" "$dev2" "$dev3"
mirrorlog_is_on_ $lv1 "$dev5"
aux disable_dev "$dev1" "$dev2" "$dev3"
vgreduce --removemissing --force $vg
not lvs $vg/$lv1
recover_vg_ "$dev1" "$dev2" "$dev3"

# ---------------------------------------------------------------------
# Multiple LVs

#COMM "fail a mirror image of one of mirrored LV"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv2 $vg "$dev3" "$dev4" "$dev5":$BLOCKS1
mimages_are_on_ $lv1 "$dev1" "$dev2"
mimages_are_on_ $lv2 "$dev3" "$dev4"
mirrorlog_is_on_ $lv1 "$dev5"
mirrorlog_is_on_ $lv2 "$dev5"
aux disable_dev "$dev2"
vgreduce --removemissing --force $vg
mimages_are_on_ $lv2 "$dev3" "$dev4"
mirrorlog_is_on_ $lv2 "$dev5"
lv_is_linear_ $lv1
lv_is_on_ $lv1 "$dev1"
recover_vg_ "$dev2"

#COMM "fail mirror images, one for each mirrored LV"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv2 $vg "$dev3" "$dev4" "$dev5":$BLOCKS1
mimages_are_on_ $lv1 "$dev1" "$dev2"
mimages_are_on_ $lv2 "$dev3" "$dev4"
mirrorlog_is_on_ $lv1 "$dev5"
mirrorlog_is_on_ $lv2 "$dev5"
aux disable_dev "$dev2"
aux disable_dev "$dev4"
vgreduce --removemissing --force $vg
lv_is_linear_ $lv1
lv_is_on_ $lv1 "$dev1"
lv_is_linear_ $lv2
lv_is_on_ $lv2 "$dev3"
recover_vg_ "$dev2" "$dev4"

# ---------------------------------------------------------------------
# no failure

#COMM "no failures"
prepare_lvs_
lvcreate -an -Zn -l2 --type mirror -m1 --nosync -n $lv1 $vg "$dev1" "$dev2" "$dev5":$BLOCKS
mimages_are_on_ $lv1 "$dev1" "$dev2"
mirrorlog_is_on_ $lv1 "$dev5"
vgreduce --removemissing --force $vg
mimages_are_on_ $lv1 "$dev1" "$dev2"
mirrorlog_is_on_ $lv1 "$dev5"
check_and_cleanup_lvs_

# ---------------------------------------------------------------------
