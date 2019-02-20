#!/usr/bin/env bash

# Copyright (C) 2010-2014 Red Hat, Inc. All rights reserved.
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

aux prepare_devs 2

vgcreate $SHARED --metadatasize 128k $vg1 "$dev1"
lvcreate -l100%FREE -n $lv1 $vg1

# Test plain vgexport vgimport tools

# Argument is needed
invalid vgexport
invalid vgimport
# Cannot combine -a and VG name
invalid vgexport -a $vg
invalid vgimport -a $vg1
# Cannot export unknonw VG
fail vgexport ${vg1}-non
fail vgimport ${vg1}-non
# Cannot export VG with active volumes
fail vgexport $vg1

vgchange -an $vg1
vgexport $vg1
# Already exported
fail vgexport $vg1

vgimport $vg1
# Already imported
fail vgimport $vg1
vgchange -ay $vg1

# Clone the LUN
dd if="$dev1" of="$dev2" bs=256K count=1

# Verify pvs works on each device to give us vgname
aux hide_dev "$dev2"
check pv_field "$dev1" vg_name $vg1
aux unhide_dev "$dev2"

aux hide_dev "$dev1"
check pv_field "$dev2" vg_name $vg1
aux unhide_dev "$dev1"

# Import the cloned PV to a new VG
vgimportclone --basevgname $vg2 "$dev2"

# Verify we can activate / deactivate the LV from both VGs
lvchange -ay $vg1/$lv1 $vg2/$lv1
vgchange -an $vg1 $vg2

vgremove -ff $vg1 $vg2

# Verify that if we provide the -n|--basevgname,
# the number suffix is not added unnecessarily.
vgcreate $SHARED --metadatasize 128k A${vg1}B "$dev1"

# vg1B is not the same as Avg1B - we don't need number suffix
dd if="$dev1" of="$dev2" bs=256K count=1
vgimportclone -n ${vg1}B "$dev2"
check pv_field "$dev2" vg_name ${vg1}B

# Avg1 is not the same as Avg1B - we don't need number suffix
dd if="$dev1" of="$dev2" bs=256K count=1
vgimportclone -n A${vg1} "$dev2"
check pv_field "$dev2" vg_name A${vg1}

# Avg1B is the same as Avg1B - we need to add the number suffix
dd if="$dev1" of="$dev2" bs=256K count=1
vgimportclone -n A${vg1}B "$dev2"
aux vgs
check pv_field "$dev2" vg_name A${vg1}B1

vgremove -ff A${vg1}B A${vg1}B1
