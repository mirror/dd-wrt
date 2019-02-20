#!/usr/bin/env bash

# Copyright (C) 2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Check whether all available pvmove resume methods works as expected.
# lvchange is able to resume pvmoves in progress.

# Multisegment variant w/ 2 pvmoves LVs per VG

SKIP_WITH_LVMLOCKD=1

. lib/inittest

aux prepare_pvs 5 30

vgcreate -s 128k $vg "$dev1" "$dev2" "$dev3"
pvcreate --metadatacopies 0 "$dev4" "$dev5"
vgextend $vg "$dev4" "$dev5"

# $1 resume fn
test_pvmove_resume() {
	# Create multisegment LV
	lvcreate -an -Zn -l30 -n $lv1 $vg "$dev1"
	lvextend -l+30 $vg/$lv1 "$dev2"
	# next LV on same VG and differetnt PV (we want to test 2 pvmoves per VG)
	lvcreate -an -Zn -l30 -n $lv2 $vg "$dev3"

	aux delay_dev "$dev4" 0 250 "$(get first_extent_sector "$dev4"):"
	test -e HAVE_DM_DELAY || { lvremove -f $vg; return 0; }
	aux delay_dev "$dev5" 0 250 "$(get first_extent_sector "$dev5"):"

	pvmove -i5 "$dev1" "$dev4" &
	PVMOVE=$!
	aux wait_pvmove_lv_ready "$vg-pvmove0" 300
	kill -9 $PVMOVE

	pvmove -i5 -n $vg/$lv2 "$dev3" "$dev5" &
	PVMOVE=$!
	aux wait_pvmove_lv_ready "$vg-pvmove1" 300
	kill -9 $PVMOVE

	if test -e LOCAL_LVMPOLLD ; then
		aux prepare_lvmpolld
	fi

	wait

	while dmsetup status "$vg-$lv1"; do dmsetup remove "$vg-$lv1" || true; done
	while dmsetup status "$vg-$lv2"; do dmsetup remove "$vg-$lv2" || true; done
	while dmsetup status "$vg-pvmove0"; do dmsetup remove "$vg-pvmove0" || true; done
	while dmsetup status "$vg-pvmove1"; do dmsetup remove "$vg-pvmove1" || true; done

	check lv_attr_bit type $vg/pvmove0 "p"
	check lv_attr_bit type $vg/pvmove1 "p"

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
			test -e "$CLVMD_PIDFILE" || break
			sleep .1
		done # wait for the pid removal
		aux prepare_clvmd
	fi

	# call resume function (see below)
	# with expected number of spawned
	# bg polling as parameter
	$1 2

	aux enable_dev "$dev4"
	aux enable_dev "$dev5"

	i=0
	while get lv_field $vg name -a | grep -E "^\[?pvmove"; do
		# wait for 30 secs at max
		test $i -ge 300 && die "Pvmove is too slow or does not progress."
		sleep .1
		i=$((i + 1))
	done

	aux kill_tagged_processes

	lvremove -ff $vg
	# drop debug logs from killed lvm2 commands
	rm -f debug.log_DEBUG*
}

lvchange_single() {
	LVM_TEST_TAG="kill_me_$PREFIX" lvchange -aey $vg/$lv1
	LVM_TEST_TAG="kill_me_$PREFIX" lvchange -aey $vg/$lv2
}

lvchange_all() {
	LVM_TEST_TAG="kill_me_$PREFIX" lvchange -aey $vg/$lv1 $vg/$lv2

	# we don't want to spawn more than $1 background pollings
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq $1
	fi
}

vgchange_single() {
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange -aey $vg

	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq "$1"
	fi
}

pvmove_fg() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes...
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"
	get lv_field $vg name -a | grep -E "^\[?pvmove1"

	# disable delay device
	# fg pvmove would take ages to complete otherwise
	aux enable_dev "$dev4"
	aux enable_dev "$dev5"

	pvmove
}

pvmove_bg() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes...
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"
	get lv_field $vg name -a | grep -E "^\[?pvmove1"

	LVM_TEST_TAG="kill_me_$PREFIX" pvmove -b
}

pvmove_fg_single() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes...
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"
	get lv_field $vg name -a | grep -E "^\[?pvmove1"

	# disable delay device
	# fg pvmove would take ages to complete otherwise
	aux enable_dev "$dev4"
	aux enable_dev "$dev5"

	pvmove "$dev1"
	pvmove "$dev3"
}

pvmove_bg_single() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes...
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove1"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"
	get lv_field $vg name -a | grep -E "^\[?pvmove1"

	LVM_TEST_TAG="kill_me_$PREFIX" pvmove -b "$dev1"
	LVM_TEST_TAG="kill_me_$PREFIX" pvmove -b "$dev3"
}

test_pvmove_resume lvchange_single
test_pvmove_resume lvchange_all
test_pvmove_resume vgchange_single
test_pvmove_resume pvmove_fg
test_pvmove_resume pvmove_fg_single
test_pvmove_resume pvmove_bg
test_pvmove_resume pvmove_bg_single

vgremove -ff $vg
