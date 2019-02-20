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

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip

aux prepare_vg 1

vgchange -s 16k $vg

lvcreate -L1 -n cow $vg

# Thin and snapshot conversion
lvcreate -T -L1 -V10 -n th $vg/pool
eval $(lvs -S 'name=~_pmspare' -a -o name --config 'report/mark_hidden_devices=0' --noheading --nameprefixes $vg)

# Cannot create snapshot of pool's meta
not lvcreate -s -L1 $vg/pool_tmeta 2>&1 | tee err
grep "not supported" err

# Cannot create snapshot of pool's data
not lvcreate -s -L1 $vg/pool_tdata 2>&1 | tee err
grep "not supported" err

# Cannot use thin-type as COW
not lvconvert --yes --type snapshot $vg/cow $vg/th 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/pool 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/$LVM2_LV_NAME 2>&1 | tee err
grep "lv_is_visible" err

not lvconvert --yes --type snapshot $vg/cow $vg/pool_tdata 2>&1 | tee err
grep "lv_is_visible" err

not lvconvert --yes --type snapshot $vg/cow $vg/pool_tmeta 2>&1 | tee err
grep "lv_is_visible" err

# Cannot use thin-pool, _tdata, _tmeta as origin
not lvconvert --yes --type snapshot $vg/pool $vg/cow 2>&1 | tee err
grep "not supported" err

not lvconvert --yes --type snapshot $vg/$LVM2_LV_NAME $vg/cow 2>&1 | tee err
grep "not supported" err

not lvconvert --yes --type snapshot $vg/pool_tdata $vg/cow 2>&1 | tee err
grep "not supported" err

not lvconvert --yes --type snapshot $vg/pool_tmeta $vg/cow 2>&1 | tee err
grep "not supported" err

lvconvert --yes -s $vg/th $vg/cow

check lv_field $vg/th segtype thin
check lv_field $vg/cow segtype linear
check lv_attr_bit type $vg/cow "s"
check lv_attr_bit type $vg/th "o"

lvs -a -o+lv_role,lv_layout $vg

vgremove -f $vg
