#!/usr/bin/env bash

# Copyright (C) 2012,2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# 'Exercise some lvcreate diagnostics'

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

# FIXME  update test to make something useful on <16T
aux can_use_16T || skip
aux have_raid 1 3 0 || skip

aux prepare_vg 5

# Fake ~2.5PiB volume group $vg1 via snapshot LVs
for device in "$lv1" "$lv2" "$lv3" "$lv4" "$lv5" 
do
	lvcreate --type snapshot -s -l 20%FREE -n $device $vg --virtualsize 520T
done

aux extend_filter_LVMTEST

pvcreate "$DM_DEV_DIR"/$vg/$lv[12345]
vgcreate $vg1 "$DM_DEV_DIR"/$vg/$lv[12345]


#
# Create and extend large RAID10 LV
#
# We need '--nosync' or our virtual devices won't work

lvcreate --type raid10 -m 1 -i 2 -L 200T -n $lv1 $vg1 --nosync
check lv_field $vg1/$lv1 size "200.00t"
lvextend -L +200T $vg1/$lv1
check lv_field $vg1/$lv1 size "400.00t"
lvextend -L +100T $vg1/$lv1
check lv_field $vg1/$lv1 size "500.00t"
lvextend -L 1P $vg1/$lv1
check lv_field $vg1/$lv1 size "1.00p"

vgremove -ff $vg1
vgremove -ff $vg
