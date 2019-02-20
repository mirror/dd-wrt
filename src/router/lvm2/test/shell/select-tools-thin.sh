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


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip

aux prepare_pvs 1 16

#########################
# special cases to test #
#########################

# if calling lvremove and an LV is removed that is related to other LV
# and we're doing selection based on this relation, check if we're
# selecting on initial state (here, thin origin LV thin_orig is removed
# first, but thin snap should be still selectable based on origin=thin_orig
# condition even though thin_orig has just been removed)
vgcreate $SHARED -s 4m $vg1 "$dev1"
lvcreate -l100%FREE -T $vg1/pool
lvcreate -V4m -T $vg1/pool -n thin_orig
lvcreate -s $vg1/thin_orig -n thin_snap
lvremove -ff -S 'lv_name=thin_orig || origin=thin_orig' > out
grep "Logical volume \"thin_orig\" successfully removed" out
grep "Logical volume \"thin_snap\" successfully removed" out
not lvs $vg1/thin_orig
not lvs $vg1/thin_snap

vgremove -ff $vg1
