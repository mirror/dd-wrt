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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

# Test skip activation flag  -k|--setactivationskip

aux prepare_vg

lvcreate -an --zero n -l 1 -n $lv1 $vg
lvcreate -ky -K -l1 -n $lv2 $vg
get lv_field $vg/$lv2 lv_attr | grep -- "-wi-a----k"

lvchange -ay -K $vg
check active $vg $lv1
lvchange -an $vg

lvchange -ay --setactivationskip y $vg/$lv1
check inactive $vg $lv1

get lv_field $vg/$lv1 lv_attr | grep -- "-wi------k"

lvchange -ay -K $vg
check active $vg $lv1

vgremove -ff $vg
