#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise activation of mirror component devices

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 5 80

lvcreate -aey --type mirror -L 2 -m 1 -n $lv1 $vg
lvchange -an $vg

lvs -a

lvchange -an $vg

for k in 1 2
do

# Activate supported components
for i in ${lv1}_mimage_0 ${lv1}_mimage_1 ${lv1}_mlog
do
	test ! -e "$DM_DEV_DIR/$vg/$i"
	lvchange -ay -y $vg/$i
	# check usable link is there
	test -e "$DM_DEV_DIR/$vg/$i"
done

# Deactivation works in 1st. pass
test $k -eq 2 || lvchange -an $vg

done

# Cannot be resized
not lvextend -L+20 $vg/$lv1 |& tee err
grep "Cannot resize" err

not lvresize -L-20 $vg/$lv1 |& tee err
grep "Cannot resize" err

# Cannot be converted
lvcreate -aey -L10 -n $lv2 $vg
not lvconvert -y -s $vg/$lv1 $lv2 |& tee err
grep "Cannot use" err

# Cannot be splitted
not lvconvert --splitmirrors 1 -n split $vg/$lv1 |& tee err
grep "Cannot convert" err

# Cannot add new leg
not lvconvert -m+1 $vg/$lv1 |& tee err
grep "Cannot convert" err

lvs -a

vgremove -f $vg
