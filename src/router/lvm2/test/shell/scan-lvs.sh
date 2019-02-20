#!/bin/bash
# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test scan_lvs config setting

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux extend_filter_LVMTEST

aux lvmconf 'devices/scan_lvs = 1'

aux prepare_pvs 1

vgcreate $SHARED $vg1 "$dev1"

lvcreate -l1 -n $lv1 $vg1

pvcreate "$DM_DEV_DIR/$vg1/$lv1"

pvs "$DM_DEV_DIR/$vg1/$lv1"

aux lvmconf 'devices/scan_lvs = 0'

not pvs "$DM_DEV_DIR/$vg1/$lv1"

pvs --config devices/scan_lvs=1 "$DM_DEV_DIR/$vg1/$lv1"

not pvremove "$DM_DEV_DIR/$vg1/$lv1"

pvremove --config devices/scan_lvs=1 "$DM_DEV_DIR/$vg1/$lv1"

lvchange -an "$vg1/$lv1"

lvremove "$vg1/$lv1"

vgremove $vg1

