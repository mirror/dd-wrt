#!/usr/bin/env bash

# Copyright (C) 2008 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise some vgchange diagnostics'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 4

pvcreate --metadatacopies 0 "$dev1"
vgcreate $SHARED -s 4M $vg "$dev1" "$dev2" "$dev3"

# cannot change anything in exported vg
vgexport $vg
fail vgchange -ay $vg
fail vgchange -p 8 $vg
fail vgchange -x n $vg
fail vgchange --addtag tag $vg
fail vgchange --deltag tag $vg
fail vgchange -s 4k $vg
fail vgchange --uuid $vg
fail vgchange --alloc anywhere $vg
vgimport $vg

# unsupported combinations of options...
invalid vgchange --ignorelockingfailure --uuid $vg
invalid vgchange --sysinit --alloc normal $vg
invalid vgchange --sysinit --poll y $vg
invalid vgchange -an --poll y $vg
invalid vgchange -an --monitor y $vg
invalid vgchange -ay --refresh $vg

vgdisplay $vg

# vgchange -p MaxPhysicalVolumes (bz202232)
check vg_field $vg max_pv 0
vgchange -p 128 $vg
check vg_field $vg max_pv 128

pv_count=$(get vg_field $vg pv_count)
not vgchange -p 2 $vg 2>err
grep "MaxPhysicalVolumes is less than the current number $pv_count of PVs for" err
check vg_field $vg max_pv 128

# try some numbers around MAX limit (uint32)
vgchange -p 4294967295 $vg
invalid vgchange -p 4294967296 $vg
invalid vgchange -p 18446744073709551615 $vg
invalid vgchange -p 18446744073709551616 $vg
check vg_field $vg max_pv 4294967295

# vgchange -l MaxLogicalVolumes
check vg_field $vg max_lv 0
invalid vgchange -l -128 $vg
vgchange -l 4294967295 $vg
invalid vgchange -l 4294967296 $vg
invalid vgchange -l 18446744073709551615 $vg
invalid vgchange -l 18446744073709551616 $vg
check vg_field $vg max_lv 4294967295
vgchange -l 128 $vg
check vg_field $vg max_lv 128

# vgchange -s
lvcreate -l4 -n $lv1 $vg
lvcreate -l4 -n $lv2 $vg
SIZELV2=$(get lv_field $vg/$lv2 size)
check lv_field $vg/$lv2 seg_size_pe "4"
vgchange -s 4K $vg
check vg_field $vg vg_extent_size "4.00k"
check lv_field $vg/$lv2 size "$SIZELV2"
check lv_field $vg/$lv2 seg_size_pe "4096"


lv_count=$(get vg_field $vg lv_count)
not vgchange -l 1 $vg 2>err
grep "MaxLogicalVolume is less than the current number $lv_count of LVs for"  err
check vg_field $vg max_lv 128

# check non-resizebility
fail vgchange -x y $vg
check vg_attr_bit resizeable $vg "z"
vgchange -x n $vg
check vg_attr_bit resizeable $vg "-"
fail vgchange -x n $vg
fail vgextend $vg "$dev4"
vgremove -ff $vg
