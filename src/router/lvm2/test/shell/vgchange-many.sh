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

# Check perfomance of activation and deactivation

SKIP_WITH_LVMPOLLD=1

. lib/inittest

# Number of LVs to create
TEST_DEVS=1000
# On low-memory boxes let's not stress too much
test "$(aux total_mem)" -gt 524288 || TEST_DEVS=256

aux prepare_pvs 1 400
get_devs

vgcreate $SHARED -s 128K "$vg" "${DEVICES[@]}"

vgcfgbackup -f data $vg

# Generate a lot of devices (size of 1 extent)
awk -v TEST_DEVS=$TEST_DEVS '/^\t\}/ {
    printf("\t}\n\tlogical_volumes {\n");
    cnt=0;
    for (i = 0; i < TEST_DEVS; i++) {
	printf("\t\tlvol%06d  {\n", i);
	printf("\t\t\tid = \"%06d-1111-2222-3333-2222-1111-%06d\"\n", i, i);
	print "\t\t\tstatus = [\"READ\", \"WRITE\", \"VISIBLE\"]";
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

vgcfgrestore -f data_new $vg

# Activate and deactivate all of them
vgchange -ay $vg
vgchange -an $vg
