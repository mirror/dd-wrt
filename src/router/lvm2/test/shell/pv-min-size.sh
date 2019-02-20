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

# use small default size  - 512KB
aux lvmconf 'devices/pv_min_size = 512'

aux prepare_pvs 1 8

check pv_field "$dev1" pv_name "$dev1"

# increase min size beyond created PV size 10MB
aux lvmconf 'devices/pv_min_size = 10240'

# and test device is not visible
not check pv_field "$dev1" pv_name "$dev1"

# set too low value errornous value
aux lvmconf 'devices/pv_min_size = -100'

# check the incorrect value is printed
pvs "$dev1" 2>&1 | grep -- -100
