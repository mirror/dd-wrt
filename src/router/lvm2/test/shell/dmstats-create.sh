#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
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

# Don't attempt to test stats with driver < 4.33.00
aux driver_at_least 4 33 || skip

# ensure we can create devices (uses dmsetup, etc)
aux prepare_devs 1

# basic dmstats create commands

dmstats create "$dev1"
dmstats create --start 0 --len 1 "$dev1"
dmstats create --segments "$dev1"
dmstats create --precise "$dev1"
dmstats create --bounds 10ms,20ms,30ms "$dev1"
