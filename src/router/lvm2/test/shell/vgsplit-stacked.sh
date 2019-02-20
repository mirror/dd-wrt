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

aux extend_filter_LVMTEST
aux prepare_pvs 3

vgcreate $SHARED $vg1 "$dev1" "$dev2"
lvcreate -n $lv1 -l 100%FREE $vg1

#top VG
pvcreate "$DM_DEV_DIR/$vg1/$lv1"
vgcreate $SHARED $vg "$DM_DEV_DIR/$vg1/$lv1" "$dev3"

vgchange -a n $vg $vg1

# this should fail but not segfault, RHBZ 481793.
not vgsplit $vg $vg1 "$dev3"

vgremove -ff $vg $vg1
