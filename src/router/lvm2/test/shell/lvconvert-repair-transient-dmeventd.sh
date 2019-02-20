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

aux prepare_dmeventd
aux mirror_recovery_works || skip
aux prepare_vg 5

lvcreate -aey --type mirror -m 3 --ignoremonitoring -L 1 -n 4way $vg
lvchange --monitor y $vg/4way
aux disable_dev "$dev2" "$dev4"
mkfs.ext3 "$DM_DEV_DIR/$vg/4way"
aux enable_dev "$dev2" "$dev4"
sleep 3
lvs -a -o +devices $vg | tee out
not grep unknown out
check mirror $vg 4way
check mirror_legs $vg 4way 2
lvs -a -o +devices $vg | tee out
not grep mimage_1 out
lvs -a -o +devices $vg | tee out
not grep mimage_3 out

vgremove -f $vg
