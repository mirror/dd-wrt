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

test_description='Exercise toollib process_each_pv'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 14

#
# process_each_pv is used by a number of pv commands:
# pvdisplay
# pvresize
# pvs
# vgreduce
#


#
# set up
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

#
# test pvresize without orphans and and without non-pv devs
#

# For pvs in vgs, pvresize setphysicalvolumesize does not give us
# the size requested, but reduces the requested size by some the
# amount for alignment, metadata areas and pv headers.  So, when we resize
# to 30M, the result is 28M, and when we resize to 20M, the result is 16M.
# For orphans, the resulting size is the same as requested.
# It suspect that these reduction amounts might be inconsistent, and
# depend on other changing factors, so it may be that we eventually
# want to give up checking the exact resulting size, but just check
# that the result is less than the original size.

old_request="30.00m"
old_reduced="28.00m"
new_request="20.00m"
new_reduced="16.00m"

pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced

# one pv
pvresize --setphysicalvolumesize $new_request -y "$dev10"
check pv_field "$dev10" pv_size $new_reduced
# unchanged
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# two pvs in separate vgs
pvresize --setphysicalvolumesize $new_request -y "$dev2" "$dev6"
check pv_field "$dev2" pv_size $new_reduced
check pv_field "$dev6" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# one tag on one pv
pvresize --setphysicalvolumesize $new_request -y @V2D4
check pv_field "$dev4" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# one tag on all pvs in one vg
pvresize --setphysicalvolumesize $new_request -y @V3
check pv_field "$dev6" pv_size $new_reduced
check pv_field "$dev7" pv_size $new_reduced
check pv_field "$dev8" pv_size $new_reduced
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# one tag on some pvs in one vg
pvresize --setphysicalvolumesize $new_request -y @V2D45
check pv_field "$dev4" pv_size $new_reduced
check pv_field "$dev5" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# one tag on multiple pvs in separate vgs
pvchange --addtag V12 "$dev10" "$dev2" "$dev3" "$dev4" "$dev5"
pvresize --setphysicalvolumesize $new_request -y @V12
check pv_field "$dev10" pv_size $new_reduced
check pv_field "$dev2" pv_size $new_reduced
check pv_field "$dev3" pv_size $new_reduced
check pv_field "$dev4" pv_size $new_reduced
check pv_field "$dev5" pv_size $new_reduced
# unchanged
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# one pv and one tag on different pv
pvresize --setphysicalvolumesize $new_request -y "$dev10" @V3D9
check pv_field "$dev10" pv_size $new_reduced
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# redundant pv and tag
pvresize --setphysicalvolumesize $new_request -y "$dev9" @V3D9
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"

# two tags on pvs in separate vgs
pvresize --setphysicalvolumesize $new_request -y @V3D9 @V2D3
check pv_field "$dev9" pv_size $new_reduced
check pv_field "$dev3" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"


#
# test pvresize with orphans
#

old_request="30.00m"
old_reduced="28.00m"
old_orphan="30.00m"
new_request="20.00m"
new_reduced="16.00m"
new_orphan="20.00m"

pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan

# one orphan
pvresize --setphysicalvolumesize $new_request -y "$dev11"
check pv_field "$dev11" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# two orphans
pvresize --setphysicalvolumesize $new_request -y "$dev11" "$dev14"
check pv_field "$dev11" pv_size $new_orphan
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one orphan, one tag
pvresize --setphysicalvolumesize $new_request -y @V3D9 "$dev14"
check pv_field "$dev9" pv_size $new_reduced
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one pv, one orphan, one tag
pvresize --setphysicalvolumesize $new_request -y @V3D9 "$dev14" "$dev10"
check pv_field "$dev9" pv_size $new_reduced
check pv_field "$dev10" pv_size $new_reduced
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"


#
# test pvresize with non-pv devs
#

# one dev (non-pv)
not pvresize --setphysicalvolumesize $new_request -y "$dev13"
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan

# one orphan and one dev (non-pv)
not pvresize --setphysicalvolumesize $new_request -y "$dev14" "$dev13"
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one pv and one dev (non-pv)
not pvresize --setphysicalvolumesize $new_request -y "$dev9" "$dev13"
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one tag and one dev (non-pv)
not pvresize --setphysicalvolumesize $new_request -y @V3D9 "$dev13"
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one pv, one orphan, one tag, one dev
not pvresize --setphysicalvolumesize $new_request -y @V3D9 "$dev13" "$dev14" "$dev10"
check pv_field "$dev9" pv_size $new_reduced
check pv_field "$dev10" pv_size $new_reduced
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"


#
# pvresize including pvs without mdas
#

pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan

# one pv without mda
pvresize --setphysicalvolumesize $new_request -y "$dev2"
check pv_field "$dev2" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# two pvs without mdas
pvresize --setphysicalvolumesize $new_request -y "$dev6" "$dev7"
check pv_field "$dev6" pv_size $new_reduced
check pv_field "$dev7" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one pv with mda and one pv without mda
pvresize --setphysicalvolumesize $new_request -y "$dev8" "$dev9"
check pv_field "$dev8" pv_size $new_reduced
check pv_field "$dev9" pv_size $new_reduced
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one orphan with mda
pvresize --setphysicalvolumesize $new_request -y "$dev11"
check pv_field "$dev11" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev14" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one orphan without mda
pvresize --setphysicalvolumesize $new_request -y "$dev14"
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
check pv_field "$dev11" pv_size $old_orphan
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one orphan with mda and one orphan without mda
pvresize --setphysicalvolumesize $new_request -y "$dev14" "$dev11"
check pv_field "$dev11" pv_size $new_orphan
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
check pv_field "$dev8" pv_size $old_reduced
check pv_field "$dev9" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"

# one pv with mda and one pv without mda, and
# one orphan with mda and one orphan without mda
pvresize --setphysicalvolumesize $new_request -y "$dev8" "$dev9" "$dev14" "$dev11"
check pv_field "$dev8" pv_size $new_reduced
check pv_field "$dev9" pv_size $new_reduced
check pv_field "$dev11" pv_size $new_orphan
check pv_field "$dev14" pv_size $new_orphan
# unchanged
check pv_field "$dev10" pv_size $old_reduced
check pv_field "$dev2" pv_size $old_reduced
check pv_field "$dev3" pv_size $old_reduced
check pv_field "$dev4" pv_size $old_reduced
check pv_field "$dev5" pv_size $old_reduced
check pv_field "$dev6" pv_size $old_reduced
check pv_field "$dev7" pv_size $old_reduced
# reset back to old size
pvresize --setphysicalvolumesize $old_request -y "$dev10" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6" "$dev7" "$dev8" "$dev9"
pvresize --setphysicalvolumesize $old_request -y "$dev11" "$dev14"
