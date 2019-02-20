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

# Check pvmove behavior when it's progress and machine is rebooted

SKIP_WITH_LVMLOCKD=1

. lib/inittest

aux prepare_vg 3

for mode in "--atomic" ""
do
lvcreate -aey -l1 -n $lv1 $vg "$dev1"

lvs -o +devices | tee out
grep "$dev1" out

LVM_TEST_TAG="kill_me_$PREFIX" pvmove $mode -i 1 -b "$dev1" "$dev2"
sleep 5 # arbitrary...
lvs -o +devices | tee out
not grep "pvmove" out
lvs -o +devices | tee out
grep "$dev2" out

lvremove -ff $vg
done
