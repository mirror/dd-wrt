#!/usr/bin/env bash

# Copyright (C) 2011 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


SKIP_WITH_LVMPOLLD=1

. lib/inittest

flatten() {
	cat > flatten.config
	for s in $(grep -E '^[a-z]+ {$' flatten.config | sed -e 's,{$,,'); do
		sed -e "/^$s/,/^}/p;d" flatten.config | sed -e '1d;$d' | sed -e "s,^[ \t]*,$s/,";
	done
}

# clvmd might not be started fast enough and
# lvm still activates locking for all commands.
# FIXME: Either make longer start delay,
#  or even better do not initialize
#  locking for commands like 'dumpconfig'
#aux lvmconf "global/locking_type=0"

lvm dumpconfig -f lvmdumpconfig
flatten < lvmdumpconfig | sort > config.dump
flatten < etc/lvm.conf | sort > config.input
# check that dumpconfig output corresponds to the lvm.conf input
diff -wu config.input config.dump

# and that merging multiple config files (through tags) works
lvm dumpconfig -f lvmdumpconfig
flatten < lvmdumpconfig | not grep 'log/verbose=1'
lvm dumpconfig -f lvmdumpconfig
flatten < lvmdumpconfig | grep 'log/indent=1'

aux lvmconf 'tags/@foo {}'
echo 'log { verbose = 1 }' > etc/lvm_foo.conf
lvm dumpconfig -f lvmdumpconfig
flatten < lvmdumpconfig | grep 'log/verbose=1'
lvm dumpconfig -f lvmdumpconfig
flatten < lvmdumpconfig | grep 'log/indent=1'
