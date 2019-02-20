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

test_description='test some blocking / non-blocking multi-vg operations'

SKIP_WITH_CLVMD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 3
pvcreate "$dev1" "$dev2"
vgcreate $SHARED $vg "$dev1" "$dev2"

# if wait_for_locks set, vgremove should wait for orphan lock
# flock process should have exited by the time first vgremove completes
flock -w 5 "$TESTDIR/var/lock/lvm/P_orphans" sleep 10 &
while ! test -f "$TESTDIR/var/lock/lvm/P_orphans" ; do sleep .1 ; done

vgremove --config 'global { wait_for_locks = 1 }' $vg
not vgremove --config 'global { wait_for_locks = 1 }' $vg

test ! -f "$TESTDIR/var/lock/lvm/P_orphans"

# if wait_for_locks not set, vgremove should fail on non-blocking lock
# we must wait for flock process at the end - vgremove won't wait
vgcreate $SHARED $vg "$dev1" "$dev2"
flock -w 5 "$TESTDIR/var/lock/lvm/P_orphans" sleep 10 &

while ! test -f "$TESTDIR/var/lock/lvm/P_orphans" ; do sleep .1 ; done
flock_pid=$(jobs -p)

not vgremove --config 'global { wait_for_locks = 0 }' $vg
test -f "$TESTDIR/var/lock/lvm/P_orphans" # still running
kill "$flock_pid"

vgremove -ff $vg
