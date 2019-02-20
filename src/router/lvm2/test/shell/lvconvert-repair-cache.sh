#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test repairing of broken cached LV

SKIP_WITH_LVMLOCKD=1

. lib/inittest

MKFS=mkfs.ext4
FSCK=fsck

which "$MKFS" || skip
which "$FSCK" || skip

#
# Main
#
# older versions of cache target reported unreliably write failures
aux have_cache 1 7 0 || skip

aux prepare_vg 4

#if [ 1 -eq 0 ] ; then
#############################
###### WRITETHROUGH #########
#############################

# Create cached LV
lvcreate --type cache-pool -L10 $vg/cpool "$dev1"
lvcreate -H -L20 --cachemode writethrough -n $lv1 $vg/cpool "$dev2"

"$MKFS" "$DM_DEV_DIR/$vg/$lv1"
sync

aux disable_dev "$dev1"

#lvchange -an $vg

# Check it is prompting for confirmation
not lvconvert --uncache $vg/$lv1
# --yes to drop when Check its prompting
lvconvert --yes --uncache $vg/$lv1
should not dmsetup remove ${vg}-cpool_cmeta-missing_0_0
should not dmsetup remove ${vg}-cpool_cdata-missing_0_0

"$FSCK" -n "$DM_DEV_DIR/$vg/$lv1"

aux enable_dev "$dev1"

##################

lvcreate --type cache-pool -L10 $vg/cpool "$dev1"
lvconvert -H --cachemode writethrough --cachepool $vg/cpool -Zy $lv1

# Check basic --repair of cachepool metadata
lvchange -an $vg/$lv1
lvconvert --repair $vg/$lv1

lvs -a $vg
check lv_exists $vg ${lv1}_meta0

eval $(lvs -S 'name=~_pmspare' -a --config 'report/mark_hidden_devices=0' -o name --noheading --nameprefixes $vg)
lvremove -f --yes "$vg/$LVM2_LV_NAME"

# check --repair without creation of _pmspare device
lvconvert --repair --poolmetadataspare n $vg/$lv1
check lv_exists $vg ${lv1}_meta1

# check no _pmspare has been created in previous --repair
test "0" = $(lvs -S 'name=~_pmspare' -a -o name --noheading --nameprefixes $vg | wc -l)


aux disable_dev "$dev2"

# Deactivate before remove
# FIXME: handle this while LV is alive
lvchange -an $vg/$lv1

# Check it is prompting for confirmation
not lvconvert --uncache $vg/$lv1
# --yes to drop when Check its prompting
lvconvert --yes --uncache $vg/$lv1

aux enable_dev "$dev2"

# FIXME: temporary workaround
lvcreate -L1 -n $lv5 $vg
lvremove -ff $vg

##########################
###### WRITEBACK #########
##########################
#fi

# Create cached LV  so metadata is on dev1 and data on dev2
lvcreate -L5 -n meta $vg "$dev1"
lvcreate -L10 -n cpool $vg "$dev2"
lvconvert --yes --poolmetadata $vg/meta  --cachepool $vg/cpool

lvcreate -H -L20 --cachemode writeback -n $lv1 $vg/cpool "$dev3"

lvs -a -o+seg_pe_ranges,cachemode $vg

"$MKFS" "$DM_DEV_DIR/$vg/$lv1"
sync

# Seriously damage cache metadata
aux error_dev "$dev1" 2054:2

# flushing status
dmsetup status $vg-$lv1

# On fixed kernel we get instant Fail here
get lv_field  $vg/$lv1 lv_attr | tee out
grep "Cwi-a-C-F-" out || {
	# while on older unfixed we just notice needs_check
	grep "Cwi-c-C---" out
	sleep .1
	# And now cache is finaly Failed
	check lv_attr_bit health $vg/$lv1 "F"
}
check lv_field $vg/$lv1 lv_health_status "failed"

aux disable_dev "$dev1"

# Check it is prompting for confirmation
not lvconvert --uncache $vg/$lv1
# Check --yes is not enought to drop writethrough caching
not lvconvert --yes --uncache $vg/$lv1
# --force needs --yes to drop when Check its prompting
not lvconvert --force --uncache $vg/$lv1

lvconvert --force --yes --uncache $vg/$lv1

not "$FSCK" -n "$DM_DEV_DIR/$vg/$lv1"

aux enable_dev "$dev1"

vgremove -ff $vg

# FIXME - device should not be here
should not dmsetup remove ${vg}-cpool_cmeta-missing_0_0
should not dmsetup remove ${vg}-cpool_cdata-missing_0_0
