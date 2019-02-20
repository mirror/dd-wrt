#!/usr/bin/env bash

# Copyright (C) 2014-2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test parallel use of lvm commands and check locks aren't dropped
# RHBZ: https://bugzilla.redhat.com/show_bug.cgi?id=1049296


SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext3 || skip
which fsck || skip

aux prepare_vg

lvcreate -L10 -n $lv1 $vg
lvcreate -l1 -n $lv2 $vg
mkfs.ext3 "$DM_DEV_DIR/$vg/$lv1"

# Slowdown PV for resized LV
aux delay_dev "$dev1" 50 50 "$(get first_extent_sector "$dev1"):"

lvresize -L-5 -r $vg/$lv1 &

# Let's wait till resize starts
for i in $(seq 1 300); do
        pgrep fsck && break
        sleep .1
done

lvremove -f $vg/$lv2

wait

aux enable_dev "$dev1"

# Check removed $lv2 does not reappear
not check lv_exists $vg $lv2

vgremove -ff $vg
