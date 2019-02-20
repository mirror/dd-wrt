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

# Excersize resize of filesystem when size of LV already matches
# https://bugzilla.redhat.com/1354396


SKIP_WITH_LVMPOLLD=1

. lib/inittest

FSCK=${FSCK-fsck}
MKFS=${MKFS-mkfs.ext3}
RESIZEFS=${RESIZEFS-resize2fs}
LVM_BINARY=$(which lvm)
export LVM_BINARY

which $FSCK || skip
which $MKFS || skip
which $RESIZEFS || skip

aux prepare_vg 2 20

lvcreate -l100%FREE -n $lv1 $vg

lvdev="$DM_DEV_DIR/$vg/$lv1"

lvs -a $vg

"$MKFS" "$lvdev"

# this should resolve to resize to same actual size
lvreduce -r -f -l-100%FREE $vg/$lv1
"$FSCK" -n "$lvdev"

# size should remain the same
lvextend -r -f -l+100%FREE $vg/$lv1
"$FSCK" -n "$lvdev"

#lvchange -an $vg/$lv1
lvresize -r -f -l+100%FREE $vg/$lv1
"$FSCK" -n "$lvdev"

# Check there is really file system resize happening
# even when LV itself has still the same size
"$RESIZEFS" -f "$lvdev" 20000
"$FSCK" -n "$lvdev" | tee out
grep "20000 blocks" out

SIZE=$(get lv_field $vg/$lv1 size)
lvresize -r -f -l-100%FREE $vg/$lv1
test "$SIZE" = "$(get lv_field $vg/$lv1 size)"

"$FSCK" -n "$lvdev" | tee out
grep -v "20000 blocks" out


# Also check it fails when the user 'resize' volume without
# resizing fs and then retries with '-r'.
lvreduce -f -l50%VG $vg/$lv1
fail lvresize -r -f -l50%VG $vg/$lv1

lvremove -ff $vg
