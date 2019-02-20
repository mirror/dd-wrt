#!/usr/bin/env bash

# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

aux prepare_vg 3

lvcreate -an -Zn --type mirror -m 1 -l 1 -n mirror $vg
lvcreate -l 1 -n lv1 $vg "$dev1"

# vgextend require vgname
invalid vgextend
# --metadatacopies => use --pvmetadatacopies
invalid vgextend --metadatacopies 3 $vg "$dev1" 2>&1 | tee out
#grep -- "use --pvmetadatacopies" out
grep -E -- "unrecognized option.*--metadatacopies" out

# VG name should exist
fail vgextend --restoremissing $vg-invalid "$dev1"

# try to just change metadata; we expect the new version (with MISSING_PV set
# on the reappeared volume) to be written out to the previously missing PV
aux disable_dev "$dev1"
lvremove $vg/mirror
# try restore the still existing device
fail vgextend --restore $vg "$dev1"
aux enable_dev "$dev1"
not vgck $vg 2>&1 | tee log
grep "missing 1 physical volume" log
not lvcreate -aey --type mirror -m 1 -l 1 -n mirror $vg # write operations fail
# try restore the non-missing device
fail vgextend --restore $vg "$dev2"
# try restore the non-existing device
fail vgextend --restore $vg "$dev2-invalid"
# restore the missing device
vgextend --restore $vg "$dev1"

vgreduce  $vg "$dev3"
vgchange --metadatacopies 1 $vg
# 'n' failing to change volume group
fail vgextend --metadataignore y --pvmetadatacopies 2 $vg "$dev3"
vgextend --yes --metadataignore y --pvmetadatacopies 2 $vg "$dev3"
vgck $vg
lvcreate -an -Zn --type mirror -m 1 -l 1 -n mirror $vg

vgremove -ff $vg
