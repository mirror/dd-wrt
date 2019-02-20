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

# 'Test pvchange option values'


SKIP_WITH_LVMPOLLD=1

. lib/inittest

check_changed_uuid_() {
	test "$1" != "$(get pv_field "$2" uuid)" || die "UUID has not changed!"
}

aux prepare_pvs 4

# check 'allocatable' pv attribute
pvcreate "$dev1"
check pv_field "$dev1" pv_attr ---
vgcreate $SHARED $vg1 "$dev1"
check pv_field "$dev1" pv_attr a--
pvchange --allocatable n "$dev1"
check pv_field "$dev1" pv_attr u--
vgremove -ff $vg1
not pvchange --allocatable y "$dev1"
pvremove -ff "$dev1"

for mda in 0 1 2
do
# "setup pv with metadatacopies = $mda"
	pvcreate --metadatacopies $mda "$dev1"
# cannot change allocatability for orphan PVs
	fail pvchange "$dev1" -x y
	fail pvchange "$dev1" -x n
	vgcreate $SHARED $vg1 "$dev4" "$dev1"

# "pvchange adds/dels tag to pvs with metadatacopies = $mda "
	pvchange "$dev1" --addtag test$mda
	check pv_field "$dev1" pv_tags test$mda
	pvchange "$dev1" --deltag test$mda
	check pv_field "$dev1" pv_tags ""

# "vgchange disable/enable allocation for pvs with metadatacopies = $mda (bz452982)"
	pvchange "$dev1" -x n
	pvchange "$dev1" -x n   # already disabled
	check pv_field "$dev1" pv_attr  u--
	pvchange "$dev1" -x y
	pvchange "$dev1" -x y   # already enabled
	check pv_field "$dev1" pv_attr  a--

# check we are able to change number of managed metadata areas
	if test $mda -gt 0 ; then
		pvchange --force --metadataignore y "$dev1"
	else
		# already ignored
		fail pvchange --metadataignore y "$dev1"
	fi
# 'remove pv'
	vgremove $vg1
	pvremove "$dev1"
done

# "pvchange uuid"
pvcreate --metadatacopies 0 "$dev1"
pvcreate --metadatacopies 2 "$dev2"
vgcreate $SHARED $vg1 "$dev1" "$dev2"

# Checking for different UUID after pvchange
UUID1=$(get pv_field "$dev1" uuid)
pvchange -u "$dev1"
check_changed_uuid_ "$UUID1" "$dev1"

UUID2=$(get pv_field "$dev2" uuid)
pvchange -u "$dev2"
check_changed_uuid_ "$UUID2" "$dev2"

UUID1=$(get pv_field "$dev1" uuid)
UUID2=$(get pv_field "$dev2" uuid)
pvchange -u --all
check_changed_uuid_ "$UUID1" "$dev1"
check_changed_uuid_ "$UUID2" "$dev2"
check pvlv_counts $vg1 2 0 0

# some args are needed
invalid pvchange
# some PV needed
invalid pvchange --addtag tag
invalid pvchange --deltag tag
# some --all & PV can go together
invalid pvchange -a "$dev1" --addtag tag
# '-a' needs more params
invalid pvchange -a
# '-a' is searching for devs, so specifying device is invalid
invalid pvchange -a "$dev1"
fail pvchange -u "$dev1-notfound"

# pvchange rejects uuid change under an active lv
lvcreate -l 16 -i 2 -n $lv --alloc anywhere $vg1
check pvlv_counts $vg1 2 1 0
not pvchange -u "$dev1"

vgremove -f $vg1

# cannot change PV tag to PV that is not in VG"
fail pvchange "$dev1" --addtag test
fail pvchange "$dev1" --deltag test

if test -n "$LVM_TEST_LVM1" ; then
# cannot add PV tag to lvm1 format
pvcreate -M1 "$dev1"
vgcreate $SHARED -M1 $vg1 "$dev1"
fail pvchange "$dev1" --addtag test
fi

