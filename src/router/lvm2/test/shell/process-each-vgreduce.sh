#!/usr/bin/env bash

# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise toollib process_each_pv with vgreduce'


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 14

#
# set up
#
# FIXME: some of the setup may not be used by the tests
# since this was split out from process-each-pv, where
# some of the setup was used by other tests that only
# remain in process-each-pv.
#
# use use dev10 instead of dev1 because simple grep for
# dev1 matchines dev10,dev11,etc
#

vgcreate $SHARED $vg1 "$dev10"
vgcreate $SHARED $vg2 "$dev2" "$dev3" "$dev4" "$dev5"
vgcreate $SHARED $vg3 "$dev6" "$dev7" "$dev8" "$dev9"

pvchange --addtag V2D3 "$dev3"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"

pvchange --addtag V3 "$dev6" "$dev7" "$dev8" "$dev9"
pvchange --addtag V3D9 "$dev9"

# orphan
pvcreate "$dev11"

# dev (a non-pv device)
pvcreate "$dev12"
pvremove "$dev12"

# dev13 is intentionally untouched so we can
# test that it is handled appropriately as a non-pv

# orphan
pvcreate "$dev14"


# fail without dev
not vgreduce $vg2


# fail with dev and -a
not vgreduce $vg2 "$dev2" -a
check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3


# remove one pv
vgreduce $vg2 "$dev2"
not check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev2"


# remove two pvs
vgreduce $vg2 "$dev2" "$dev3"
not check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev2" "$dev3"
pvchange --addtag V2D3 "$dev3"


# remove one pv with tag
vgreduce $vg2 @V2D3
check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev3"
pvchange --addtag V2D3 "$dev3"


# remove two pvs, each with different tag
vgreduce $vg2 @V2D3 @V2D4
check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
not check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev3" "$dev4"
pvchange --addtag V2D3 "$dev3"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"


# remove two pvs, both with same tag
vgreduce $vg2 @V2D45
check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
not check pv_field "$dev4" vg_name $vg2
not check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev4" "$dev5"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"


# remove two pvs, one by name, one by tag
vgreduce $vg2 "$dev2" @V2D3
not check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev2" "$dev3"
pvchange --addtag V2D3 "$dev3"


# remove one pv by tag, where another vg has a pv with same tag
pvchange --addtag V2D5V3D9 "$dev5"
pvchange --addtag V2D5V3D9 "$dev9"
vgreduce $vg2 @V2D5V3D9
check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
not check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev5"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"


# fail to remove last pv (don't know which will be last)
not vgreduce -a $vg2
# reset
vgremove $vg2
vgcreate $SHARED $vg2 "$dev2" "$dev3" "$dev4" "$dev5"
pvchange --addtag V2D3 "$dev3"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"


# lvcreate on one pv to make it used
# remove all unused pvs
lvcreate -n $lv1 -l 2 $vg2 "$dev2"
not vgreduce -a $vg2
check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
not check pv_field "$dev4" vg_name $vg2
not check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev3" "$dev4" "$dev5"
pvchange --addtag V2D3 "$dev3"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"
lvchange -an $vg2/$lv1
lvremove $vg2/$lv1


#
# tests including pvs without mdas
#

# remove old config
vgremove $vg1
vgremove $vg2
vgremove $vg3
pvremove "$dev11"
pvremove "$dev14"

# new config with some pvs that have zero mdas

# for vg1
pvcreate "$dev10"

# for vg2
pvcreate "$dev2" --metadatacopies 0
pvcreate "$dev3"
pvcreate "$dev4"
pvcreate "$dev5"

# for vg3
pvcreate "$dev6" --metadatacopies 0
pvcreate "$dev7" --metadatacopies 0
pvcreate "$dev8" --metadatacopies 0
pvcreate "$dev9"

# orphan with mda
pvcreate "$dev11"
# orphan without mda
pvcreate "$dev14" --metadatacopies 0

# non-pv devs
# dev12
# dev13

vgcreate $SHARED $vg1 "$dev10"
vgcreate $SHARED $vg2 "$dev2" "$dev3" "$dev4" "$dev5"
vgcreate $SHARED $vg3 "$dev6" "$dev7" "$dev8" "$dev9"

pvchange --addtag V2D3 "$dev3"
pvchange --addtag V2D4 "$dev4"
pvchange --addtag V2D45 "$dev4"
pvchange --addtag V2D5 "$dev5"
pvchange --addtag V2D45 "$dev5"

pvchange --addtag V3 "$dev6" "$dev7" "$dev8" "$dev9"
pvchange --addtag V3D8 "$dev8"
pvchange --addtag V3D9 "$dev9"


#
# vgreduce including pvs without mdas
#

# remove pv without mda
vgreduce $vg2 "$dev2"
not check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev2"

# remove pv with mda and pv without mda
vgreduce $vg2 "$dev2" "$dev3"
not check pv_field "$dev2" vg_name $vg2
not check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
# reset
vgextend $vg2 "$dev2"
vgextend $vg2 "$dev3"

# fail to remove only pv with mda
not vgreduce $vg3 "$dev9"
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2

# remove by tag a pv without mda
vgreduce $vg3 @V3D8
check pv_field "$dev6" vg_name $vg3
check pv_field "$dev7" vg_name $vg3
not check pv_field "$dev8" vg_name $vg3
check pv_field "$dev9" vg_name $vg3
check pv_field "$dev2" vg_name $vg2
check pv_field "$dev3" vg_name $vg2
check pv_field "$dev4" vg_name $vg2
check pv_field "$dev5" vg_name $vg2
# reset
vgextend $vg3 "$dev8"

vgremove $vg1 $vg2 $vg3
