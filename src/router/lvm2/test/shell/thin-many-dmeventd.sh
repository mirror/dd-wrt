#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test activation of monitoring with more thin-pool

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

#
# Main
#
aux have_thin 1 0 0 || skip

aux prepare_dmeventd
aux prepare_vg 2 64

# Create couple pools to later cause race in dmeventd during activation.
# each pool may add 1sec. extra delay

for i in $(seq 1 5)
do
	lvcreate --errorwhenfull y -Zn -T -L4M -V4M $vg/pool_${i} -n $lv${i}
        # Fill thin-pool to some capacity >50%
	dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv${i}" bs=256K count=9 conv=fdatasync
done

lvs -a $vg
vgchange -an $vg


# Try to now activate all existing pool - this will generate in about 10sec later
# storm of intial call of 'lvextend --use-policies'
vgchange -ay $vg

# Every 10sec. ATM there is DM status monitoring made by dmeventd
sleep 9

# Here try to hit the race by creating several new thin-pools in sequence.
# Creation meets with dmeventd running 'lvextend' command and taking
# it's internal lvm2 library lock - this used to make impossible to proceed with
# new thin-pool registration.
for i in $(seq 11 15)
do
	#/usr/bin/time -o TM -f %e lvcreate --errorwhenfull y -Zn -T -L4M -V4M $vg/pool_${i} -n $lv${i}
	#read -r t < TM
	#test ${t%%.*} -lt 8 || die "Creation of thin pool took more then 8 second! ($t seconds)"
	START=$(date +%s)
	lvcreate --errorwhenfull y -Zn -T -L4M -V4M $vg/pool_${i} -n $lv${i}
	END=$(date +%s)
	DIFF=$(( END - START ))
	test "$DIFF" -lt 8 || die "Creation of thin pool took more then 8 second! ($DIFF seconds)"
	# Fill thin-pool to some capacity >50%
	dd if=/dev/zero of="$DM_DEV_DIR/$vg/$lv${i}" bs=256K count=9 conv=fdatasync
done

vgremove -f $vg
