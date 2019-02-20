#!/usr/bin/env bash

# Copyright (C) 2013-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test repairing of broken thin pool metadata

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

which mkfs.ext2 || skip

#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_vg 4

# Create LV
# TODO: investigate problem with --zero n and my repairable damage trick
#lvcreate -T -L20 -V10 -n $lv1 $vg/pool --discards ignore --zero n --chunksize 128 "$dev1" "$dev2"
lvcreate -T -L20 -V10 -n $lv1 $vg/pool --chunksize 128 --discards ignore "$dev1" "$dev2"
lvcreate -T -V10 -n $lv2 $vg/pool

mkfs.ext2 "$DM_DEV_DIR/$vg/$lv1"
mkfs.ext2 "$DM_DEV_DIR/$vg/$lv2"

lvcreate -L20 -n repair $vg
lvcreate -L2 -n fixed $vg

lvs -a -o+seg_pe_ranges $vg
#aux error_dev "$dev2" 2050:1

lvchange -an $vg/$lv2 $vg/$lv1 $vg/pool $vg/repair

# Manual repair steps:
# Test swapping - swap out thin-pool's metadata with our repair volume
lvconvert -y -f --poolmetadata $vg/repair --thinpool $vg/pool

lvchange -ay $vg/repair

#
# To continue this test - we need real tools available
# When they are not present mark test as skipped, but still
# let proceed initial part which should work even without tools
#
aux have_tool_at_least "$LVM_TEST_THIN_CHECK_CMD" 0 3 1 || skip
aux have_tool_at_least "$LVM_TEST_THIN_DUMP_CMD" 0 3 1 || skip
aux have_tool_at_least "$LVM_TEST_THIN_REPAIR_CMD" 0 3 1 || skip

# Make some 'repairable' damage??
dd if=/dev/zero of="$DM_DEV_DIR/$vg/repair" bs=1 seek=40960 count=1

not "$LVM_TEST_THIN_CHECK_CMD" "$DM_DEV_DIR/$vg/repair"

not "$LVM_TEST_THIN_DUMP_CMD" "$DM_DEV_DIR/$vg/repair" | tee dump

"$LVM_TEST_THIN_REPAIR_CMD" -i "$DM_DEV_DIR/$vg/repair" -o "$DM_DEV_DIR/$vg/fixed"

"$LVM_TEST_THIN_DUMP_CMD" --repair "$DM_DEV_DIR/$vg/repair" | tee repaired_xml

"$LVM_TEST_THIN_CHECK_CMD" "$DM_DEV_DIR/$vg/fixed"

lvchange -an $vg

# Swap repaired metadata back
lvconvert -y -f --poolmetadata $vg/fixed --thinpool $vg/pool

# Check pool still preserves its original settings
check lv_field $vg/pool chunksize "128.00k"
check lv_field $vg/pool discards "ignore"
check lv_field $vg/pool zero "zero"

# Activate pool - this should now work
vgchange -ay $vg

vgchange -an $vg

# Put back 'broken' metadata
lvconvert -y -f --poolmetadata $vg/repair --thinpool $vg/pool

# Check --repair usage
lvconvert -v --repair $vg/pool

# Check repaired pool could be activated
lvchange -ay $vg/pool

vgchange -an $vg

# Restore damaged metadata
lvconvert -y -f --poolmetadata $vg/pool_meta0 --thinpool $vg/pool

# Check lvremove -ff works even with damaged pool
lvremove -ff $vg
