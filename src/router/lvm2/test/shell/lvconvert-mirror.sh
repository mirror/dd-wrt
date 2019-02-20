#!/usr/bin/env bash

# Copyright (C) 2010-2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA



. lib/inittest

aux prepare_pvs 5
get_devs

# proper DEVRANGE needs to be set according to extent size
DEVRANGE="0-32"
vgcreate $SHARED -s 32k "$vg" "${DEVICES[@]}"

# convert from linear to 2-way mirror ("mirror" default type)
lvcreate -aey -l2 -n $lv1 $vg "$dev1"
lvconvert -i1 -m+1 -R32k $vg/$lv1 "$dev2" "$dev3:0-1" \
	--config 'global { mirror_segtype_default = "mirror" }'
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*m'
lvremove -ff $vg

# convert from linear to 2-way mirror (override "raid1" default type)
lvcreate -aey -l2 -n $lv1 $vg "$dev1"
lvconvert -i1 --type mirror -m+1 $vg/$lv1 "$dev2" "$dev3:0-1" \
	--config 'global { mirror_segtype_default = "raid1" }'
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*m'
lvremove -ff $vg

# convert from linear to 2-way mirror - with tags and volume_list (bz683270)
lvcreate -aey -l2 -n $lv1 $vg --addtag hello
lvconvert -i1 --type mirror -m+1 $vg/$lv1 \
    --config 'activation { volume_list = [ "@hello" ] }'
lvremove -ff $vg

# convert from 2-way to 3-way mirror - with tags and volume_list (bz683270)
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg --addtag hello
lvconvert -i1 -m+1 $vg/$lv1 \
    --config 'activation { volume_list = [ "@hello" ] }'
lvremove -ff $vg

# convert from 2-way mirror to linear
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:0-1"
lvconvert -m-1 $vg/$lv1
check linear $vg $lv1
lvremove -ff $vg
# and now try removing a specific leg (bz453643)
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:0-1"
lvconvert -m0 $vg/$lv1 "$dev2"
check lv_on $vg $lv1 "$dev1"
lvremove -ff $vg

# convert from disklog to corelog, active
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:0-1"
lvconvert -f --mirrorlog core $vg/$lv1
check mirror $vg $lv1 core
lvremove -ff $vg

# convert from corelog to disklog, active
lvcreate -aey -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg "$dev1" "$dev2"
lvconvert --mirrorlog disk $vg/$lv1 "$dev3:0-1"
check mirror $vg $lv1 "$dev3"
lvremove -ff $vg

# convert linear to 2-way mirror with 1 PV
lvcreate -aey -l2 -n $lv1 $vg "$dev1"
not lvconvert -m+1 --mirrorlog core $vg/$lv1 "$dev1"
lvremove -ff $vg

# add 1 mirror to core log mirror, but
#  implicitly keep log as 'core'
lvcreate -aey -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg "$dev1" "$dev2"
lvconvert -m +1 -i1 $vg/$lv1

check mirror $vg $lv1 core
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 3
lvremove -ff $vg

# remove 1 mirror from corelog'ed mirror; should retain 'core' log type
lvcreate -aey -l2 --type mirror -m2 --corelog -n $lv1 $vg
lvconvert -m -1 -i1 $vg/$lv1

check mirror $vg $lv1 core
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 2
lvremove -ff $vg

# add 1 mirror then add 1 more mirror during conversion
# FIXME this has been explicitly forbidden?
#lvcreate -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3":0
#lvconvert -m+1 -b $vg/$lv1 "$dev4"
#lvconvert -m+1 $vg/$lv1 "$dev5"
#
#check mirror $vg $lv1 "$dev3"
#check mirror_no_temporaries $vg $lv1
#check mirror_legs $vg $lv1 4
#lvremove -ff $vg

# convert inactive mirror and start polling
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
lvchange -an $vg/$lv1
lvconvert -m+1 $vg/$lv1 "$dev4"
lvchange -aey $vg/$lv1
lvconvert $vg/$lv1 # wait
check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
lvremove -ff $vg

# ---------------------------------------------------------------------
# removal during conversion

# "remove newly added mirror"
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+1 -b $vg/$lv1 "$dev4"
lvconvert -m-1 $vg/$lv1 "$dev4"
lvconvert $vg/$lv1 # wait

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 2
lvremove -ff $vg

# "remove one of newly added mirrors"
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+2 -b $vg/$lv1 "$dev4" "$dev5"
lvconvert -m-1 $vg/$lv1 "$dev4"
lvconvert $vg/$lv1 # wait

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 3
lvremove -ff $vg

