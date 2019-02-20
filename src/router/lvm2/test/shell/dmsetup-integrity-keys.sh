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

INTERNAL_HASH_CRYPT="hmac(sha256)"
INTERNAL_HASH_NOCRYPT=crc32
JOURNAL_CRYPT="ctr(aes)"
HEXKEY_32=0102030405060708090a0102030405060102030405060708090a010203040506
HEXKEY2_32=0102030405060708090a010203040b060102030405060708090a010203040506
HIDENKEY_32=0000000000000000000000000000000000000000000000000000000000000000

aux target_at_least dm-integrity 1 0 0 || skip "missing dm-integrity target"
aux target_at_least dm-zero 1 0 0 || skip "missing dm-zero target"

function _teardown() {
	aux teardown_devs_prefixed "$PREFIX"
}

trap '_teardown' EXIT

dmsetup create "$PREFIX-zero" --table "0 10000 zero"
dmsetup create "$PREFIX-integrity" --table "0 7856 integrity $DM_DEV_DIR/mapper/$PREFIX-zero 0 32 J 7 journal_sectors:88 interleave_sectors:32768 buffer_sectors:128 journal_watermark:50 commit_time:10000 internal_hash:$INTERNAL_HASH_NOCRYPT journal_crypt:$JOURNAL_CRYPT:$HEXKEY_32"

str=$(dmsetup table "$PREFIX-integrity" | cut -d ' ' -f 15)
test "$str" = "journal_crypt:$JOURNAL_CRYPT:$HIDENKEY_32"
str=$(dmsetup table --showkeys "$PREFIX-integrity" | cut -d ' ' -f 15)
test "$str" = "journal_crypt:$JOURNAL_CRYPT:$HEXKEY_32"
str=$(dmsetup table "$PREFIX-integrity" | cut -d ' ' -f 14)
test "$str" = "internal_hash:$INTERNAL_HASH_NOCRYPT"

dmsetup remove "$PREFIX-integrity"
dmsetup create "$PREFIX-integrity" --table "0 7856 integrity $DM_DEV_DIR/mapper/$PREFIX-zero 0 32 J 7 journal_sectors:88 interleave_sectors:32768 buffer_sectors:128 journal_watermark:50 commit_time:10000 internal_hash:$INTERNAL_HASH_CRYPT:$HEXKEY2_32 journal_crypt:$JOURNAL_CRYPT:$HEXKEY_32"

str=$(dmsetup table "$PREFIX-integrity" | cut -d ' ' -f 15)
test "$str" = "journal_crypt:$JOURNAL_CRYPT:$HIDENKEY_32"
str=$(dmsetup table --showkeys "$PREFIX-integrity" | cut -d ' ' -f 15)
test "$str" = "journal_crypt:$JOURNAL_CRYPT:$HEXKEY_32"
str=$(dmsetup table "$PREFIX-integrity" | cut -d ' ' -f 14)
test "$str" = "internal_hash:$INTERNAL_HASH_CRYPT:$HIDENKEY_32"
str=$(dmsetup table --showkeys "$PREFIX-integrity" | cut -d ' ' -f 14)
test "$str" = "internal_hash:$INTERNAL_HASH_CRYPT:$HEXKEY2_32"

dmsetup remove "$PREFIX-integrity"
dmsetup remove "$PREFIX-zero"
