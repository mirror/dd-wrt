#!/usr/bin/env bash

# Copyright (C) 017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA2110-1301 USA


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_raid 1 9 1 || skip

aux prepare_vg 6

function check_sub_lvs
{
	local vg=$1
	local lv=$2
	local end=$3

	for s in $(seq 0 "$end")
	do
		check lv_exists $vg ${lv}_rmeta_$s
		check lv_exists $vg ${lv}_rimage_$s
	done
}

function check_no_sub_lvs
{
	local vg=$1
	local lv=$2
	local start=$3
	local end=$4

	for s in $(seq "$start" "$end")
	do
		check lv_not_exists $vg ${lv}_rmeta_$s
		check lv_not_exists $vg ${lv}_rimage_$s
	done
}

# Check takover upconversion fails allocation errors nicely without leaving image pair remnants behind

# 6-way striped: neither conversion to raid5 nor raid6 possible
lvcreate -aey --yes --stripes 6 --size 4M --name $lv1 $vg
not lvconvert --yes --type raid4 $vg/$lv1
check lv_field $vg/$lv1 segtype "striped"
check_no_sub_lvs $vg $lv1 0 5

not lvconvert --yes --type raid5 $vg/$lv1
check lv_field $vg/$lv1 segtype "striped"
check_no_sub_lvs $vg $lv1 0 5

not lvconvert --yes --type raid6 $vg/$lv1
check lv_field $vg/$lv1 segtype "striped"
check_no_sub_lvs $vg $lv1 0 5

# raid0_meta conversion is possible
lvconvert --yes --type raid0_meta $vg/$lv1
check lv_field $vg/$lv1 segtype "raid0_meta"
check_sub_lvs $vg $lv1 0 5

lvremove -y $vg

# 5-way striped: conversion to raid5 possible but not to raid6
lvcreate -aey --stripes 5 --size 4M --name $lv1 $vg
not lvconvert --yes --type raid6 $vg/$lv1
check lv_field $vg/$lv1 segtype "striped"
check_no_sub_lvs $vg $lv1 0 5

lvconvert --yes --type raid5 $vg/$lv1
check lv_field $vg/$lv1 segtype "raid5_n"
check lv_field $vg/$lv1 stripes 6
check lv_field $vg/$lv1 datastripes 5
check_sub_lvs $vg $lv1 0 5

lvremove -y $vg

# 4-way striped: conversion to raid5 and raid6 possible
lvcreate -aey --stripes 4 --size 4M --name $lv1 $vg
lvconvert --yes --type raid5 $vg/$lv1
check lv_field $vg/$lv1 segtype "raid5_n"
check lv_field $vg/$lv1 stripes 5
check lv_field $vg/$lv1 datastripes 4
check_sub_lvs $vg $lv1 0 4
check_no_sub_lvs $vg $lv1 5 5

lvremove -y $vg

lvcreate -aey --stripes 4 --size 4M --name $lv1 $vg
lvconvert --yes --type raid6 $vg/$lv1
check lv_field $vg/$lv1 segtype "raid6_n_6"
check lv_field $vg/$lv1 stripes 6
check lv_field $vg/$lv1 datastripes 4
check_sub_lvs $vg $lv1 0 5

vgremove -ff $vg
