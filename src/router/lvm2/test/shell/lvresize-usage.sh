#!/usr/bin/env bash

# Copyright (C) 2007-2016 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 2 80

lvcreate -L 10M -n lv -i2 $vg
lvresize -l +4 $vg/lv
not lvextend -L+0 $vg/lv
not lvextend -l+0 $vg/lv
lvremove -ff $vg

lvcreate -L 64M -n $lv -i2 $vg
not lvresize -v -l +4 xxx/$lv

# Check stripe size is reduced to extent size when it's bigger
ESIZE=$(get vg_field $vg vg_extent_size --units b)
lvextend -L+64m -i 2 -I$(( ${ESIZE%%B} * 2 ))B $vg/$lv 2>&1 | tee err
grep "Reducing stripe size" err

lvremove -ff $vg

lvcreate -L 10M -n lv $vg "$dev1"
lvextend -L +10M $vg/lv "$dev2"
lvextend --type striped -m0 -L +10M $vg/lv "$dev2"

# Attempt to reduce with lvextend and vice versa:
not lvextend -L 16M $vg/lv
not lvreduce -L 32M $vg/lv

lvremove -ff $vg

lvcreate --type mirror -aey -L 4 -n $lv1 $vg
# Incorrent name for resized LV
not lvextend --type mirror -L 10 -n $lv1 $vg
# Same size
not lvextend --type mirror -L 4 $vg/$lv1
# Cannot use any '-' or '+'  sign for --mirror arg
not lvextend --type mirror -L+2 -m-1 $vg/$lv1
not lvextend --type mirror -L+2 -m+1 $vg/$lv1

lvextend --type mirror -L+4 -m1 $vg/$lv1

lvs -a $vg
check  lv_field $vg/$lv1 size "8.00m"

lvremove -ff $vg
