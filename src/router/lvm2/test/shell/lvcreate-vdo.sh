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

#
# Main
#
aux have_vdo 6 2 0 || skip
which mkfs.ext4 || skip

aux prepare_pvs 2 6400
get_devs

#aux lvmconf 'allocation/vdo_use_read_cache = 1' 'allocation/vdo_read_cache_size_mb = 64'

#aux lvmconf 'allocation/vdo_use_compression = 0' 'allocation/vdo_use_deduplication = 0'

#aux lvmconf 'allocation/vdo_hash_zone_threads = 0' \
#	'allocation/vdo_logical_threads = 0' \
#	'allocation/vdo_physical_threads = 0' \
#	'allocation/vdo_cpu_threads = 1'

aux lvmconf 'allocation/vdo_slab_size_mb = 128'


vgcreate $SHARED -s 64K "$vg" "${DEVICES[@]}"

# Create VDO device  (vdo-pool is ATM internal volume type)
lvcreate --type vdo -L4G -n $lv1 $vg/$lv2
check lv_field $vg/$lv1 size "1.24g"
check lv_field $vg/${lv2} size "4.00g"
check lv_field $vg/${lv2}_vdata size "4.00g"
lvremove -ff $vg


lvcreate --vdo -L4G -V8G -n $lv1 $vg/$lv2
check lv_field $vg/$lv1 size "8.00g"
check lv_field $vg/${lv2} size "4.00g"
check lv_field $vg/${lv2}_vdata size "4.00g"
lvs -a $vg

dmsetup table | grep $vg
dmsetup status | grep $vg

# Resize not yet supported
not lvresize -y $vg/$lv1
not lvresize -y $vg/${lv2}
not lvresize -y $vg/${lv2}_vdata

# Discard is very slow with VDO ATM so try to avoid it
#time blkdiscard "$DM_DEV_DIR/$vg/$lv1"
time mkfs.ext4 -E nodiscard "$DM_DEV_DIR/$vg/$lv1"
#time mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
fsck -n "$DM_DEV_DIR/$vg/$lv1"

# vpool itself is NOT usable filesystem
not fsck -n "$DM_DEV_DIR/mapper/$vg-${lv2}"
# not usable even when there is no linear mapping on top of it
dmsetup remove ${vg}-$lv1
not fsck -n "$DM_DEV_DIR/mapper/$vg-${lv2}"

lvremove -ff $vg


lvcreate --type vdo -L10G -V1T -n $lv1 $vg
lvs -a $vg
lvremove -ff $vg

vgremove -ff $vg
