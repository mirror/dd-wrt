#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise toollib process_each_lv'

SKIP_WITH_LVMPOLLD=1

. lib/inittest


aux prepare_devs 10

#
# process_each_lv is used by a number of lv commands:
# lvconvert lv      (none is error)
# lvchange  vg|lv   (none is error)
# lvremove  vg|lv   (none is error)
# lvdisplay [vg|lv] (none is all)
# vgmknodes [vg|lv] (none is all)
# lvs       [vg|lv] (none is all)
# lvscan            (none is all)
#
# (lv can also be a tag matching an lv tag, and
#  vg can also be a tag matching a vg tag.)
#
# The logic in process_each_vl is mainly related to
# selecting which vgs/lvs to process.
#

#
# test lvremove vg|lv names
#

prepare_vgs_() {
	# set up vgs/lvs that we will remove
	vgcreate $SHARED $vg1 "$dev1" "$dev2"
	vgcreate $SHARED $vg2 "$dev3" "$dev4"
	vgcreate $SHARED $vg3 "$dev5" "$dev6"
	vgcreate $SHARED $vg4 "$dev7" "$dev8"
	vgcreate $SHARED $vg5 "$dev9" "$dev10"
	lvcreate -Zn -an -l 2 -n $lv1 $vg1
	lvcreate -Zn -an -l 2 -n $lv1 $vg2
	lvcreate -Zn -an -l 2 -n $lv2 $vg2
	lvcreate -Zn -an -l 2 -n $lv1 $vg3
	lvcreate -Zn -an -l 2 -n $lv2 $vg3
	lvcreate -Zn -an -l 2 -n $lv3 $vg3
	lvcreate -Zn -an -l 2 -n $lv1 $vg5
	lvcreate -Zn -an -l 2 -n $lv2 $vg5
	lvcreate -Zn -an -l 2 -n $lv3 $vg5
	lvcreate -Zn -an -l 2 -n $lv4 $vg5
	lvcreate -Zn -an -l 2 -n $lv5 $vg5
}

#
#
#
prepare_vgs_

not lvremove
not lvremove garbage
not lvremove $vg1/garbage

lvremove $vg1
check lv_exists $vg1
check lv_not_exists $vg1 $lv1
vgremove $vg1

lvremove $vg2
check lv_exists $vg2
check lv_not_exists $vg2 $lv1 $lv2
vgremove $vg2

lvremove $vg3/$lv1
lvremove $vg3/$lv2 $vg3/$lv3
check lv_exists $vg3
check lv_not_exists $vg3 $lv1 $lv2 $lv3
vgremove $vg3

lvremove $vg4
check lv_exists $vg4
vgremove $vg4

lvremove $vg5/$lv1 $vg5 $vg5/$lv3
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5
vgremove $vg5


#
# test lvremove vg|lv names from multiple vgs
#
prepare_vgs_

lvremove $vg2 $vg3/$lv3 $vg5/$lv1
check lv_not_exists $vg2 $lv1 $lv2
check lv_not_exists $vg3 $lv3
check lv_not_exists $vg5 $lv1

lvremove $vg2 $vg1
check lv_not_exists $vg1 $lv1

lvremove $vg3/$lv1 $vg3 $vg4 $vg5/$lv2
check lv_not_exists $vg3 $lv1 $lv2
check lv_not_exists $vg5 $lv2

lvremove $vg5 $vg1 $vg5/$lv3
check lv_not_exists $vg5 $lv3 $lv4 $lv5

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove @lvtags
#
prepare_vgs_

lvchange --addtag V1L1 $vg1/$lv1
lvchange --addtag V2L1 $vg2/$lv1
lvchange --addtag V2L2 $vg2/$lv2
lvchange --addtag V23  $vg2/$lv1
lvchange --addtag V23  $vg2/$lv2
lvchange --addtag V23  $vg3/$lv1
lvchange --addtag V23  $vg3/$lv2
lvchange --addtag V23  $vg3/$lv3
lvchange --addtag V3L2 $vg3/$lv2
lvchange --addtag V3L3A $vg3/$lv3
lvchange --addtag V3L3B $vg3/$lv3
lvchange --addtag V5L1   $vg5/$lv1
lvchange --addtag V5L234 $vg5/$lv2
lvchange --addtag V5L234 $vg5/$lv3
lvchange --addtag V5L234 $vg5/$lv4
lvchange --addtag V5L5   $vg5/$lv5
vgchange -an $vg1 $vg2 $vg3 $vg4 $vg5

# verify all exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2 $lv3
check lv_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5

lvremove @garbage

lvremove @V3L3A
check lv_not_exists $vg3 $lv3
# verify unremoved still exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2
check lv_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5

lvremove @V5L234
check lv_not_exists $vg5 $lv2 $lv3 $lv4
# verify unremoved still exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2
check lv_exists $vg5 $lv1 $lv5

lvremove @V5L1 @V5L5
check lv_not_exists $vg5 $lv1 $lv5
# verify unremoved still exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2

lvremove @V23 @V1L1 @V3L2
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg2 $lv1 $lv2
check lv_not_exists $vg3 $lv1 $lv2

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove @vgtags
#
prepare_vgs_

vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5
vgchange -an $vg1 $vg2 $vg3 $vg4 $vg5

lvremove @V4
# verify unremoved exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2 $lv3
check lv_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5

lvremove @V5
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5
# verify unremoved exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv2 $lv3

lvremove @V1 @V23
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg2 $lv1 $lv2
check lv_not_exists $vg3 $lv1 $lv2 $lv3

vgremove $vg1 $vg2 $vg3 $vg4 $vg5

#
#
#
prepare_vgs_

vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5

lvremove @V35 @V5
check lv_not_exists $vg3 $lv1 $lv2 /$lv3
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5
# verify unremoved exist
check lv_exists $vg1 $lv1
check lv_exists $vg2 $lv1 $lv2

lvremove @V1 @V23
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg2 $lv1 $lv2

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove vg|lv names and @lvtags
#
prepare_vgs_

lvchange --addtag V1L1 $vg1/$lv1
lvchange --addtag V2L1 $vg2/$lv1
lvchange --addtag V2L2 $vg2/$lv2
lvchange --addtag V23  $vg2/$lv1
lvchange --addtag V23  $vg2/$lv2
lvchange --addtag V23  $vg3/$lv1
lvchange --addtag V23  $vg3/$lv2
lvchange --addtag V23  $vg3/$lv3
lvchange --addtag V3L2 $vg3/$lv2
lvchange --addtag V3L3A $vg3/$lv3
lvchange --addtag V3L3B $vg3/$lv3
lvchange --addtag V5L1   $vg5/$lv1
lvchange --addtag V5L234 $vg5/$lv2
lvchange --addtag V5L234 $vg5/$lv3
lvchange --addtag V5L234 $vg5/$lv4
lvchange --addtag V5L5   $vg5/$lv5
vgchange -an $vg1 $vg2 $vg3 $vg4 $vg5

lvremove $vg1/$lv1 @V3L2 @V5L234
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg3 $lv2
check lv_not_exists $vg5 $lv2 $lv3 $lv4
# verify unremoved exist
check lv_exists $vg2 $lv1 $lv2
check lv_exists $vg3 $lv1 $lv3
check lv_exists $vg5 $lv1 $lv5

lvremove $vg2/$lv1 @V23 $vg5/$lv1 @V5L5

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove vg|lv names and @vgtags
#
prepare_vgs_

vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5

lvremove $vg1/$lv1 @V35
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg3 $lv1 $lv2 $lv3
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5
# verify unremoved exist
check lv_exists $vg2 $lv1 $lv2

lvremove $vg2/$lv1 @V23 $vg2/$lv2

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove @lvtags and @vgtags
#
prepare_vgs_

lvchange --addtag V1L1 $vg1/$lv1
lvchange --addtag V2L1 $vg2/$lv1
lvchange --addtag V2L2 $vg2/$lv2
lvchange --addtag V23  $vg2/$lv1
lvchange --addtag V23  $vg2/$lv2
lvchange --addtag V23  $vg3/$lv1
lvchange --addtag V23  $vg3/$lv2
# to check that vg tag @V23 includes this
# lvchange --addtag V23  $vg3/$lv3
lvchange --addtag V3L2 $vg3/$lv2
lvchange --addtag V3L3A $vg3/$lv3
lvchange --addtag V3L3B $vg3/$lv3
lvchange --addtag V5L1   $vg5/$lv1
lvchange --addtag V5L234 $vg5/$lv2
lvchange --addtag V5L234 $vg5/$lv3
lvchange --addtag V5L234 $vg5/$lv4
lvchange --addtag V5L5   $vg5/$lv5
vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5

lvremove @V23 @V35
check lv_not_exists $vg2 $lv1 $lv2
check lv_not_exists $vg3 $lv1 $lv2 $lv3
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4 $lv5
# verify unremoved exist
check lv_exists $vg1 $lv1

lvremove @V1 @V1L1
check lv_not_exists $vg1 $lv1

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvremove vg|lv names and @lvtags and @vgtags
#
prepare_vgs_

lvchange --addtag V1L1 $vg1/$lv1
lvchange --addtag V2L1 $vg2/$lv1
lvchange --addtag V2L2 $vg2/$lv2
lvchange --addtag V23  $vg2/$lv1
lvchange --addtag V23  $vg2/$lv2
lvchange --addtag V23  $vg3/$lv1
lvchange --addtag V23  $vg3/$lv2
# to check that vg tag @V23 includes this
# lvchange --addtag V23  $vg3/$lv3
lvchange --addtag V3L2 $vg3/$lv2
lvchange --addtag V3L3A $vg3/$lv3
lvchange --addtag V3L3B $vg3/$lv3
lvchange --addtag V5L1   $vg5/$lv1
lvchange --addtag V5L234 $vg5/$lv2
lvchange --addtag V5L234 $vg5/$lv3
lvchange --addtag V5L234 $vg5/$lv4
lvchange --addtag V5L5   $vg5/$lv5
vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5

