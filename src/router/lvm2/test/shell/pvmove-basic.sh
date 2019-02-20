#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
# Copyright (C) 2007 NEC Corporation
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description="ensure that pvmove works with basic options"

SKIP_WITH_LVMLOCKD=1

. lib/inittest

which md5sum || skip

# ---------------------------------------------------------------------
# Utilities

create_vg_() {
	vgcreate -s 128k "$vg" "${DEVICES[@]}"
}

# ---------------------------------------------------------------------
# Common environment setup/cleanup for each sub testcases
prepare_lvs_() {
	lvcreate -aey -l2 -n $lv1 $vg "$dev1"
	check lv_on $vg $lv1 "$dev1"
	lvcreate -aey -l9 -i3 -n $lv2 $vg "$dev2" "$dev3" "$dev4"
	check lv_on $vg $lv2 "$dev2" "$dev3" "$dev4"
	lvextend -l+2 $vg/$lv1 "$dev2"
	check lv_on $vg $lv1 "$dev1" "$dev2"
	lvextend -l+2 $vg/$lv1 "$dev3"
	lvextend -l+2 $vg/$lv1 "$dev1"
	check lv_on $vg $lv1 "$dev1" "$dev2" "$dev3"
	lvcreate -aey -l1 -n $lv3 $vg "$dev2"
	check lv_on $vg $lv3 "$dev2"
	aux mkdev_md5sum $vg $lv1
	aux mkdev_md5sum $vg $lv2
	aux mkdev_md5sum $vg $lv3
	get lv_devices "$vg/$lv1" > "${lv1}_devs"
	get lv_devices "$vg/$lv2" > "${lv2}_devs"
	get lv_devices "$vg/$lv3" > "${lv3}_devs"
	lvs -a -o name,size,seg_pe_ranges $vg
	vgcfgbackup -f bak-$$ $vg
}

# Restore metadata content, since data are pvmove-ed
# original content should be preserved
restore_lvs_() {
	vgcfgrestore -f bak-$$ $vg
	vgchange -aey $vg
}

lvs_not_changed_() {
	for i in "${@}"; do
		get lv_devices "$vg/$i" | tee out
		diff "${i}_devs" out || \
			(cat "${i}_devs"; die "Devices for LV $vg/$i differs!")
	done
}

check_and_cleanup_lvs_() {
	check dev_md5sum $vg $lv1
	check dev_md5sum $vg $lv2
	check dev_md5sum $vg $lv3
	get lv_field $vg name -a >out
	not grep "^\[pvmove" out
	vgchange -an $vg
	lvremove -ff $vg
	(dm_table | not grep $vg) || \
	      die "ERROR: lvremove did leave some mappings in DM behind!"
}

# ---------------------------------------------------------------------
# Initialize PVs and VGs

aux prepare_pvs 5 5
get_devs

create_vg_

for mode in "--atomic" ""
do

#COMM "check environment setup/cleanup"
prepare_lvs_
check_and_cleanup_lvs_

# ---------------------------------------------------------------------
# pvmove tests

# ---
# filter by LV

#COMM "only specified LV is moved: from pv2 to pv5 only for lv1"
restore_lvs_
pvmove $mode -i1 -n $vg/$lv1 "$dev2" "$dev5"
check lv_on $vg $lv1 "$dev1" "$dev5" "$dev3"
lvs_not_changed_ $lv2 $lv3
check_and_cleanup_lvs_

# ---
# segments in a LV

#COMM "the 1st seg of 3-segs LV is moved: from pv1 of lv1 to pv4"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv1 "$dev1" "$dev4"
check lv_on $vg $lv1 "$dev4" "$dev2" "$dev3"
lvs_not_changed_ $lv2 $lv3
check_and_cleanup_lvs_

#COMM "the 2nd seg of 3-segs LV is moved: from pv2 of lv1 to pv4"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv1 "$dev2" "$dev4"
check lv_on $vg $lv1 "$dev1" "$dev4" "$dev3"
lvs_not_changed_ $lv2 $lv3
check_and_cleanup_lvs_

#COMM "the 3rd seg of 3-segs LV is moved: from pv3 of lv1 to pv4"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv1 "$dev3" "$dev4"
check lv_on $vg $lv1 "$dev1" "$dev2" "$dev4"
lvs_not_changed_ $lv2 $lv3
check_and_cleanup_lvs_

# ---
# multiple LVs matching

