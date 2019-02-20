#!/usr/bin/env bash

# Copyright (C) 2008-2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.

test_description='Test vgs with duplicate vg names'
SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2

pvcreate "$dev1"
pvcreate "$dev2"

aux disable_dev "$dev1" "$dev2"

aux enable_dev "$dev1"
vgscan
vgcreate $vg1 "$dev1"
UUID1=$(vgs --noheading -o vg_uuid $vg1)
aux disable_dev "$dev1"

aux enable_dev "$dev2"
vgscan
vgcreate $vg1 "$dev2"
UUID2=$(vgs --noheading -o vg_uuid $vg1)

aux enable_dev "$dev1"
# need vgscan after enabling/disabling devs
# so that the next commands properly see them
vgscan
pvs "$dev1"
pvs "$dev2"

vgs -o+vg_uuid | tee err
grep $UUID1 err
grep $UUID2 err

# should we specify and test which should be displayed?
# vgs --noheading -o vg_uuid $vg1 >err
# grep $UUID1 err

aux disable_dev "$dev2"
vgs -o+vg_uuid | tee err
grep $UUID1 err
not grep $UUID2 err
aux enable_dev "$dev2"
vgscan

aux disable_dev "$dev1"
vgs -o+vg_uuid | tee err
grep $UUID2 err
not grep $UUID1 err
aux enable_dev "$dev1"
vgscan
