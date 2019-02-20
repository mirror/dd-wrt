#!/usr/bin/env bash

# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
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

aux have_raid 1 3 0 || skip

aux prepare_vg 5

for deactivate in true false; do
# Extend RAID10 (2-stripes, 2-mirror)
	lvcreate --type raid10 -m 1 -i 2 -l 2 -n $lv1 $vg

	test $deactivate && lvchange -an $vg/$lv1

	lvresize -l +2 $vg/$lv1

	#check raid_images_contiguous $vg $lv1

# Reduce RAID10 (2-stripes, 2-mirror)

	should lvresize -y -l -2 $vg/$lv1

	#check raid_images_contiguous $vg $lv1

	lvremove -ff $vg
done
