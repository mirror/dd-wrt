#!/usr/bin/env bash

# Copyright (C) 2008 Red Hat, Inc. All rights reserved.
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

aux prepare_dmeventd

aux prepare_vg 5

lvcreate -aey --type mirror -m 3 --nosync --ignoremonitoring -l1 -n 4way $vg
lvchange --monitor y $vg/4way
lvcreate -aey --type mirror -m 2 --nosync --ignoremonitoring -l1 -n 3way $vg
lvchange --monitor y $vg/3way

lvcreate -aey -l1 -n $lv1 $vg
lvcreate -s -l1 -n $lv2 $vg/$lv1

dmeventd -R -f &
echo $! >LOCAL_DMEVENTD
sleep 2 # wait a bit, so we talk to the new dmeventd later

lvchange --monitor y --verbose $vg/3way 2>&1 | tee lvchange.out
grep 'already monitored' lvchange.out
lvchange --monitor y --verbose $vg/4way 2>&1 | tee lvchange.out
grep 'already monitored' lvchange.out

# now try what happens if no dmeventd is running
kill -9 "$(< LOCAL_DMEVENTD)"
rm LOCAL_DMEVENTD

dmeventd -R -f &
echo $! >LOCAL_DMEVENTD

# wait longer as tries to communicate with killed daemon
sleep 9
# now dmeventd should not be running
not pgrep dmeventd
rm LOCAL_DMEVENTD

lvchange --monitor y --verbose $vg/3way 2>&1 | tee lvchange.out
pgrep -o dmeventd >LOCAL_DMEVENTD
not grep 'already monitored' lvchange.out

lvchange --monitor y --verbose $vg/$lv2 2>&1 | tee lvchange.out
not grep 'already monitored' lvchange.out

vgremove -ff $vg
