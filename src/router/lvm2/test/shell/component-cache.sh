#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise activation of cache component devices


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux have_cache 1 3 0 || skip

aux prepare_vg 5 80

lvcreate --type cache-pool -L 2 -n cpool $vg
lvcreate -H -L 4 -n corigin --cachepool $vg/cpool
lvchange -an $vg

for j in 1 2
do

# Activate supported components
for i in cpool_cmeta cpool_cdata corigin_corig
do
	test ! -e "$DM_DEV_DIR/$vg/$i"
	lvchange -ay -y $vg/$i
	# check usable link is there
	test -e "$DM_DEV_DIR/$vg/$i"

        # cannot take snapshot of any active component LV
        test "$j" -eq 2 || not lvcreate -s -L1 $vg/$i
done

# After 1st. phase deactivation works
# Volumes are left active for vgremove on 2nd.. pass
test "$j" -eq 2 || lvchange -an $vg

done

# Cannot active cached LV while any component LV is active
not lvchange -ay $vg/corigin |& tee err
grep "prohibited" err

lvs -a $vg

# Can split for writethrough|passthrough
# deactivates all components as well...
lvconvert --splitcache $vg/corigin
lvs -a $vg

# Cannot cache LV while components are active
lvcreate -L 4 -n $lv2 $vg
lvchange -ay -y $vg/cpool_cmeta

not lvconvert -y --cachepool $vg/cpool -H $lv2

lvremove -f $vg
lvs -a $vg

if aux have_thin 1 0 0 ; then

lvcreate --type cache-pool -L 2 -n cpool $vg
lvcreate -H -L 4 -n tpool --cachepool $vg/cpool
lvchange -an $vg
lvs -a $vg
# Cannot convert to thin-pool with component LV active
lvchange -ay -y $vg/cpool_cmeta

# Conversion does not need to activate data device, so it can proceed ??
lvconvert -y --thinpool $vg/tpool

# Thin-pool cannot be activated
not lvchange -ay $vg/tpool |& tee err
grep "prohibited" err

lvs -a $vg

fi

lvs -a $vg

# And final removal works
vgremove -f $vg
