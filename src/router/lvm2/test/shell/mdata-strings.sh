#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# 'Test for proper escaping of strings in metadata (bz431474)'

SKIP_WITH_LVMPOLLD=1

. lib/inittest

# For udev impossible to create
test "$LVM_TEST_DEVDIR" = "/dev" && skip

aux prepare_devs 2
aux extend_filter_LVMTEST

# Setup mangling to 'none' globaly for all libdm users
export DM_DEFAULT_NAME_MANGLING_MODE=none

pv_ugly="__\"!@#\$%^&*,()|@||'\\\"__pv1"

# 'set up temp files, loopback devices'
name=$(basename "$dev1")
dmsetup rename "$name" "$PREFIX$pv_ugly"
dev1=$(dirname "$dev1")/"$PREFIX$pv_ugly"

dm_table | grep -F "$pv_ugly"

# 'pvcreate, vgcreate on filename with backslashed chars'
created="$dev1"
# when used with real udev without fallback, it will fail here
pvcreate "$dev1" || created="$dev2"
pvdisplay 2>&1 | tee err
should grep -F "$pv_ugly" err
should check pv_field "$dev1" pv_name "$dev1"
vgcreate $SHARED $vg "$created"
# 'no parse errors and VG really exists'
vgs $vg 2>err
not grep "Parse error" err

dmsetup remove "${PREFIX}${pv_ugly}"
