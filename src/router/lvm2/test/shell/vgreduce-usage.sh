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

aux prepare_devs 4
get_devs

if test -n "$LVM_TEST_LVM1" ; then
mdatypes='1 2'
else
mdatypes='2'
fi

for mdatype in $mdatypes
do
    # setup PVs
    pvcreate -M$mdatype "$dev1" "$dev2"

    # (lvm$mdatype) vgreduce removes only the specified pv from vg (bz427382)" '
    vgcreate $SHARED -M$mdatype $vg1 "$dev1" "$dev2"
    vgreduce $vg1 "$dev1"
    check pv_field "$dev2" vg_name $vg1
    vgremove -f $vg1

    # (lvm$mdatype) vgreduce rejects removing the last pv (--all)
    vgcreate $SHARED -M$mdatype $vg1 "$dev1" "$dev2"
    not vgreduce --all $vg1
    vgremove -f $vg1

    # (lvm$mdatype) vgreduce rejects removing the last pv
    vgcreate $SHARED -M$mdatype $vg1 "$dev1" "$dev2"
    not vgreduce $vg1 "$dev1" "$dev2"
    vgremove -f $vg1

    pvremove -ff "$dev1" "$dev2"
done

mdatype=2 # we only expect the following to work for lvm2 metadata

# (lvm$mdatype) setup PVs (--metadatacopies 0)
pvcreate -M$mdatype "$dev1" "$dev2"
pvcreate --metadatacopies 0 -M$mdatype "$dev3" "$dev4"

# (lvm$mdatype) vgreduce rejects removing pv with the last mda copy (bz247448)
vgcreate $SHARED -M$mdatype $vg1 "$dev1" "$dev3"
not vgreduce $vg1 "$dev1"
vgremove -f $vg1

#COMM "(lvm$mdatype) vgreduce --removemissing --force repares to linear (bz221921)"
# (lvm$mdatype) setup: create mirror & damage one pv
vgcreate $SHARED -M$mdatype $vg1 "$dev1" "$dev2" "$dev3"
lvcreate -aey -n $lv1 --type mirror -m1 -l 4 $vg1
lvcreate -n $lv2  -l 4 $vg1 "$dev2"
lvcreate -n $lv3 -l 4 $vg1 "$dev3"
vgchange -an $vg1
aux disable_dev "$dev1"
# (lvm$mdatype) vgreduce --removemissing --force repares to linear
vgreduce --removemissing --force $vg1
check lv_field $vg1/$lv1 segtype linear
check pvlv_counts $vg1 2 3 0
# cleanup
aux enable_dev "$dev1"
pvscan
vgremove -f $vg1
not vgs $vg1 # just double-check it's really gone

#COMM "vgreduce rejects --removemissing --mirrorsonly --force when nonmirror lv lost too"
# (lvm$mdatype) setup: create mirror + linear lvs
vgcreate $SHARED -M$mdatype "$vg1" "${DEVICES[@]}"
lvcreate -n $lv2 -l 4 $vg1
lvcreate -aey --type mirror -m1 -n $lv1 -l 4 $vg1 "$dev1" "$dev2" "$dev3"
lvcreate -n $lv3 -l 4 $vg1 "$dev3"
pvs --segments -o +lv_name "${DEVICES[@]}" # for record only
# (lvm$mdatype) setup: damage one pv
vgchange -an $vg1
aux disable_dev "$dev1"
#pvcreate -ff -y "$dev1"
# vgreduce rejects --removemissing --mirrorsonly --force when nonmirror lv lost too
#not vgreduce -c n --removemissing --mirrorsonly --force $vg1
# CHECKME - command above was rejected becuase of '-c n'
vgreduce --removemissing --mirrorsonly --force $vg1

aux enable_dev "$dev1"

pvs -P "${DEVICES[@]}" # for record
lvs -P $vg1           # for record
vgs -P $vg1           # for record

vgremove -ff $vg1
