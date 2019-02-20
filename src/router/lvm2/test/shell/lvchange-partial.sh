#!/usr/bin/env bash

# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 4

TYPE=raid1
aux have_raid 1 3 0 || TYPE=mirror

lvcreate -aey --type $TYPE -m 1 -l 2 -n $lv1 $vg
lvchange -an $vg/$lv1
aux disable_dev "$dev1"

#
# Test for allowable metadata changes
#
# contiguous_ARG
lvchange -C y $vg/$lv1
lvchange -C n $vg/$lv1

# permission_ARG
lvchange -p r $vg/$lv1
lvchange -p rw $vg/$lv1

# readahead_ARG
lvchange -r none $vg/$lv1
lvchange -r auto $vg/$lv1

# persistent_ARG
lvchange -M y --minor 56 --major 253 $vg/$lv1
lvchange -M n $vg/$lv1

# addtag_ARG
# deltag_ARG
lvchange --addtag foo $vg/$lv1
lvchange --deltag foo $vg/$lv1

#
# Test for disallowed metadata changes
#
# resync_ARG
not lvchange --resync $vg/$lv1

# alloc_ARG
not lvchange --alloc anywhere $vg/$lv1

# discards_ARG
not lvchange --discards ignore $vg/$lv1

# zero_ARG
not lvchange --zero y $vg/$lv1

#
# Ensure that allowed args don't cause disallowed args to get through
#
not lvchange --resync -ay $vg/$lv1
not lvchange --resync --addtag foo $vg/$lv1

aux enable_dev "$dev1"

vgremove -ff $vg
