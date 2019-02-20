#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Demonstrate problem when upconverting and cutting leg in clvmd



. lib/inittest

aux prepare_pvs 3 100
get_devs

vgcreate $SHARED -s 64k "$vg" "${DEVICES[@]}"

# Use zero devices for big mirror legs
aux zero_dev "$dev2" $(get first_extent_sector "$dev2"):
aux zero_dev "$dev3" $(get first_extent_sector "$dev3"):

lvcreate -aey -L90 --type mirror --corelog --regionsize 16k -m1 -n $lv1 $vg "$dev1" "$dev2"

lvconvert -m+1 -b $vg/$lv1 "$dev3"


# We want here ongoing conversion

lvs -a -o+seg_pe_ranges $vg

# Now it should be able to drop 2nd. leg
lvconvert -m-1 $vg/$lv1 "$dev2"

lvs -a $vg

vgremove -f $vg
