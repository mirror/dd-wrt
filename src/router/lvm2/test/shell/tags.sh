#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
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

aux prepare_pvs 4

# vgcreate $SHARED with --addtag
vgcreate $SHARED --addtag firstvg $vg1 "$dev1" "$dev2"
vgcreate $SHARED --addtag secondvg $vg2 "$dev3" "$dev4"
check vg_field $vg1 tags "firstvg"
check vg_field $vg2 tags "secondvg"
vgremove -f $vg1 $vg2

# vgchange with --addtag and --deltag
vgcreate $SHARED $vg1 "$dev1" "$dev2"
vgcreate $SHARED $vg2 "$dev3" "$dev4"
vgchange --addtag firstvgtag1 $vg1
# adding a tag multiple times is not an error
vgchange --addtag firstvgtag2 $vg1
vgchange --addtag firstvgtag2 $vg1
vgchange --addtag firstvgtag3 $vg1
vgchange --addtag secondvgtag1 $vg2
vgchange --addtag secondvgtag2 $vg2
vgchange --addtag secondvgtag3 $vg2
check vg_field @firstvgtag2 tags "firstvgtag1,firstvgtag2,firstvgtag3"
check vg_field @secondvgtag1 tags "secondvgtag1,secondvgtag2,secondvgtag3"
vgchange --deltag firstvgtag2 $vg1
check vg_field @firstvgtag1 tags "firstvgtag1,firstvgtag3"
# deleting a tag multiple times is not an error
vgchange --deltag firstvgtag2 $vg1
vgchange --deltag firstvgtag1 $vg2
vgremove -f $vg1 $vg2

# lvcreate with --addtag
vgcreate $SHARED $vg1 "$dev1" "$dev2"
lvcreate --addtag firstlvtag1 -l 4 -n $lv1 $vg1
lvcreate --addtag secondlvtag1 -l 4 -n $lv2 $vg1
check lv_field @firstlvtag1 tags "firstlvtag1"
not check lv_field @secondlvtag1 tags "firstlvtag1"
check lv_field $vg1/$lv2 tags "secondlvtag1"
not check lv_field $vg1/$lv1 tags "secondlvtag1"
vgremove -f $vg1

# lvchange with --addtag and --deltag
vgcreate $SHARED $vg1 "$dev1" "$dev2"
lvcreate -l 4 -n $lv1 $vg1
lvcreate -l 4 -n $lv2 $vg1
lvchange --addtag firstlvtag1 $vg1/$lv1
# adding a tag multiple times is not an error
lvchange --addtag firstlvtag2 $vg1/$lv1
lvchange --addtag firstlvtag2 $vg1/$lv1
lvchange --addtag firstlvtag3 $vg1/$lv1
lvchange --addtag secondlvtag1 $vg1/$lv2
lvchange --addtag secondlvtag2 $vg1/$lv2
lvchange --addtag secondlvtag3 $vg1/$lv2
check lv_field $vg1/$lv1 tags "firstlvtag1,firstlvtag2,firstlvtag3"
not check lv_field $vg1/$lv1 tags "secondlvtag1"
check lv_field $vg1/$lv2 tags "secondlvtag1,secondlvtag2,secondlvtag3"
not check lv_field $vg1/$lv1 tags "secondlvtag1"
# deleting a tag multiple times is not an error
lvchange --deltag firstlvtag2 $vg1/$lv1
lvchange --deltag firstlvtag2 $vg1/$lv1
check lv_field $vg1/$lv1 tags "firstlvtag1,firstlvtag3"
check lv_field $vg1/$lv2 tags "secondlvtag1,secondlvtag2,secondlvtag3"

vgremove -ff $vg1
