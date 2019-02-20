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

# Testing calculation of snapshot space
# https://bugzilla.redhat.com/show_bug.cgi?id=1035871


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_pvs 1
get_devs

vgcreate $SHARED -s 4K "$vg" "${DEVICES[@]}"

lvcreate -aey -L1 -n $lv1 $vg
# Snapshot should be large enough to handle any writes
lvcreate -L2 -s $vg/$lv1 -n $lv2

dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv2" bs=1M count=1 conv=fdatasync

# Snapshot must not be 'I'nvalid here
check lv_attr_bit state $vg/$lv2 "a"

vgremove -f $vg
