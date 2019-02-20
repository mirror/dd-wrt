#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
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

# We need "dm" directory for dm devices in sysfs.
aux driver_at_least 4 15 || skip

aux prepare_devs 3 8

vgcreate $SHARED "$vg" "$dev1" "$dev2"
lvcreate -l100%FREE -n $lv $vg
dd if="$dev1" of="$dev3" bs=1M
pvs --config "devices/global_filter = [ \"a|$dev2|\", \"a|$dev3|\", \"r|.*|\" ]" 2>err
grep "WARNING: Device mismatch detected for $vg/$lv which is accessing $dev1 instead of $dev3" err

dd if=/dev/zero of="$dev3" bs=1M count=8
lvremove -ff $vg

# Also test if sub LVs with suffixes are correctly processed.
# Check with thick snapshot which has sub LVs with -real and -cow suffix in UUID.
lvcreate -l1 -aey -n $lv $vg
lvcreate -l1 -aey -s $vg/$lv
pvs 2>err
not grep "WARNING: Device mismatch detected for $vg/$lv" err

vgremove -ff $vg
