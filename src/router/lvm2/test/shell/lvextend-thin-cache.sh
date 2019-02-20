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

# Exercise resize of cached thin pool data volumes


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip
aux have_cache 1 3 0 || skip

aux prepare_vg 2

lvcreate -l2 -T $vg/pool
# Caching of thin-pool's dataLV
lvcreate -H -L10 $vg/pool

# Resize is unsupported
not lvextend -l+2 $vg/pool 2>&1 | tee out

grep "Unable to resize" out

vgremove -ff $vg
