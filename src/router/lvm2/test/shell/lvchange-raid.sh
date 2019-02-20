#!/usr/bin/env bash

# Copyright (C) 2013 Red Hat, Inc. All rights reserved.
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

# Writemostly has been in every version since the begining
# Device refresh in 1.5.1 upstream and 1.3.4 < x < 1.4.0 in RHEL6
# Sync action    in 1.5.0 upstream and 1.3.3 < x < 1.4.0 in RHEL6
# Proper mismatch count 1.5.2 upstream,1.3.5 < x < 1.4.0 in RHEL6
#
# We will simplify and simple test for 1.5.2 and 1.3.5 < x < 1.4.0
aux have_raid 1 3 5 &&
  ! aux have_raid 1 4 0 ||
  aux have_raid 1 5 2 || skip

# DEVICE "$dev6" is reserved for non-RAID LVs that
# will not undergo failure
aux prepare_vg 6

# run_writemostly_check <VG> <LV>
run_writemostly_check() {
	local vg=$1
	local lv=${2}${THIN_POSTFIX}
	local segtype=
	local d0
	local d1

	segtype=$(get lv_field $vg/$lv segtype -a)
	d0=$(get lv_devices $vg/${lv}_rimage_0)
	d1=$(get lv_devices $vg/${lv}_rimage_1)

	printf "#\n#\n#\n# %s/%s (%s): run_writemostly_check\n#\n#\n#\n" \
		$vg $lv $segtype

	# I've seen this sync fail.  when it does, it looks like sync
	# thread has not been started... haven't repo'ed yet.
	aux wait_for_sync $vg $lv

	# No writemostly flag should be there yet.
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"
	check lv_attr_bit health $vg/${lv}_rimage_1 "-"

	if [ "$segtype" != "raid1" ]; then
		not lvchange --writemostly $d0 $vg/$lv
		return
	fi

	# Set the flag
	lvchange --writemostly $d0 $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# Running again should leave it set (not toggle)
	lvchange --writemostly $d0 $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# Running again with ':y' should leave it set
	lvchange --writemostly $d0:y $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# ':n' should unset it
	lvchange --writemostly $d0:n $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"

	# ':n' again should leave it unset
	lvchange --writemostly $d0:n $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"

	# ':t' toggle to set
	lvchange --writemostly $d0:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# ':t' toggle to unset
	lvchange --writemostly $d0:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"

	# ':y' to set
	lvchange --writemostly $d0:y $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# Toggle both at once
	lvchange --writemostly $d0:t --writemostly $d1:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"
	check lv_attr_bit health $vg/${lv}_rimage_1 "w"

	# Toggle both at once again
	lvchange --writemostly $d0:t --writemostly $d1:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"
	check lv_attr_bit health $vg/${lv}_rimage_1 "-"

	# Toggle one, unset the other
	lvchange --writemostly $d0:n --writemostly $d1:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"
	check lv_attr_bit health $vg/${lv}_rimage_1 "w"

	# Toggle one, set the other
	lvchange --writemostly $d0:y --writemostly $d1:t $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"
	check lv_attr_bit health $vg/${lv}_rimage_1 "-"

	# Partial flag supercedes writemostly flag
	aux disable_dev $d0
	check lv_attr_bit health $vg/${lv}_rimage_0 "p"

	# It is possible for the kernel to detect the failed device before
	# we re-enable it.  If so, the field will be set to 'r'efresh since
	# that also takes precedence over 'w'ritemostly.  If this has happened,
	# we refresh the LV and then check for 'w'.
	aux enable_dev $d0
	check lv_attr_bit health $vg/${lv}_rimage_0 "r" && lvchange --refresh $vg/$lv
	check lv_attr_bit health $vg/${lv}_rimage_0 "w"

	# Catch Bad writebehind values
	invalid lvchange --writebehind "invalid" $vg/$lv
	invalid lvchange --writebehind -256 $vg/$lv

	# Set writebehind
	check lv_field $vg/$lv raid_write_behind ""
	lvchange --writebehind 512 $vg/$lv
	check lv_field $vg/$lv raid_write_behind "512"

	# Converting to linear should clear flags and writebehind
	not lvconvert -m 0 $vg/$lv $d1
	lvconvert -y -m 0 $vg/$lv $d1
	lvconvert -y --type raid1 -m 1 $vg/$lv $d1
	check lv_field $vg/$lv raid_write_behind ""
	check lv_attr_bit health $vg/${lv}_rimage_0 "-"
	check lv_attr_bit health $vg/${lv}_rimage_1 "-"
}

