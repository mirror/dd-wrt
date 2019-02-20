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

SKIP_WITH_LVMPOLLD=1


. lib/inittest

aux have_thin 1 0 0 || skip
aux prepare_pvs 1 16
get_devs

aux lvmconf "metadata/record_lvs_history=1"

vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

lvcreate -l100%FREE -T ${vg}/pool

# Thin snap chain with 2 branches starting at lv3.
#
# lv1 --> lv2 --> lv3 --> lv4 --> lv5
#                    \
#                     --> lv6 --> lv7

lvcreate -V1 -T ${vg}/pool -n lv1
lvcreate -s ${vg}/lv1 -n lv2
lvcreate -s ${vg}/lv2 -n lv3
lvcreate -s ${vg}/lv3 -n lv4
lvcreate -s ${vg}/lv4 -n lv5
lvcreate -s ${vg}/lv3 -n lv6
lvcreate -s ${vg}/lv6 -n lv7

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "lv2,lv3,lv4,lv5,lv6,lv7"

check lvh_field ${vg}/lv2 full_ancestors "lv1"
check lvh_field ${vg}/lv2 full_descendants "lv3,lv4,lv5,lv6,lv7"

check lvh_field ${vg}/lv3 full_ancestors "lv2,lv1"
check lvh_field ${vg}/lv3 full_descendants "lv4,lv5,lv6,lv7"

check lvh_field ${vg}/lv4 full_ancestors "lv3,lv2,lv1"
check lvh_field ${vg}/lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "lv4,lv3,lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/lv6 full_ancestors "lv3,lv2,lv1"
check lvh_field ${vg}/lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "lv6,lv3,lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""


# lv1 --> lv2 --> lv3 --> -lv4 --> lv5
#                    \
#                     -->  lv6 --> lv7
lvremove -ff ${vg}/lv4

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "lv2,lv3,lv6,lv7,-lv4,lv5"

check lvh_field ${vg}/lv2 full_ancestors "lv1"
check lvh_field ${vg}/lv2 full_descendants "lv3,lv6,lv7,-lv4,lv5"

check lvh_field ${vg}/lv3 full_ancestors "lv2,lv1"
check lvh_field ${vg}/lv3 full_descendants "lv6,lv7,-lv4,lv5"

check lvh_field ${vg}/-lv4 full_ancestors "lv3,lv2,lv1"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,lv3,lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/lv6 full_ancestors "lv3,lv2,lv1"
check lvh_field ${vg}/lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "lv6,lv3,lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""


# lv1 --> lv2 --> -lv3 --> -lv4 --> lv5
#                     \
#                      -->  lv6 --> lv7
lvremove -ff ${vg}/lv3

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "lv2,-lv3,-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/lv2 full_ancestors "lv1"
check lvh_field ${vg}/lv2 full_descendants "-lv3,-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/-lv3 full_ancestors "lv2,lv1"
check lvh_field ${vg}/-lv3 full_descendants "-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/-lv4 full_ancestors "-lv3,lv2,lv1"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv3,lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/lv6 full_ancestors "-lv3,lv2,lv1"
check lvh_field ${vg}/lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "lv6,-lv3,lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""

# lv1 --> -lv2 --> -lv3 --> -lv4 --> lv5
#                      \
#                       -->  lv6 --> lv7
lvremove -ff $vg/lv2

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "-lv2,-lv3,-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/-lv2 full_ancestors "lv1"
check lvh_field ${vg}/-lv2 full_descendants "-lv3,-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/-lv3 full_ancestors "-lv2,lv1"
check lvh_field ${vg}/-lv3 full_descendants "-lv4,lv5,lv6,lv7"

check lvh_field ${vg}/-lv4 full_ancestors "-lv3,-lv2,lv1"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv3,-lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/lv6 full_ancestors "-lv3,-lv2,lv1"
check lvh_field ${vg}/lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "lv6,-lv3,-lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""

# lv1 --> -lv2 --> -lv3 --> -lv4 --> lv5
#                      \
#                       --> -lv6 --> lv7
lvremove -ff ${vg}/lv6

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "-lv2,-lv3,-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv2 full_ancestors "lv1"
check lvh_field ${vg}/-lv2 full_descendants "-lv3,-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv3 full_ancestors "-lv2,lv1"
check lvh_field ${vg}/-lv3 full_descendants "-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv4 full_ancestors "-lv3,-lv2,lv1"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv3,-lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/-lv6 full_ancestors "-lv3,-lv2,lv1"
check lvh_field ${vg}/-lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "-lv6,-lv3,-lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""

# lv1 --> -lv2 -----------> -lv4 --> lv5
#                      \
#                       --> -lv6 --> lv7
lvremove -ff ${vg}/-lv3

check lvh_field ${vg}/lv1 full_ancestors ""
check lvh_field ${vg}/lv1 full_descendants "-lv2,-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv2 full_ancestors "lv1"
check lvh_field ${vg}/-lv2 full_descendants "-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv4 full_ancestors "-lv2,lv1"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv2,lv1"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/-lv6 full_ancestors "-lv2,lv1"
check lvh_field ${vg}/-lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "-lv6,-lv2,lv1"
check lvh_field ${vg}/lv7 full_descendants ""

#  -lv2 -----------> -lv4 --> lv5
#               \
#                --> -lv6 --> lv7

lvremove --nohistory -ff ${vg}/lv1

check lvh_field ${vg}/-lv2 full_ancestors ""
check lvh_field ${vg}/-lv2 full_descendants "-lv4,lv5,-lv6,lv7"

check lvh_field ${vg}/-lv4 full_ancestors "-lv2"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv2"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/-lv6 full_ancestors "-lv2"
check lvh_field ${vg}/-lv6 full_descendants "lv7"

check lvh_field ${vg}/lv7 full_ancestors "-lv6,-lv2"
check lvh_field ${vg}/lv7 full_descendants ""

#  -lv2 -----------> -lv4 --> lv5
#
#                             lv7
lvremove --nohistory -ff ${vg}/-lv6

check lvh_field ${vg}/-lv2 full_ancestors ""
check lvh_field ${vg}/-lv2 full_descendants "-lv4,lv5"

check lvh_field ${vg}/-lv4 full_ancestors "-lv2"
check lvh_field ${vg}/-lv4 full_descendants "lv5"

check lvh_field ${vg}/lv5 full_ancestors "-lv4,-lv2"
check lvh_field ${vg}/lv5 full_descendants ""

check lvh_field ${vg}/lv7 full_ancestors ""
check lvh_field ${vg}/lv7 full_descendants ""

vgremove -ff $vg
