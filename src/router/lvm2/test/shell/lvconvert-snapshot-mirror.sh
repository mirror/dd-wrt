#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test various supported conversion of snapshot with mirrors

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 3

vgchange -s 16k $vg

lvcreate -L1 -n cow $vg

# Mirror and snapshot conversion
lvcreate -aye --type mirror -L1 -m1 -n mir $vg

# Cannot create snapshot of mirror leg
not lvcreate -s -L1 $vg/mir_mimage_0 2>&1 | tee err
grep "not supported" err

# cannot use 'mirror' as COW
not lvconvert --yes --type snapshot $vg/cow $vg/mir 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/mir_mimage_0 2>&1 | tee err
grep "lv_is_visible" err

not lvconvert --yes --type snapshot $vg/cow $vg/mir_mlog 2>&1 | tee err
grep "lv_is_visible" err

# cannot use _mimage
not lvconvert --yes --type snapshot $vg/mir_mimage_0 $vg/cow 2>&1 | tee err
grep "not supported" err

# cannot use _mlog
not lvconvert --yes --type snapshot $vg/mir_mlog $vg/cow 2>&1 | tee err
grep "not supported" err

lvconvert --yes -s $vg/mir $vg/cow

check lv_field $vg/mir segtype mirror
check lv_field $vg/cow segtype linear
check lv_attr_bit type $vg/cow "s"
check lv_attr_bit type $vg/mir "o"

lvs -a -o+lv_role,lv_layout $vg

vgremove -f $vg
