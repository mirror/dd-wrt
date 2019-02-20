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

# Exercise activation of thin component devices


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_thin 1 0 0 || skip

aux prepare_vg 5 80

lvcreate -T -L2 -V20 $vg/pool -n $lv1

lvs -a

lvchange -an $vg

for i in pool_tdata pool_tmeta
do
	lvchange -ay -y $vg/$i
	# check usable is there
	test -e "$DM_DEV_DIR/$vg/$i"
done

lvs -a

# When component LVs are active, thin-pool cannot be actived
not lvcreate -V20 $vg/pool


# FIXME: ATM removal of thin volumes goes 1-by-1 and needs to activate thin-pool.
# And thin-pool cannot be activate thus vgremove fails.
# To fix this it would need to be able to remove volumes without activation.
should vgremove -f $vg

# -ff bypass activation failure
vgremove -ff $vg
