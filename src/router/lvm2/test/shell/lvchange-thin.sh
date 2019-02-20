#!/usr/bin/env bash

# Copyright (C) 2013-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

aux have_thin 1 0 0 || skip

aux prepare_pvs 3

vgcreate $SHARED -s 128k $vg  "$dev1" "$dev2"
vgcreate $SHARED -s 128k $vg2 "$dev3"

lvcreate -L10M -T $vg/pool

# When PV does not support discard
# tests for checking thin-pool discard passdown are skipped
pvmajor=$(get pv_field "$dev1" major)
pvminor=$(get pv_field "$dev1" minor)
test "$(< "/sys/dev/block/$pvmajor\:$pvminor/queue/discard_granularity")" -ne 0 || \
        no_discard=1

#
# Check change operations on a thin-pool without any thin LV
#
# discards_ARG  (default is passdown)
test -n "$no_discard" || check grep_dmsetup status $vg-pool " discard_passdown" || {
	# trace device layout
	grep -r "" /sys/block/*
	die "Device was expected to support passdown"
}

lvchange --discards nopassdown $vg/pool
check grep_dmsetup table $vg-pool " no_discard_passdown"
test -n "$no_discard" || check grep_dmsetup status $vg-pool " no_discard_passdown"

lvchange --discards passdown $vg/pool
check grep_dmsetup table $vg-pool -v "passdown"
test -n "$no_discard" || check grep_dmsetup status $vg-pool " discard_passdown"

# zero_ARG  (default is 'yes')
check grep_dmsetup table $vg-pool -v "zeroing"
lvchange --zero n $vg/pool
check grep_dmsetup table $vg-pool " skip_block_zeroing"
lvchange --zero y $vg/pool
check grep_dmsetup table $vg-pool -v "zeroing"

# errorwhenfull_ARG  (default is 'no')
check grep_dmsetup status $vg-pool "queue_if_no_space"
lvchange --errorwhenfull y $vg/pool
check grep_dmsetup status $vg-pool "error_if_no_space"
check grep_dmsetup table $vg-pool "error_if_no_space"
lvchange --errorwhenfull n $vg/pool
check grep_dmsetup status $vg-pool "queue_if_no_space"
check grep_dmsetup table $vg-pool -v "error_if_no_space"


# Attach thin volume
lvcreate -V10M -n $lv1 $vg/pool
lvcreate -L10M -n $lv2 $vg

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
#should lvchange -p r $vg/pool
#should lvchange -p rw $vg/pool

# readahead_ARG
lvchange -r none $vg/$lv1
lvchange -r auto $vg/$lv1
# FIXME
# Think about more support

# minor_ARG
lvchange --yes -M y --minor 234 --major 253 $vg/$lv1
lvchange -M n $vg/$lv1

# cannot change major minor for pools
not lvchange --yes -M y --minor 235 --major 253 $vg/pool
not lvchange -M n $vg/pool

# addtag_ARG
lvchange --addtag foo $vg/$lv1
lvchange --addtag foo $vg/pool

# deltag_ARG
lvchange --deltag foo $vg/$lv1
lvchange --deltag foo $vg/pool

# discards_ARG
lvchange --discards nopassdown $vg/pool
check grep_dmsetup table $vg-pool-tpool " no_discard_passdown"
test -n "$no_discard" || check grep_dmsetup status $vg-pool-tpool " no_discard_passdown"
lvchange --discards passdown $vg/pool
check grep_dmsetup table $vg-pool-tpool -v "passdown"
test -n "$no_discard" || check grep_dmsetup status $vg-pool-tpool " discard_passdown"

# zero_ARG
lvchange --zero n $vg/pool
check grep_dmsetup table $vg-pool-tpool " skip_block_zeroing"
lvchange --zero y $vg/pool
check grep_dmsetup table $vg-pool-tpool -v "zeroing"


lvchange --errorwhenfull y $vg/pool
check grep_dmsetup status $vg-pool-tpool "error_if_no_space"
check grep_dmsetup table $vg-pool-tpool "error_if_no_space"
lvchange --errorwhenfull n $vg/pool
check grep_dmsetup status $vg-pool-tpool "queue_if_no_space"
check grep_dmsetup table $vg-pool-tpool -v "error_if_no_space"


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

# Fails here since it cannot clear device header
not lvcreate -Zy -L10 -n $lv3 $vg2
# OK when zeroing is disabled
lvcreate -Zn -L10 -n $lv3 $vg2
check inactive $vg2 $lv3

aux lvmconf "activation/volume_list = [ \"$vg2\" ]"
vgchange -an $vg
vgchange -ay $vg $vg2
lvs -a -o+lv_active $vg $vg2

aux lvmconf "activation/volume_list = [ \"$vg\", \"$vg2\" ]"

vgremove -ff $vg $vg2
