#!/usr/bin/env bash

# Copyright (C) 2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# test activation race for raid's --syncaction check


SKIP_WITH_LVMPOLLD=1


# Current support for syncaction in cluster is broken
# might get fixed one day though
# meanwhile skipped
SKIP_WITH_CLVMD=1

. lib/inittest

aux have_cache 1 5 0 || skip

# Proper mismatch count 1.5.2+ upstream, 1.3.5 < x < 1.4.0 in RHEL6
aux have_raid 1 3 5 &&
  ! aux have_raid 1 4 0 ||
  aux have_raid 1 5 2 || skip
aux prepare_vg 3


# Bug 1169495 - RFE: allow raid scrubbing on cache origin raid volumes
# lvcreate RAID1 origin, lvcreate cache-pool, and lvconvert to cache
#  then test that the origin can be scrubbed.
lvcreate --type raid1 -m 1 --nosync -l 2 -n $lv1 $vg
lvcreate --type cache-pool -l 1 -n ${lv1}_cachepool $vg
lvconvert --cache -Zy --cachepool $vg/${lv1}_cachepool $vg/$lv1
lvchange --syncaction check $vg/${lv1}_corig
# Check may go too quickly to verify with check of syncaction

vgremove -ff $vg
