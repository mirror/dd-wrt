#!/usr/bin/env bash

# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
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

# Just skip this test if minor is already in use...
dmsetup info | tee info
grep -E "^Major, minor: *[0-9]+, 123" info && skip

aux prepare_vg 2
lvcreate -a n --zero n -l 1 -n foo $vg
lvchange $vg/foo -My --major=255 --minor=123
lvchange $vg/foo -a y
dmsetup info $vg-foo | tee info
grep -E "^Major, minor: *[0-9]+, 123" info

vgremove -ff $vg
