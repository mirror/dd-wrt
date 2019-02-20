#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Hello world for vgcreate $SHARED with lvmlockd and sanlock'

. lib/inittest

[ -z "$LVM_TEST_LVMLOCKD" ] && skip

aux prepare_pvs 1

vgcreate $SHARED $vg "$dev1"

vgs -o+locktype,lockargs $vg

vgremove $vg

