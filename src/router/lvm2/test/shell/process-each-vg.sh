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

test_description='Exercise toollib process_each_vg'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 6

#
# process_each_vg is used by a number of vg commands;
# use 'vgremove' and 'vgs' to test it.
#
# The logic in process_each_vg is mainly related to
# selecting which vg's to process.
#

#
# set up four vgs that we will remove
#
vgcreate $SHARED $vg1 "$dev1"
vgcreate $SHARED $vg2 "$dev2"
vgcreate $SHARED $vg3 "$dev3"
vgcreate $SHARED $vg4 "$dev4"

# these two vgs will not be removed
vgcreate $SHARED $vg5 "$dev5"
vgchange --addtag tagvg5 $vg5
lvcreate -l 4 -n $lv1 $vg5
vgcreate $SHARED $vg6 "$dev6"
lvcreate -l 4 -n $lv2 $vg6

# should fail without any arg
not vgremove

# should succeed
vgremove $vg1
vgremove $vg2 $vg3 $vg4

# these should fail because they are already removed
not vgremove $vg1
not vgremove $vg2
not vgremove $vg3
not vgremove $vg4

# these should fail because they have lvs in them
not vgremove $vg5
not vgremove $vg6

# check that the vgs we removed are gone
not vgs $vg1
not vgs $vg2
not vgs $vg3
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

# should do nothing and fail
not vgremove garbage

# should find nothing to remove
vgremove @garbage

# should find nothing to remove
vgremove @$vg1

# should succeed
vgremove $vg1
not vgs $vg1

vgremove $vg2 $vg3 $vg4
not vgs $vg2
not vgs $vg3
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

vgremove @tagfoo
not vgs $vg1
not vgs $vg2

vgremove @tagfoo2 @tagbar
not vgs $vg3
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

vgremove $vg1 @tagfoo2
not vgs $vg1
not vgs $vg3

vgremove @tagbar $vg2
not vgs $vg2
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

vgremove @foo @tagfoo2 $vg1 $vg2
not vgs $vg1
not vgs $vg2
not vgs $vg3
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

vgremove @tagfoo $vg1 @tagfoo @tagfoo2 $vg3 @tagbar
not vgs $vg1
not vgs $vg2
not vgs $vg3
not vgs $vg4


#
# set up four vgs that we will remove
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

not vgremove garbage $vg1
not vgs $vg1

not vgremove $vg2 garbage
not vgs $vg2

vgremove $vg3 @garbage
not vgs $vg3

vgremove @garbage $vg4
not vgs $vg4


#
# end vgremove tests
# check that the two vgs we did not intend to remove
# are still there, and then remove them
#
vgs $vg5
vgs $vg6
vgremove -f $vg5
vgremove -f $vg6
not vgs $vg5
not vgs $vg6


#
# set up four vgs that we will report
#
vgcreate $SHARED --addtag tagfoo $vg1 "$dev1"
vgcreate $SHARED --addtag tagfoo $vg2 "$dev2"
vgcreate $SHARED --addtag tagfoo2 $vg3 "$dev3"
vgcreate $SHARED --addtag tagbar $vg4 "$dev4"
vgchange --addtag foo $vg4

vgs >err
grep $vg1 err
grep $vg2 err
grep $vg3 err
grep $vg4 err

vgs $vg1 $vg2 >err
grep $vg1 err
grep $vg2 err
not grep $vg3 err
not grep $vg4 err

vgs @tagfoo >err
grep $vg1 err
grep $vg2 err
not grep $vg3 err
not grep $vg4 err

vgs @tagfoo2 >err
grep $vg3 err
not grep $vg1 err
not grep $vg2 err
not grep $vg4 err

vgs @tagfoo2 @tagbar >err
grep $vg3 err
grep $vg4 err
not grep $vg1 err
not grep $vg2 err

vgs $vg1 @tagbar >err
grep $vg1 err
grep $vg4 err
not grep $vg2 err
not grep $vg3 err

vgs $vg1 @tagfoo >err
grep $vg1 err
grep $vg2 err
not grep $vg3 err
not grep $vg4 err

not vgs garbage >err
not grep $vg1 err
not grep $vg2 err
not grep $vg3 err
not grep $vg4 err

not vgs garbage $vg1 >err
grep $vg1 err
not grep $vg2 err
not grep $vg3 err
not grep $vg4 err

vgs @garbage @foo >err
grep $vg4 err
not grep $vg1 err
not grep $vg2 err
not grep $vg3 err

vgremove -f $vg1 $vg2 $vg3 $vg4

