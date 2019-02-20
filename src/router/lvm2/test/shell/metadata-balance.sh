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

aux prepare_devs 6

echo Make sure we can ignore / un-ignore mdas on a per-PV basis
for pv_in_vg in 1 0; do
for mdacp in 1 2; do
	pvcreate --metadatacopies $mdacp "$dev1" "$dev2"
        pvcreate --metadatacopies 0 "$dev3"
	if [ $pv_in_vg = 1 ]; then
		vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3"
	fi
	pvchange --metadataignore y "$dev1"
	check pv_field "$dev1" pv_mda_count $mdacp
	check pv_field "$dev1" pv_mda_used_count 0
	check pv_field "$dev2" pv_mda_count $mdacp
	check pv_field "$dev2" pv_mda_used_count $mdacp
	if [ $pv_in_vg = 1 ]; then
		check vg_field $vg vg_mda_count $(( mdacp * 2 ))
		check vg_field $vg vg_mda_used_count $mdacp
		check vg_field $vg vg_mda_copies unmanaged
	fi
	pvchange --metadataignore n "$dev1"
	check pv_field "$dev1" pv_mda_count $mdacp
	check pv_field "$dev1" pv_mda_used_count $mdacp
	if [ $pv_in_vg = 1 ]; then
		check vg_field $vg vg_mda_count $(( mdacp * 2 ))
		check vg_field $vg vg_mda_used_count $(( mdacp * 2 ))
		check vg_field $vg vg_mda_copies unmanaged
		vgremove -f $vg
	fi
done
done

# Check if a PV has unignored (used) mdas, and if so, ignore
pvignore_ () {
	pv_mda_used_count=$(get pv_field "$1" pv_mda_used_count)
	if [ $pv_mda_used_count -ne 0 ]; then
	    pvchange --metadataignore y $1
	fi
}

# Check if a PV has ignored mdas, and if so, unignore (make used)
pvunignore_ () {
	pv_mda_count=$(get pv_field "$1" pv_mda_count)
	pv_mda_used_count=$(get pv_field "$1" pv_mda_used_count)
	if [ $pv_mda_count -gt $pv_mda_used_count ]; then
	    pvchange --metadataignore n $1
	fi
}

