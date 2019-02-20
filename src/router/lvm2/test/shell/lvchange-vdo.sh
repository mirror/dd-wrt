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


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_vdo 6 2 0 || skip

aux prepare_vg 2 6400

lvcreate --vdo -L5G -n $lv1 $vg/vdopool

# deduplication_ARG  (default is 'yes')
# compression_ARG  (default is 'yes')

# Wait till index gets openned
for i in {1..10} ; do
	sleep .1
	check grep_dmsetup status $vg-vdopool " online online " || continue
	break
done


# compression_ARG
lvchange --compression n $vg/vdopool
check grep_dmsetup status $vg-vdopool " online offline "
lvchange --compression y $vg/vdopool
check grep_dmsetup status $vg-vdopool " online online "

# dedulication_ARG
lvchange --deduplication n $vg/vdopool
check grep_dmsetup status $vg-vdopool " offline online "
lvchange --deduplication y $vg/vdopool
check grep_dmsetup status $vg-vdopool " online online "


lvchange --compression n --deduplication n $vg/vdopool
check grep_dmsetup status $vg-vdopool " offline offline "


lvchange -an $vg/$lv1

# Test activation
lvchange -aly $vg/$lv1
check active $vg $lv1

lvchange -aln $vg/$lv1
check inactive $vg $lv1

# Test for allowable changes
#
# contiguous_ARG
lvchange -C y $vg/$lv1
lvchange -C n $vg/$lv1

# permission_ARG
lvchange -p r $vg/$lv1
lvchange -p rw $vg/$lv1

# FIXME
#should lvchange -p r $vg/vdopool
#should lvchange -p rw $vg/vdopool

# readahead_ARG
lvchange -r none $vg/$lv1
lvchange -r auto $vg/$lv1
# FIXME
# Think about more support

# minor_ARG
lvchange --yes -M y --minor 234 --major 253 $vg/$lv1
lvchange -M n $vg/$lv1

# cannot change major minor for pools
not lvchange --yes -M y --minor 235 --major 253 $vg/vdopool
not lvchange -M n $vg/vdopool

# addtag_ARG
lvchange --addtag foo $vg/$lv1
lvchange --addtag foo $vg/vdopool

# deltag_ARG
lvchange --deltag foo $vg/$lv1
lvchange --deltag foo $vg/vdopool


#
# Test for disallowed metadata changes
#
# resync_ARG
not lvchange --resync $vg/$lv1

# alloc_ARG
#not lvchange --alloc anywhere $vg/$lv1

# discards_ARG
not lvchange --discards ignore $vg/$lv1

# zero_ARG
not lvchange --zero y $vg/$lv1


#
# Ensure that allowed args don't cause disallowed args to get through
#
not lvchange --resync -ay $vg/$lv1
not lvchange --resync --addtag foo $vg/$lv1

#
# Play with tags and activation
#
TAG=$(uname -n)
aux lvmconf "activation/volume_list = [ \"$vg/$lv2\", \"@mytag\" ]"

lvchange -ay $vg/$lv1
check inactive $vg $lv1

lvchange --addtag mytag $vg/$lv1

lvchange -ay @mytag_fake
check inactive $vg $lv1

lvchange -ay $vg/$lv1
# Volume has matching tag
check active $vg $lv1
lvchange -an $vg/$lv1

lvchange -ay @mytag
check active $vg $lv1

vgremove -ff $vg
