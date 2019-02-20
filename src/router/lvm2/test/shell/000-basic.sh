#!/usr/bin/env bash

# Copyright (C) 2009-2011 Red Hat, Inc. All rights reserved.
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

lvm version

lvm pvmove --version|sed -n "1s/.*: *\([0-9][^ ]*\) .*/\1/p" | tee version

# ensure they are the same
diff -u version lib/version-expected

dmstats version |sed -n "1s/.*: *\([0-9][^ ]*\) .*/\1/p" | tee dmstats-version

# ensure dmstats version matches build
diff -u dmstats-version lib/dm-version-expected

# ensure we can create devices (uses dmsetup, etc)
aux prepare_devs 5
get_devs

# ensure we do not crash on a bug in config file
aux lvmconf 'log/prefix = 1""'
not lvs "${DEVICES[@]}"
