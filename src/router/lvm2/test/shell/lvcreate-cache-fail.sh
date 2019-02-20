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

# Exercise creation of cache and cache pool volumes and failure path
# https://bugzilla.redhat.com/1355923


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

#aux prepare_pvs 1 4707950
#vgcreate $SHARED $vg "$dev1"
#lvcreate -L4T -n $lv1 $vg
#lvcreate -H -L500G -n cache $vg/$lv1
#fail lvcreate -H -l 127999 -n cache $vg/$lv1

aux prepare_vg 1 20
lvcreate -L10 -n $lv1 $vg
fail lvcreate -H -L2 -n cache $vg/$lv1

lvs -a $vg
vgs $vg
lvdisplay $vg

#dmsetup table
#dmsetup status
#time dmsetup suspend ${vg}-${lv1}
#time dmsetup resume ${vg}-${lv1}

vgremove -ff $vg
