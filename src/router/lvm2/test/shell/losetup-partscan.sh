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

# Check how lvm2 handles partitions over losetup -P devices


SKIP_WITH_LVMPOLLD=1

. lib/inittest

which sfdisk || skip

aux prepare_loop 1000 -P || skip

test -f LOOP
LOOP=$(< LOOP)

echo "1 2" | sfdisk "$LOOP"

# wait for links
aux udev_wait

# losetup -P should provide partition
ls -la "${LOOP}"*
test -e "${LOOP}p1"

aux extend_filter "a|$LOOP|"

# creation should fail for 'partitioned' loop device
not pvcreate -y "$LOOP"
not vgcreate $SHARED vg "$LOOP"

aux teardown_devs

aux prepare_loop 1000 || skip

test -f LOOP
LOOP=$(< LOOP)


echo "1 2" | sfdisk "$LOOP"

# wait for links
aux udev_wait

# no partitione should be actually there
ls -la "${LOOP}"*
test ! -e "${LOOP}p1"

aux extend_filter "a|$LOOP|"

# creation should pass for 'non-partitioned' loop device
pvcreate -y "$LOOP"

vgcreate $SHARED vg "$LOOP"