echo Test of vgmetadatacopies with vgcreate $SHARED and vgchange
for mdacp in 1 2; do
	pvcreate --metadatacopies $mdacp "$dev1" "$dev2" "$dev4" "$dev5"
	check pv_field "$dev1" pv_mda_used_count $mdacp
	check pv_field "$dev2" pv_mda_used_count $mdacp
	check pv_field "$dev4" pv_mda_used_count $mdacp
	check pv_field "$dev5" pv_mda_used_count $mdacp
	pvcreate --metadatacopies 0 "$dev3"
	vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3"
	check vg_field $vg vg_mda_copies unmanaged
	echo ensure both --vgmetadatacopies and --metadatacopies accepted
	vgchange --metadatacopies $(( mdacp * 1 )) $vg
	echo --vgmetadatacopies is persistent on disk
	echo --vgmetadatacopies affects underlying pv mda ignore
	check vg_field $vg vg_mda_copies $(( mdacp * 1 ))
	check vg_field $vg vg_mda_used_count $(( mdacp * 1 ))
	vgchange --vgmetadatacopies $(( mdacp * 2 )) $vg
	check vg_field $vg vg_mda_copies $(( mdacp * 2 ))
	check vg_field $vg vg_mda_used_count $(( mdacp * 2 ))
	echo allow setting metadatacopies larger than number of PVs
	vgchange --vgmetadatacopies $(( mdacp * 5 )) $vg
	check vg_field $vg vg_mda_copies $(( mdacp * 5 ))
	check vg_field $vg vg_mda_used_count $(( mdacp * 2 ))
	echo setting to 0 disables automatic balancing
	vgchange --vgmetadatacopies unmanaged $vg
	check vg_field $vg vg_mda_copies unmanaged
	vgremove -f $vg
	echo vgcreate $SHARED succeeds even when creating a VG w/all ignored mdas
	pvchange --metadataignore y "$dev1" "$dev2"
	check pv_field "$dev1" pv_mda_count $mdacp
	check pv_field "$dev2" pv_mda_used_count 0
	vgcreate $SHARED $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies unmanaged
	vgremove -f $vg
	echo vgcreate $SHARED succeeds with a specific number of metadata copies
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 2 )) $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies $(( mdacp * 2 ))
	vgremove -f $vg
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 1 )) $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies $(( mdacp * 1 ))
	vgremove -f $vg
	echo vgcreate $SHARED succeeds with a larger value than total metadatacopies
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 5 )) $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies $(( mdacp * 5 ))
	vgremove -f $vg
	echo vgcreate $SHARED succeeds with --vgmetadatacopies unmanaged
	vgcreate $SHARED --vgmetadatacopies unmanaged $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies unmanaged
	vgremove -f $vg
	pvunignore_ "$dev1"
	pvunignore_ "$dev2"
	pvunignore_ "$dev4"
	pvunignore_ "$dev5"
	echo vgcreate $SHARED succeds with small value of --metadatacopies, ignores mdas
	vgcreate $SHARED --vgmetadatacopies 1 $vg "$dev1" "$dev2" "$dev4" "$dev5"
	check vg_field $vg vg_mda_copies 1
	check vg_field $vg vg_mda_count $(( mdacp * 4 ))
	check vg_field $vg vg_mda_used_count 1
	echo Setting a larger value should trigger non-ignore of mdas
	vgchange --metadatacopies 3 $vg
	check vg_field $vg vg_mda_copies 3
	check vg_field $vg vg_mda_used_count 3
	echo Setting all should trigger unignore of all mdas
	vgchange --vgmetadatacopies all $vg
	check vg_field $vg vg_mda_count $(( mdacp * 4 ))
	check vg_field $vg vg_mda_copies unmanaged
	check vg_field $vg vg_mda_used_count $(( mdacp * 4 ))
	echo --vgmetadatacopies 0 should be unmanaged for vgchange and vgcreate $SHARED
	vgchange --vgmetadatacopies 0 $vg
	check vg_field $vg vg_mda_copies unmanaged
	vgremove -f $vg
	vgcreate $SHARED --vgmetadatacopies 0 $vg "$dev1" "$dev2" "$dev4" "$dev5"
	check vg_field $vg vg_mda_copies unmanaged
	vgremove -f $vg
done

echo Test vgextend / vgreduce with vgmetadatacopies
for mdacp in 1 2; do
	pvcreate --metadatacopies $mdacp "$dev1" "$dev2" "$dev4" "$dev5"
	pvcreate --metadatacopies 0 "$dev3"
	echo Set a large value of vgmetadatacopies
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 5 )) $vg "$dev1" "$dev2" "$dev3"
	check vg_field $vg vg_mda_copies $(( mdacp * 5 ))
	echo Ignore mdas on devices to be used for vgextend
	echo Large value of vgetadatacopies should automatically un-ignore mdas
	pvchange --metadataignore y "$dev4" "$dev5"
	check pv_field "$dev4" pv_mda_used_count 0
	vgextend $vg "$dev4" "$dev5"
	check pv_field "$dev4" pv_mda_used_count $mdacp
	check pv_field "$dev5" pv_mda_used_count $mdacp
	vgremove -f $vg
	echo Set a small value of vgmetadatacopies
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 1 )) $vg "$dev1" "$dev2" "$dev3"
	check vg_field $vg vg_mda_copies $(( mdacp * 1 ))
	echo Ignore mdas on devices to be used for vgextend
	echo Small value of vgetadatacopies should leave mdas as ignored
	pvchange --metadataignore y "$dev4" "$dev5"
	check pv_field "$dev4" pv_mda_used_count 0
	vgextend $vg "$dev4" "$dev5"
	check pv_field "$dev4" pv_mda_used_count 0
	check pv_field "$dev5" pv_mda_used_count 0
	echo vgreduce of ignored pv w/mda should not trigger any change to ignore bits
	vgreduce $vg "$dev4"
	check pv_field "$dev4" pv_mda_used_count 0
	check pv_field "$dev5" pv_mda_used_count 0
	echo vgreduce of un-ignored pv w/mda should trigger un-ignore on an mda
	vgreduce $vg "$dev1" "$dev2" "$dev3"
	check pv_field "$dev5" pv_mda_used_count $mdacp
	check vg_field $vg vg_mda_copies $(( mdacp * 1 ))
	pvunignore_ "$dev1"
	pvunignore_ "$dev2"
	echo setting vgmetadatacopies to unmanaged should allow vgextend to add w/out balancing
	vgchange --vgmetadatacopies unmanaged $vg
	vgextend $vg "$dev1" "$dev2"
	check vg_field $vg vg_mda_copies unmanaged
	check vg_field $vg vg_mda_count $(( mdacp * 3 ))
	check vg_field $vg vg_mda_used_count $(( mdacp * 3 ))
	check pv_field "$dev1" pv_mda_used_count $mdacp
	check pv_field "$dev2" pv_mda_used_count $mdacp
	vgremove -f $vg
