#!/usr/bin/env bash

# Copyright (C) 2010-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs
get_devs

vgcreate $SHARED -s 4k "$vg" "${DEVICES[@]}"

# 3 Chunks
lvcreate -aey -n one -l 10 $vg
lvcreate -s -l 3 -n snapA $vg/one
lvcreate -s -c 4k -l 3 -n snapX1 $vg/one
lvcreate -s -c 8k -l 6 -n snapX2 $vg/one

# Check that snapshots that are too small are caught with correct error.
not lvcreate -s -c 8k -l 2 -n snapX3 $vg/one 2>&1 | tee lvcreate.out
not grep "suspend origin one" lvcreate.out
grep "smaller" lvcreate.out

not lvcreate -s -l 1 -n snapB $vg/one 2>&1 | tee lvcreate.out
not grep "suspend origin one" lvcreate.out
grep "smaller" lvcreate.out

vgremove -ff $vg
