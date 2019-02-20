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

# 'Exercise some lvcreate diagnostics'

. lib/inittest

aux prepare_pvs 4
get_devs

aux pvcreate --metadatacopies 0 "$dev1"
aux vgcreate $SHARED "$vg" "${DEVICES[@]}"

invalid lvcreate --type free -l1 -n $lv1 $vg 2>err
grep "Invalid argument for --type" err
invalid lvcreate --type $RANDOM -l1 -n $lv1 $vg
invalid lvcreate --type unknown -l1 -n $lv1 $vg

invalid lvcreate -L10000000000000000000 -n $lv $vg 2>&1 | tee err
grep "Size is too big" err
invalid lvcreate -L+-10 -n $lv $vg 2>&1 | tee err
grep "Multiple sign" err
invalid lvcreate -L-.1 -n $lv $vg  2>&1 | tee err
grep "Size may not be negative" err
invalid lvcreate -L..1 -n $lv $vg  2>&1 | tee err
grep "Can't parse size" err

lvcreate --type linear -aey -m0 -l1 -n $lv1 $vg
lvcreate --type snapshot -l1 -n $lv2 $vg/$lv1
# Supporting decimal point with size
lvcreate -L.1 -n $lv3 $vg

# Reject repeated invocation (run 2 times) (bz178216)
lvcreate -n $lv -l 4 $vg
not lvcreate -n $lv -l 4 $vg
lvremove -ff $vg/$lv
# Try to remove it again - should fail (but not segfault)
not lvremove -ff $vg/$lv

# Reject a negative stripe_size
invalid lvcreate -L 64m -n $lv -i2 --stripesize -4 $vg 2>err;
grep "may not be negative" err

# Reject a too-large stripesize
invalid lvcreate -L 64m -n $lv -i2 --stripesize 4294967291 $vg 2>err
grep "Stripe size cannot be larger than" err

# w/single stripe succeeds with diagnostics to stdout
lvcreate -L 64m -n $lv -i1 --stripesize 4 $vg 2> err | tee out
grep "Ignoring stripesize argument with single stripe" out
lvdisplay $vg
lvremove -ff $vg

# w/default (64KB) stripe size succeeds with diagnostics to stdout
lvcreate -L 64m -n $lv -i2 $vg > out
grep "Using default stripesize" out
lvdisplay $vg
check lv_field $vg/$lv stripesize "64.00k"
lvremove -ff $vg

# Reject an invalid number of stripes
invalid lvcreate -L 64m -n $lv -i129 $vg 2>err
grep "Number of stripes (129) must be between 1 and 128" err

# Reject an invalid stripe size
invalid lvcreate -L 64m -n $lv -i2 --stripesize 3 $vg 2>err
grep "Invalid stripe size" err
# Verify that the LV was not created via lvdisplay empty output
test -z "$(lvdisplay $vg)"

# Setting max_lv works. (bz490298)
check vg_field $vg max_lv "0"
vgchange -l 3 $vg
check vg_field $vg max_lv "3"
lvcreate -aey -l1 -n $lv1 $vg
lvcreate -l1 -s -n $lv2 $vg/$lv1
lvcreate -l1 -n $lv3 $vg
fail lvcreate -l1 -n $lv4 $vg
lvremove -ff $vg/$lv3

# Check snapshot of inactive origin
lvchange -an $vg/$lv1
lvcreate -l1 -s -n $lv3 $vg/$lv1
fail lvcreate -l1 -n $lv4 $vg
fail lvcreate -l1 --type mirror -m1 -n $lv4 $vg

lvremove -ff $vg/$lv3
lvcreate -aey -l1 --type mirror -m1 -n $lv3 $vg
not lvcreate -l1 -n $lv4 $vg
not lvcreate -l1 --type mirror -m1 -n $lv4 $vg

lvconvert -m0 $vg/$lv3
lvconvert -m2 --type mirror -i 1 $vg/$lv3
lvconvert -m1 $vg/$lv3

fail vgchange -l 2
check vg_field $vg max_lv "3"
vgchange -l 4
check vg_field $vg max_lv "4"

lvremove -ff $vg
vgchange -l 0 $vg
check vg_field $vg max_lv "0"

# Rejects invalid chunksize, accepts between 4K and 512K
# and validate origin_size
lvcreate -aey -L 32m -n $lv1 $vg
not lvcreate -L 8m -n $lv2 -s --chunksize 3k $vg/$lv1
not lvcreate -L 8m -n $lv2 -s --chunksize 1024k $vg/$lv1
lvcreate -L 8m -n $lv2 -s --chunksize 4k $vg/$lv1
check lv_field $vg/$lv2 chunk_size "4.00k"
check lv_field $vg/$lv2 origin_size "32.00m"
lvcreate -L 8m -n $lv3 -s --chunksize 512k $vg/$lv1
check lv_field $vg/$lv3 chunk_size "512.00k"
check lv_field $vg/$lv3 origin_size "32.00m"
lvremove -f $vg

