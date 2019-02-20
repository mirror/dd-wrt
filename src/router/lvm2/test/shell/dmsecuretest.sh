#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test secure table is not leaking data in user land

SKIP_WITH_LVMPOLLD=1

# AES key matching rot13 string from dmsecuretest.c */
SECURE="434e0cbab02ca68ffba9268222c3789d703fe62427b78b308518b3228f6a2122"

. lib/inittest

DMTEST="${PREFIX}-test-secure"

# Test needs installed gdb package with gcore app
which gcore || skip

aux driver_at_least 4 6 || skip

# ensure we can create devices (uses dmsetup, etc)
aux prepare_devs 1

# check both code versions - linked libdm  and internal device_mapper version
# there should not be any difference
for i in securetest dmsecuretest ; do

# 1st. try with empty table
# 2nd. retry with already exiting DM node - exercize error path also wipes
for j in empty existing ; do

"$i" "$dev1" "$DMTEST" >cmdout 2>&1 &
PID=$!
sleep .5

# crypt device should be loaded
dmsetup status "$DMTEST"

# generate core file for running&sleeping binary
gcore "$PID"
kill "$PID" || true
wait

cat cmdout

# $SECURE string must NOT be present in core file
not grep "$SECURE" "core.$PID" || {
	## cp "core.$PID" /dev/shm/core
	rm -f "core.$PID"
        dmsetup remove "$DMTEST"
	die "!!! Secure string $SECURE found present in core.$PID !!!"
}
rm -f "core.$PID"

if test "$j" = empty ; then
	not grep "Device or resource busy" cmdout
else
	# Device should be already present resulting into error message
	grep "Device or resource busy" cmdout
	dmsetup remove "$DMTEST"
fi

done

done
