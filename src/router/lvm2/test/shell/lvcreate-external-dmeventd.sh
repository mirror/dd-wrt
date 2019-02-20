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

# Test check converted external origin remains monitored

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 3 0 || skip
aux have_raid 1 3 0 || skip

aux prepare_dmeventd
aux prepare_vg 2

# Test validation for external origin being multiple of thin pool chunk size
lvcreate -L10M -T $vg/pool

# Create raid LV (needs monitoring) for external origin.
lvcreate -m1 -L1 -n $lv1 $vg

lvconvert -T --thinpool $vg/pool --originname $lv2 $vg/$lv1

# Check  raid LV now as external origing with $lv2 name is still monitored
check lv_first_seg_field  $vg/$lv2 seg_monitor "monitored"

lvchange -an $vg

lvchange -ay $vg/$lv1
check lv_first_seg_field  $vg/$lv2 seg_monitor "monitored"

vgremove -ff $vg
