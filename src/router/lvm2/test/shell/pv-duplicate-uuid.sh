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

# Test 'Found duplicate' is shown

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 3

pvcreate "$dev1"
UUID1=$(get pv_field "$dev1" uuid)
pvcreate --config "devices{filter=[\"a|$dev2|\",\"r|.*|\"]}" -u "$UUID1" --norestorefile "$dev2"
pvcreate --config "devices{filter=[\"a|$dev3|\",\"r|.*|\"]}" -u "$UUID1" --norestorefile "$dev3"

pvscan --cache 2>&1 | tee out

pvs -o+uuid 2>&1 | tee out

grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c $UUID1 main)" -eq 1

COUNT=$(grep --count "Not using device" warn)
[ "$COUNT" -eq 2 ]

pvs -o+uuid --config "devices{filter=[\"a|$dev2|\",\"r|.*|\"]}" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

not grep "$dev1" main
grep "$dev2" main
not grep "$dev3" main

not grep "Not using device" warn

