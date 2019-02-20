#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#
# play with thin-pool resize in corner cases
#

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip

test -n "$LVM_TEST_THIN_RESTORE_CMD" || LVM_TEST_THIN_RESTORE_CMD=$(which thin_restore) || skip
"$LVM_TEST_THIN_RESTORE_CMD" -V || skip

aux have_thin 1 10 0 || skip

aux prepare_vg 3 256

aux lvmconf 'activation/thin_pool_autoextend_percent = 30' \
	    'activation/thin_pool_autoextend_threshold = 70'

aux prepare_thin_metadata 400 0 | tee data
lvcreate -L200 -T $vg/pool
lvchange -an $vg

lvcreate -L2M -n $lv1 $vg
"$LVM_TEST_THIN_RESTORE_CMD" -i data -o "$DM_DEV_DIR/mapper/$vg-$lv1"
lvconvert -y --thinpool $vg/pool --poolmetadata $vg/$lv1

# Cannot resize if set to 0%
not lvextend --use-policies --config 'activation{thin_pool_autoextend_percent = 0}' $vg/pool 2>&1 | tee err
grep "0%" err

# Locally active LV is needed
not lvextend --use-policies $vg/pool 2>&1 | tee err
grep "locally" err

lvchange -ay $vg

# Creation of new LV is not allowed when thinpool is over threshold
not lvcreate -V10 $vg/pool


lvextend --use-policies $vg/pool "$dev2" "$dev3"
#should lvextend -l+100%FREE $vg/pool2

lvs -a $vg

vgremove -ff $vg
