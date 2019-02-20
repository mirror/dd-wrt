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

# Exercise obtaining cache parameter from various sources
# Either commmand line or metadata profile or implicit default...


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 8 0 || skip

PDIR="$LVM_SYSTEM_DIR/profile"
PFILE="cache-test"

aux prepare_profiles

cat <<EOF > "$PDIR/${PFILE}.profile"
allocation {
	cache_pool_chunk_size = 128
	cache_mode = "writeback"
	cache_policy = "mq"
	cache_metadata_format = 1

	cache_settings {
		smq {
			sequential_threshold = 300
			random_threshold = 500
		}
		mq {
		}
		mq {
			sequential_threshold = 100
			random_threshold = 200
		}
	}
}
EOF

cat <<EOF > "$PDIR/${PFILE}1.profile"
allocation {
	cache_pool_chunk_size = 512
	cache_mode = "passthrough"
	cache_policy = "smq"
	cache_metadata_format = 1
}
EOF

aux prepare_vg 2 1000000

# Check chunk_size is grabbed from configuration
lvcreate -L1G --config 'allocation/cache_pool_chunk_size=512' --type cache-pool $vg/cpool
check lv_field $vg/cpool chunksize "512.00k"

# Check chunk_size can be overruled when caching LV.
lvcreate -H --chunksize 128K -L10 --cachepool $vg/cpool -n $lv1
check lv_field $vg/$lv1 chunksize "128.00k"

lvremove -f $vg


# Check chunk_size is grabbed from metadata profile
lvcreate -L1G --metadataprofile $PFILE --type cache-pool $vg/cpool
#lvcreate -L1G --commandprofile $PFILE --type cache-pool $vg/cpool

# profile name is stored with cache-pool
check lv_field $vg/cpool profile "$PFILE"
# cache chunk size is selected and stored on creation time
check lv_field $vg/cpool chunksize "128.00k"
# cache metadata format is not stored with cache-pool
check lv_field $vg/cpool cachemetadataformat ""
# cache mode is not stored with cache-pool
check lv_field $vg/cpool cachemode ""
# cache policy is not stored with cache-pool
check lv_field $vg/cpool cachepolicy ""
# cache settings are not stored with cache-pool
check lv_field $vg/cpool cachesettings ""


lvcreate -L10 -n $lv1 $vg
lvconvert --metadataprofile "${PFILE}1" -y -H --cachepool $vg/cpool $vg/$lv1
# chunk size 128k is replace with 512k from PFILE1
check lv_field $vg/$lv1 chunksize "512.00k"
# cachemode is from PFILE1
check lv_field $vg/$lv1 cachemode "passthrough"
lvremove -f $vg

lvcreate -L1G --metadataprofile "$PFILE" --type cache-pool $vg/cpool
lvcreate -H -L10 -n $lv1 --cachepool $vg/cpool
# profile name is stored with cache
check lv_field $vg/$lv1 profile "$PFILE"
# cache chunk size is selected and stored on creation time
check lv_field $vg/$lv1 chunksize "128.00k"
# cache metadata format is stored with cache
check lv_field $vg/$lv1 cachemetadataformat "1"
# cache mode is stored with cache
check lv_field $vg/$lv1 cachemode "writeback"
# cache policy is stored with cache
check lv_field $vg/$lv1 cachepolicy "mq"
# cache settings are stored with cache
check lv_field $vg/$lv1 cachesettings "sequential_threshold=100,random_threshold=200"

lvremove -f $vg

#####

lvcreate -L1G --metadataprofile "$PFILE" --type cache-pool $vg/cpool
lvcreate --cachesettings 'sequential_threshold=300'  -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 profile "$PFILE"
check lv_field $vg/$lv1 cachesettings "sequential_threshold=300"
lvremove -f $vg

#####

lvcreate -L1G --metadataprofile "$PFILE" --type cache-pool $vg/cpool
lvcreate --chunksize 256    -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 cachemode "writeback"
check lv_field $vg/$lv1 chunksize "256.00k"
lvremove -f $vg


#####

lvcreate -L1G --metadataprofile "$PFILE" --type cache-pool $vg/cpool
lvcreate --metadataprofile "${PFILE}1"   -H -L10 -n $lv1 --cachepool $vg/cpool
check lv_field $vg/$lv1 chunksize "512.00k"
check lv_field $vg/$lv1 cachemode "passthrough"
lvremove -f $vg

#lvs -a -o+chunksize,cachemode,cachemetadataformat,cachepolicy,cachesettings $vg

vgremove -ff $vg
