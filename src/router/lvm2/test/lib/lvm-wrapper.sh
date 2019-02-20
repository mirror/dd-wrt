#!/bin/sh
# Copyright (C) 2011-2017 Red Hat, Inc.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

. lib/paths

CMD=${0##*/}
test "$CMD" != lvm || unset CMD

# When needed to trace command from test suite use env var before program
# and run program directly via shell in test dir i.e.:
# sh shell/activate-mirror.sh
# 'LVM_GDB=1 lvcreate -l1 $vg'
# > run
test -z "$LVM_GDB" || exec gdb --readnow --args "$abs_top_builddir/tools/lvm" $CMD "$@"

# Multiple level of LVM_VALGRIND support
# the higher level the more commands are traced
if test -n "$LVM_VALGRIND"; then
	RUN_DBG="${VALGRIND:-valgrind}";
fi

if test -n "$LVM_STRACE"; then
	RUN_DBG="strace $LVM_STRACE -o strace.log"
fi

case "$CMD" in
  lvs|pvs|vgs|vgck|vgscan)
	test "${LVM_DEBUG_LEVEL:-0}" -lt 2 && RUN_DBG="" ;;
  pvcreate|pvremove|lvremove|vgcreate|vgremove)
	test "${LVM_DEBUG_LEVEL:-0}" -lt 1 && RUN_DBG="" ;;
esac

# Capture parallel users of debug.log file
#test -z "$(fuser debug.log 2>/dev/null)" || {
#	echo "TEST WARNING: \"debug.log\" is still in use while running $CMD $@" >&2
#	fuser -v debug.log >&2
#}

# the exec is important, because otherwise fatal signals inside "not" go unnoticed
if test -n "$abs_top_builddir"; then
    exec $RUN_DBG "$abs_top_builddir/tools/lvm" $CMD "$@"
else # we are testing the lvm on $PATH
    PATH=$(echo "$PATH" | sed -e 's,[^:]*lvm2-testsuite[^:]*:,,g')
    exec $RUN_DBG lvm $CMD "$@"
fi
