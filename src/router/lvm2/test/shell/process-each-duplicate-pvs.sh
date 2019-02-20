#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.

test_description='Test duplicate PVs'

SKIP_WITH_LVMPOLLD=1
SKIP_WITH_CLVMD=1

. lib/inittest

aux prepare_devs 6 16

# The LV-using-PV tests (DEV_USED_FOR_LV, where a PV is
# preferred if an active LV is using it) depend on sysfs
# info that is not available in RHEL5 kernels.
aux driver_at_least 4 15 || skip

aux lvmconf 'devices/allow_changes_with_duplicate_pvs = 0'

pvcreate "$dev1"
pvcreate "$dev2"
vgcreate $SHARED $vg1 "$dev1"
vgcreate $SHARED $vg2 "$dev2"
pvresize --setphysicalvolumesize 8m -y "$dev2"
lvcreate -an -l1 -n $lv1 $vg1

# Both devs are shown and used by the VG

pvs 2>&1 | tee out

grep "$dev1" out
grep "$dev2" out
grep "$dev1" out | grep $vg1
grep "$dev2" out | grep $vg2
check pv_field "$dev1" pv_allocatable "allocatable"
check pv_field "$dev2" pv_allocatable "allocatable"
not grep WARNING out

UUID1=$(get pv_field "$dev1" uuid)
UUID2=$(get pv_field "$dev2" uuid)

SIZE1=$(get pv_field "$dev1" dev_size)
SIZE2=$(get pv_field "$dev2" dev_size)

MINOR1=$(get pv_field "$dev1" minor)
MINOR2=$(get pv_field "$dev2" minor)

check pv_field "$dev1" dev_size "$SIZE1"
check pv_field "$dev2" dev_size "$SIZE2"

# Copy dev1 over dev2.
dd if="$dev1" of="$dev2" bs=1M iflag=direct oflag=direct,sync
pvscan --cache

# The single preferred dev is shown from 'pvs'.
pvs -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

# Don't know yet if dev1 or dev2 is preferred, so count just one is.
test "$(grep -c "$vg1" main)" -eq 1
test "$(grep -c "$UUID1" main)" -eq 1
not grep duplicate main
not grep $vg2 main
not grep $UUID2 main

grep "Not using device" warn
grep "prefers device" warn

# Find which is the preferred dev and which is the duplicate.
PV=$(pvs --noheadings -o name -S uuid="$UUID1" | xargs)
if [ "$PV" = "$dev1" ]; then
	DUP=$dev2
else
	DUP=$dev1
fi

echo "PV is $PV"
echo "DUP is $DUP"

grep "$PV" main
not grep "$DUP" main

# Repeat above checking preferred/dup in output
pvs 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$PV" main
not grep "$DUP" main

# The duplicate dev is included in 'pvs -a'
pvs -a -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev1" main
grep "$dev2" main
grep $PV main
grep $DUP main
test "$(grep -c duplicate main)" -eq 1
grep $DUP main | grep duplicate
not grep $vg2 main
not grep $UUID2 main
grep "$dev1" main | grep $vg1
grep "$dev2" main | grep $vg1
grep "$dev1" main | grep $UUID1
grep "$dev2" main | grep $UUID1

grep "Not using device" warn
grep "prefers device" warn

#
# Passing a dev name arg always includes that dev.
#

pvs -o+uuid "$dev1" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev1" main
not grep "$dev2" main
grep "$UUID1" main
grep "$vg1" main
grep "Not using device" warn
grep "prefers device" warn

pvs -o+uuid "$dev2" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev2" main
not grep "$dev1" main
grep "$UUID1" main
grep "$vg1" main
grep "Not using device" warn
grep "prefers device" warn

pvs -o+uuid,duplicate "$dev1" "$dev2" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev1" main
grep "$dev2" main
grep "$dev1" main | grep $vg1
grep "$dev2" main | grep $vg1
grep "$dev1" main | grep $UUID1
grep "$dev2" main | grep $UUID1
test "$(grep -c duplicate main)" -eq 1
grep $DUP main | grep duplicate

#
# Test specific report fields for each dev.
#

pvs --noheadings -o vg_name,vg_uuid "$dev1" 2>&1 | tee out1
pvs --noheadings -o vg_name,vg_uuid "$dev2" 2>&1 | tee out2

