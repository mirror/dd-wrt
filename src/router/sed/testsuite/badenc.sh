#!/bin/sh

# Test runner for old 'badenc' test

# Copyright (C) 2017-2022 Free Software Foundation, Inc.

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

require_en_utf8_locale_

# The input (and also the expected output)
# containing an invalid multibyte sequences in utf-8 (octet \320 = 0xD0).
printf "abc\nde\320f\nghi\njkl\nmno\npqr\nstu\nvwx\nyz\n" > badenc-inp \
    || framework_failure_


# The progarm: using 'z' to clear the pattern-space even
# if it contains invalid multibyte sequences.
# Using 's/.*//' would not be able to clear the pattern-space.
cat << \EOF > badenc.sed || framework_failure_
/.*/ { H ; g ; s/\n// ; p ; z ; x }
EOF


env LC_ALL=en_US.UTF-8 sed -nf badenc.sed badenc-inp > badenc-out || fail=1
remove_cr_inplace badenc-out
compare badenc-inp badenc-out || fail=1


Exit $fail