lvremove $vg1/$lv1 @V23 @V5L5
check lv_not_exists $vg1 $lv1
check lv_not_exists $vg2 $lv1 $lv2
check lv_not_exists $vg3 $lv1 $lv2 $lv3
check lv_not_exists $vg5 $lv5
# verify unremoved exist
check lv_exists $vg5 $lv1 $lv2 $lv3 $lv4

lvremove $vg5/$lv2 @V5L234 @V5
check lv_not_exists $vg5 $lv1 $lv2 $lv3 $lv4

vgremove $vg1 $vg2 $vg3 $vg4 $vg5


#
# test lvs: empty, vg(s), lv(s), vgtag(s), lvtag(s), garbage, combinations
#
prepare_vgs_

lvchange --addtag V1L1 $vg1/$lv1
lvchange --addtag V2L1 $vg2/$lv1
lvchange --addtag V2L2 $vg2/$lv2
lvchange --addtag V23  $vg2/$lv1
lvchange --addtag V23  $vg2/$lv2
lvchange --addtag V23  $vg3/$lv1
lvchange --addtag V23  $vg3/$lv2
lvchange --addtag V23  $vg3/$lv3
lvchange --addtag V3L2 $vg3/$lv2
lvchange --addtag V3L3A $vg3/$lv3
lvchange --addtag V3L3B $vg3/$lv3
lvchange --addtag V5L1   $vg5/$lv1
lvchange --addtag V5L234 $vg5/$lv2
lvchange --addtag V5L234 $vg5/$lv3
lvchange --addtag V5L234 $vg5/$lv4
lvchange --addtag V5L5   $vg5/$lv5
vgchange --addtag V1  $vg1
vgchange --addtag V23 $vg2
vgchange --addtag V23 $vg3
vgchange --addtag V35 $vg3
vgchange --addtag V4  $vg4
vgchange --addtag V35 $vg5
vgchange --addtag V5  $vg5

# empty
lvs -o vg_name,lv_name --separator '-' >err
grep $vg1-$lv1 err
grep $vg2-$lv1 err
grep $vg2-$lv2 err
grep $vg3-$lv1 err
grep $vg3-$lv2 err
grep $vg3-$lv3 err
grep $vg5-$lv1 err
grep $vg5-$lv2 err
grep $vg5-$lv3 err
grep $vg5-$lv4 err
grep $vg5-$lv5 err

# vg
lvs -o vg_name,lv_name --separator '-' $vg1 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# vgs
lvs -o vg_name,lv_name --separator '-' $vg1 $vg2 >err
grep $vg1-$lv1 err
grep $vg2-$lv1 err
grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# lv
lvs -o vg_name,lv_name --separator '-' $vg1/$lv1 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# lvs
lvs -o vg_name,lv_name --separator '-' $vg1/$lv1 $vg2/$lv1 $vg2/$lv2 >err
grep $vg1-$lv1 err
grep $vg2-$lv1 err
grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# vgtag
lvs -o vg_name,lv_name --separator '-' @V1 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# vgtags
lvs -o vg_name,lv_name --separator '-' @V1 @V35 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
grep $vg3-$lv1 err
grep $vg3-$lv2 err
grep $vg3-$lv3 err
grep $vg5-$lv1 err
grep $vg5-$lv2 err
grep $vg5-$lv3 err
grep $vg5-$lv4 err
grep $vg5-$lv5 err

# lvtag
lvs -o vg_name,lv_name --separator '-' @V1L1 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# lvtags
lvs -o vg_name,lv_name --separator '-' @V1L1 @V5L234 >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
grep $vg5-$lv2 err
grep $vg5-$lv3 err
grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# vg and lv and vgtag and lvtag
lvs -o vg_name,lv_name --separator '-' $vg2 $vg5/$lv5 @V1 @V5L234 >err
grep $vg1-$lv1 err
grep $vg2-$lv1 err
grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
grep $vg5-$lv2 err
grep $vg5-$lv3 err
grep $vg5-$lv4 err
grep $vg5-$lv5 err

# garbage name gives an error if used without a tag

not lvs -o vg_name,lv_name --separator '-' garbage >err
not grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

not lvs -o vg_name,lv_name --separator '-' $vg1/$lv1 garbage >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# garbage name does not give an error if used with a tag

lvs -o vg_name,lv_name --separator '-' @V1 garbage >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

lvs -o vg_name,lv_name --separator '-' @garbage garbage >err
not grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

# garbage tag never gives an error

lvs -o vg_name,lv_name --separator '-' @V1 @garbage >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

lvs -o vg_name,lv_name --separator '-' $vg1/$lv1 @garbage >err
grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

lvs -o vg_name,lv_name --separator '-' @garbage >err
not grep $vg1-$lv1 err
not grep $vg2-$lv1 err
not grep $vg2-$lv2 err
not grep $vg3-$lv1 err
not grep $vg3-$lv2 err
not grep $vg3-$lv3 err
not grep $vg5-$lv1 err
not grep $vg5-$lv2 err
not grep $vg5-$lv3 err
not grep $vg5-$lv4 err
not grep $vg5-$lv5 err

vgremove -f $vg1 $vg2 $vg3 $vg4 $vg5
