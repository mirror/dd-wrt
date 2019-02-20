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

test_description='Test pvcreate bootloader area support'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 1
aux lvmconf 'global/suffix=0' 'global/units="b"'

#COMM 'pvcreate sets/aligns bootloader area correctly'
pvcreate --dataalignment 262144b --bootloaderareasize 614400b "$dev1"
# ba_start must be aligned based on dataalignment
# pe_start starts at next dataalignment multiple
# ba_size is the whole space in between ba_start and pe_start
check pv_field "$dev1" ba_start "262144"
check pv_field "$dev1" ba_size "786432"
check pv_field "$dev1" pe_start "1048576"

#COMM 'pvcreate with booloader area size - test corner cases'
dev_size=$(pvs -o pv_size --noheadings "$dev1")
pv_size=$(( dev_size - 1048576 )) # device size - 1m pe_start = area for data

# try to use the whole data area for bootloader area, remaining data area is zero then (pe_start = pv_size)
pvcreate --bootloaderareasize ${pv_size}b --dataalignment 1048576b "$dev1"
check pv_field "$dev1" pe_start $dev_size
check pv_field "$dev1" ba_start 1048576
check pv_field "$dev1" ba_size $pv_size

# try to use the whole data area for bootloader area only and add one more byte - this must error out
not pvcreate --bootloaderareasize $(( pv_size + 1 )) --dataalignment 1048576b "$dev1" 2>err
grep "Bootloader area with data-aligned start must not exceed device size" err

# restoring the PV should also restore the bootloader area correctly
pvremove -ff "$dev1"
pvcreate --dataalignment 256k --bootloaderareasize 600k "$dev1"
vgcreate $SHARED $vg "$dev1"
vgcfgbackup -f "$TESTDIR/vg_with_ba_backup" "$vg"
pv_uuid=$(get pv_field "$dev1" pv_uuid)
vgremove -ff $vg
pvremove -ff "$dev1"
pvcreate --dataalignment 256k --restorefile "$TESTDIR/vg_with_ba_backup" --uuid "$pv_uuid" "$dev1"
check pv_field "$dev1" ba_start "262144"
check pv_field "$dev1" ba_size "786432"
check pv_field "$dev1" pe_start "1048576"

pvremove -ff "$dev1"
