#!/usr/bin/env bash

# Copyright (C) 2011-2015 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# set before test's clvmd is started, so it's passed in environ
export LVM_CLVMD_BINARY=clvmd
export LVM_BINARY=lvm

SKIP_WITH_LVMLOCKD=1
SKIP_WITHOUT_CLVMD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

# only clvmd based test, skip otherwise
read -r LOCAL_CLVMD < LOCAL_CLVMD

# TODO read from build, for now hardcoded
CLVMD_SOCKET="/var/run/lvm/clvmd.sock"

restart_clvmd_() {
	"$LVM_CLVMD_BINARY" -S
	ls -la "$CLVMD_SOCKET" || true

	for i in $(seq 1 20) ; do
		test -S "$CLVMD_SOCKET" && break
		sleep .1
	done
	# restarted clvmd has the same PID (no fork, only execvp)
	NEW_LOCAL_CLVMD=$(pgrep clvmd)
	test "$LOCAL_CLVMD" -eq "$NEW_LOCAL_CLVMD"
}

aux prepare_vg

lvcreate -an --zero n -n $lv1 -l1 $vg
lvcreate -an --zero n -n $lv2 -l1 $vg
lvcreate -l1 $vg

lvchange -aey $vg/$lv1
lvchange -aey $vg/$lv2

restart_clvmd_

# try restart once more
restart_clvmd_

# FIXME: Hmm - how could we test exclusivity is preserved in singlenode ?
lvchange -an $vg/$lv1
lvchange -aey $vg/$lv1
lvcreate -s -l3 -n snap $vg/$lv1

"$LVM_CLVMD_BINARY" -R

vgchange -an $vg

# Test what happens after 'reboot'
kill "$LOCAL_CLVMD"
while test -e "$CLVMD_PIDFILE"; do echo -n .; sleep .1; done # wait for the pid removal
aux prepare_clvmd

vgchange -ay $vg
lvremove -f $vg/snap

vgremove -ff $vg
