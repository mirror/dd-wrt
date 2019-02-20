#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Exercise some vgcreate diagnostics'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 3
pvcreate "$dev1" "$dev2"
pvcreate --metadatacopies 0 "$dev3"

vg=${PREFIX}vg

#COMM 'vgcreate $SHARED accepts 8.00m physicalextentsize for VG'
vgcreate $SHARED $vg --physicalextentsize 8.00m "$dev1" "$dev2"
check vg_field  $vg vg_extent_size 8.00m
vgremove $vg
# try vgck and to remove it again - should fail (but not segfault)
not vgremove $vg
not vgck $vg

#COMM 'vgcreate $SHARED accepts smaller (128) maxlogicalvolumes for VG'
vgcreate $SHARED $vg --maxlogicalvolumes 128 "$dev1" "$dev2"
check vg_field $vg max_lv 128 
vgremove $vg

#COMM 'vgcreate $SHARED accepts smaller (128) maxphysicalvolumes for VG'
vgcreate $SHARED $vg --maxphysicalvolumes 128 "$dev1" "$dev2"
check vg_field $vg max_pv 128
vgremove $vg

#COMM 'vgcreate $SHARED rejects a zero physical extent size'
not vgcreate $SHARED --physicalextentsize 0 $vg "$dev1" "$dev2" 2>err
grep "Physical extent size may not be zero" err

#COMM 'vgcreate $SHARED rejects "inherit" allocation policy'
not vgcreate $SHARED --alloc inherit $vg "$dev1" "$dev2" 2>err
grep "Volume Group allocation policy cannot inherit from anything" err

#COMM 'vgcreate $SHARED rejects vgname "."'
vginvalid=.; 
not vgcreate $SHARED $vginvalid "$dev1" "$dev2" 2>err
grep "New volume group name \"$vginvalid\" is invalid" err

#COMM 'vgcreate $SHARED rejects vgname greater than 128 characters'
vginvalid=thisnameisridiculouslylongtotestvalidationcodecheckingmaximumsizethisiswhathappenswhenprogrammersgetboredandorarenotcreativedonttrythisathome
not vgcreate $SHARED $vginvalid "$dev1" "$dev2" 2>err
grep "New volume group name \"$vginvalid\" is invalid" err

#COMM 'vgcreate $SHARED rejects already existing vgname "/tmp/$vg"'
#touch /tmp/$vg
#not vgcreate $SHARED $vg "$dev1" "$dev2" 2>err
#grep "New volume group name \"$vg\" is invalid\$" err

#COMM "vgcreate $SHARED rejects repeated invocation (run 2 times) (bz178216)"
vgcreate $SHARED $vg "$dev1" "$dev2"
not vgcreate $SHARED $vg "$dev1" "$dev2"
vgremove -ff $vg

#COMM "vgcreate $SHARED fails when the only pv has --metadatacopies 0"
not vgcreate $SHARED $vg "$dev3"

# Test default (4MB) vg_extent_size as well as limits of extent_size
not vgcreate $SHARED --physicalextentsize 0k $vg "$dev1" "$dev2"
vgcreate $SHARED --physicalextentsize 4k $vg "$dev1" "$dev2"
check vg_field $vg vg_extent_size 4.00k
vgremove -ff $vg
not vgcreate $SHARED --physicalextentsize 7K $vg "$dev1" "$dev2"
not vgcreate $SHARED --physicalextentsize 1024t $vg "$dev1" "$dev2"
#not vgcreate $SHARED --physicalextentsize 1T $vg "$dev1" "$dev2"
# FIXME: vgcreate $SHARED allows physicalextentsize larger than pv size!

# Test default max_lv, max_pv, extent_size, alloc_policy, clustered
vgcreate $SHARED $vg "$dev1" "$dev2"
check vg_field $vg vg_extent_size 4.00m
check vg_field $vg max_lv 0
check vg_field $vg max_pv 0
ATTRS="wz--n-"
test -e LOCAL_CLVMD && ATTRS="wz--nc"
if test -n "$LVM_TEST_LVMLOCKD"; then
ATTRS="wz--ns"
fi
check vg_field $vg vg_attr $ATTRS
vgremove -ff $vg

# Implicit pvcreate tests, test pvcreate options on vgcreate $SHARED
# --force, --yes, --metadata{size|copies|type}, --zero
# --dataalignment[offset]
pvremove "$dev1" "$dev2"
vgcreate $SHARED --force --yes --zero y $vg "$dev1" "$dev2"
vgremove -f $vg
pvremove -f "$dev1"

for i in 0 1 2 3
do
# vgcreate $SHARED (lvm2) succeeds writing LVM label at sector $i
    vgcreate $SHARED --labelsector $i $vg "$dev1"
    dd if="$dev1" bs=512 skip=$i count=1 2>/dev/null | strings | grep LABELONE >/dev/null
    vgremove -f $vg
    pvremove -f "$dev1"
done

# pvmetadatacopies
for i in 1 2
do
    vgcreate $SHARED --pvmetadatacopies $i $vg "$dev1"
    check pv_field "$dev1" pv_mda_count $i
    vgremove -f $vg
    pvremove -f "$dev1"
done
not vgcreate $SHARED --pvmetadatacopies 0 $vg "$dev1"
pvcreate --metadatacopies 1 "$dev2"
vgcreate $SHARED --pvmetadatacopies 0 $vg "$dev1" "$dev2"
check pv_field "$dev1" pv_mda_count 0
check pv_field "$dev2" pv_mda_count 1
vgremove -f $vg
pvremove -f "$dev1"

# metadatasize, dataalignment, dataalignmentoffset
#COMM 'pvcreate sets data offset next to mda area'
vgcreate $SHARED --metadatasize 100k --dataalignment 100k $vg "$dev1"
check pv_field "$dev1" pe_start 200.00k
vgremove -f $vg
pvremove -f "$dev1"

# data area is aligned to 1M by default,
# data area start is shifted by the specified alignment_offset
pv_align=1052160 # 1048576 + (7*512)
vgcreate $SHARED --metadatasize 128k --dataalignmentoffset 7s $vg "$dev1"
check pv_field "$dev1" pe_start ${pv_align}B --units b
vgremove -f $vg
pvremove -f "$dev1"

if test -n "$LVM_TEST_LVM1" ; then
mdatypes='1 2'
else
mdatypes='2'
fi

# metadatatype
for i in $mdatypes
do
    vgcreate $SHARED -M $i $vg "$dev1"
    check vg_field $vg vg_fmt lvm$i
    vgremove -f $vg
    pvremove -f "$dev1"
done

# vgcreate $SHARED fails if pv belongs to existing vg
vgcreate $SHARED $vg1 "$dev1" "$dev2"
not vgcreate $SHARED $vg2 "$dev2"
vgremove -f $vg1
pvremove -f "$dev1" "$dev2"

# all PVs exist in the VG after created
pvcreate "$dev1"
vgcreate $SHARED $vg1 "$dev1" "$dev2" "$dev3"
check pv_field "$dev1" vg_name $vg1
check pv_field "$dev2" vg_name $vg1
check pv_field "$dev3" vg_name $vg1
vgremove -f $vg1
