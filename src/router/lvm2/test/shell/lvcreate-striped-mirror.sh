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

aux prepare_vg 9

lvcreate -aey --nosync -i2 -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv1

lvcreate -aey --nosync -i2 -l4 --type mirror -m1 --mirrorlog core -n $lv2 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv2

lvcreate -aey --nosync -i3 -l3 --type mirror -m1 --mirrorlog core -n $lv3 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv3

lvcreate -aey --nosync -i4 -l4 --type mirror -m1 --mirrorlog core -n $lv4 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv4

lvcreate -aey --nosync -i2 -l2 --type mirror -m2 --mirrorlog core -n $lv5 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv5

lvcreate -aey --nosync -i3 -l3 --type mirror -m2 --mirrorlog core -n $lv6 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv6

lvcreate -aey --nosync -i2 -l2 --type mirror -m3 --mirrorlog core -n $lv7 $vg 2>&1 | tee log
not grep "Rounding" log
check mirror_images_redundant $vg $lv7

lvremove -ff $vg

lvcreate -aey --nosync -i3 -l4 --type mirror -m1 --mirrorlog core -n $lv1 $vg 2>&1 | tee log
grep "Rounding size .*(4 extents) up to .*(6 extents)" log

lvcreate -aey --nosync -i3 -l4 --type mirror -m2 --mirrorlog core -n $lv2 $vg 2>&1 | tee log
grep "Rounding size .*(4 extents) up to .*(6 extents)" log

lvcreate -aey --nosync -i3 -l2 --type mirror -m2 --mirrorlog core -n $lv3 $vg 2>&1 | tee log
grep "Rounding size .*(2 extents) up to .*(3 extents)" log

lvremove -ff $vg
