#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# 'Exercise logic around boundary sizes of thin-pool data and chunksize


SKIP_WITH_LVMPOLLD=1

. lib/inittest

# FIXME  update test to make something useful on <16T
aux can_use_16T || skip

aux have_thin 1 0 0 || skip

# Prepare ~1P sized devices
aux prepare_vg 1 1000000000

lvcreate -an -T -L250T $vg/pool250

lvcreate -an -T -L250T --poolmetadatasize 16G $vg/pool16

fail lvcreate -an -T -L250T --chunksize 64K --poolmetadatasize 16G $vg/pool64

# Creation of thin-pool with proper chunk-size but not enough metadata size
# which can grow later needs to pass
lvcreate -an -T -L250T --chunksize 1M --poolmetadatasize 4G $vg/pool1024

# Creation of chunk should fit
lvcreate -an -T -L12T --chunksize 64K --poolmetadatasize 16G $vg/pool64

check lv_field $vg/pool64 chunksize "64.00k"

lvremove -ff $vg


### Check also lvconvert ###

lvcreate -an -L250T -n pool $vg

fail lvconvert -y --chunksize 64 --thinpool $vg/pool
lvconvert -y --chunksize 1M --thinpool $vg/pool

check lv_field $vg/pool chunksize "1.00m"

vgremove -ff $vg
