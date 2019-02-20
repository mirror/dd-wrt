#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
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

aux prepare_devs 1 8

pvcreate --setphysicalvolumesize 8m --metadatacopies 2 "$dev1"
check pv_field "$dev1" pv_size 8.00m
check pv_field "$dev1" pv_mda_count 2
pvs "$dev1"

pvresize --setphysicalvolumesize 4m -y "$dev1"
check pv_field "$dev1" pv_size 4.00m
check pv_field "$dev1" pv_mda_count 2
pvs "$dev1"

# Check physical size is checked agains metadatasize
pvcreate --metadatasize 2m --metadatacopies 1 -y "$dev1"
not pvresize --setphysicalvolumesize 2m -y "$dev1" |& tee out
grep "Size must exceed physical extent start of 6144 sectors on" out
# 3MiB shall pass with 1M default alignment
pvresize --setphysicalvolumesize 3m -y "$dev1"
check pv_field "$dev1" pv_size 3.00m
check pv_field "$dev1" pv_mda_count 1
pvs "$dev1"
