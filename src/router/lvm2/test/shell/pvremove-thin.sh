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

# Checks we are not reading our own devices
# https://bugzilla.redhat.com/show_bug.cgi?id=1064374


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg

aux have_thin 1 8 0 || skip

aux extend_filter_LVMTEST

lvcreate -L10 -V10 -n $lv1 -T $vg/pool1

pvcreate "$DM_DEV_DIR/$vg/$lv1"
pvremove "$DM_DEV_DIR/$vg/$lv1"

vgremove -ff $vg
