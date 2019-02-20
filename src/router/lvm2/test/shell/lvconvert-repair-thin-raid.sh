#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test repairing of broken thin pool on raid


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_thin 1 0 0 || skip
aux have_raid 1 4 0 || skip

#
# To continue this test - we need real tools available
# When they are not present mark test as skipped, but still
# let proceed initial part which should work even without tools
#
aux have_tool_at_least "$LVM_TEST_THIN_CHECK_CMD" 0 3 1 || skip
aux have_tool_at_least "$LVM_TEST_THIN_DUMP_CMD" 0 3 1 || skip
aux have_tool_at_least "$LVM_TEST_THIN_REPAIR_CMD" 0 3 1 || skip

#
# Main
#

aux prepare_vg 4

lvcreate --type raid1 -L1 -n pool $vg
lvcreate --type raid1 -L2 -n meta $vg
# raid _tdata & _tmeta
lvconvert -y --thinpool $vg/pool --poolmetadata $vg/meta

lvcreate -V1G $vg/pool

# Pool has to be inactive (ATM) for repair
fail lvconvert -y --repair $vg/pool "$dev3"

lvchange -an $vg

check lv_field $vg/pool_tmeta lv_role "private,thin,pool,metadata"

lvconvert -y --repair $vg/pool "$dev3"

lvs -a -o+devices,seg_pe_ranges,role,layout $vg
check lv_field $vg/pool_meta0 lv_role "public"
check lv_field $vg/pool_meta0 lv_layout "raid,raid1"
check lv_field $vg/pool_tmeta lv_layout "linear"
check lv_on $vg pool_tmeta "$dev1"

# Hmm name is generated in order
SPARE=$(lvs --noheadings -a --select "name=~_pmspare" -o name $vg)
SPARE=${SPARE##*[}
SPARE=${SPARE%%]*}

check lv_on $vg $SPARE "$dev3"

lvchange -ay $vg

vgremove -ff $vg
