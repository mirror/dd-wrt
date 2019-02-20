#!/usr/bin/env bash

# Copyright (C) 2012-2016 Red Hat, Inc. All rights reserved.
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

lv_devices() {
	test "$3" -eq "$(get lv_devices "$1/$2" | wc -w)"
}

########################################################
# MAIN
########################################################
aux have_raid 1 3 0 || skip

aux prepare_vg 6 20  # 6 devices for RAID10 (2-mirror,3-stripe) test
#
# Create RAID10:
#

# Should not allow more than 2-way mirror
not lvcreate --type raid10 -m 2 -i 2 -l 2 -n $lv1 $vg

# 2-way mirror, 2-stripes
lvcreate --type raid10 -m 1 -i 2 -l 2 -n $lv1 $vg
aux wait_for_sync $vg $lv1
lvremove -ff $vg/$lv1

# 2-way mirror, 2-stripes - Set min/max recovery rate
lvcreate --type raid10 -m 1 -i 2 -l 2 \
	--minrecoveryrate 50 --maxrecoveryrate 1M \
	-n $lv1 $vg
check lv_field $vg/$lv1 raid_min_recovery_rate 50
check lv_field $vg/$lv1 raid_max_recovery_rate 1024
aux wait_for_sync $vg $lv1

# 2-way mirror, 3-stripes
lvcreate --type raid10 -m 1 -i 3 -l 3 -n $lv2 $vg
aux wait_for_sync $vg $lv2

lvremove -ff $vg

# Test 100%FREE option
# 38 extents / device
# 1 image = 37 extents (1 for meta)
# 3 images = 111 extents = 55.50m
lvcreate --type raid10 -i 3 -l 100%FREE -an -Zn -n raid10 $vg
check lv_field $vg/raid10 size "55.50m"
lvremove -ff $vg

# Create RAID (implicit stripe count based on PV count)
#######################################################

# Not enough drives
not lvcreate --type raid10 -l2 $vg "$dev1" "$dev2" "$dev3"

# Implicit count comes from #PVs given (always 2-way mirror)
# Defaults -i2, which works with 4 PVs listed
lvcreate --type raid10 -l2 -an -Zn -n raid10 $vg "$dev1" "$dev2" "$dev3" "$dev4"
lv_devices $vg raid10 4

# Defaults -i2 even though more PVs listed
lvcreate --type raid10 -l2 -an -Zn -n raid10_6 $vg "$dev1" "$dev2" "$dev3" "$dev4" "$dev5" "$dev6"
lv_devices $vg raid10_6 4

lvremove -ff $vg

#
# FIXME: Add tests that specify particular PVs to use for creation
#


########################################################
# Try again with backward compatible old logic applied #
########################################################
aux lvmconf 'allocation/raid_stripe_all_devices = 1'

# Implicit count comes from #PVs given (always 2-way mirror)
lvcreate --type raid10 -l2 -an -Zn -n raid10 $vg "$dev1" "$dev2" "$dev3" "$dev4"
lv_devices $vg raid10 4

# Implicit count comes from total #PVs in VG (always 2 for mirror though)
lvcreate --type raid10 -l2 -an -Zn -n raid10_vg $vg
lv_devices $vg raid10_vg 6

vgremove -ff $vg
