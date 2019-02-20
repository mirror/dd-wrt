#!/usr/bin/env bash

# Copyright (C) 2013-2014 Red Hat, Inc. All rights reserved.
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

aux have_thin 1 10 0 || skip

aux prepare_vg 3 1256

for deactivate in true false; do
# Create some thin volumes
	lvcreate -L20 -V30 -n $lv1 -T $vg/pool
	lvcreate -s $vg/$lv1
# Confirm we have basic 2M metadata
	check lv_field $vg/pool_tmeta size "2.00m"

	test $deactivate && lvchange -an $vg

	lvresize --poolmetadatasize +2M $vg/pool
# Test it's been resized to 4M
	check lv_field $vg/pool_tmeta size "4.00m"

	lvresize --poolmetadatasize +256M $vg/pool
	check lv_field $vg/pool_tmeta size "260.00m"

	lvresize --poolmetadatasize +3G $vg/pool
	check lv_field $vg/pool_tmeta size "3.25g"

	vgchange -an $vg
	vgchange -ay $vg

# TODO: Add more tests

	lvremove -ff $vg
done
