#!/usr/bin/env bash

# Copyright (C) 2008 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#
# tests lvm initialization, and especially negative tests of error paths
#


SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 5

# invalid units
not pvs --config 'global { units = "<" }'
