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

SKIP_WITH_LVMLOCKD=1

. lib/inittest

# test if snapshot-merge target is available
aux target_at_least dm-snapshot-merge 1 0 0 || skip

which mkfs.ext3 || skip

lvdev_() {
    echo "$DM_DEV_DIR/$1/$2"
}

snap_lv_name_() {
    echo ${1}_snap
}

setup_merge_() {
    local VG_NAME=$1
    local LV_NAME=$2
    local NUM_EXTRA_SNAPS=${3:-0}
    local BASE_SNAP_LV_NAME

    BASE_SNAP_LV_NAME=$(snap_lv_name_ $LV_NAME)

    lvcreate -aey -n $LV_NAME -l 50%FREE $VG_NAME
    lvs
    lvcreate -s -n $BASE_SNAP_LV_NAME -l 20%FREE ${VG_NAME}/${LV_NAME}
    mkfs.ext3 "$(lvdev_ $VG_NAME $LV_NAME)"

    if [ $NUM_EXTRA_SNAPS -gt 0 ]; then
	for i in $(seq 1 $NUM_EXTRA_SNAPS); do
	    lvcreate -s -n ${BASE_SNAP_LV_NAME}_${i} -l 20%ORIGIN ${VG_NAME}/${LV_NAME}
	done
    fi
}

aux prepare_pvs 1 50

vgcreate $vg1 "$dev1"
mkdir test_mnt

setup_merge_ $vg1 $lv1
mount "$(lvdev_ $vg1 $lv1)" test_mnt
lvconvert --merge "$vg1/$(snap_lv_name_ "$lv1")"
umount test_mnt
vgchange -an $vg1

# check snapshot get removed on autoactivation
pvscan --cache -aay "$dev1"

check active $vg1 $lv1
i=100
while ! check lv_not_exists "$vg1/$(snap_lv_name_ "$lv1")"; do
	test $i -lt 0 && fail "Background polling failed to remove merged snapshot LV"
	sleep .1
	i=$((i-1))
done

# TODO: add similar simple tests for other interrupted/unfinished polling operation

vgremove -ff $vg1