# "remove from original mirror (the original is still mirror)"
lvcreate -aey -l2 --type mirror -m2 -n $lv1 $vg "$dev1" "$dev2" "$dev5" "$dev3:$DEVRANGE"
LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+1 -b $vg/$lv1 "$dev4"
# FIXME: Extra wait here for mirror upconvert synchronization
# otherwise we may fail her on parallel upconvert and downconvert
# lvconvert-mirror-updown.sh tests this errornous case separately
lvconvert $vg/$lv1
lvconvert -m-1 $vg/$lv1 "$dev2"
lvconvert $vg/$lv1

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 3
lvremove -ff $vg

# "remove from original mirror (the original becomes linear)"
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+1 -b $vg/$lv1 "$dev4"
# FIXME: Extra wait here for mirror upconvert synchronization
# otherwise we may fail her on parallel upconvert and downconvert
# lvconvert-mirror-updown.sh tests this errornous case separately
lvconvert $vg/$lv1
lvconvert -m-1 $vg/$lv1 "$dev2"
lvconvert $vg/$lv1

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 2
lvremove -ff $vg

# Check the same with new --startpool lvconvert command option
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+1 -b $vg/$lv1 "$dev4"
# FIXME: Extra wait here for mirror upconvert synchronization
# otherwise we may fail her on parallel upconvert and downconvert
# lvconvert-mirror-updown.sh tests this errornous case separately
lvconvert $vg/$lv1
lvconvert -m-1 $vg/$lv1 "$dev2"
lvconvert $vg/$lv1

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 2
lvremove -ff $vg

# ---------------------------------------------------------------------

# "rhbz264241: lvm mirror doesn't lose it's "M" --nosync attribute
# after being down and the up converted"
lvcreate -aey -l2 --type mirror -m1 -n $lv1 --nosync $vg
lvconvert -m0 $vg/$lv1
lvconvert --type mirror -m1 $vg/$lv1
lvs --noheadings -o attr $vg/$lv1 | grep '^[[:space:]]*m'
lvremove -ff $vg

# lvconvert from linear (on multiple PVs) to mirror
lvcreate -aey -l 8 -n $lv1 $vg "$dev1:0-3" "$dev2:0-3"
lvconvert --type mirror -m1 $vg/$lv1

# FIXME: lvm should be able to make legs redundant
#should check mirror $vg $lv1
check mirror_legs $vg $lv1 2
lvremove -ff $vg

# BZ 463272: disk log mirror convert option is lost if downconvert option is also given
lvcreate -aey -l1 --type mirror -m2 --corelog -n $lv1 $vg "$dev1" "$dev2" "$dev3"
aux wait_for_sync $vg $lv1
lvconvert --type mirror -m1 --mirrorlog disk $vg/$lv1
check mirror $vg $lv1
not check mirror $vg $lv1 core
lvremove -ff $vg

# ---
# add mirror and disk log

# "add 1 mirror and disk log"
lvcreate -aey -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg "$dev1" "$dev2"

# FIXME on next line, specifying $dev3:0 $dev4 (i.e log device first) fails (!)
lvconvert -m+1 --mirrorlog disk -i1 $vg/$lv1 "$dev4" "$dev3:$DEVRANGE"

check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 3
lvremove -ff $vg

# simple mirrored stripe
lvcreate -aey -i2 -l10 -n $lv1 $vg
# FIXME: ATM reduce LV still must be bigger then region size!
#        LVM should do a better job here
lvconvert --type mirror -m1 -i1 --regionsize 16k $vg/$lv1
lvreduce -f -l1 $vg/$lv1
lvextend -f -l10 $vg/$lv1
lvremove -ff $vg/$lv1

# extents must be divisible
lvcreate -aey -l15 -n $lv1 $vg
not lvconvert --type mirror -m1 --corelog --stripes 2 $vg/$lv1
lvremove -ff $vg


if test -e LOCAL_CLVMD; then
: # FIXME - cases which needs to be fixed to work in cluster
else
# Should not be able to add images to --nosync mirror
# but should be able to after 'lvchange --resync'
lvcreate -aey --type mirror -m 1 -l1 -n $lv1 $vg --nosync
not lvconvert -m +1 $vg/$lv1
lvchange -aey --resync -y $vg/$lv1
lvconvert -m +1 $vg/$lv1
lvremove -ff $vg

