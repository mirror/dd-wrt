#!/usr/bin/env bash

# Copyright (C) 2014-2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test activation race for raid's --syncaction check


SKIP_WITH_LVMPOLLD=1


# Current support for syncaction in cluster is broken
# might get fixed one day though
# meanwhile skipped
SKIP_WITH_CLVMD=1

. lib/inittest

# Proper mismatch count 1.5.2+ upstream, 1.3.5 < x < 1.4.0 in RHEL6
aux have_raid 1 3 5 &&
  ! aux have_raid 1 4 0 ||
  aux have_raid 1 5 2 || skip
aux prepare_vg 3

lvcreate -n $lv1 $vg -l1 --type raid1

aux wait_for_sync $vg $lv1

START=$(get pv_field "$dev2" pe_start --units 1k)
METASIZE=$(get lv_field $vg/${lv1}_rmeta_1 size -a --units 1k)
SEEK=$((${START%\.00k} + ${METASIZE%\.00k}))
# Overwrite some portion of  _rimage_1

#aux delay_dev "$dev2" 10 10
dd if=/dev/urandom of="$dev2" bs=1K count=1 seek=$SEEK oflag=direct
# FIXME
# Some delay - there is currently race in upstream kernel
# test may occasionaly fail with:
# device-mapper: message ioctl on  failed: Device or resource busy
#
# Heinz's kernel seems to fix this particular issue but
# has some other problem for now
aux udev_wait

lvchange --syncaction check $vg/$lv1

# Wait till scrubbing is finished
aux wait_for_sync $vg $lv1

check lv_field $vg/$lv1 raid_mismatch_count "128"

# Let's deactivate
lvchange -an $vg/$lv1

lvchange -ay $vg/$lv1
# noone has it open and target is read & running
dmsetup info -c | grep $vg

#sleep 10 < "$DM_DEV_DIR/$vg/$lv1" &
# "check" should find discrepancies but not change them
# 'lvs' should show results

# FIXME
# this looks like some race with 'write' during activation
# and syncaction...
# For now it fails with:
# device-mapper: message ioctl on  failed: Device or resource busy
#
# As solution for now - user needs to run --synaction on synchronous raid array
#
aux wait_for_sync $vg $lv1

# Check raid array doesn't know about error yet
check lv_field $vg/$lv1 raid_mismatch_count "0"

# Start scrubbing
lvchange --syncaction check $vg/$lv1

# Wait till scrubbing is finished
aux wait_for_sync $vg $lv1

# Retest mistmatch exists
check lv_field $vg/$lv1 raid_mismatch_count "128"

vgremove -ff $vg
