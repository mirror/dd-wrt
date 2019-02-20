#!/usr/bin/env bash

# Copyright (C) 2012-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# test support of thin discards
#


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 1 0 || skip

aux prepare_vg 2 64
get_devs

aux extend_filter_LVMTEST

# Create named pool only
lvcreate -l1 --discards ignore -T $vg/pool
check lv_field $vg/pool discards "ignore"
check lv_field $vg/pool kernel_discards "ignore"
lvcreate -l1 --discards nopassdown -T $vg/pool1
check lv_field $vg/pool1 discards "nopassdown"
check lv_field $vg/pool1 kernel_discards "nopassdown"
lvcreate -l1 --discards passdown -T $vg/pool2
check lv_field $vg/pool2 discards "passdown"
check lv_field $vg/pool2 discards "passdown"

lvchange --discards nopassdown $vg/pool2

lvcreate -V1M -n origin -T $vg/pool
lvcreate -s $vg/origin -n snap

# Cannot convert active  nopassdown -> ignore
not lvchange --discards nopassdown $vg/pool

# Cannot convert active  ignore -> passdown
not lvchange --discards passdown $vg/pool

# Cannot convert active  nopassdown -> ignore
not lvchange --discards ignore $vg/pool1

# Deactivate pool only
lvchange -an $vg/pool $vg/pool1

# Cannot convert, since thin volumes are still active
not lvchange --discards passdown $vg/pool

# Deactive thin volumes
lvchange -an $vg/origin $vg/snap

lvchange --discards passdown $vg/pool
check lv_field $vg/pool discards "passdown"

lvchange --discards ignore $vg/pool1
check lv_field $vg/pool1 discards "ignore"

vgremove -ff $vg

# Create thin pool with discards set to "ignore".
# If we create a thin volume which we use for a PV
# which we use to create another thin pool on top
# with discards set to "passdown", the discards value
# in metadata is still "passdown", but because the
# device below does not support it, the kernel value
# of discards actually used will be "nopassdown".
# This is why we have "-o discards" and "-o kernel_discards".
vgcreate $SHARED -s 1m "${vg}_1" "${DEVICES[@]}"
lvcreate -l 10 -T ${vg}_1/pool --discards ignore
lvcreate -V 9m -T ${vg}_1/pool -n device_with_ignored_discards
vgcreate $SHARED -s 1m ${vg}_2 "$DM_DEV_DIR/${vg}_1/device_with_ignored_discards"
lvcreate -l 1 -T ${vg}_2/pool --discards passdown
lvcreate -V 1 -T ${vg}_2/pool
check lv_field ${vg}_1/pool discards "ignore"
check lv_field ${vg}_1/pool kernel_discards "ignore"
check lv_field ${vg}_2/pool discards "passdown"
check lv_field ${vg}_2/pool kernel_discards "nopassdown"

vgremove -ff ${vg}_2
vgremove -ff ${vg}_1
