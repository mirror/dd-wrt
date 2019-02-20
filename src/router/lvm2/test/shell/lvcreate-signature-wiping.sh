#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# 'Exercise signature wiping during lvcreate'


SKIP_WITH_LVMPOLLD=1

. lib/inittest

init_lv_() {
	mkswap "$DM_DEV_DIR/$vg/$lv1"
}

test_blkid_() {
	local type
	type=$(blkid -s TYPE -o value -c /dev/null "$DM_DEV_DIR/$vg/$lv1")
	test "$type" = "swap"
}

test_msg_() {
	grep "Wiping swap signature" out
}

aux prepare_vg

# lvcreate wipes signatures when found on newly created LV - test this on "swap".
# Test all combinatios with -Z{y|n} and -W{y|n} and related lvm.conf settings.

lvcreate -l1 -n $lv1 $vg
init_lv_
# This system has unusable blkid (does not recognize small swap, needs fix...)
test_blkid_ || skip
lvremove -f $vg/$lv1

aux lvmconf "allocation/wipe_signatures_when_zeroing_new_lvs = 0"

lvcreate -y -Zn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
test_blkid_
lvremove -f $vg/$lv1

lvcreate -y -Zn -Wn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
test_blkid_
lvremove -f $vg/$lv1

lvcreate -y -Zn -Wy -l1 -n $lv1 $vg 2>&1 | tee out
test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -Wn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -Wy -l1 -n $lv1 $vg 2>&1 | tee out
test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1


aux lvmconf "allocation/wipe_signatures_when_zeroing_new_lvs = 1"

lvcreate -y -Zn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
test_blkid_
lvremove -f $vg/$lv1

lvcreate -y -Zn -Wn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
test_blkid_
lvremove -f $vg/$lv1

lvcreate -y -Zn -Wy -l1 -n $lv1 $vg 2>&1 | tee out
test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -l1 -n $lv1 $vg 2>&1 | tee out
test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -Wn -l1 -n $lv1 $vg 2>&1 | tee out
not test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

lvcreate -y -Zy -Wy -l1 -n $lv1 $vg 2>&1 | tee out
test_msg_
not test_blkid_
init_lv_
lvremove -f $vg/$lv1

vgremove -f $vg
