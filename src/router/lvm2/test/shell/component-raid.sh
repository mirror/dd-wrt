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

# Exercise activation of raid component devices


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_raid 1 3 0 || skip

aux prepare_vg 5 80

lvcreate --type raid1 -L 2 -m 1 -n $lv1 $vg
lvchange -an $vg

lvs -a $vg

for k in 1 2
do

# Activate supported components
for j in 0 1
do
for i in ${lv1}_rimage_$j ${lv1}_rmeta_$j
do
	test ! -e "$DM_DEV_DIR/$vg/$i"
	lvchange -ay -y $vg/$i
	# check usable link is there
	test -e "$DM_DEV_DIR/$vg/$i"
done
done

# Deactivation works in 1st. pass
test $k -eq 2 || lvchange -an $vg

done

# And final removal works
vgremove -f $vg
