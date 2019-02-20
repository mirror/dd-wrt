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



. lib/inittest

aux prepare_vg 4

lvcreate -aey --type mirror -m 1 --mirrorlog disk --ignoremonitoring -L 1 -n mirror $vg
not lvconvert -m 2 --mirrorlog core $vg/mirror "$dev3" 2>&1 | tee errs
grep "two steps" errs

lvconvert -m 2 $vg/mirror "$dev3"
lvconvert --mirrorlog core $vg/mirror
not lvconvert -m 1 --mirrorlog disk $vg/mirror "$dev3" 2>&1 | tee errs
grep "two steps" errs

vgremove -ff $vg