lvcreate -aey --type mirror -m 1 --corelog -l1 -n $lv1 $vg --nosync
not lvconvert -m +1 $vg/$lv1
lvchange -aey --resync -y $vg/$lv1
lvconvert -m +1 $vg/$lv1
lvremove -ff $vg

# FIXME: Cluster exclusive activation does not work here
# unsure why  lib/metadata/mirror.c
# has this code:
#
#       } else if (vg_is_clustered(vg)) {
#                log_error("Unable to convert the log of an inactive "
#                          "cluster mirror, %s", lv->name);
#                return 0;
# disabling this in the code passes this test

# bz192865: lvconvert log of an inactive mirror lv
# convert from disklog to corelog, inactive
lvcreate -aey -l2 --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:0-1"
lvchange -an $vg/$lv1
lvconvert -y -f --mirrorlog core $vg/$lv1
check mirror $vg $lv1 core
lvremove -ff $vg

# convert from corelog to disklog, inactive
lvcreate -aey -l2 --type mirror -m1 --mirrorlog core -n $lv1 $vg "$dev1" "$dev2"
lvchange -an $vg/$lv1
lvconvert --mirrorlog disk $vg/$lv1 "$dev3:0-1"
check mirror $vg $lv1 "$dev3"
lvremove -ff $vg

# bz1272175: check lvconvert reports progress while waiting for mirror
# to get synced
lvcreate -l2 -n $lv1 $vg
lvconvert --type mirror -i1 -m1 $vg/$lv1 | tee out
grep -e "$vg/$lv1: Converted:" out || die "Missing sync info in foreground mode"
lvremove -ff $vg
fi


#########################################################################
# Start w/ 3-way mirror
# Test that the correct devices remain in the mirror
# Make $dev2 & $dev4  zero backend device so large mirrors can be user
# without consuming any real space. Clearly such mirrors can't be read back
# but tests here are validating possibilies of those conversions
#
# Test pulling primary image before mirror in-sync (should fail)
# Test pulling primary image after mirror in-sync (should work)
#
aux zero_dev "$dev2" $(get first_extent_sector "$dev2"):
aux zero_dev "$dev4" $(get first_extent_sector "$dev4"):

SHOULD=
aux throttle_dm_mirror || SHOULD=should

# Use large enough mirror that takes time to sychronize with small regionsize
lvcreate -aey -L20 -Zn -Wn --type mirror --regionsize 16k -m2 -n $lv1 $vg "$dev1" "$dev2" "$dev4" "$dev3:$DEVRANGE"
$SHOULD not lvconvert -m-1 $vg/$lv1 "$dev1" 2>&1 | tee out
aux restore_dm_mirror
grep "not in-sync" out

lvconvert $vg/$lv1 # wait

lvconvert -m-1 $vg/$lv1 "$dev1"
check mirror_images_on $vg $lv1 "$dev2" "$dev4"
lvconvert -m-1 $vg/$lv1 "$dev2"
check linear $vg $lv1
check lv_on $vg $lv1 "$dev4"
lvremove -ff $vg


aux throttle_dm_mirror || :
# No parallel lvconverts on a single LV please
# Use big enough mirror size and small regionsize to run on all test machines succesfully
lvcreate -aey -Zn -Wn -L20 --type mirror --regionsize 16k -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:0-8"
check mirror $vg $lv1
check mirror_legs $vg $lv1 2

LVM_TEST_TAG="kill_me_$PREFIX" lvconvert -m+1 -b $vg/$lv1 "$dev4"
# ATM upconversion should be running

# Next convert should fail b/c we can't have 2 at once
$SHOULD not lvconvert -m+1 $vg/$lv1 "$dev5"  2>&1 | tee out
aux restore_dm_mirror
grep "is already being converted" out

lvconvert $vg/$lv1 # wait
check mirror $vg $lv1 "$dev3"
check mirror_no_temporaries $vg $lv1
check mirror_legs $vg $lv1 3
lvremove -ff $vg


# "rhbz440405: lvconvert -m0 incorrectly fails if all PEs allocated"
lvcreate -aey -l "$(get pv_field "$dev1" pe_count)" --type mirror -m1 -n $lv1 $vg "$dev1" "$dev2" "$dev3:$DEVRANGE"
lvs -a -o+seg_pe_ranges $vg
aux wait_for_sync $vg $lv1
lvconvert -m0 $vg/$lv1 "$dev1"
check linear $vg $lv1
lvremove -ff $vg

vgremove -ff $vg
