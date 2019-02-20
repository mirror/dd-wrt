#!/usr/bin/env bash

# Copyright (C) 2011-2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# disable lvmetad logging as it bogs down test systems

SKIP_WITH_LVMPOLLD=1

. lib/inittest

get_image_pvs() {
	local d
	local images=()

	images=( $(dmsetup ls | grep "${1}-${2}_.image_.*" | cut -f1 | sed -e s:-:/:) )
	lvs --noheadings -a -o devices "${images[@]}" | sed s/\(.\)//
}

########################################################
# MAIN
########################################################
aux have_raid 1 3 0 || skip

aux prepare_pvs 9
get_devs

# vgcreate -s 256k "$vg" "${DEVICES[@]}"
vgcreate $SHARED -s 2m "$vg" "${DEVICES[@]}"

###########################################
# RAID1 convert tests
###########################################
for under_snap in false true; do
for i in 1 2 3; do
	for j in 1 2 3; do
		if [ $i -eq 1 ]; then
			from="linear"
		else
			from="$i-way"
		fi
		if [ $j -eq 1 ]; then
			to="linear"
		else
			to="$j-way"
		fi

		echo -n "Converting from $from to $to"
		if $under_snap; then
			echo -n " (while under a snapshot)"
		fi
		echo

		if [ $i -eq 1 ]; then
			# Shouldn't be able to create with just 1 image
			not lvcreate --type raid1 -m 0 -l 2 -n $lv1 $vg

			lvcreate -aey -l 2 -n $lv1 $vg
		else
			lvcreate --type raid1 -m $(( i - 1 )) -l 2 -n $lv1 $vg
			aux wait_for_sync $vg $lv1
		fi

		if $under_snap; then
			lvcreate -aey -s $vg/$lv1 -n snap -l 2
		fi

		mirrors=$((j - 1))
		if [ $i -eq 1 ]
		then
			[ $mirrors -eq 0 ] && lvconvert -y -m $mirrors $vg/$lv1
		else
			if [ $mirrors -eq 0 ]
			then
				not lvconvert -m $mirrors $vg/$lv1
				lvconvert -y -m $mirrors $vg/$lv1
			else
				lvconvert -y -m $mirrors $vg/$lv1
			fi
		fi

		# FIXME: ensure no residual devices

		if [ $j -eq 1 ]; then
			check linear $vg $lv1
		fi
		lvremove -ff $vg
	done
done
done

##############################################
# RAID1 - shouldn't be able to add image
#         if created '--nosync', but should
#         be able to after 'lvchange --resync'
##############################################
lvcreate --type raid1 -m 1 -l 2 -n $lv1 $vg --nosync
not lvconvert -m +1 $vg/$lv1
lvchange --resync -y $vg/$lv1
aux wait_for_sync $vg $lv1
lvconvert -y -m +1 $vg/$lv1
lvremove -ff $vg

# 3-way to 2-way convert while specifying devices
lvcreate --type raid1 -m 2 -l 2 -n $lv1 $vg "$dev1" "$dev2" "$dev3"
aux wait_for_sync $vg $lv1
lvconvert -y -m 1 $vg/$lv1 "$dev2"
lvremove -ff $vg

#
# FIXME: Add tests that specify particular devices to be removed
#

###########################################
# RAID1 split tests
###########################################
# 3-way to 2-way/linear
lvcreate --type raid1 -m 2 -l 2 -n $lv1 $vg
aux wait_for_sync $vg $lv1
lvconvert --splitmirrors 1 -n $lv2 $vg/$lv1
check lv_exists $vg $lv1
check linear $vg $lv2
check active $vg $lv2
# FIXME: ensure no residual devices
lvremove -ff $vg

# 2-way to linear/linear
lvcreate --type raid1 -m 1 -l 2 -n $lv1 $vg
aux wait_for_sync $vg $lv1
not lvconvert --splitmirrors 1 -n $lv2 $vg/$lv1
lvconvert --yes --splitmirrors 1 -n $lv2 $vg/$lv1
check linear $vg $lv1
check linear $vg $lv2
check active $vg $lv2
# FIXME: ensure no residual devices
lvremove -ff $vg

# 4-way
lvcreate --type raid1 -m 4 -l 2 -n $lv1 $vg
aux wait_for_sync $vg $lv1
lvconvert --splitmirrors 1 --name $lv2 $vg/$lv1 "$dev2"
lvremove -ff $vg

###########################################
# RAID1 split + trackchanges / merge with content check
###########################################
# 3-way to 2-way/linear
lvcreate --type raid1 -m 2 -l 1 -n $lv1 $vg
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv1"
aux wait_for_sync $vg $lv1
fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv1"
lvconvert --splitmirrors 1 --trackchanges $vg/$lv1
check lv_exists $vg $lv1
check linear $vg ${lv1}_rimage_2
fsck.ext4 -fn "$DM_DEV_DIR/mapper/$vg-${lv1}_rimage_2"
dd of="$DM_DEV_DIR/$vg/$lv1" if=/dev/zero bs=512 oflag=direct count="$(blockdev --getsz "$DM_DEV_DIR/$vg/$lv1")"
not fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv1"
fsck.ext4 -fn "$DM_DEV_DIR/mapper/$vg-${lv1}_rimage_2"
# FIXME: needed on tiny loop but not on real block backend ?
lvchange --refresh $vg/$lv1
lvconvert --merge $vg/${lv1}_rimage_2
aux wait_for_sync $vg $lv1
lvconvert --splitmirrors 1 --trackchanges $vg/$lv1
not fsck.ext4 -fn "$DM_DEV_DIR/mapper/$vg-${lv1}_rimage_2"
# FIXME: ensure no residual devices
lvremove -ff $vg

