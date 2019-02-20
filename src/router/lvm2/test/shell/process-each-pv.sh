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

test_description='Exercise toollib process_each_pv'
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 14

#
# process_each_pv is used by a number of pv commands:
# pvdisplay
# pvresize
# pvs
#
# process-each-pvresize.sh covers pvresize.
# process-each-vgreduce.sh covers vgreduce.
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
# test pvdisplay
#

# pv in vg
pvdisplay -s "$dev10" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv not in vg (one orphan)
pvdisplay -s "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# dev is not a pv
not pvdisplay -s "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# two pvs in different vgs
pvdisplay -s "$dev10" "$dev2" | tee err
grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# -a is invalid when used alone
not pvdisplay -a | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one pv and one orphan
pvdisplay -s "$dev10" "$dev11" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one pv and one dev (dev refers to a non-pv device)
not pvdisplay -s "$dev10" "$dev12" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one orphan and one dev
not pvdisplay -s "$dev11" "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# all pvs (pvs in vgs and orphan pvs)
pvdisplay -s | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# all devs (pvs in vgs, orphan pvs, and devs)
pvdisplay -a -C | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
grep "$dev12" err
grep "$dev13" err
grep "$dev14" err

# pv and orphan and dev
not pvdisplay -s "$dev9" "$dev11" "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# -s option not allowed with -a -C
not pvdisplay -s -a -C | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv and all (all ignored)
pvdisplay -a -C "$dev9" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan and all (all ignored)
pvdisplay -a -C "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one tag
pvdisplay -s @V2D3 | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# two tags
pvdisplay -s @V2D3 @V2D45 | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and pv
pvdisplay -s @V2D3 "$dev4" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and orphan
pvdisplay -s @V2D3 "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and dev
not pvdisplay -s @V2D3 "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and all (all ignored)
pvdisplay @V2D3 -a -C | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and pv redundant
pvdisplay -s @V2D3 "$dev3" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err


#
# test pvs
#

# pv in vg
pvs "$dev10" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv not in vg (one orphan)
pvs "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# dev is not a pv
not pvs "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# two pvs in different vgs
pvs "$dev10" "$dev2" | tee err
grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one pv and one orphan
pvs "$dev10" "$dev11" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one pv and one dev
not pvs "$dev10" "$dev12" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one orphan and one dev
not pvs "$dev11" "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# all pvs (pvs in vgs and orphan pvs)
pvs | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# all devs (pvs in vgs, orphan pvs, and devs)
pvs -a | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
grep "$dev12" err
grep "$dev13" err
grep "$dev14" err

# pv and orphan and dev
not pvs "$dev9" "$dev11" "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv and all (all ignored)
pvs -a "$dev9" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan and all (all ignored)
pvs -a "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# one tag
pvs @V2D3 | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# two tags
pvs @V2D3 @V2D45 | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and pv
pvs @V2D3 "$dev4" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and orphan
pvs @V2D3 "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and dev
not pvs @V2D3 "$dev12" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and all (all ignored)
pvs @V2D3 -a | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag and pv redundant
pvs @V2D3 "$dev3" | tee err
not grep "$dev10" err
not grep "$dev2" err
grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err


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
# pvdisplay including pvs without mdas
#

# pv with mda
pvdisplay -s "$dev10" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv without mda
pvdisplay -s "$dev2" | tee err
not grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan with mda
pvdisplay -s "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan without mda
pvdisplay -s "$dev14" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# pv with mda, pv without mda, orphan with mda, orphan without mda
pvdisplay -s "$dev10" "$dev2" "$dev11" "$dev14" | tee err
grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# tag refering to pv with mda and pv without mda
pvdisplay -s @V3 | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag refering to one pv without mda
pvdisplay -s @V3D8 | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# all pvs (pvs in vgs and orphan pvs)
pvdisplay -s | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# all devs (pvs in vgs, orphan pvs, and devs)
pvdisplay -a -C | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
grep "$dev12" err
grep "$dev13" err
grep "$dev14" err

#
# pvs including pvs without mdas
#

# pv with mda
pvs "$dev10" | tee err
grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# pv without mda
pvs "$dev2" | tee err
not grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan with mda
pvs "$dev11" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# orphan without mda
pvs "$dev14" | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# pv with mda, pv without mda, orphan with mda, orphan without mda
pvs "$dev10" "$dev2" "$dev11" "$dev14" | tee err
grep "$dev10" err
grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
not grep "$dev8" err
not grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# tag refering to pv with mda and pv without mda
pvs @V3 | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# tag refering to one pv without mda
pvs @V3D8 | tee err
not grep "$dev10" err
not grep "$dev2" err
not grep "$dev3" err
not grep "$dev4" err
not grep "$dev5" err
not grep "$dev6" err
not grep "$dev7" err
grep "$dev8" err
not grep "$dev9" err
not grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
not grep "$dev14" err

# all pvs (pvs in vgs and orphan pvs)
pvs | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
not grep "$dev12" err
not grep "$dev13" err
grep "$dev14" err

# all devs (pvs in vgs, orphan pvs, and devs)
pvs -a | tee err
grep "$dev10" err
grep "$dev2" err
grep "$dev3" err
grep "$dev4" err
grep "$dev5" err
grep "$dev6" err
grep "$dev7" err
grep "$dev8" err
grep "$dev9" err
grep "$dev11" err
grep "$dev12" err
grep "$dev13" err
grep "$dev14" err

vgremove $vg1 $vg2 $vg3
