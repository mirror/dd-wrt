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

# Check --splitmirrors for mirror segtype

. lib/inittest

aux prepare_vg 3

###########################################
# Mirror split tests
###########################################
# 3-way to 2-way/linear
lvcreate --type mirror -m 2 -l 2 -n $lv1 $vg
aux wait_for_sync $vg $lv1
lvconvert --splitmirrors 1 -n $lv2 -vvvv $vg/$lv1

check lv_exists $vg $lv1
check linear $vg $lv2
check active $vg $lv2
# FIXME: ensure no residual devices

vgremove -ff $vg
