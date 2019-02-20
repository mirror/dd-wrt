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

aux prepare_vg 2

aux disable_dev "$dev1"

#
# Test for allowable metadata changes
# addtag_ARG
# deltag_ARG
vgchange --addtag foo $vg
vgchange --deltag foo $vg

#
# Test for disallowed metadata changes
#
# maxphysicalvolumes_ARG
not vgchange -p 10 $vg

# resizeable_ARG
not vgchange -x n $vg

# uuid_ARG
not vgchange -u $vg

# physicalextentsize_ARG
not vgchange -s 2M $vg

# alloc_ARG
not vgchange --alloc anywhere $vg

# vgmetadatacopies_ARG
not vgchange --vgmetadatacopies 2 $vg

#
# Ensure that allowed args don't cause disallowed args to get through
#
not vgchange -p 10 --addtag foo $vg

vgremove -ff $vg
