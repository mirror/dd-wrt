#!/usr/bin/env bash

# Copyright (C) 2008-2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#
# tests functionality of lvs, pvs, vgs, *display tools
#

SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 5
get_devs

# Check there is no PV
pvscan | tee out
grep "No matching" out

pvcreate --uuid BADBEE-BAAD-BAAD-BAAD-BAAD-BAAD-BADBEE --norestorefile "$dev1"
pvcreate --metadatacopies 0 "$dev2"
pvcreate --metadatacopies 0 "$dev3"
pvcreate "$dev4"
pvcreate --metadatacopies 0 "$dev5"

#COMM bz195276 -- pvs doesn't show PVs until a VG is created
pvs --noheadings "${DEVICES[@]}"
test "$(pvs --noheadings "${DEVICES[@]}" | wc -l)" -eq 5
pvdisplay

#COMM pvs with segment attributes works even for orphans
test "$(pvs --noheadings -o seg_all,pv_all,lv_all,vg_all "${DEVICES[@]}" | wc -l)" -eq 5

vgcreate $SHARED $vg "${DEVICES[@]}"

check pv_field "$dev1" pv_uuid BADBEE-BAAD-BAAD-BAAD-BAAD-BAAD-BADBEE

#COMM pvs and vgs report mda_count, mda_free (bz202886, bz247444)
pvs -o +pv_mda_count,pv_mda_free "${DEVICES[@]}"
for I in "$dev2" "$dev3" "$dev5"; do
	check pv_field "$I" pv_mda_count 0
	check pv_field "$I" pv_mda_free 0
done
vgs -o +vg_mda_count,vg_mda_free $vg
check vg_field $vg vg_mda_count 2

#COMM pvs doesn't display --metadatacopies 0 PVs as orphans (bz409061)
pvdisplay "$dev2"|grep "VG Name.*$vg"
check pv_field "$dev2" vg_name $vg

#COMM lvs displays snapshots (bz171215)
lvcreate -aey -l4 -n $lv1 $vg
lvcreate -l4 -s -n $lv2 $vg/$lv1
test "$(lvs --noheadings $vg | wc -l)" -eq 2
# should lvs -a display cow && real devices? (it doesn't)
test "$(lvs -a --noheadings $vg | wc -l)"  -eq 2
dmsetup ls | grep "$PREFIX" | grep -v "LVMTEST.*pv."
lvremove -f $vg/$lv2

#COMM lvs -a displays mirror legs and log
lvcreate -aey -l2 --type mirror -m2 -n $lv3 $vg
test "$(lvs --noheadings $vg | wc -l)" -eq 2
test "$(lvs -a --noheadings $vg | wc -l)" -eq 6
dmsetup ls | grep "$PREFIX" | grep -v "LVMTEST.*pv."

# Check we parse /dev/mapper/vg-lv
lvdisplay "$DM_DEV_DIR/mapper/$vg-$lv3"
# Check we parse /dev/vg/lv
lvdisplay "$DM_DEV_DIR/$vg/$lv3"

lvcreate -l2 -s $vg/$lv3
lvcreate -l1 -s -n inval $vg/$lv3
lvcreate -l4 -I4 -i2 -n stripe $vg
# Invalidate snapshot
not dd if=/dev/zero of="$DM_DEV_DIR/$vg/inval" bs=4K
invalid lvscan "$dev1"
lvdisplay --maps
lvscan --all

#COMM vgs with options from pvs still treats arguments as VGs (bz193543)
vgs -o pv_name,vg_name $vg
# would complain if not
vgs -o all $vg

#COMM pvdisplay --maps feature (bz149814)
pvdisplay "${DEVICES[@]}" >out
pvdisplay --maps "${DEVICES[@]}" >out2
not diff out out2

aux disable_dev "$dev1"
pvs -o +pv_uuid | grep BADBEE-BAAD-BAAD-BAAD-BAAD-BAAD-BADBEE
aux enable_dev "$dev1"

pvscan --uuid
pvscan -e
pvscan -s
pvscan --novolumegroup
vgscan --mknodes
vgmknodes --refresh
lvscan
lvmdiskscan

invalid pvscan "$dev1"
invalid pvscan -aay
invalid pvscan --major 254
invalid pvscan --minor 0
invalid pvscan --novolumegroup -e
invalid vgscan $vg
invalid lvscan $vg

if aux have_readline; then
cat <<EOF | lvm
vgdisplay --units k $vg
vgdisplay -c $vg
vgdisplay -C $vg
vgdisplay -s $vg
vgdisplay -v $vg
lvdisplay -c $vg
lvdisplay -C $vg
lvdisplay -m $vg
lvdisplay --units g $vg
EOF
else
pvdisplay -c "$dev1"
pvdisplay -s "$dev1"
vgdisplay --units k $vg
vgdisplay -c $vg
vgdisplay -C $vg
vgdisplay -s $vg
vgdisplay -v $vg
lvdisplay -c $vg
lvdisplay -C $vg
lvdisplay -m $vg
lvdisplay --units g $vg
fi

pvdisplay -c "$dev1"
pvdisplay -s "$dev1"

for i in h b s k m g t p e H B S K M G T P E; do
	pvdisplay --units $i "$dev1"
done

invalid lvdisplay -C -m $vg
invalid lvdisplay -c -m $vg
invalid lvdisplay --aligned $vg
invalid lvdisplay --noheadings $vg
invalid lvdisplay --options lv_name $vg
invalid lvdisplay --separator : $vg
invalid lvdisplay --sort size $vg
invalid lvdisplay --unbuffered $vg

invalid vgdisplay -C -A
invalid vgdisplay -C -c
invalid vgdisplay -C -s
invalid vgdisplay -c -s
invalid vgdisplay --aligned
invalid vgdisplay --noheadings
invalid vgdisplay --options
invalid vgdisplay --separator :
invalid vgdisplay --sort size
invalid vgdisplay --unbuffered
invalid vgdisplay -A $vg1

invalid pvdisplay -C -A
invalid pvdisplay -C -c
invalid pvdisplay -C -m
invalid pvdisplay -C -s
invalid pvdisplay -c -m
invalid pvdisplay -c -s
invalid pvdisplay --aligned
invalid pvdisplay --all
invalid pvdisplay --noheadings
invalid pvdisplay --options
invalid pvdisplay --separator :
invalid pvdisplay --sort size
invalid pvdisplay --unbuffered
invalid pvdisplay -A $vg1

# Check exported VG listing
vgchange -an $vg
vgexport -a
pvscan
pvdisplay --noheadings -C -o attr,name | tee out
not grep -v "ax-" out
vgimport -a
pvdisplay --noheadings -C -o attr,name | tee out
grep -v "ax-" out

vgremove -ff $vg

#test vgdisplay -A to select only active VGs
# all LVs active - VG considered active
pvcreate "$dev1" "$dev2" "$dev3"

vgcreate $SHARED $vg1 "$dev1"
lvcreate -l1 $vg1
lvcreate -l1 $vg1

# at least one LV active - VG considered active
vgcreate $SHARED $vg2 "$dev2"
lvcreate -l1 $vg2
lvcreate -l1 -an -Zn $vg2

# no LVs active - VG considered inactive
vgcreate $SHARED $vg3 "$dev3"
lvcreate -l1 -an -Zn $vg3
lvcreate -l1 -an -Zn $vg3

vgdisplay -s -A | grep $vg1
vgdisplay -s -A | grep $vg2
vgdisplay -s -A | not grep $vg3

vgremove -f $vg1 $vg2 $vg3
