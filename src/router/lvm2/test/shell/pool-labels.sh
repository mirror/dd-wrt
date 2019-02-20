#!/usr/bin/env bash

# Copyright (C) 2007 Red Hat, Inc. All rights reserved.
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

env printf "" || skip # skip if printf is not available

# create the old GFS pool labeled linear devices
create_pool_label_()
{
  # FIXME
  # echo -e is bashism, dash builtin sh doesn't do \xNN in printf either
  # printf comes from coreutils, and is probably not posix either
  env printf "\x01\x16\x70\x06\x5f\xcf\xff\xb9\xf8\x24\x8apool1" | dd of="$2" bs=5 seek=1 conv=notrunc
  env printf "\x04\x01\x03\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x0$1\x68\x01\x16\x70\x00\x00\x00\x00\x00\x06\x5f\xd0" | dd of=$2 bs=273 seek=1 conv=notrunc
}


aux prepare_devs 2

create_pool_label_ 0 "$dev1"
create_pool_label_ 1 "$dev2"

# verify that lvm will ignore and not use a gfs-pool device

not pvs "$dev1"

# check that pvcreate fails without -ff on the pool device
not pvcreate "$dev1"

