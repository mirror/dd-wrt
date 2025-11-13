#!/bin/sh
# Test multibyte locale which is not UTF-8 (ja_JP.shift_jis)
# This is a stateful locale. Same byte value can be either
# a single-byte character, or the second byte of a multibyte
# character.

# Copyright (C) 2016-2022 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
. "${srcdir=.}/testsuite/init.sh"; path_prepend_ ./sed
print_ver_ sed

# If found, LOCALE_JA_SJIS will contain the locale name.
require_ja_shiftjis_locale_

# Ensure the implementation is not buggy (skip otherwise)
require_valid_ja_shiftjis_locale_ "$LOCALE_JA_SJIS"

# This test uses two characters:
# Unicode Character 'KATAKANA LETTER ZE' (U+30BC)
# Unicode Character 'KATAKANA LETTER ZO' (U+30BE)
#
# In SHIFT-JIS locale, these multibyte characters contain
# open/close brackets (ASCII 0x5B/0x5D) as the trailing byte.
#
# See also:
# https://en.wikipedia.org/wiki/Shift_JIS
# http://www.rikai.com/library/kanjitables/kanji_codes.sjis.shtml

# Unicode Character 'KATAKANA LETTER ZE' (U+30BC)
#
# UTF-8:    hex: 0xE3     0x82      0xBC
#           bin: 11100011 10000010  10111100
#
# Shift-jis hex:  0x83     0x5B
#           oct:  203      133
#           bin:  10000011 01011011
#
# Conversion example:
#   $ printf '\x83\x5B' | iconv -f SHIFT-JIS -t UTF-8 | od -tx1o1c
#   0000000  e3  82  bc
#            343 202 274
#            343 202 274

# Unicode Character 'KATAKANA LETTER ZO' (U+30BE)
#
# UTF-8:    hex: 0xE3     0x82      0xBE
#           bin: 11100011 10000010  10111110
#
# Shift-jis hex:  0x83     0x5D
#           oct:  203      135
#           bin:  10000011 01011101
#
# Conversion example:
#   $ printf '\x83\x5D' | iconv -f SHIFT-JIS -t UTF-8 | od -tx1o1c
#   0000000  e3  82  be
#            343 202 276
#            343 202 276
#


#
# Tests 1,2: Test y/// command with multibyte, non-utf8 seqeunce.
# Implmenetation notes: str_append() has special code path for non-utf8 cases.
#

# Test 1: valid multibyte seqeunce
printf 'y/a/\203\133/' > p1 || framework_failure_
echo Xa > in1 || framework_failure_
printf 'X\203\133\n' > exp1 || framework_failure_

LC_ALL="$LOCALE_JA_SJIS" sed -f p1 <in1 >out1 || fail=1
compare_ exp1 out1 || fail=1

# Test 2: invalid multibyte seqeunce, treated as two single-byte characters.
printf 'y/aa/\203\060/' > p2 || framework_failure_
LC_ALL="$LOCALE_JA_SJIS" sed -f p2 </dev/null 2>out2 || fail=1
compare_ /dev/null out2 || fail=1

#
# Test 3: multibyte character class with these characters.
#
# Before sed-4.3, snarf_char_class would parse it incorrectly,
# Treating the first closing-bracket as closing the character-class,
# instead of being part of a multibyte sequence.

printf '/[\203]/]/p' > p3 || framework_failure_
LC_ALL="$LOCALE_JA_SJIS" sed -f p3 </dev/null >out3 || fail=1
compare_ /dev/null out3 || fail=1

# Test 4:
# Same as test 3, but with the other multibyte character.
# (this did not cause a failure before sed-4.3, but the code was incorrect).
# Keep this test for code-coverage purposes.
printf '/[\203[/]/p' > p4 || framework_failure_
LC_ALL="$LOCALE_JA_SJIS" sed -f p4 </dev/null >out4 || fail=1
compare_ /dev/null out4 || fail=1

# TODO: Find a locale in which ':.=' can be part of a valid multibyte octet.
#
# snarf_char_class specifically tests for five bytes: ':.=[]' .
# '[' and ']' are tested above, yet '.:=' are not valid as part of a
# multibyte shift-jis sequence.
#
# valid:
#   $ printf '\203]' | iconv -f SHIFT-JIS -t utf-8
#   $ printf '\203[' | iconv -f SHIFT-JIS -t utf-8
#
# invalid:
#   $ printf '\203:' | iconv -f SHIFT-JIS -t utf-8
#   iconv: (stdin):1:0: cannot convert
#
#   $ printf '\203=' | iconv -f SHIFT-JIS -t utf-8
#   iconv: (stdin):1:0: cannot convert
#
#   $ printf '\203.' | iconv -f SHIFT-JIS -t utf-8
#   iconv: (stdin):0:0: cannot convert

Exit $fail
