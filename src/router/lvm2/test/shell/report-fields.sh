#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
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

# Test only that there's correct set of fields displayed on output.

aux prepare_pvs 1

OPTS="--nameprefixes --noheadings --rows"

aux lvmconf 'report/pvs_cols="pv_name,pv_size"'
aux lvmconf 'report/compact_output=0'
aux lvmconf 'report/compact_output_cols=""'

pvs $OPTS > out
grep LVM2_PV_NAME out
grep LVM2_PV_SIZE out

pvs $OPTS -o pv_attr > out
grep LVM2_PV_ATTR out
not grep -v LVM2_PV_ATTR out

pvs $OPTS -o+pv_attr > out
grep LVM2_PV_NAME out
grep LVM2_PV_SIZE out
grep LVM2_PV_ATTR out

pvs $OPTS -o-pv_name > out
not grep LVM2_PV_NAME out
grep LVM2_PV_SIZE out

pvs $OPTS -o+pv_attr -o-pv_attr > out
grep LVM2_PV_NAME out
grep LVM2_PV_SIZE out
not grep LVM2_PV_ATTR out

pvs $OPTS -o-pv_attr -o+pv_attr > out
grep LVM2_PV_NAME out
grep LVM2_PV_SIZE out
grep LVM2_PV_ATTR out

pvs $OPTS -o+pv_attr -o-pv_attr -o pv_attr > out
grep LVM2_PV_ATTR out
not grep -v LVM2_PV_ATTR out

# -o-size is the same as -o-pv_size - the prefix is recognized
pvs $OPTS -o-size > out
not grep LVM2_PV_SIZE out

# PV does not have tags nor is it exported if we haven't done that explicitly.
# Check compaction per field is done correctly.
pvs $OPTS -o pv_name,pv_exported,pv_tags -o#pv_tags > out
grep LVM2_PV_NAME out
grep LVM2_PV_EXPORTED out
not grep LVM2_PV_TAGS out

aux lvmconf 'report/compact_output_cols="pv_tags"'

pvs $OPTS -o pv_name,pv_exported,pv_tags > out
grep LVM2_PV_NAME out
grep LVM2_PV_EXPORTED out
not grep LVM2_PV_TAGS out

pvs $OPTS -o pv_name,pv_exported,pv_tags -o#pv_exported > out
grep LVM2_PV_NAME out
not grep LVM2_PV_EXPORTED out
grep LVM2_PV_TAGS out

aux lvmconf 'report/compact_output=1'
pvs $OPTS -o pv_name,pv_exported,pv_tags > out
grep LVM2_PV_NAME out
not grep LVM2_PV_EXPORTED out
not grep LVM2_PV_TAGS out

pvs $OPTS -o pv_name,pv_exported,pv_tags -o#pv_exported > out
grep LVM2_PV_NAME out
not grep LVM2_PV_EXPORTED out
not grep LVM2_PV_TAGS out
