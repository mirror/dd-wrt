#!/usr/bin/env bash

# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 5

for deactivate in true false; do

# extend 2-way mirror
	lvcreate -aye -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3":0-1

	test $deactivate && lvchange -an $vg/$lv1

	lvextend -l+2 $vg/$lv1
	check mirror $vg $lv1 "$dev3"
	check mirror_images_contiguous $vg $lv1

# reduce 2-way mirror
	lvreduce -f -l-2 $vg/$lv1
	check mirror $vg $lv1 "$dev3"

# extend 2-way mirror (cling if not contiguous)
	lvcreate -aye -l2 --type mirror -m1 -n $lv2 $vg "$dev1" "$dev2" "$dev3":0-1
	lvcreate -l1 -n $lv3 $vg "$dev1"
	lvcreate -l1 -n $lv4 $vg "$dev2"

	test $deactivate && lvchange -an $vg/$lv2

	lvextend -l+2 $vg/$lv2
	check mirror $vg $lv2 "$dev3"
	check mirror_images_clung $vg $lv2

	lvremove -ff $vg
done

vgremove -ff $vg
