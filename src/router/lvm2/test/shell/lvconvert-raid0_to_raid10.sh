#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
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

# rhbz1514500

aux have_raid 1 12 0 || skip

# 8 PVs needed for RAID10 testing (4-stripes/2-mirror)
aux prepare_pvs 8 64
get_devs
vgcreate $SHARED -s 256k "$vg" "${DEVICES[@]}"

lvcreate -y --ty raid0 -R32.00k -i 4 -n $lv1 -L 64M $vg
lvcreate -y -i4 -l4 -n $lv2 $vg
lvextend -y -l +4 $vg/$lv1
lvconvert -y -R512K --ty raid10 $vg/$lv1

vgremove -ff $vg
