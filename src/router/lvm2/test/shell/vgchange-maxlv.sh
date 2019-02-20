#!/usr/bin/env bash

# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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
aux prepare_pvs 3
get_devs

vgcreate $SHARED -l 2 "$vg" "${DEVICES[@]}"
lvcreate -aey -n one -l 1 $vg
lvcreate -n two -l 1 $vg
not lvcreate -n three -l 1 $vg
vgremove -ff $vg

vgcreate $SHARED -l 3 "$vg" "${DEVICES[@]}"
lvcreate -aey -n one -l 1 $vg
lvcreate -n snap -s -l 1 $vg/one
lvcreate -n two -l 1 $vg
not lvcreate -n three -l 1 $vg
vgchange --monitor y $vg
vgchange -an $vg 2>&1 | tee vgchange.out
not grep "event server" vgchange.out

vgremove -ff $vg
