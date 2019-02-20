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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 4 12

vgcreate $SHARED -s 4m $vg1 "$dev1" "$dev2"
vgcreate $SHARED -s 4m $vg2 "$dev3" "$dev4"

# vg1/lv1 mapped onto dev1
lvcreate -l1 -n "lv1" $vg1 "$dev1"

# vg1/lv2 mapped onto dev1 and dev2 (2 segments)
lvcreate -l3 -n "lv2" $vg1 "$dev1" "$dev2"

# vg2/lv3 mapped onto dev3
lvcreate -l1 -n "lv3" $vg2 "$dev3"

# vg2/lv4 mapped onto dev3
lvcreate -l1 -n "lv4" $vg2 "$dev3" "$dev4"

# vg2/lv1 mapped onto "$dev4" (same LV name as vg1/lv1)
lvcreate -l1 -n "lv1" $vg2 "$dev4"

###########################################
# exercise process_each_vg with selection #
###########################################

# select contains VGS field
# direct vg name match
vgchange --addtag 101 -S "vg_name=$vg1"
check vg_field $vg1 vg_tags 101
not check vg_field $vg2 vg_tags 101
vgchange --deltag 101

# select contains LVS fiels
vgchange --addtag 102 -S "lv_name=lv2"
check vg_field $vg1 vg_tags 102
not check vg_field $vg2 vg_tags 102
vgchange --deltag 102
vgchange --addtag 103 -S "lv_name=lv1"
check vg_field $vg1 vg_tags 103
check vg_field $vg2 vg_tags 103
vgchange --deltag 103

# select contains SEGS field
vgchange --addtag 104 -S 'seg_start=8m'
check vg_field $vg1 vg_tags 104
not check vg_field $vg2 vg_tags 104
vgchange --deltag 104
vgchange --addtag 105 -S "seg_start=0m"
check vg_field $vg1 vg_tags 105
check vg_field $vg2 vg_tags 105
vgchange --deltag 105

# select contains PVS field
vgchange --addtag 106 -S pv_name="$dev1"
check vg_field $vg1 vg_tags 106
not check vg_field $vg2 vg_tags 106
vgchange --deltag 106
vgchange --addtag 107 -S "pv_size>0m"
check vg_field $vg1 vg_tags 107
check vg_field $vg2 vg_tags 107
vgchange --deltag 107

# select contains PVSEGS field
vgchange --addtag 108 -S "pvseg_size=2"
check vg_field $vg1 vg_tags 108
not check vg_field $vg2 vg_tags 108
vgchange --deltag 108
vgchange --addtag 109 -S "pvseg_size=1"
check vg_field $vg1 vg_tags 109
check vg_field $vg2 vg_tags 109
vgchange --deltag 109

# if VG name or tag is supplied together with the
# selection, the result is an intersection of both
vgchange --addtag 110 -S "vg_name=$vg1" $vg2
not check vg_field $vg1 vg_tags 110
not check vg_field $vg2 vg_tags 110
vgchange --deltag 110
vgchange --addtag 111 -S "vg_name=$vg1" $vg1
check vg_field $vg1 vg_tags 111
not check vg_field $vg2 vg_tags 111
vgchange --deltag 111
vgchange --addtag "tag" $vg1
vgchange --addtag 112 -S "vg_name=$vg2" @tag
not check vg_field $vg1 vg_tags "tag,112"
not check vg_field $vg2 vg_tags "tag,112"
vgchange --deltag 112
vgchange --addtag 113 -S "vg_name=$vg1" @tag
check vg_field $vg1 vg_tags "113,tag"
not check vg_field $vg2 vg_tags "113,tag"
vgchange --deltag 113 --deltag tag

###########################################
# exercise process_each_lv with selection #
###########################################

# select contains VGS field
lvchange --addtag 201 -S "vg_name=$vg1"
check lv_field $vg1/lv1 lv_tags 201
check lv_field $vg1/lv2 lv_tags 201
not check lv_field $vg2/lv3 lv_tags 201
not check lv_field $vg2/lv4 lv_tags 201
not check lv_field $vg2/lv1 lv_tags 201
lvchange --deltag 201 $vg1 $vg2

