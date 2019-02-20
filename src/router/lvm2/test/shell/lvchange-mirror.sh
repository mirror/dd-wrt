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

# FIXME RESYNC doesn't work in cluster with exclusive activation
# seriously broken!

SKIP_WITH_CLVMD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_dmeventd
aux prepare_vg 3

# force resync 2-way active mirror
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3":0-1
check mirror $vg $lv1 "$dev3"
lvchange -y --resync $vg/$lv1
check mirror $vg $lv1 "$dev3"
lvremove -ff $vg

# force resync 2-way inactive mirror
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3":0-1
lvchange -an $vg/$lv1
check mirror $vg $lv1 "$dev3"
lvchange --resync $vg/$lv1
check mirror $vg $lv1 "$dev3"

vgremove -ff $vg
