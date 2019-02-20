#!/usr/bin/env bash

# Copyright (C) 2011 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 5
aux lvmconf 'allocation/maximise_cling = 0' \
	    'allocation/mirror_logs_require_separate_pvs = 1'

lvcreate -aey --type mirror -m 3 --ignoremonitoring -L 2M -n 4way $vg "$dev1" "$dev2" "$dev3" "$dev4" "$dev5":0
lvcreate -s $vg/4way -L 2M -n snap

aux disable_dev "$dev2" "$dev4"
echo n | lvconvert --repair $vg/4way 2>&1 | tee 4way.out
lvs -a -o +devices $vg | not grep unknown
vgreduce --removemissing $vg
aux enable_dev "$dev2" "$dev4"
lvs -a -o +devices $vg
check mirror $vg 4way "$dev5"

vgchange -an $vg
vgremove -ff $vg
