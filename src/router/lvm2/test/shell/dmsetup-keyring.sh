#!/usr/bin/env bash

# Copyright (C) 2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# unrelated to lvm2 daemons
SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

CIPHER=aes-xts-plain64
HEXKEY_32=0102030405060708090a0102030405060102030405060708090a010203040506
HIDENKEY_32=0000000000000000000000000000000000000000000000000000000000000000
KEY_NAME="$PREFIX:keydesc"

function _teardown() {
	keyctl unlink "%:$PREFIX-keyring"
	aux teardown_devs_prefixed "$PREFIX"
}

aux target_at_least dm-zero 1 0 0 || skip "missing dm-zero target"
aux target_at_least dm-crypt 1 15 0 || skip "dm-crypt doesn't support keys in kernel keyring service"
which keyctl || skip "test requires keyctl utility"

keyctl new_session || true   # fails with 'su', works with 'su -'
keyctl newring "$PREFIX-keyring" @s
keyctl timeout "%:$PREFIX-keyring" 60

trap '_teardown' EXIT

keyctl add logon "$KEY_NAME" "${HEXKEY_32:0:32}" "%:$PREFIX-keyring"

dmsetup create "$PREFIX-zero" --table "0 1 zero"
# put key in kernel keyring for active table
dmsetup create "$PREFIX-crypt" --table "0 1 crypt $CIPHER :32:logon:$KEY_NAME 0 $DM_DEV_DIR/mapper/$PREFIX-zero 0"
# put hexbyte key in dm-crypt directly in inactive table
dmsetup load "$PREFIX-crypt" --table "0 1 crypt $CIPHER $HEXKEY_32 0 $DM_DEV_DIR/mapper/$PREFIX-zero 0"

# test dmsetup doesn't hide key descriptions...
str=$(dmsetup table "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = ":32:logon:$KEY_NAME"
str=$(dmsetup table --showkeys "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = ":32:logon:$KEY_NAME"

# ...but it hides hexbyte representation of keys...
str=$(dmsetup table --inactive "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = "$HIDENKEY_32"
#...unless --showkeys explictly requested
str=$(dmsetup table --showkeys --inactive "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = "$HEXKEY_32"

# let's swap the tables
dmsetup resume "$PREFIX-crypt"
dmsetup load "$PREFIX-crypt" --table "0 1 crypt $CIPHER :32:logon:$KEY_NAME 0 $DM_DEV_DIR/mapper/$PREFIX-zero 0"

str=$(dmsetup table --inactive "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = ":32:logon:$KEY_NAME"
str=$(dmsetup table --showkeys --inactive "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = ":32:logon:$KEY_NAME"

str=$(dmsetup table "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = "$HIDENKEY_32"
str=$(dmsetup table --showkeys "$PREFIX-crypt" | cut -d ' ' -f 5)
test "$str" = "$HEXKEY_32"

dmsetup remove "$PREFIX-crypt"
dmsetup remove "$PREFIX-zero"
