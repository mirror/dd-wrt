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

# Test various supported conversion of snapshot with raid

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_raid 1 3 0 || skip

aux prepare_vg 3

vgchange -s 16k $vg

lvcreate -L1 -n cow $vg

# Raid and snapshot conversion
lvcreate --type raid1 -L1 -m1 -n rd $vg

# Cannot create snapshot of raid leg
not lvcreate -s -L1 $vg/rd_rimage_0 2>&1 | tee err
grep "not supported" err

# Cannot use raid-type as COW
not lvconvert --yes --type snapshot $vg/cow $vg/rd 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/rd_rimage_0 2>&1 | tee err
grep "lv_is_visible" err

not lvconvert --yes --type snapshot $vg/cow $vg/rd_rmeta_0 2>&1 | tee err
grep "lv_is_visible" err

# Cannot use _rimage
not lvconvert --yes --type snapshot $vg/rd_rimage_0 $vg/cow 2>&1 | tee err
grep "not supported" err

# Cannot use _rmeta
not lvconvert --yes --type snapshot $vg/rd_rmeta_0 $vg/cow 2>&1 | tee err
grep "not supported" err

lvconvert --yes -s $vg/rd $vg/cow

check lv_field $vg/rd segtype raid1
check lv_field $vg/cow segtype linear
check lv_attr_bit type $vg/cow "s"
check lv_attr_bit type $vg/rd "o"

lvs -a -o+lv_role,lv_layout $vg

vgremove -f $vg
