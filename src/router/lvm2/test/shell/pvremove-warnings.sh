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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2
pvcreate "$dev1" "$dev2"
pvremove "$dev1" "$dev2" 2>&1 | tee pvremove.txt
not grep "No physical" pvremove.txt

pvcreate "$dev1" "$dev2"
vgcreate $SHARED bla "$dev1" "$dev2"
pvremove -ff -y "$dev1" "$dev2" 2>&1 | tee pvremove.txt
not grep "device missing" pvremove.txt