grep -v WARNING out1 > main1 || true
grep -v WARNING out2 > main2 || true
diff main1 main2
rm out1 out2 main1 main2 || true

check pv_field "$dev1" pv_in_use "used"
check pv_field "$dev2" pv_in_use "used"

check pv_field "$PV"  pv_allocatable "allocatable"
check pv_field "$DUP" pv_allocatable ""

check pv_field "$PV"  pv_duplicate ""
check pv_field "$DUP" pv_duplicate "duplicate"

pvs --noheadings -o name,pv_allocatable "$dev1" "$dev2" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$PV" main
grep "$DUP" main
grep "$dev1" main
grep "$dev2" main
test "$(grep -c allocatable main)" -eq 1

pvs --noheadings -o name,pv_duplicate "$dev1" "$dev2" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$PV" main
grep "$DUP" main
grep "$dev1" main
grep "$dev2" main
test "$(grep -c duplicate main)" -eq 1

#
# A filter can be used to show only one.
#

pvs --config "devices { filter=[ \"a|$dev2|\", \"r|.*|\" ] }" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

not grep "$dev1" main
grep "$dev2" main

not grep "Not using device" warn
not grep "prefers device" warn


pvs --config "devices { filter=[ \"a|$dev1|\", \"r|.*|\"] }" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev1" main
not grep "$dev2" main

not grep "Not using device" warn
not grep "prefers device" warn

# PV size and minor is still reported correctly for each.

check pv_field "$dev1" dev_size "$SIZE1"
check pv_field "$dev2" dev_size "$SIZE2"

check pv_field "$dev1" minor "$MINOR1"
check pv_field "$dev2" minor "$MINOR2"

# With allow_changes_with_duplicate_pvs=0, a VG with duplicate devs
# cannot be modified or activated.

not lvcreate -an -l1 -n $lv2 $vg1
not lvremove $vg1/$lv1
not lvchange -ay $vg1/$lv1
not vgremove $vg1


# With allow_changes_with_duplicate_pvs=1, changes above are permitted.

aux lvmconf 'devices/allow_changes_with_duplicate_pvs = 1'

lvcreate -an -l1 -n $lv2 $vg1
lvremove $vg1/$lv1
lvchange -ay $vg1/$lv2
lvchange -an $vg1/$lv2
lvremove $vg1/$lv2
vgremove -f $vg1
pvremove -ff -y "$dev1"
pvremove -ff -y "$dev2"


# dev3 and dev4 are copies, orphans

pvcreate "$dev3"
pvcreate "$dev4"
pvresize --setphysicalvolumesize 8m -y "$dev4"

UUID3=$(get pv_field "$dev3" uuid)
UUID4=$(get pv_field "$dev4" uuid)

SIZE3=$(get pv_field "$dev3" dev_size)
SIZE4=$(get pv_field "$dev4" dev_size)

check pv_field "$dev3" dev_size "$SIZE3"
check pv_field "$dev4" dev_size "$SIZE4"

pvs 2>&1 | tee out

grep "$dev3" out
grep "$dev4" out

dd if="$dev3" of="$dev4" bs=1M iflag=direct oflag=direct,sync
pvscan --cache

# One appears with 'pvs'

pvs -o+uuid 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c "$UUID3" main)" -eq 1
not grep "$UUID4" main

grep "Not using device" warn
grep "prefers device" warn

# Both appear with 'pvs -a'

pvs -a -o+uuid 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c "$UUID3" main)" -eq 2

grep "$dev3" main
grep "$dev4" main

grep $UUID3 main
not grep $UUID4 main

grep "Not using device" warn
grep "prefers device" warn

# Show each dev individually and both together

pvs -o+uuid "$dev3" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev3" main
not grep "$dev4" main

grep "Not using device" warn
grep "prefers device" warn

pvs -o+uuid "$dev4" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

not grep "$dev3" main
grep "$dev4" main

grep "Not using device" warn
grep "prefers device" warn

pvs -o+uuid "$dev3" "$dev4" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev3" main
grep "$dev4" main

grep "Not using device" warn
grep "prefers device" warn

# Same sizes shown.

check pv_field "$dev3" dev_size "$SIZE3"
check pv_field "$dev4" dev_size "$SIZE4"