done

if test -n "$LVM_TEST_LVMLOCKD"; then
echo skip vgsplit and vgmerge with lvmlockd
else

echo Test special situations, vgsplit, vgmerge, etc
for mdacp in 1 2; do
	pvcreate --metadatacopies $mdacp "$dev1" "$dev2" "$dev3" "$dev4" "$dev5"
	vgcreate $SHARED --vgmetadatacopies 2 $vg1 "$dev1" "$dev2" "$dev3"
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 1 )) $vg2 "$dev4" "$dev5"
	echo vgsplit/vgmerge preserves value of metadata copies
	check vg_field $vg1 vg_mda_copies 2
	check vg_field $vg2 vg_mda_copies $(( mdacp * 1 ))
	vgsplit $vg1 $vg2 "$dev1"
	check vg_field $vg2 vg_mda_copies $(( mdacp * 1 ))
	vgmerge $vg1 $vg2
	check vg_field $vg1 vg_mda_copies 2
	check vg_field $vg1 vg_mda_count $(( mdacp * 5 ))
	echo vgsplit into new vg sets proper value of vgmetadatacopies
	vgsplit --vgmetadatacopies $(( mdacp * 2 )) $vg1 $vg2 "$dev1" "$dev2"
	check vg_field $vg2 vg_mda_copies $(( mdacp * 2 ))
	echo vgchange fails if given both vgmetadatacopies and metadatacopies
	not vgchange --vgmetadatacopies 5 --metadatacopies 7 $vg2
	vgremove -f $vg1 $vg2
done

fi

echo Test combination of --vgmetadatacopies and pvchange --metadataignore
for mdacp in 1 2; do
	pvcreate --metadatacopies $mdacp "$dev1" "$dev2" "$dev3" "$dev4" "$dev5"
	vgcreate $SHARED --vgmetadatacopies $(( mdacp * 1 )) $vg1 "$dev1" "$dev2"
	check vg_field $vg1 vg_mda_copies $(( mdacp * 1 ))
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 1 ))
	pvignore_ "$dev3"
	echo Ensure vgextend of PVs with ignored MDAs does not add to vg_mda_used_count
	vgextend $vg1 "$dev3"
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 1 ))
	echo Using pvchange to unignore should update vg_mda_used_count
	pvchange -f --metadataignore n "$dev3"
	check pv_field "$dev3" pv_mda_used_count $mdacp
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 2 ))
	echo Set unmanaged on the vg should keep ignore bits the same during vgextend
	vgchange --vgmetadatacopies unmanaged $vg1
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 2 ))
	pvunignore_ "$dev4"
	vgextend $vg1 "$dev4"
	check pv_field "$dev4" pv_mda_used_count $mdacp
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 3 ))
	echo Using pvchange to ignore should update vg_mda_used_count
	pvchange -f --metadataignore y "$dev4"
	check pv_field "$dev4" pv_mda_used_count 0
	check vg_field $vg1 vg_mda_used_count $(( mdacp * 2 ))
	vgremove -f $vg1
done
