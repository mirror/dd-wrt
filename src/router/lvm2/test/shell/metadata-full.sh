#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Test full metadata'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

# this test needs lot of memory
test "$(aux total_mem)" -gt 524288 || skip

LVM_TEST_PVS=${LVM_TEST_PVS:-64}

# aux prepare_vg $LVM_TEST_PVS

unset LVM_LOG_FILE_MAX_LINES

aux prepare_devs 64 1000
get_devs

vgcreate $SHARED -s 512K --metadatacopies 8 $vg "${DEVICES[@]}"


# Create a large metadata set, that getting close to 1/2MiB in size
#
# uses long tags to increase the size of the metadata
# more quickly
#
# the specific number of LVs in these loops isn't great
# because it doesn't depend specified behavior, but it's
# based on how much metadata it produces at the time this
# is written.

vgcfgbackup -f data $vg
TEST_DEVS=925
# Generate a lot of LV devices (size of 1 extent)
awk -v TEST_DEVS=$TEST_DEVS '/^\t\}/ {
    printf("\t}\n\tlogical_volumes {\n");
    cnt=0;
    for (i = 0; i < TEST_DEVS; i++) {
	printf("\t\tlvol%d  {\n", i);
	printf("\t\t\tid = \"%06d-1111-2222-3333-2222-1111-%06d\"\n", i, i);
	print "\t\t\tstatus = [\"READ\", \"WRITE\", \"VISIBLE\"]";
        print "\t\t\ttags = [\"A123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\"]";
	print "\t\t\tsegment_count = 1";
	print "\t\t\tsegment1 {";
	print "\t\t\t\tstart_extent = 0";
	print "\t\t\t\textent_count = 1";
	print "\t\t\t\ttype = \"striped\"";
	print "\t\t\t\tstripe_count = 1";
	print "\t\t\t\tstripes = [";
	print "\t\t\t\t\t\"pv0\", " cnt++;
	printf("\t\t\t\t]\n\t\t\t}\n\t\t}\n");
      }
  }
  {print}
' data >data_new
# Restoring big data set of LVs
vgcfgrestore -f data_new $vg


# should show non-zero
vgs -o+pv_mda_free

# these addtag's will fail at some point when metadata space is full

for i in $(seq 1 "$TEST_DEVS"); do
	lvchange --addtag B123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 $vg/lvol$i || break;
done

# test we hit 'out-of-metadata-space'
test "$i" -gt 2
test "$i" -lt "$TEST_DEVS"

# should show 0
vgs -o+pv_mda_free
check vg_field $vg vg_mda_free 0

# remove some of the tags to check that we can reduce the size of the
# metadata, and continue using the vg

for j in $(seq 1 "$i"); do
	lvchange --deltag B123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 $vg/lvol$j;
done

# should show non-zero
vgs -o+pv_mda_free

# these will fail at some point when metadata space is full again

for i in $(seq 1 50); do
	lvcreate -l1 -an --addtag C123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 $vg || break;
done

# should show 0
vgs -o+pv_mda_free
check vg_field $vg vg_mda_free 0

# as long as we have a lot of LVs around, try to activate them all
# (filters are already set up that exclude the activated LVs from
# being scanned)

time vgs

# Avoid activation of large set of volumes - this is tested in  vgchange-many.sh
#vgchange -ay $vg
#vgchange -an $vg

# see if we can remove LVs to make more metadata space,
# and then create more LVs

for i in $(seq 1 30); do lvremove -y $vg/lvol$i; done

for i in $(seq 1 10); do lvcreate -l1 $vg; done

# should show non-zero
vgs -o+pv_mda_free

# FIXME:
#    takes extreme amount of time, despite the fact, there are only few LVs active.
vgremove -ff $vg
