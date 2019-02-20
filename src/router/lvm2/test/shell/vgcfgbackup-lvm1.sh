#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
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

aux prepare_pvs 4
get_devs

if test -n "$LVM_TEST_LVM1" ; then

pvcreate --metadatacopies 0 "$dev4"

# No automatic backup
aux lvmconf "backup/backup = 0"

# vgcfgbackup correctly stores metadata LVM1 with missing PVs

pvcreate -M1 "${DEVICES[@]}"
vgcreate $SHARED -M1 "$vg" "${DEVICES[@]}"
lvcreate -l1 -n $lv1 $vg "$dev1"
pvremove -ff -y "$dev2"
not lvcreate -l1 -n $lv1 $vg "$dev3"
lvchange -an $vg
vgcfgbackup -f "backup.$$" $vg

fi