# run_syncaction_check <VG> <LV>
run_syncaction_check() {
	local device
	local seek
	local size
	local tmp
	local vg=$1
	local lv=${2}${THIN_POSTFIX}

	printf "#\n#\n#\n# %s/%s (%s): run_syncaction_check\n#\n#\n#\n" \
		$vg $lv "$(get lv_field "$vg/$lv" segtype -a)"
	aux wait_for_sync $vg $lv

	device=$(get lv_devices $vg/${lv}_rimage_1)

	size=$(get lv_field $vg/${lv}_rimage_1 size -a --units 1k)
	size=$(( ${size%\.00k} / 2 ))

	tmp=$(get pv_field "$device" mda_size --units 1k)
	seek=${tmp%\.00k} # Jump over MDA

	tmp=$(get lv_field $vg/${lv}_rmeta_1 size -a --units 1k)
	seek=$(( seek + ${tmp%\.00k} ))  # Jump over RAID metadata image

	seek=$(( seek + size )) # Jump halfway through the RAID image

	check lv_attr_bit health $vg/$lv "-"
	check lv_field $vg/$lv raid_mismatch_count "0"

	# Overwrite the last half of one of the PVs with crap
	dd if=/dev/urandom of="$device" bs=1k count=$size seek=$seek
	sync

	# Cycle the LV so we don't grab stripe cache buffers instead
	#  of reading disk.  This can happen with RAID 4/5/6.  You
	#  may think this is bad because those buffers could prevent
	#  us from seeing bad disk blocks, however, the stripe cache
	#  is not long lived.  (RAID1/10 are immediately checked.)
	lvchange -an $vg/$lv
	lvchange -ay $vg/$lv

	# "check" should find discrepancies but not change them
	# 'lvs' should show results
	lvchange --syncaction check $vg/$lv
	aux wait_for_sync $vg $lv
	check lv_attr_bit health $vg/$lv "m"
	not check lv_field $vg/$lv raid_mismatch_count "0"

	# "repair" will fix discrepancies
	lvchange --syncaction repair $vg/$lv
	aux wait_for_sync $vg $lv

	# Final "check" should show no mismatches
	# 'lvs' should show results
	lvchange --syncaction check $vg/$lv
	aux wait_for_sync $vg $lv
	check lv_attr_bit health $vg/$lv "-"
	check lv_field $vg/$lv raid_mismatch_count "0"
}

# run_refresh_check <VG> <LV>
#   Assumes "$dev2" is in the array
run_refresh_check() {
	local size
	local sizelv
	local vg=$1
	local lv=${2}${THIN_POSTFIX}

	printf "#\n#\n#\n# %s/%s (%s): run_refresh_check\n#\n#\n#\n" \
		$vg $lv "$(get lv_field $vg/$lv segtype -a)"

	aux wait_for_sync $vg $lv

	sizelv=$vg/$lv
	test -z "$THIN_POSTFIX" || sizelv=$vg/thinlv
	size=$(get lv_field $sizelv size --units 1k)
	size=${size%\.00k}

	# Disable dev2 and do some I/O to make the kernel notice
	aux disable_dev "$dev2"
	dd if=/dev/urandom of="$DM_DEV_DIR/$sizelv" bs=1k count=$size
	sync

	# Check for 'p'artial flag
	check lv_attr_bit health $vg/$lv "p"
	dmsetup status
	lvs -a -o name,attr,devices $vg

	aux enable_dev "$dev2"

	dmsetup status
	lvs -a -o name,attr,devices $vg

	# Check for 'r'efresh flag
	check lv_attr_bit health $vg/$lv "r"

	lvchange --refresh $vg/$lv
	aux wait_for_sync $vg $lv
	check lv_attr_bit health $vg/$lv "-"

	# Writing random data above should mean that the devices
	# were out-of-sync.  The refresh should have taken care
	# of properly reintegrating the device.
	lvchange --syncaction repair $vg/$lv
	aux wait_for_sync $vg $lv
	check lv_attr_bit health $vg/$lv "-"
}

