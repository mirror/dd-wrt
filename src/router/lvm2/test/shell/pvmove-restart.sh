#!/usr/bin/env bash

# Copyright (C) 2013-2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Check pvmove behavior when it's progress and machine is rebooted

. lib/inittest

aux prepare_pvs 3 60

vgcreate $SHARED -s 128k $vg "$dev1" "$dev2"
pvcreate --metadatacopies 0 "$dev3"
vgextend $vg "$dev3"

# Slowdown writes
# (FIXME: generates interesting race when not used)
aux delay_dev "$dev3" 0 800 "$(get first_extent_sector "$dev3"):"
test -e HAVE_DM_DELAY || skip

for mode in "--atomic" ""
do

# Create multisegment LV
lvcreate -an -Zn -l5 -n $lv1 $vg "$dev1"
lvextend -l+10 $vg/$lv1 "$dev2"
lvextend -l+5 $vg/$lv1 "$dev1"
lvextend -l+10 $vg/$lv1 "$dev2"

pvmove -i10 -n $vg/$lv1 "$dev1" "$dev3" $mode &
PVMOVE=$!
# Let's wait a bit till pvmove starts and kill it
aux wait_pvmove_lv_ready "$vg-pvmove0"
kill -9 $PVMOVE

if test -e LOCAL_LVMPOLLD; then
	aux prepare_lvmpolld
fi

wait

# Simulate reboot - forcibly remove related devices

# First take down $lv1 then it's pvmove0
j=0
for i in $lv1 pvmove0 pvmove0_mimage_0 pvmove0_mimage_1 ; do
	while dmsetup status "$vg-$i"; do
		dmsetup remove "$vg-$i" || {
			j=$(( j + 1 ))
			test $j -le 100 || die "Cannot take down devices."
			sleep .1;
		}
	done
done
dmsetup table | grep $PREFIX

# Check we really have pvmove volume
check lv_attr_bit type $vg/pvmove0 "p"

if test -e LOCAL_CLVMD ; then
	# giveup all clvmd locks (faster then restarting clvmd)
	# no deactivation happen, nodes are already removed
	#vgchange -an $vg
	# FIXME: However above solution has one big problem
	# as clvmd starts to abort on internal errors on various
	# errors, based on the fact pvmove is killed -9
	# Restart clvmd
	kill "$(< LOCAL_CLVMD)"
	for i in $(seq 1 100) ; do
		test $i -eq 100 && die "Shutdown of clvmd is too slow."
		pgrep clvmd || break
		sleep .1
	done # wait for the pid removal
	aux prepare_clvmd
fi

# Only PVs should be left in table...
dmsetup table

# Restart pvmove
# use exclusive activation to have usable pvmove without cmirrord
LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey $vg
aux wait_pvmove_lv_ready "$vg-pvmove0"
dmsetup table

pvmove --abort "$dev1"

lvs -a -o+devices $vg

lvremove -ff $vg
aux kill_tagged_processes
done

# Restore delayed device back
aux delay_dev "$dev3"

vgremove -ff $vg
