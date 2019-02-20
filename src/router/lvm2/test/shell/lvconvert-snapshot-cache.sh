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

# Test various supported conversion of snapshot with cache

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 1

vgchange -s 16k $vg

lvcreate -L1 -n cow $vg

# Thin and snapshot conversion
lvcreate -aey -L1 -n ch $vg
lvcreate -H -L1 -n cpool $vg/ch

# Cannot create snapshot of cpool
not lvcreate -s -L1 $vg/cpool 2>&1 | tee err
grep "not supported" err

# Cannot create snapshot of cpool's meta
not lvcreate -s -L1 $vg/cpool_cmeta 2>&1 | tee err
grep "not supported" err

# Cannot create snapshot of cpool's data
not lvcreate -s -L1 $vg/cpool_cdata 2>&1 | tee err
grep "not supported" err

# Cannot use cache-type as COW
not lvconvert --yes --type snapshot $vg/cow $vg/ch 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/cpool 2>&1 | tee err
grep "not accept" err

not lvconvert --yes --type snapshot $vg/cow $vg/cpool_cdata 2>&1 | tee err
grep "lv_is_visible" err

not lvconvert --yes --type snapshot $vg/cow $vg/cpool_cmeta 2>&1 | tee err
grep "lv_is_visible" err

# Cannot use thin-pool, _tdata, _tmeta as origin
not lvconvert --yes --type snapshot $vg/cpool $vg/cow 2>&1 | tee err
grep "not supported" err

not lvconvert --yes --type snapshot $vg/cpool_cdata $vg/cow 2>&1 | tee err
grep "not supported" err

not lvconvert --yes --type snapshot $vg/cpool_cmeta $vg/cow 2>&1 | tee err
grep "not supported" err

lvconvert --yes -s $vg/ch $vg/cow

check lv_field $vg/ch segtype cache
check lv_field $vg/cow segtype linear
check lv_attr_bit type $vg/cow "s"
check lv_attr_bit type $vg/ch "o"

lvs -a -o+lv_role,lv_layout $vg

vgremove -f $vg