# run_recovery_rate_check <VG> <LV>
#   Assumes "$dev2" is in the array
run_recovery_rate_check() {
	local vg=$1
	local lv=${2}${THIN_POSTFIX}

	printf "#\n#\n#\n# %s/%s (%s): run_recovery_rate_check\n#\n#\n#\n" \
		 $vg $lv "$(get lv_field $vg/$lv segtype -a)"
	lvchange --minrecoveryrate 50 $vg/$lv
	lvchange --maxrecoveryrate 100 $vg/$lv

	check lv_field $vg/$lv raid_min_recovery_rate "50"
	check lv_field $vg/$lv raid_max_recovery_rate "100"
}

# run_checks <VG> <LV> <"-"|snapshot_dev|"thinpool_data"|"thinpool_meta">
run_checks() {
	THIN_POSTFIX=""

	if [ -z "$3" ]; then
		printf "#\n#\n# run_checks: Too few arguments\n#\n#\n"
		return 1
	elif [ '-' = "$3" ]; then
		printf "#\n#\n# run_checks: Simple check\n#\n#\n"

		run_writemostly_check $1 $2
		run_syncaction_check $1 $2
		run_refresh_check $1 $2
		run_recovery_rate_check $1 $2
	elif [ "thinpool_data" = "$3" ]; then
		printf "#\n#\n# run_checks: RAID as thinpool data\n#\n#\n"

# Hey, specifying devices for thin allocation doesn't work
#		lvconvert -y --thinpool $1/$2 "$dev6"
		lvcreate -aey -L 2M -n ${2}_meta $1 "$dev6"
		lvconvert --thinpool $1/$2 --poolmetadata ${2}_meta
		lvcreate -T $1/$2 -V 1 -n thinlv
		THIN_POSTFIX="_tdata"

		run_writemostly_check $1 $2
		run_syncaction_check $1 $2
		run_refresh_check $1 $2
		run_recovery_rate_check $1 $2
	elif [ "thinpool_meta" = "$3" ]; then
		printf "#\n#\n# run_checks: RAID as thinpool metadata\n#\n#\n"

		lvrename $1/$2 ${2}_meta
		lvcreate -aey -L 2M -n $2 $1 "$dev6"
		lvconvert -y --thinpool $1/$2 --poolmetadata ${2}_meta
		lvcreate -T $1/$2 -V 1 -n thinlv
		THIN_POSTFIX="_tmeta"

		run_writemostly_check $1 $2
		run_syncaction_check $1 $2
		run_refresh_check $1 $2
		run_recovery_rate_check $1 $2
	elif [ "snapshot" = "$3" ]; then
		printf "#\n#\n# run_checks: RAID under snapshot\n#\n#\n"
		lvcreate -aey -s $1/$2 -l 4 -n snap "$dev6"

		run_writemostly_check $1 $2
		run_syncaction_check $1 $2
		run_refresh_check $1 $2
		run_recovery_rate_check $1 $2

		lvremove -ff $1/snap
	else
		printf "#\n#\n# run_checks: Invalid argument\n#\n#\n"
		return 1
	fi
}

run_types() {
	for i in $TEST_TYPES ; do
		lvcreate -n $lv1 $vg -L2M --type "$@"
		run_checks $vg $lv1 $i
		lvremove -ff $vg
	done
}

########################################################
# MAIN
########################################################

TEST_TYPES="- snapshot"
# RAID works EX in cluster
# thinpool works EX in cluster
# but they don't work together in a cluster yet
#  (nor does thinpool+mirror work in a cluster yet)
test ! -e LOCAL_CLVMD && aux have_thin 1 8 0 && TEST_TYPE="$TEST_TYPES thinpool_data thinpool_meta"

# Implicit test for 'raid1' only
if test "${TEST_RAID:-raid1}" = raid1 ; then
	run_types raid1 -m 1 "$dev1" "$dev2"
	vgremove -ff $vg
fi