#COMM "1 out of 3 LVs is moved: from pv4 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev4" "$dev5"
check lv_on $vg $lv2 "$dev2" "$dev3" "$dev5"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "2 out of 3 LVs are moved: from pv3 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev3" "$dev5"
check lv_on $vg $lv1 "$dev1" "$dev2" "$dev5"
check lv_on $vg $lv2 "$dev2" "$dev5" "$dev4"
lvs_not_changed_ $lv3
check_and_cleanup_lvs_

#COMM "3 out of 3 LVs are moved: from pv2 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev2" "$dev5"
check lv_on $vg $lv1 "$dev1" "$dev5" "$dev3"
check lv_on $vg $lv2 "$dev5" "$dev3" "$dev4"
check lv_on $vg $lv3 "$dev5"
check_and_cleanup_lvs_

# ---
# areas of striping

#COMM "move the 1st stripe: from pv2 of lv2 to pv1"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv2 "$dev2" "$dev1"
check lv_on $vg $lv2 "$dev1" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "move the 2nd stripe: from pv3 of lv2 to pv1"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv2 "$dev3" "$dev1"
check lv_on $vg $lv2 "$dev2" "$dev1" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "move the 3rd stripe: from pv4 of lv2 to pv1"
restore_lvs_
pvmove $mode -i0 -n $vg/$lv2 "$dev4" "$dev1"
check lv_on $vg $lv2 "$dev2" "$dev3" "$dev1"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

# ---
# partial segment match (source segment splitted)

#COMM "match to the start of segment:from pv2:0-0 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev2":0-0 "$dev5"
check lv_on $vg $lv2 "$dev5" "$dev2" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_
#exit 0
#COMM "match to the middle of segment: from pv2:1-1 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev2":1-1 "$dev5"
check lv_on $vg $lv2 "$dev2" "$dev3" "$dev4" "$dev5"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "match to the end of segment: from pv2:2-2 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev2":2-2 "$dev5"
check lv_on $vg $lv2 "$dev2" "$dev5" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

# ---
# destination segment splitted

#COMM "no destination split: from pv2:0-2 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev2":0-2 "$dev5"
check lv_on $vg $lv2 "$dev5" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "destination split into 2: from pv2:0-2 to pv5:5-5 and pv4:5-6"
restore_lvs_
pvmove $mode -i0 --alloc anywhere "$dev2":0-2 "$dev5":5-5 "$dev4":5-6
check lv_on $vg $lv2 "$dev5" "$dev4" "$dev3"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "destination split into 3: from pv2:0-2 to {pv3,4,5}:5-5"
restore_lvs_
pvmove $mode -i0 --alloc anywhere "$dev2":0-2 "$dev3":5-5 "$dev4":5-5 "$dev5":5-5
check lv_on $vg $lv2 "$dev3" "$dev4" "$dev5"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

# ---
# alloc policy (anywhere, contiguous) with both success and failure cases

#COMM "alloc normal on same PV for source and destination: from pv3:0-2 to pv3:5-7"
restore_lvs_
not pvmove $mode -i0 "$dev3":0-2 "$dev3":5-7
# "(cleanup previous test)"
lvs_not_changed_ $lv1 $lv2 $lv3
check_and_cleanup_lvs_

#COMM "alloc anywhere on same PV for source and destination: from pv3:0-2 to pv3:5-7"
restore_lvs_
pvmove $mode -i0 --alloc anywhere "$dev3":0-2 "$dev3":5-7
check lv_on $vg $lv2 "$dev2" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "alloc anywhere but better area available: from pv3:0-2 to pv3:5-7 or pv5:5-6,pv4:5-5"
restore_lvs_
#lvs -a -o name,size,seg_pe_ranges $vg
#LV2    1.12m @TESTDIR@/dev/mapper/@PREFIX@pv2:0-2 @TESTDIR@/dev/mapper/@PREFIX@pv3:0-2 @TESTDIR@/dev/mapper/@PREFIX@pv4:0-2

pvmove $mode -i0 --alloc anywhere "$dev3":0-2 "$dev3":5-7 "$dev5":5-6 "$dev4":5-5

#lvs -a -o name,size,seg_pe_ranges $vg
# Hmm is this correct ? - why pv2 is split
#LV2    1.12m @TESTDIR@/dev/mapper/@PREFIX@pv2:0-1 @TESTDIR@/dev/mapper/@PREFIX@pv5:5-6 @TESTDIR@/dev/mapper/@PREFIX@pv4:0-1
#LV2    1.12m @TESTDIR@/dev/mapper/@PREFIX@pv2:2-2 @TESTDIR@/dev/mapper/@PREFIX@pv3:5-5 @TESTDIR@/dev/mapper/@PREFIX@pv4:2-2
check lv_on $vg $lv2 "$dev2" "$dev3" "$dev4" "$dev5"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