# Mirror regionsize must be
# - nonzero (bz186013)
# - a power of 2 and a multiple of page size
# - <= size of LV
invalid lvcreate --type mirror -m 1 -L 32m -n $lv -R 0 $vg 2>err
grep "may not be zero" err
invalid lvcreate --type mirror -m 1 -L 32m -n $lv -R 11k $vg
invalid lvcreate --type mirror -m 1 -L 32m -n $lv -R 1k $vg
lvcreate -aey -L 32m -n $lv --regionsize 128m  --type mirror -m 1 $vg
check lv_field $vg/$lv regionsize "32.00m"
lvremove -f $vg
lvcreate -aey -L 32m -n $lv --regionsize 4m --type mirror -m 1 $vg
check lv_field $vg/$lv regionsize "4.00m"

# -m0 is creating non-mirrored segment and give info about redundant option
lvcreate -m 0 -l1 -n $lv1 $vg 2>&1 | tee err
grep "Redundant" err
check lv_field $vg/$lv1 segtype "linear"
lvremove -ff $vg

if test -n "$LVM_TEST_LVMLOCKD"; then
echo "skip snapshot without origin"
else

# Old --type snapshot works with -s
lvcreate --type snapshot -s -V64 -L32 -n $lv1 $vg
check lv_field $vg/$lv1 segtype "linear"
lvcreate --type snapshot -V64 -L32 -n $lv2 $vg
check lv_field $vg/$lv2 segtype "linear"
lvremove -ff $vg

# --virtualoriginsize  always makes old snapshot
lvcreate -s --virtualoriginsize 64m -L 32m -n $lv1 $vg
check lv_field $vg/$lv1 segtype "linear"
lvrename $vg/$lv1 $vg/$lv2
lvcreate -s --virtualoriginsize 64m -L 32m -n $lv1 $vg
lvchange -a n $vg/$lv1
lvremove -ff $vg/$lv1
lvremove -ff $vg

fi

# readahead default (auto), none, #, auto
lvcreate -L 8 -n $lv1 $vg
check lv_field $vg/$lv1 lv_read_ahead "auto"
lvcreate -L 8 -n $lv2 --readahead none $vg
check lv_field $vg/$lv2 lv_read_ahead "0"
check lv_field $vg/$lv2 lv_kernel_read_ahead "0"
lvcreate -L 8 -n $lv3 --readahead 8k $vg
check lv_field $vg/$lv3 lv_read_ahead "8.00k"
check lv_field $vg/$lv3 lv_kernel_read_ahead "8.00k"
lvcreate -L 8 -n $lv4 --readahead auto $vg
check lv_field $vg/$lv4 lv_read_ahead "auto"
check lv_field $vg/$lv4 lv_kernel_read_ahead "128.00k"
lvcreate -L 8 -n $lv5 -i2 --stripesize 16k --readahead auto $vg
check lv_field $vg/$lv5 lv_read_ahead "auto"
check lv_field $vg/$lv5 lv_kernel_read_ahead "128.00k"
lvcreate -L 8 -n $lv6 -i2 --stripesize 128k --readahead auto $vg
check lv_field $vg/$lv6 lv_read_ahead "auto"
check lv_field $vg/$lv6 lv_kernel_read_ahead "512.00k"
lvremove -ff $vg

#
# Validate --major --minor,  we need to know VG, thus failing
#
fail lvcreate -My --major 234 -l1 $vg
# cannot specify --major or --minor with -Mn
fail lvcreate -Mn --major 234 -l1 $vg
fail lvcreate --persistent n --minor 234 -l1 $vg
# out-of-range minor value
fail lvcreate --minor 9999999 -l1 $vg
if aux kernel_at_least 2 4 0; then
# On >2.4 we ignore --major
lvcreate --major 234 -l1 $vg 2>&1 | tee err;
grep "Ignoring" err
# Try some bigger possibly unused minor
if test ! -d /sys/block/dm-2345; then
	lvcreate --minor 2345 -l1 -n $lv1 $vg
	check lv_field $vg/$lv1 lv_kernel_minor "2345"
fi
if test ! -d /sys/block/dm-23456; then
	lvcreate -My --minor 23456 -j 122 -l1 -n $lv2 $vg
	check lv_field $vg/$lv2 lv_kernel_minor "23456"
fi
fi # 2.4
lvremove -f $vg

# prohibited names
for i in pvmove snapshot ; do
	invalid lvcreate -l1 -n ${i}1 $vg
done
for i in _cdata _cmeta _mimage _mlog _pmspare _tdata _tmeta _vorigin ; do
	invalid lvcreate -l1 -n s_${i}_1 $vg
done

# Check invalid error for pool-only options
invalid lvcreate --poolmetadataspare y -l1 $vg
invalid lvcreate --poolmetadatasize 10 -l1 $vg
invalid lvcreate --discards passdown -l1 $vg
