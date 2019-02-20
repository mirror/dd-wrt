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



. lib/inittest

aux prepare_vg 6
aux lvmconf 'allocation/maximise_cling = 0' \
	    'allocation/mirror_logs_require_separate_pvs = 1'

# 3-way, disk log
# multiple failures, full replace
lvcreate -aey --mirrorlog disk --type mirror -m 2 --ignoremonitoring --nosync -L 1 -n 3way $vg "$dev1" "$dev2" "$dev3" "$dev4":0-1
aux disable_dev "$dev1" "$dev2"
lvconvert -y --repair $vg/3way 2>&1 | tee 3way.out
lvs -a -o +devices $vg | not grep unknown
not grep "WARNING: Failed" 3way.out
vgreduce --removemissing $vg
check mirror $vg 3way
aux enable_dev "$dev1" "$dev2"
vgremove -ff $vg

# 3-way, disk log
# multiple failures, partial replace
vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3" "$dev4" "$dev5"
lvcreate -aey --mirrorlog disk --type mirror -m 2 --ignoremonitoring --nosync -L 1 -n 3way $vg "$dev1" "$dev2" "$dev3" "$dev4"
aux disable_dev "$dev1" "$dev2"
lvconvert -y --repair $vg/3way 2>&1 | tee 3way.out
grep "WARNING: Failed" 3way.out
lvs -a -o +devices $vg | not grep unknown
vgreduce --removemissing $vg
check mirror $vg 3way
aux enable_dev "$dev1" "$dev2"
vgremove -ff $vg

vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3"
lvcreate -aey --mirrorlog disk --type mirror -m 1 --ignoremonitoring --nosync -l 1 -n 2way $vg "$dev1" "$dev2" "$dev3"
aux disable_dev "$dev1"
lvconvert -y --repair $vg/2way 2>&1 | tee 2way.out
grep "WARNING: Failed" 2way.out
lvs -a -o +devices $vg | not grep unknown
vgreduce --removemissing $vg
check mirror $vg 2way
aux enable_dev "$dev1" "$dev2"
vgremove -ff $vg

# FIXME  - exclusive activation for mirrors should work here
# conversion of inactive cluster logs is also unsupported
test -e LOCAL_CLVMD && exit 0


# Test repair of inactive mirror with log failure
#  Replacement should fail, but convert should succeed (switch to corelog)
vgcreate $SHARED $vg "$dev1" "$dev2" "$dev3" "$dev4"
lvcreate -aey --type mirror -m 2 --ignoremonitoring -l 2 -n mirror2 $vg "$dev1" "$dev2" "$dev3" "$dev4":0
vgchange -a n $vg
pvremove -ff -y "$dev4"
lvconvert -y --repair $vg/mirror2
check mirror $vg mirror2
vgs $vg
vgremove -ff $vg