# Check split track changes gets rejected w/o -y on 2-legged raid1
lvcreate --type raid1 -m 1 -l 1 -n $lv1 $vg
mkfs.ext4 "$DM_DEV_DIR/$vg/$lv1"
fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv1"
aux wait_for_sync $vg $lv1
fsck.ext4 -fn "$DM_DEV_DIR/$vg/$lv1"
not lvconvert --splitmirrors 1 --trackchanges $vg/$lv1
lvconvert --yes --splitmirrors 1 --trackchanges $vg/$lv1
# FIXME: ensure no residual devices
lvremove -ff $vg

###########################################
# Linear to RAID1 conversion ("raid1" default segtype)
###########################################
lvcreate -aey -l 2 -n $lv1 $vg
lvconvert -y -m 1 $vg/$lv1 \
	--config 'global { mirror_segtype_default = "raid1" }'
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*r'
lvremove -ff $vg

###########################################
# Linear to RAID1 conversion (override "mirror" default segtype)
###########################################
lvcreate -aey -l 2 -n $lv1 $vg
lvconvert --yes --type raid1 -m 1 $vg/$lv1 \
	--config 'global { mirror_segtype_default = "mirror" }'
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*r'
lvremove -ff $vg

###########################################
# Must not be able to convert non-EX LVs in a cluster
###########################################
if [ -e LOCAL_CLVMD ]; then
	lvcreate -l 2 -n $lv1 $vg
	not lvconvert -y --type raid1 -m 1 $vg/$lv1 \
		--config 'global { mirror_segtype_default = "mirror" }'
	lvremove -ff $vg
fi

###########################################
# Mirror to RAID1 conversion
###########################################
for i in 1 2 3 ; do
	lvcreate -aey --type mirror -m $i -l 2 -n $lv1 $vg
	aux wait_for_sync $vg $lv1
	lvconvert -y --type raid1 $vg/$lv1
	lvremove -ff $vg
done

###########################################
# Upconverted RAID1 should not allow loss of primary
#  - don't allow removal of primary while syncing
#  - DO allow removal of secondaries while syncing
###########################################
aux delay_dev "$dev2" 0 100
lvcreate -aey -l 2 -n $lv1 $vg "$dev1"
lvconvert -y -m 1 $vg/$lv1 \
	--config 'global { mirror_segtype_default = "raid1" }' "$dev2"
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*r'
not lvconvert --yes -m 0 $vg/$lv1 "$dev1"
lvconvert --yes -m 0 $vg/$lv1 "$dev2"
aux enable_dev "$dev2"
lvremove -ff $vg

###########################################
# lvcreated RAID1 should allow all down-conversion
#  - DO allow removal of primary while syncing
#  - DO allow removal of secondaries while syncing
###########################################
aux delay_dev "$dev2" 0 100
lvcreate --type raid1 -m 2 -aey -l 2 -n $lv1 $vg "$dev1" "$dev2" "$dev3"
case "$(uname -r)" in
4.8.14*)
echo "Skippen test that kills this kernel"
;;
*)
lvconvert --yes -m 1 $vg/$lv1 "$dev3"
lvconvert --yes -m 0 $vg/$lv1 "$dev1"
aux enable_dev "$dev2"
;;
esac
lvremove -ff $vg

###########################################
# Converting from 2-way RAID1 to 3-way
#  - DO allow removal of one of primary sources
#  - Do not allow removal of all primary sources
###########################################
lvcreate --type raid1 -m 1 -aey -l 2 -n $lv1 $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv1
aux delay_dev "$dev3" 0 100
lvconvert --yes -m +1 $vg/$lv1 "$dev3"
# should allow 1st primary to be removed
lvconvert --yes -m -1 $vg/$lv1 "$dev1"
# should NOT allow last primary to be removed
not lvconvert --yes -m -1 $vg/$lv1 "$dev2"
# should allow non-primary to be removed
lvconvert --yes -m 0 $vg/$lv1 "$dev3"
aux enable_dev "$dev3"
lvremove -ff $vg

###########################################
# Converting from 2-way RAID1 to 3-way
#  - Should allow removal of two devices,
#    as long as they aren't both primary
###########################################
lvcreate --type raid1 -m 1 -aey -l 2 -n $lv1 $vg "$dev1" "$dev2"
aux wait_for_sync $vg $lv1
aux delay_dev "$dev3" 0 100
lvconvert --yes -m +1 $vg/$lv1 "$dev3"
# should NOT allow both primaries to be removed
not lvconvert -m 0 $vg/$lv1 "$dev1" "$dev2"
# should allow primary + non-primary
lvconvert --yes -m 0 $vg/$lv1 "$dev1" "$dev3"
aux enable_dev "$dev3"
lvremove -ff $vg

###########################################
# Device Replacement Testing
###########################################
# RAID1: Replace up to n-1 devices - trying different combinations
# Test for 2-way to 4-way RAID1 LVs
for i in {1..3}; do
	lvcreate --type raid1 -m "$i" -l 2 -n $lv1 $vg

	for j in $(seq $(( i + 1 ))); do # The number of devs to replace at once
	for o in $(seq 0 $i); do        # The offset into the device list
		replace=()

		devices=( $(get_image_pvs $vg $lv1) )

		for k in $(seq "$j"); do
			index=$(( ( k +  o ) % ( i + 1 ) ))
			replace+=( "--replace" )
			replace+=( "${devices[$index]}" )
		done
		aux wait_for_sync $vg $lv1

		if [ "$j" -ge $(( i + 1 )) ]; then
			# Can't replace all at once.
			not lvconvert "${replace[@]}" $vg/$lv1
		else
			lvconvert "${replace[@]}" $vg/$lv1
		fi
	done
	done

	lvremove -ff $vg
done

vgremove -ff $vg
