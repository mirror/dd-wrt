#!/usr/bin/env bash

# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# test support for non-power-of-2 thin chunk size
#


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 4 0 || skip

aux prepare_pvs 2 64
get_devs

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

# create non-power-of-2 pool
lvcreate -l100 -c 192 -T $vg/pool

check lv_field $vg/pool discards "passdown"

# check we cannot change discards settings
not lvchange --discard ignore $vg/pool
lvchange --discard nopassdown $vg/pool
check lv_field $vg/pool discards "nopassdown"

# must be multiple of 64KB
not lvcreate -l100 -c 168 -T $vg/pool1

vgremove -ff $vg
