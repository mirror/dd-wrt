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

# Test activation behaviour with devices missing.
# - snapshots and their origins are only activated together; if one fails, both
#   fail
# - partial mirrors are not activated (but maybe they should? maybe we should
#   instead lvconvert --repair them?)
# - linear LVs with bits missing are not activated


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 4

lvcreate -l1 -n linear1 $vg "$dev1"
lvcreate -l1 -n linear2 $vg "$dev2"
lvcreate -l2 -n linear12 $vg "$dev1":4 "$dev2":4

lvcreate -aey -l1 -n origin1 $vg "$dev1"
lvcreate -s $vg/origin1 -l1 -n s_napshot2 "$dev2"

lvcreate -aey -l1 --type mirror -m1 -n mirror12 --mirrorlog core $vg "$dev1" "$dev2"
lvcreate -aey -l1 --type mirror -m1 -n mirror123 $vg "$dev1" "$dev2" "$dev3"

vgchange -a n $vg
aux disable_dev "$dev1"
not vgchange -a y $vg
not vgck $vg

check inactive $vg linear1
check active $vg linear2
check inactive $vg origin1
check inactive $vg s_napshot2
check inactive $vg linear12
check inactive $vg mirror12
check inactive $vg mirror123

vgchange -a n $vg
aux enable_dev "$dev1"
aux disable_dev "$dev2"
not vgchange -aey $vg
not vgck $vg

check active $vg linear1
check inactive $vg linear2
check inactive $vg linear12
check inactive $vg origin1
check inactive $vg s_napshot2
check inactive $vg mirror12
check inactive $vg mirror123

vgchange -a n $vg
aux enable_dev "$dev2"
aux disable_dev "$dev3"
not vgchange -aey $vg
not vgck $vg

check active $vg origin1
check active $vg s_napshot2
check active $vg linear1
check active $vg linear2
check active $vg linear12
check inactive $vg mirror123
check active $vg mirror12

vgchange -a n $vg
aux enable_dev "$dev3"
aux disable_dev "$dev4"
vgchange -aey $vg
not vgck $vg

check active $vg origin1
check active $vg s_napshot2
check active $vg linear1
check active $vg linear2
check active $vg linear12
check active $vg mirror12
check active $vg mirror123

vgremove -ff $vg
