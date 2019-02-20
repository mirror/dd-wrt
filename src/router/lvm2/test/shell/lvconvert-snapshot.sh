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

# Test various supported conversion of snapshot

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 2
get_devs

vgcreate -s 4k "$vg" "${DEVICES[@]}"

lvcreate --type snapshot -V50 -L1 -n $lv1 -s $vg

lvcreate -aey -L1 -n $lv2 $vg
lvcreate -L1 -s -n $lv3 $vg/$lv2

lvcreate -l1 -n $lv4 $vg
lvcreate -L1 -n $lv5 $vg
lvcreate -L1 -n $lv6 $vg

not lvconvert -s $vg/$lv1 $vg/not_exist

# Can't convert to snapshot of origin
not lvconvert -s $vg/$lv1 $vg/$lv2
not lvconvert -s $vg/$lv2 $vg/$lv1
not lvconvert -s $vg/$lv5 $vg/$lv1

not lvconvert -s $vg/$lv5 $vg/$lv2
not lvconvert -s $vg/$lv5 $vg/$lv3

# Can't be itself
not lvconvert -s $vg/$lv1 $vg/$lv1
not lvconvert -s $vg/$lv2 $vg/$lv2

# Can't convert snapshot to snapshot
not lvconvert -s $vg/$lv1 $vg/$lv3
not lvconvert -s $vg/$lv2 $vg/$lv3

# Can't make a real LV snapshot of virtual 'zero' snapshot
not lvconvert -s $vg/$lv1 $vg/$lv4

# Check minimum size
not lvconvert -s $vg/$lv2 $vg/$lv4 2>&1 | tee err
grep "smaller" err

# This should pass
lvconvert --yes -s $vg/$lv2 $vg/$lv5
lvconvert --yes --type snapshot $vg/$lv2 $vg/$lv6

vgremove -f $vg

# FIXME: older stripe target can't handle 1K chunks
vgcreate -s 4k "$vg" "${DEVICES[@]}"
lvcreate -aey -L1 -n $lv2 $vg
lvcreate -L1 -i2 -n $lv7 $vg

# Striped LV is also supported
lvconvert --yes --snapshot $vg/$lv2 $vg/$lv7

vgremove -f $vg
