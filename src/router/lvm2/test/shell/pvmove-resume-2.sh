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

# Moving 2 LVs in VG variant

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_CLVMD=1

. lib/inittest

aux prepare_pvs 2 30

vgcreate -s 128k $vg "$dev1"
pvcreate --metadatacopies 0 "$dev2"
vgextend $vg "$dev2"

test_pvmove_resume() {
	# 2 LVs on same device
	lvcreate -an -Zn -l15 -n $lv1 $vg "$dev1"
	lvcreate -an -Zn -l15 -n $lv2 $vg "$dev1"

	aux delay_dev "$dev2" 0 1000 "$(get first_extent_sector "$dev2"):"

	pvmove -i5 "$dev1" &
	PVMOVE=$!
	aux wait_pvmove_lv_ready "$vg-pvmove0" 300
	kill -9 $PVMOVE

	if test -e LOCAL_LVMPOLLD ; then
		aux prepare_lvmpolld
	fi

	wait

	local finished
	for i in {1..100}; do
		finished=1
		for d in  "$vg-$lv1" "$vg-$lv2" "$vg-pvmove0" ; do
			dmsetup status "$d" 2>/dev/null && {
				dmsetup remove "$d" || finished=0
			}
		done
		test "$finished" -eq 0 || break
	done
	test "$finished" -eq 0 && die "Can't remove device"

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
		for i in {1 100} ; do
			test $i -eq 100 && die "Shutdown of clvmd is too slow."
			test -e "$CLVMD_PIDFILE" || break
			sleep .1
		done # wait for the pid removal
		aux prepare_clvmd
	fi

	# call resume function (see below)
	# with expected number of spawned
	# bg polling as parameter
	$1 1

	aux enable_dev "$dev2"

	i=0
	while get lv_field $vg name -a | grep -E "^\[?pvmove"; do
		# wait for 30 secs at max
		test $i -ge 300 && die "Pvmove is too slow or does not progress."
		sleep .1
		i=$((i + 1))
	done

	aux kill_tagged_processes

	lvremove -ff $vg
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
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove0" || should false
	elif test -e HAVE_DM_DELAY; then
		test "$(aux count_processes_with_tag)" -eq "$1" || should false
	fi
}

vgchange_single() {
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange -aey $vg

	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 1 "$vg/pvmove0"
	elif test -e HAVE_DM_DELAY; then
		test "$(aux count_processes_with_tag)" -eq "$1"
	fi
}

pvmove_fg() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"

	aux enable_dev "$dev2"

	pvmove
}

pvmove_bg() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"

	LVM_TEST_TAG="kill_me_$PREFIX" pvmove -b
}

pvmove_fg_single() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"

	aux enable_dev "$dev2"

	pvmove "$dev1"
}

pvmove_bg_single() {
	# pvmove resume requires LVs active...
	LVM_TEST_TAG="kill_me_$PREFIX" vgchange --config 'activation{polling_interval=10}' -aey --poll n $vg

	# ...also vgchange --poll n must not spawn any bg processes...
	if test -e LOCAL_LVMPOLLD; then
		aux lvmpolld_dump | tee lvmpolld_dump.txt
		aux check_lvmpolld_init_rq_count 0 "$vg/pvmove0"
	else
		test "$(aux count_processes_with_tag)" -eq 0
	fi

	# ...thus finish polling
	get lv_field $vg name -a | grep -E "^\[?pvmove0"

	LVM_TEST_TAG="kill_me_$PREFIX" pvmove -b "$dev1"
}

test_pvmove_resume lvchange_single
test_pvmove_resume lvchange_all
test_pvmove_resume vgchange_single
test_pvmove_resume pvmove_fg
test_pvmove_resume pvmove_fg_single
test_pvmove_resume pvmove_bg
test_pvmove_resume pvmove_bg_single

vgremove -ff $vg
