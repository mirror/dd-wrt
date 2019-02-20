#!/usr/bin/env bash

# Copyright (C) 2008 Red Hat, Inc. All rights reserved.
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

aux mirror_recovery_works || skip
aux prepare_vg 5

# ordinary mirrors

lvcreate -aey --type mirror -m 3 --ignoremonitoring -L 1 -n 4way $vg
aux wait_for_sync $vg 4way
aux disable_dev --error --silent "$dev2" "$dev4"
mkfs.ext3 "$DM_DEV_DIR/$vg/4way" &
sleep 1
dmsetup status
echo n | lvconvert --repair $vg/4way 2>&1 | tee 4way.out
aux enable_dev --silent "$dev2" "$dev4"

lvs -a -o +devices $vg | tee out
not grep unknown out
vgreduce --removemissing $vg
check mirror $vg 4way
lvchange -a n $vg/4way
wait

vgremove -f $vg
