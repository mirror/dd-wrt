#!/bin/sh

# Test GNU extension "\u" and "\U" (uppercase conversion)
# in "s///" command.
# This is an adaptation of the old utf8-1/2/3/4 tests.

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

require_ru_utf8_locale_

# The letter used in these tests are:
#      UTF8:Octal  UTF8:HEX   CodePoint Name
#  А   \320\220    \xD0\x90   U+0410   \N{CYRILLIC CAPITAL LETTER A}
#  Д   \320\224    \xD0\x94   U+0414   \N{CYRILLIC CAPITAL LETTER DE}
#  а   \320\260    \xD0\xB0   U+0430   \N{CYRILLIC SMALL LETTER A}
#  д   \320\264    \xD0\xB4   U+0434   \N{CYRILLIC SMALL LETTER DE}

# Using octal values, as these are the most portable access various printfs.


# Input: Same input for all test (all lower case letters)
#       д       а        д
printf '\320\264\320\260 \320\264\n' > utf8-inp || framework_failure_


# Test 1: Convert "small DE" to upper case (with \U)
#       s/д/\U&/g
printf 's/\320\264/\\U&/g' > utf8-1.sed || framework_failure_

# Test 1: Expected output - two capital DE letters.
#       Д       а        Д
printf '\320\224\320\260 \320\224\n' > utf8-1-exp || framework_failure_


# Test 2: Convert "small DE" to upper case (with \u - next character only)
#       s/д/\u&/g
printf 's/\320\264/\\u&/g\n' > utf8-2.sed || framework_failure_

# The expected output of test 2 is identical to test 1.
# We create the file to make the test loop (below) simpler.
cp utf8-1-exp utf8-2-exp || framework_failure_



# Test 3: Capitalize only the next character (\u)
# Only the first "DE" should be capitilized.
#       s/д.*/\u&/g
printf 's/\320\264.*/\\u&/g' > utf8-3.sed || framework_failure_

# Test 3: Expected output - First DE capitilized, second DE not.
#       Д       а        д
printf '\320\224\320\260 \320\264\n' > utf8-3-exp || framework_failure_


# Test 4: Capitalize all matched characters
#       s/д.*/\U&/g
printf 's/\320\264.*/\\U&/g' > utf8-4.sed || framework_failure_


# Test 4: Expected output - All capital letters:
#       Д       А        Д
printf '\320\224\320\220 \320\224\n' > utf8-4-exp || framework_failure_

# Step 1: force Russian UTF8 locale.
# The case-conversion should either work, or not modify the input.
for i in 1 2 3 4;
do
    LC_ALL=ru_RU.UTF-8 \
          sed -f utf8-$i.sed < utf8-inp > utf8-$i-ru-out || fail=1

    remove_cr_inplace utf8-$i-ru-out

    # If we have the expected output - continue to next text
    compare utf8-$i-exp utf8-$i-ru-out && continue

    # Otherwise, ensure the input wasn't modified
    # (i.e. sed did not modify partial octets resulting in
    #  invalid multibyte sequences)
    compare utf8-$i-inp utf8-$i-ru-out || fail=1
done


# Step 2: If the current locale supports UTF8, repeat the above tests.
l=$(locale | grep '^LC_CTYPE=' | sed 's/^.*="// ; s/"$//')
case "$n" in
    *UTF-8 | *UTF8 | *utf8 | *utf-8) utf8=yes;;
    *) utf8=no;;
esac

if test "$utf8" = yes ; then
    for i in 1 2 3 4;
    do
        sed -f utf8-$i.sed < utf8-inp > utf8-$i-out || fail=1

        remove_cr_inplace utf8-$i-out

        # If we have the expected output - continue to next text
        compare utf8-$i-exp utf8-$i-out && continue

        # Otherwise, ensure the input wasn't modified
        # (i.e. sed did not modify partial octets resulting in
        #  invalid multibyte sequences)
        compare utf8-$i-inp utf8-$i-out || fail=1
    done
fi


Exit $fail
