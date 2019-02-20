#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2

pvcreate --metadatacopies 0 "$dev1"
pvcreate --metadatacopies 1 "$dev2"
vgcreate $vg1 "$dev1" "$dev2"
lvcreate -n foo -l 1 -an --zero n $vg1

check inactive $vg1 foo

RUNDIR="/run"
test -d "$RUNDIR" || RUNDIR="/var/run"
# create a file in pvs_online to disable the pvscan init
# case which scans everything when the first dev appears.
mkdir -p "$RUNDIR/lvm/pvs_online" || true
touch "$RUNDIR/lvm/pvs_online/foo"

pvscan --cache --background "$dev2" -aay

check inactive $vg1 foo

pvscan --cache --background "$dev1" -aay

check active $vg1 foo

rm "$RUNDIR/lvm/pvs_online/foo"
vgremove -ff $vg1
