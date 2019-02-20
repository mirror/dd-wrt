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

# test defaults entered through lvm.conf


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_vg 2

lvcreate -T -L8M $vg/pool0

aux lvmconf "allocation/thin_pool_chunk_size = 128" \
	    "allocation/thin_pool_discards = \"ignore\"" \
	    "allocation/thin_pool_zero = 0"

lvcreate -T -L8M $vg/pool1

check lv_field $vg/pool1 chunksize "128.00k"
check lv_field $vg/pool1 discards "ignore"
check lv_field $vg/pool1 zero ""

vgremove -f $vg
