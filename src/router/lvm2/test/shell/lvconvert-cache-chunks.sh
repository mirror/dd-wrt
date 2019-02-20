#!/usr/bin/env bash

# Copyright (C) 2016-2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise number of cache chunks in cache pool
# Skips creation of real cached device for older cache targets...


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 2 1000000

# Really large cache pool data LV
lvcreate -L1T -n cpool $vg

# Works and pick higher chunks size then default
lvconvert -y --type cache-pool $vg/cpool

# Check chunk size in sectors is more then 512K
test "$(get lv_field "$vg/cpool" chunk_size --units s --nosuffix)" -gt 1000

lvcreate -L1M -n $lv1 $vg

# Not let pass small chunks when caching origin
fail lvconvert -y -H --chunksize 128K --cachepool $vg/cpool $vg/$lv1 >out 2>&1
cat out
grep "too small chunk size" out

# Thought 2M is valid
if aux have_cache 1 8 0 ; then
	# Without SMQ we run out of kernel memory easily
	lvconvert -y -H  --chunksize 2M --cachepool $vg/cpool $vg/$lv1
fi

lvremove -f $vg

###

# Really large cache pool data LV
lvcreate -L1T -n cpool $vg
# Not allowed to create more then 10e6 chunks
fail lvconvert -y --type cache-pool --chunksize 128K $vg/cpool

if aux have_cache 1 8 0 ; then
	# Let operation pass when max_chunk limit is raised
	lvconvert -y --type cache-pool --chunksize 128K $vg/cpool \
		--config 'allocation/cache_pool_max_chunks=10000000'
fi

vgremove -f $vg
