#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 3
lvcreate -n blabla -L 1 $vg

dd if=/dev/urandom bs=512 seek=2 count=32 of="$dev2"

# TODO: aux lvmconf "global/locking_type = 4"

vgscan 2>&1 | tee vgscan.out || true

grep "Failed" vgscan.out

dd if=/dev/urandom bs=512 seek=2 count=32 of="$dev2"

vgck $vg 2>&1 | tee vgck.out || true
grep Incorrect vgck.out

vgremove -ff $vg
