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

# Basic usage of zero target


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 1

lvcreate --type error -L1 -n $lv1 $vg
lvextend -L+1 $vg/$lv1

# has to match

check lv_field $vg/$lv1 lv_modules "error"
check lv_field $vg/$lv1 segtype "error"
check lv_field $vg/$lv1 seg_count "1"
check lv_field $vg/$lv1 seg_size_pe "4"   # 4 * 512 => 2M

# FIXME should we print info we are ignoring stripping?
lvextend -L+1 -I64 -i2 $vg/$lv1

vgremove -ff $vg
