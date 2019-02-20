#!/usr/bin/env bash

# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 5 80
aux lvmconf 'allocation/maximise_cling = 0' \
	    'allocation/mirror_logs_require_separate_pvs = 1'

# 2-way mirror with corelog, 2 PVs
lvcreate -aey -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg "$dev1" "$dev2"
check mirror_images_redundant $vg $lv1

# 2-way mirror with disklog, 3 PVs
# lvcreate --nosync is in 100% sync after creation (bz429342)
lvcreate -aey -l2 --type mirror -m1 --nosync -n $lv2 $vg "$dev1" "$dev2" "$dev3":0-1 2>&1 | tee out
grep "New mirror won't be synchronised." out
check lv_field $vg/$lv2 copy_percent "100.00"
check mirror_images_redundant $vg $lv2
check mirror_log_on $vg $lv2 "$dev3"

# 3-way mirror with disklog, 4 PVs
lvcreate -aey -l2 --type mirror -m2 --nosync --mirrorlog disk -n $lv3 $vg "$dev1" "$dev2" "$dev4" "$dev3":0-1
check mirror_images_redundant $vg $lv3
check mirror_log_on $vg $lv3 "$dev3"
lvremove -ff $vg

# creating 2-way mirror with disklog from 2 PVs fails
not lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2"

vgremove -ff $vg