# Verify that devs being used by an active LV are
# preferred over duplicates that are not used by an LV.

dd if=/dev/zero of="$dev3" bs=1M oflag=direct,sync || true
dd if=/dev/zero of="$dev4" bs=1M oflag=direct,sync || true
pvscan --cache

# The previous steps prevent us from nicely cleaning up
# the vg lockspace in lvmlockd, so just restart it;
# what follows could also just be split into a separate test.
if test -n "$LVM_TEST_LVMLOCKD_TEST" ; then
	killall -9 lvmlockd
	sleep 2
	aux prepare_lvmlockd
fi

vgcreate $SHARED "$vg2" "$dev3" "$dev4"
lvcreate -l1 -n $lv1 $vg2 "$dev3"
lvcreate -l1 -n $lv2 $vg2 "$dev4"

dd if="$dev3" of="$dev5" bs=1M iflag=direct oflag=direct,sync
dd if="$dev4" of="$dev6" bs=1M iflag=direct oflag=direct,sync
pvscan --cache

pvs -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev3" main
grep "$dev4" main
not grep duplicate main
check pv_field "$dev3" pv_duplicate ""
check pv_field "$dev4" pv_duplicate ""
check pv_field "$dev5" pv_duplicate "duplicate"
check pv_field "$dev6" pv_duplicate "duplicate"

grep "prefers device $dev3" warn
grep "prefers device $dev4" warn
not grep "prefers device $dev5" warn
not grep "prefers device $dev6" warn

pvs -a -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c duplicate main)" -eq 2
grep "$dev3" main
grep "$dev4" main
grep "$dev5" main
grep "$dev6" main

grep "prefers device $dev3" warn
grep "prefers device $dev4" warn
not grep "prefers device $dev5" warn
not grep "prefers device $dev6" warn

pvs -o+uuid,duplicate "$dev3" "$dev4" "$dev5" "$dev6" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c duplicate main)" -eq 2
grep "$dev3" main
grep "$dev4" main
grep "$dev5" main
grep "$dev6" main

grep "prefers device $dev3" warn
grep "prefers device $dev4" warn
not grep "prefers device $dev5" warn
not grep "prefers device $dev6" warn


dd if=/dev/zero of="$dev5" bs=1M oflag=direct,sync || true
dd if=/dev/zero of="$dev6" bs=1M oflag=direct,sync || true
pvscan --cache

lvremove -y $vg2/$lv1
lvremove -y $vg2/$lv2
vgremove $vg2
pvremove -ff -y "$dev3"
pvremove -ff -y "$dev4"

dd if=/dev/zero of="$dev3" bs=1M oflag=direct,sync || true
dd if=/dev/zero of="$dev4" bs=1M oflag=direct,sync || true
pvscan --cache

# Reverse devs in the previous in case dev3/dev4 would be
# preferred even without an active LV using them.

vgcreate $SHARED $vg2 "$dev5" "$dev6"
lvcreate -l1 -n $lv1 $vg2 "$dev5"
lvcreate -l1 -n $lv2 $vg2 "$dev6"

dd if="$dev5" of="$dev3" bs=1M iflag=direct oflag=direct,sync
dd if="$dev6" of="$dev4" bs=1M iflag=direct oflag=direct,sync
pvscan --cache

pvs -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

grep "$dev5" main
grep "$dev6" main
not grep duplicate main
check pv_field "$dev5" pv_duplicate ""
check pv_field "$dev6" pv_duplicate ""
check pv_field "$dev3" pv_duplicate "duplicate"
check pv_field "$dev4" pv_duplicate "duplicate"

pvs -a -o+uuid,duplicate 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

test "$(grep -c duplicate main)" -eq 2
grep "$dev3" main
grep "$dev4" main
grep "$dev5" main
grep "$dev6" main

grep "prefers device $dev5" warn
grep "prefers device $dev6" warn
not grep "prefers device $dev3" warn
not grep "prefers device $dev4" warn

dd if=/dev/zero of="$dev3" bs=1M oflag=direct,sync || true
dd if=/dev/zero of="$dev4" bs=1M oflag=direct,sync || true
pvscan --cache

lvremove -y $vg2/$lv1
lvremove -y $vg2/$lv2
vgremove $vg2
