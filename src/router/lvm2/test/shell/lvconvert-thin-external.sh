#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test conversion to thin external origin

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

which mkfs.ext2 || skip
which fsck || skip

#
# Main
#
aux have_thin 1 5 0 || skip

aux prepare_pvs 2 64
get_devs

vgcreate "$vg" --metadatasize 128K -s 64K "${DEVICES[@]}"

if test 0 -eq 1 ; then
# FIXME: needs patch to allow inactive old-snap creation
lvcreate -l10 -T $vg/pool
lvcreate -an -pr --zero n -l10 --name $lv1 $vg
lvcreate -s $vg/$lv1 --name $lv2 --thinpool $vg/pool
vgchange -an $vg
# oldstyle read-only inactive snapshot
lvcreate -an -s $vg/$lv2 -l10 -p r --name $lv3

lvcreate -s $vg/$lv3 --name $lv4 --thinpool $vg/pool
lvremove -ff $vg/$lv3

lvremove -ff $vg
fi

#lvcreate -L20M --name orig $vg
#lvconvert -T  --thinpool $vg/pool $vg/orig
#lvcreate -s -aey -L10M $vg/orig
#lvremove -f $vg
#exit 0

lvcreate -l10 -T $vg/pool
# Can't convert pool to external origin
lvcreate -l10 -T $vg/pool1 -c 192k
not lvconvert -T --thinpool $vg/pool1 $vg/pool --originname origin
# Create pool1 chunk_size unaligned LV and check failing conversion
lvcreate -l2 -n $lv1 $vg
# Newer thin-pool target (>= 1.13) supports unaligned external origin
aux lvmconf 'global/thin_disabled_features = [ "external_origin_extend" ]'
not lvconvert -T --thinpool $vg/pool1 $vg/$lv1

lvremove -f $vg/pool1 $vg/$lv1

# create plain LV (will be used for external origin)
lvcreate -L8M -n $lv1 $vg

# Can't convert same LV to the thin pool and thin volume
not lvconvert --thinpool $vg/$lv1 -T $vg/$lv1
check lv_field $vg/$lv1 segtype linear

mkfs.ext2 "$DM_DEV_DIR/$vg/$lv1"
mkdir mnt
mount "$DM_DEV_DIR/$vg/$lv1" mnt

dd if=/dev/zero of=mnt/test1 bs=1M count=1

# convert plain LV into thin external snapshot volume
# during conversion dd above could be still flushed

lvconvert -T --originname extorg --thinpool $vg/pool $vg/$lv1

check active $vg $lv1
# FIXME  handling attr is ...
get lv_field $vg/extorg attr | grep "^ori"
check inactive $vg extorg

touch mnt/test
umount mnt

# check fs is without errors
fsck -n "$DM_DEV_DIR/$vg/$lv1"

lvchange -aey $vg/extorg
lvchange -an $vg/$lv1

check active $vg extorg
check inactive $vg $lv1

# fsck in read-only mode
fsck -n "$DM_DEV_DIR/$vg/extorg"

not lvresize -l+8 $vg/extorg
not lvresize -l-4 $vg/extorg
not lvchange -p rw $vg/extorg

#lvresize -L+8M $vg/$lv1
#lvresize -L-4M $vg/$lv1
#lvchange -p r $vg/$lv1
#lvchange -p rw $vg/$lv1

lvchange -aey $vg

lvs -a -o+origin_size,seg_size $vg

# Chain external origins
lvconvert --type thin --originname extorg1 --thinpool $vg/pool $vg/extorg
check inactive $vg extorg1

lvconvert --originname extorg2 --thinpool $vg/pool -T $vg/extorg1
check inactive $vg extorg1
check inactive $vg extorg2

lvchange -an $vg/extorg
lvchange -ay $vg/extorg1

lvcreate -l4 -s $vg/$lv1  -n $lv2
lvcreate -l8 -s $vg/extorg -n $lv3
lvcreate -l12 -s $vg/extorg1 -n $lv4
lvcreate -l16 -s $vg/extorg2 -n $lv5
#vgchange -aey $vg
#lvremove -f $vg/extorg2
#exit 0
# Converting old-snapshot into external origin is not supported
not lvconvert -T --thinpool $vg/pool --originname lv5origin $vg/$lv4

lvs -a -o +segtype $vg

check lv_field $vg/$lv1 segtype thin
check lv_field $vg/$lv2 segtype linear
check lv_field $vg/$lv3 segtype linear
check lv_field $vg/$lv4 segtype linear
check lv_field $vg/$lv5 segtype linear
check lv_field $vg/extorg segtype thin
check lv_field $vg/extorg1 segtype thin
check lv_field $vg/extorg2 segtype linear

vgchange -ay $vg

lvs -a -o+origin_size,seg_size $vg

lvchange -an $vg/extorg2
check inactive $vg extorg2

# Remove all volumes dependent on external origin
lvs -a -o+origin_size,seg_size,segtype $vg
lvremove -f $vg/extorg2
# Only pool is left
check vg_field $vg lv_count 1
lvremove -ff $vg

# Test conversion to the pool and thin external at the same time (rhbz #1003461)
lvcreate -l50 -n pool $vg
lvcreate -l100 -n thin $vg
lvconvert --yes --thin --thinpool $vg/pool $vg/thin --originname thin-origin
check lv_field $vg/thin segtype thin
check lv_field $vg/thin-origin segtype linear
lvremove -ff $vg

# Test conversion with non-zeroing thin-pool, should not WARN about zeroing
lvcreate -l50 -n pool $vg
lvcreate -l100 -n thin $vg
lvconvert --yes --thin --thinpool $vg/pool $vg/thin --zero n --originname thin-origin 2>&1 | tee out
not grep "not zeroed" out
check lv_field $vg/pool zero ""

vgremove -ff $vg
