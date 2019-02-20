#!/usr/bin/env bash

# Copyright (C) 2009-2015 Red Hat, Inc. All rights reserved.
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

# skip this test if mdadm or sfdisk (or others) aren't available
which sfdisk || skip

test -f /proc/mdstat && grep -q raid0 /proc/mdstat || \
	modprobe raid0 || skip

aux lvmconf 'devices/md_component_detection = 1'
aux extend_filter_LVMTEST
aux extend_filter "a|/dev/md.*|"

aux prepare_devs 2

# create 2 disk MD raid0 array (stripe_width=128K)
aux prepare_md_dev 0 64 2 "$dev1" "$dev2"

mddev=$(< MD_DEV)
pvdev=$(< MD_DEV_PV)

# Test alignment of PV on MD without any MD-aware or topology-aware detection
# - should treat $mddev just like any other block device
pvcreate --metadatasize 128k \
    --config 'devices {md_chunk_alignment=0 data_alignment_detection=0 data_alignment_offset_detection=0}' \
    "$pvdev"

check pv_field "$pvdev" pe_start "1.00m"

# Test md_chunk_alignment independent of topology-aware detection
pvcreate --metadatasize 128k \
    --config 'devices {data_alignment_detection=0 data_alignment_offset_detection=0}' \
    "$pvdev"
check pv_field "$pvdev" pe_start "1.00m"

# Test newer topology-aware alignment detection
# - first added to 2.6.31 but not "reliable" until 2.6.33
if aux kernel_at_least 2 6 33 ; then
    # optimal_io_size=131072, minimum_io_size=65536
    pvcreate --metadatasize 128k \
	--config 'devices { md_chunk_alignment=0 }' "$pvdev"
    check pv_field "$pvdev" pe_start "1.00m"
    pvremove "$pvdev"
fi

# partition MD array directly, depends on blkext in Linux >= 2.6.28
if aux kernel_at_least 2 6 28 ; then
    # create one partition
    sfdisk "$mddev" <<EOF
,,83
EOF
    # Wait till all partition links in udev are created
    aux udev_wait

    # Skip test if udev rule has not created proper links for partitions
    test -b "${mddev}p1" || { ls -laR /dev ; skip "Missing partition link" ; }

    pvscan
    # make sure partition on MD is _not_ removed
    # - tests partition -> parent lookup via sysfs paths
    not pvcreate --metadatasize 128k "$pvdev"

    # verify alignment_offset is accounted for in pe_start
    # - topology infrastructure is available in Linux >= 2.6.31
    # - also tests partition -> parent lookup via sysfs paths

    # Checking for 'alignment_offset' in sysfs implies Linux >= 2.6.31
    # but reliable alignment_offset support requires kernel.org Linux >= 2.6.33
    if aux kernel_at_least 2 6 33 ; then
	# in case the system is running without devtmpfs /dev
	# wait here for created device node on tmpfs
	test "$DM_DEV_DIR" = "/dev" || cp -LR "${mddev}p1" "${pvdev%/*}"

	pvcreate --metadatasize 128k "${pvdev}p1"

	maj=$(($(stat -L --printf=0x%t "${mddev}p1")))
	min=$(($(stat -L --printf=0x%T "${mddev}p1")))

	ls /sys/dev/block/$maj:$min/
	ls /sys/dev/block/$maj:$min/holders/
	cat /sys/dev/block/$maj:$min/dev
	cat /sys/dev/block/$maj:$min/stat
	cat /sys/dev/block/$maj:$min/size

	sysfs_alignment_offset="/sys/dev/block/$maj:$min/alignment_offset"
	[ -f "$sysfs_alignment_offset" ] && \
		alignment_offset=$(< "$sysfs_alignment_offset") || \
		alignment_offset=0

	# default alignment is 1M, add alignment_offset
	pv_align=$(( 1048576 + alignment_offset ))
	check pv_field "${pvdev}p1" pe_start $pv_align --units b --nosuffix

	pvremove "${pvdev}p1"
	test "$DM_DEV_DIR" = "/dev" || rm -f "${pvdev}p1"
    fi
fi

# Test newer topology-aware alignment detection w/ --dataalignment override
if aux kernel_at_least 2 6 33 ; then
    # make sure we're clean for another test
    dd if=/dev/zero of="$mddev" bs=512 count=4 conv=fdatasync
    partprobe -s "$mddev"
    aux prepare_md_dev 0 1024 2 "$dev1" "$dev2"
    pvdev=$(< MD_DEV_PV)

    # optimal_io_size=2097152, minimum_io_size=1048576
    pvcreate --metadatasize 128k \
	--config 'devices { md_chunk_alignment=0 }' "$pvdev"

    # to see the processing of scanning
    pvs -vvvv

    check pv_field "$pvdev" pe_start "2.00m"

    # now verify pe_start alignment override using --dataalignment
    pvcreate --dataalignment 64k --metadatasize 128k \
	--config 'devices { md_chunk_alignment=0 }' "$pvdev"
    check pv_field "$pvdev" pe_start "192.00k"
fi
