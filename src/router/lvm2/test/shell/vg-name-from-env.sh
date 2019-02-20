#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.

test_description='Test the vg name for an lv from env var'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2

pvcreate "$dev1"
pvcreate "$dev2"

vgcreate $SHARED $vg1 "$dev1"
vgcreate $SHARED $vg2 "$dev2"

export LVM_VG_NAME=$vg1

# should use env
lvcreate -n $lv1 -l 2
lvcreate -n $lv3 -l 2

lvcreate -n $lv2 -l 2 $vg2
lvcreate -n $lv4 -l 2 $vg2

lvs >err
grep $lv1 err
grep $lv3 err
grep $lv2 err
grep $lv4 err

not lvs $vg1 >err
not grep $lv1 err
not grep $lv3 err
not grep $lv2 err
not grep $lv4 err

not lvs $vg2 >err
not grep $lv1 err
not grep $lv3 err
not grep $lv2 err
not grep $lv4 err

lvs $lv1 >err
grep $lv1 err
not grep $lv3 err
not grep $lv2 err
not grep $lv4 err

lvs $lv1 $lv3 >err
grep $lv1 err
grep $lv3 err
not grep $lv2 err
not grep $lv4 err

# should use env and fail to fine lv4 in vg1
not lvs $lv4 >err
not grep $lv1 err
not grep $lv3 err
not grep $lv2 err
not grep $lv4 err

lvs $vg2/$lv4 >err
not grep $lv1 err
not grep $lv3 err
not grep $lv2 err
grep $lv4 err

lvs $vg2/$lv2 $vg2/$lv4 >err
not grep $lv1 err
not grep $lv3 err
grep $lv2 err
grep $lv4 err

# should use env
lvchange -an $lv3
lvremove $lv3
not lvremove $lv4

lvs >err
grep $lv1 err
not grep $lv3 err
grep $lv2 err
grep $lv4 err

# should use env
lvcreate -n $lv3 -l 2
lvchange --addtag foo $lv3
lvchange -an $lv3

# lvremove by tag should apply to all vgs, not env vg
lvchange --addtag foo $vg2/$lv4
lvchange -an $vg2/$lv4
lvremove @foo

lvs >err
grep $lv1 err
not grep $lv3 err
grep $lv2 err
not grep $lv4 err

vgremove -ff $vg1 $vg2