# select contains LVS fiels
lvchange --addtag 202 -S "lv_name=lv2"
not check lv_field $vg1/lv1 lv_tags 202
check lv_field $vg1/lv2 lv_tags 202
not check lv_field $vg2/lv3 lv_tags 202
not check lv_field $vg2/lv4 lv_tags 202
not check lv_field $vg2/lv1 lv_tags 202
lvchange --deltag 202 $vg1 $vg2
lvchange --addtag 203 -S "lv_name=lv1"
check lv_field $vg1/lv1 lv_tags 203
not check lv_field $vg1/lv2 lv_tags 203
not check lv_field $vg2/lv3 lv_tags 203
not check lv_field $vg2/lv4 lv_tags 203
check lv_field $vg2/lv1 lv_tags 203
lvchange --deltag 203 $vg1 $vg2

# select contains SEGS field
lvchange --addtag 204 -S "seg_start=8m"
not check lv_field $vg1/lv1 lv_tags 204
check lv_field $vg1/lv2 lv_tags 204
not check lv_field $vg2/lv3 lv_tags 204
not check lv_field $vg2/lv4 lv_tags 204
not check lv_field $vg2/lv1 lv_tags 204
lvchange --deltag 204 $vg1 $vg2

# select contains PVS field - COMBINATION NOT ALLOWED!
lvchange --addtag 205 -S pv_name="$dev1" 2>err
grep "Can't report LV and PV fields at the same time" err
grep "Selection failed for LV" err
not check lv_field $vg1/lv1 lv_tags 205
not check lv_field $vg1/lv2 lv_tags 205
not check lv_field $vg2/lv3 lv_tags 205
not check lv_field $vg2/lv4 lv_tags 205
not check lv_field $vg2/lv1 lv_tags 205

# select contains PVSEGS field - COMBINATION NOT ALLOWED!
lvchange --addtag 206 -S "pvseg_start>=0" 2>err
grep "Can't report LV and PV fields at the same time" err
grep "Selection failed for LV" err
not check lv_field $vg1/lv1 lv_tags 206
not check lv_field $vg1/lv2 lv_tags 206
not check lv_field $vg2/lv3 lv_tags 206
not check lv_field $vg2/lv4 lv_tags 206
not check lv_field $vg2/lv1 lv_tags 206

# if LV name or tag is supplied together with the
# selection, the result is an intersection of both
lvchange --addtag 207 -S "lv_name=lv2" $vg1/lv1
not check lv_field $vg1/lv1 lv_tags 207
not check lv_field $vg1/lv2 lv_tags 207
not check lv_field $vg2/lv3 lv_tags 207
not check lv_field $vg2/lv4 lv_tags 207
not check lv_field $vg2/lv1 lv_tags 207
lvchange --deltag 207 $vg1 $vg2
lvchange --addtag 208 -S "lv_name=lv2" $vg1/lv2
not check lv_field $vg1/lv1 lv_tags 208
check lv_field $vg1/lv2 lv_tags 208
not check lv_field $vg2/lv3 lv_tags 208
not check lv_field $vg2/lv4 lv_tags 208
not check lv_field $vg2/lv1 lv_tags 208
lvchange --deltag 208 $vg1 $vg2
lvchange --addtag "tag" $vg1/lv2
lvchange --addtag 209 -S "lv_name=lv3" @tag
not check lv_field $vg1/lv1 lv_tags "209,tag"
not check lv_field $vg1/lv2 lv_tags "209,tag"
not check lv_field $vg2/lv3 lv_tags "209,tag"
not check lv_field $vg2/lv4 lv_tags "209,tag"
not check lv_field $vg2/lv1 lv_tags "209,tag"
lvchange --deltag 209 $vg1 $vg2
lvchange --addtag 210 -S "lv_name=lv2" @tag
not check lv_field $vg1/lv1 lv_tags "210,tag"
check lv_field $vg1/lv2 lv_tags "210,tag"
not check lv_field $vg2/lv3 lv_tags "210,tag"
not check lv_field $vg2/lv4 lv_tags "210,tag"
not check lv_field $vg2/lv1 lv_tags "210,tag"
lvchange --deltag 210 --deltag tag $vg1 $vg2