#COMM "alloc contiguous but area not available: from pv2:0-2 to pv5:5-5 and pv4:5-6"
restore_lvs_
not pvmove $mode -i0 --alloc contiguous "$dev2":0-2 "$dev5":5-5 "$dev4":5-6
# "(cleanup previous test)"
lvs_not_changed_ $lv1 $lv2 $lv3
check_and_cleanup_lvs_

#COMM "alloc contiguous and contiguous area available: from pv2:0-2 to pv5:0-0,pv5:3-5 and pv4:5-6"
restore_lvs_
pvmove $mode -i0 --alloc contiguous "$dev2":0-2 "$dev5":0-0 "$dev5":3-5 "$dev4":5-6
check lv_on $vg $lv2 "$dev5" "$dev3" "$dev4"
lvs_not_changed_ $lv1 $lv3
check_and_cleanup_lvs_

# ---
# multiple segments in a LV

#COMM "multiple source LVs: from pv3 to pv5"
restore_lvs_
pvmove $mode -i0 "$dev3" "$dev5"
check lv_on $vg $lv1 "$dev1" "$dev2" "$dev5"
check lv_on $vg $lv2 "$dev2" "$dev5" "$dev4"
lvs_not_changed_ $lv3
check_and_cleanup_lvs_

# ---
# move inactive LV

#COMM "move inactive LV: from pv2 to pv5"
restore_lvs_
lvchange -an $vg/$lv1
lvchange -an $vg/$lv3
pvmove $mode -i0 "$dev2" "$dev5"
check lv_on $vg $lv1 "$dev1" "$dev5" "$dev3"
check lv_on $vg $lv2 "$dev5" "$dev3" "$dev4"
check lv_on $vg $lv3 "$dev5"
check_and_cleanup_lvs_

# ---
# other failure cases

#COMM "no PEs to move: from pv3 to pv1"
restore_lvs_
pvmove $mode -i0 "$dev3" "$dev1"
not pvmove $mode -i0 "$dev3" "$dev1"
# "(cleanup previous test)"
check lv_on $vg $lv1 "$dev1" "$dev2" "$dev1"
check lv_on $vg $lv2 "$dev2" "$dev1" "$dev4"
lvs_not_changed_ $lv3
check_and_cleanup_lvs_

#COMM "no space available: from pv2:0-0 to pv1:0-0"
restore_lvs_
not pvmove $mode -i0 "$dev2":0-0 "$dev1":0-0
# "(cleanup previous test)"
lvs_not_changed_ $lv1 $lv2 $lv3
check_and_cleanup_lvs_

#COMM 'same source and destination: from pv1 to pv1'
restore_lvs_
not pvmove $mode -i0 "$dev1" "$dev1"
#"(cleanup previous test)"
lvs_not_changed_ $lv1 $lv2 $lv3
check_and_cleanup_lvs_

#COMM "sum of specified destination PEs is large enough, but it includes source PEs and the free PEs are not enough"
restore_lvs_
not pvmove $mode --alloc anywhere "$dev1":0-2 "$dev1":0-2 "$dev5":0-0 2> err
#"(cleanup previous test)"
grep "Insufficient free space" err
lvs_not_changed_ $lv1 $lv2 $lv3
check_and_cleanup_lvs_

# ---------------------------------------------------------------------

#COMM "pvmove abort"
restore_lvs_
LVM_TEST_TAG="kill_me_$PREFIX" pvmove $mode -i100 -b "$dev1" "$dev3"
pvmove --abort
check_and_cleanup_lvs_

#COMM "pvmove out of --metadatacopies 0 PV (bz252150)"
vgremove -ff $vg
pvcreate "${DEVICES[@]}"
pvcreate --metadatacopies 0 "$dev1" "$dev2"
create_vg_
lvcreate -aey -l4 -n $lv1 $vg "$dev1"
pvmove $mode "$dev1"

#COMM "pvmove fails activating mirror, properly restores state before pvmove"
dmsetup create $vg-pvmove0 --notable
not pvmove $mode -i 1 "$dev2"
dmsetup info --noheadings -c -o suspended $vg-$lv1
test "$(dmsetup info --noheadings -c -o suspended "$vg-$lv1")" = "Active"
if dmsetup info $vg-pvmove0_mimage_0 > /dev/null; then
        dmsetup remove $vg-pvmove0 $vg-pvmove0_mimage_0 $vg-pvmove0_mimage_1
else
        dmsetup remove $vg-pvmove0
fi

lvremove -ff $vg
done
