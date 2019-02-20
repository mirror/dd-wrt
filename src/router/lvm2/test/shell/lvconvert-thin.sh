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

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

prepare_lvs() {
	lvremove -f $vg
	lvcreate -L10M -n $lv1 $vg
	lvcreate -L8M -n $lv2 $vg
}

#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_pvs 4 64
get_devs

# build one large PV
vgcreate $vg1 "$dev1" "$dev2" "$dev3"

# 32bit linux kernels are fragille with device size >= 16T
# maybe  uname -m    [ x86_64 | i686 ]
TSIZE=64T
aux can_use_16T || TSIZE=15T
lvcreate --type snapshot -l 100%FREE -n $lv $vg1 --virtualsize $TSIZE
aux extend_filter_LVMTEST

pvcreate "$DM_DEV_DIR/$vg1/$lv"
vgcreate $vg -s 64K "$dev4" "$DM_DEV_DIR/$vg1/$lv"

lvcreate -L1T -n $lv1 $vg
invalid lvconvert --yes -c 8M --type thin --poolmetadatasize 1G $vg/$lv1

# needs some --cachepool or --thinpool
invalid lvconvert --yes --poolmetadatasize 1G $vg/$lv1
lvremove -f $vg

# create mirrored LVs for data and metadata volumes
lvcreate -aey -L10M --type mirror -m1 --mirrorlog core -n $lv1 $vg
lvcreate -aey -L10M -n $lv2 $vg
lvchange -an $vg/$lv1

# conversion fails for mirror segment type
fail lvconvert --thinpool $vg/$lv1

# FIXME: temporarily we return error code 5
INVALID=not
# cannot use same LV
$INVALID lvconvert --yes --thinpool $vg/$lv2 --poolmetadata $vg/$lv2

prepare_lvs

# conversion fails for internal volumes
# can't use --readahead with --poolmetadata
invalid lvconvert --thinpool $vg/$lv1 --poolmetadata $vg/$lv2 --readahead 512
lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $vg/$lv2

prepare_lvs
lvconvert --yes -c 64 --stripes 2 --thinpool $vg/$lv1 --readahead 48
lvremove -f $vg


# Swaping of metadata volume
lvcreate -L1T -n $lv1 $vg
lvcreate -L32 -n $lv2 $vg
lvconvert --yes -c 8M --type thin-pool $vg/$lv1 2>&1 | tee err
# Check there is a warning for large chunk size and zeroing enabled
grep "WARNING: Pool zeroing and" err
UUID=$(get lv_field $vg/$lv2 uuid)
# Fail is pool is active
# TODO  maybe detect inactive pool and deactivate
fail lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $lv2
lvchange -an $vg
lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $lv2
check lv_field $vg/${lv1}_tmeta uuid "$UUID"

# and swap again with new command --swapmetadata
lvconvert --yes --swapmetadata $vg/$lv1 --poolmetadata $lv2
check lv_field $vg/$lv2 uuid "$UUID"
lvremove -f $vg


# test with bigger sizes
lvcreate -L1T -n $lv1 $vg
lvcreate -L8M -n $lv2 $vg
lvcreate -L1M -n $lv3 $vg

# chunk size is bigger then size of thin pool data
fail lvconvert --yes -c 1G --thinpool $vg/$lv3
# stripes can't be used with poolmetadata
invalid lvconvert --stripes 2 --thinpool $vg/$lv1 --poolmetadata $vg/$lv2
# too small metadata (<2M)
fail lvconvert --yes -c 64 --thinpool $vg/$lv1 --poolmetadata $vg/$lv3
# too small chunk size fails
$INVALID lvconvert -c 4 --thinpool $vg/$lv1 --poolmetadata $vg/$lv2
# too big chunk size fails
$INVALID lvconvert -c 2G --thinpool $vg/$lv1 --poolmetadata $vg/$lv2
# negative chunk size fails
$INVALID lvconvert -c -256 --thinpool $vg/$lv1 --poolmetadata $vg/$lv2
# non multiple of 64KiB fails
$INVALID lvconvert -c 88 --thinpool $vg/$lv1 --poolmetadata $vg/$lv2

# cannot use same LV for pool and convertion
$INVALID lvconvert --yes --thinpool $vg/$lv3 -T $vg/$lv3

# Warning about smaller then suggested
lvconvert --yes -c 256 --thinpool $vg/$lv1 --poolmetadata $vg/$lv2 2>&1 | tee err
grep "WARNING: Chunk size is smaller" err
lvremove -f $vg


lvcreate -L1T -n $lv1 $vg
lvcreate -L32G -n $lv2 $vg
# Warning about bigger then needed
lvconvert --yes --thinpool $vg/$lv1 --poolmetadata $vg/$lv2 2>&1 | tee err
grep "WARNING: Maximum" err
lvremove -f $vg


if test "$TSIZE" = 64T; then
lvcreate -L24T -n $lv1 $vg
# Warning about bigger then needed (24T data and 16G -> 128K chunk)
fail lvconvert --yes -c 64 --thinpool $vg/$lv1 2>&1 | tee err
grep "WARNING: Chunk size is too small" err
lvremove -f $vg
fi

#lvs -a -o+chunk_size,stripe_size,seg_pe_ranges

####################################
# Prohibites thin pool conversions #
####################################
lvcreate -L32 -n $lv1 $vg
lvcreate -L16 -n $lv2 $vg
lvconvert --yes --thinpool $vg/$lv1

not aux have_cache 1 3 0 || fail lvconvert --yes --type cache-pool $vg/$lv1
fail lvconvert --yes --type mirror -m1 $vg/$lv1
not aux have_raid 1 0 0 || fail lvconvert --yes --type raid1 -m1 $vg/$lv1
fail lvconvert --yes --type snapshot $vg/$lv1 $vg/$lv2
fail lvconvert --yes --type snapshot $vg/$lv2 $vg/$lv1
fail lvconvert --yes --type thin-pool $vg/$lv1

vgremove -ff $vg
vgremove -ff $vg1
