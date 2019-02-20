#!/usr/bin/env bash

# Copyright (C) 2008-2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

test_description='Remove the dlm test setup'

. lib/inittest

[ -z "$LVM_TEST_LOCK_TYPE_DLM" ] && skip;

# FIXME: collect debug logs (only if a test failed?)
# lvmlockctl -d > lvmlockd-debug.txt
# dlm_tool dump > dlm-debug.txt

lvmlockctl --stop-lockspaces
sleep 1
killall lvmlockd
sleep 1
killall lvmlockd || true
sleep 1
systemctl stop dlm
systemctl stop corosync
