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

test_description="ensure pvmove works with lvs"
SKIP_WITH_LVMLOCKD=1

. lib/inittest

aux throttle_dm_mirror || skip

aux prepare_vg 5 180

lvcreate -aey -L30 -n $lv1 $vg "$dev1"
lvextend -L+30 $vg/$lv1 "$dev2"
lvextend -L+30 $vg/$lv1 "$dev1"
lvextend -L+30 $vg/$lv1 "$dev2"
lvextend -L+30 $vg/$lv1 "$dev1"

pvmove -b "$dev1" "$dev5" 2>&1 | tee out

#lvchange -an $vg/$lv1
lvs -a $vg

pvmove --abort

lvremove -ff $vg
