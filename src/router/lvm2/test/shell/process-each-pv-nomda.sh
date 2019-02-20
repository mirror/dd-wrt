#!/usr/bin/env bash

# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Test process_each_pv with zero mda'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2

pvcreate "$dev1" --metadatacopies 0
pvcreate "$dev2"

vgcreate $SHARED $vg1 "$dev1" "$dev2"

pvdisplay -a -C | tee err
grep "$dev1" err
grep "$dev2" err

vgremove $vg1
