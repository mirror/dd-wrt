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

aux prepare_vg 3

lvcreate -aey --type mirror -m 1 -l 1 --nosync -n mirror $vg
lvchange -a n $vg/mirror
aux disable_dev "$dev1"

not vgreduce --removemissing $vg
not lvchange -v -aey $vg/mirror
lvchange -v --partial -aey $vg/mirror
not lvchange -v --refresh $vg/mirror
lvchange -v --refresh --partial $vg/mirror

# also check that vgchange works
vgchange -a n --partial $vg
vgchange -aey --partial $vg

# check vgremove
vgremove -ff $vg