###########################################
# exercise process_each_pv with selection #
###########################################

# select contains VGS field
pvchange --addtag 301 -S "vg_name=$vg1"
check pv_field "$dev1" pv_tags 301
check pv_field "$dev2" pv_tags 301
not check pv_field "$dev3" pv_tags 301
not check pv_field "$dev4" pv_tags 301
pvchange -a --deltag 301

# select contains LVS field
pvchange --addtag 302 -S "lv_name=lv2"
check pv_field "$dev1" pv_tags 302
check pv_field "$dev2" pv_tags 302
not check pv_field "$dev3" pv_tags 302
not check pv_field "$dev4" pv_tags 302
pvchange -a --deltag 302

# select contains SEGS field
pvchange --addtag 303 -S "seg_start=8m"
check pv_field "$dev1" pv_tags 303
not check pv_field "$dev2" pv_tags 303
not check pv_field "$dev3" pv_tags 303
not check pv_field "$dev4" pv_tags 303
pvchange -a --deltag 303

# select contains PVS field
pvchange --addtag 304 -S pv_name="$dev1"
check pv_field "$dev1" pv_tags 304
not check pv_field "$dev2" pv_tags 304
not check pv_field "$dev3" pv_tags 304
not check pv_field "$dev4" pv_tags 304
pvchange -a --deltag 304

# select contains PVSEGS field
pvchange --addtag 305 -S "pvseg_size=2"
not check pv_field "$dev1" pv_tags 305
check pv_field "$dev2" pv_tags 305
not check pv_field "$dev3" pv_tags 305
not check pv_field "$dev4" pv_tags 305
pvchange -a --deltag 305

# if PV name or tag is supplied together with the
# selection, the result is an intersection of both
pvchange --addtag 306 -S pv_name="$dev1" "$dev2"
not check pv_field "$dev1" pv_tags 306
not check pv_field "$dev2" pv_tags 306
not check pv_field "$dev3" pv_tags 306
not check pv_field "$dev4" pv_tags 306
pvchange -a --deltag 306
pvchange --addtag 307 -S pv_name="$dev1" "$dev1"
check pv_field "$dev1" pv_tags 307
not check pv_field "$dev2" pv_tags 307
not check pv_field "$dev3" pv_tags 307
not check pv_field "$dev4" pv_tags 307
pvchange -a --deltag 307
pvchange --addtag "tag" "$dev1"
pvchange --addtag 308 -S pv_name="$dev2" @tag
not check pv_field "$dev1" pv_tags "308,tag"
not check pv_field "$dev2" pv_tags "308,tag"
not check pv_field "$dev3" pv_tags "308,tag"
not check pv_field "$dev4" pv_tags "308,tag"
pvchange --deltag 308 "$dev1"
pvchange --addtag 309 -S pv_name="$dev1" @tag
check pv_field "$dev1" pv_tags "309,tag"
not check pv_field "$dev2" pv_tags "309,tag"
not check pv_field "$dev3" pv_tags "309,tag"
not check pv_field "$dev4" pv_tags "309,tag"
pvchange -a --deltag 309 --deltag tag

#########################
# special cases to test #
#########################

# if calling vgremove, make sure we're doing selection per-VG, not per-LV
# (vgremove calls process_each_vg with vgremove_single which itself
# iterates over LVs with process_each_lv_in_vg - so internally it actually
# operates per-LV, but we still need the selection to be done per-VG)
vgremove --yes -S 'lv_name=lv2' # should remove whole vg1, not just the lv2
vgremove --yes $vg2
