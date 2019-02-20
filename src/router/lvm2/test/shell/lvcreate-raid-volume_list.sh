#!/usr/bin/env bash
#
# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

# bz1161347 - When raid creation is aborted, left-over devices appear

. lib/inittest

########################################################
# MAIN
########################################################
aux have_raid 1 3 0 || skip

aux prepare_pvs 2 # 2 devices for RAID1
get_devs
vgcreate $SHARED -s 512k "$vg" "${DEVICES[@]}"

aux lvmconf "activation/volume_list = [ \"vg_not_exist\" ]"

##########################################################
# Create 2-way raid1 which fails due to $vg not listed on
# activation/volume_list.  Check for any (Sub)LV remnants.
##########################################################
not lvcreate --yes --type raid1 -l 2 -n $lv $vg
check lv_not_exists $vg/${lv}_rmeta_0
check lv_not_exists $vg/${lv}_rmeta_1
check lv_not_exists $vg/${lv}_rimage_0
check lv_not_exists $vg/${lv}_rimage_1
check lv_not_exists $vg/$lv

vgremove -ff $vg
