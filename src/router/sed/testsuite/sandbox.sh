#!/bin/sh
# Test --sandbox mode

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

echo a > a || framework_failure_
echo b > b || framework_failure_

# Same error message, different character position in the sed program.
for i in 1 6 14 ; do
  err="sed: -e expression #1, char $i: e/r/w commands disabled in sandbox mode"
  echo "$err" > exp-err$i || framework_failure_
done

# read command - without sandbox
printf "a\nb\n" > exp || framework_failure_
sed rb a > out || fail=1
compare exp out || fail=1

# read command - with sandbox
returns_ 1 sed --sandbox -e 'ra' b >/dev/null 2>err1 || fail=1
compare exp-err1 err1 || fail=1


# write command (create file 'c') - without sandbox
sed wc a > out || fail=1
compare a c || fail=1
compare out a || fail=1

# write command - with sandbox
returns_ 1 sed --sandbox -e 'wd' a >/dev/null 2>err2 || fail=1
compare exp-err1 err1 || fail=1
# ensure file 'd' was not created
test -e d && fail=1



# execute command - without sandbox
sed 'etouch e' b > out || fail=1
compare b out || fail=1
# ensure 'e' was created
test -e e || fail=1

# execute command - with sandbox
returns_ 1 sed --sandbox -e 'etouch f' b >/dev/null 2>err3 || fail=1
compare exp-err1 err3 || fail=1
# ensure 'f' was not created
test -e f && fail=1



# substitute+write option - without sandbox
sed 's/^//wg' a > out || fail=1
test -e g || fail=1

# substitute+write option  - with sandbox
returns_ 1 sed --sandbox 's/^//wh' a >/dev/null 2>err4 || fail=1
compare exp-err6 err4 || fail=1
# ensure file 'h' was not created
test -e h && fail=1



# substitute+execute option - without sandbox
sed 's/.*/touch i/e' a > out || fail=1
test -e i || fail=1

# substitute+execute option - with sandbox
returns_ 1 sed --sandbox 's/.*/touch j/e' a >/dev/null 2>err5 || fail=1
compare exp-err14 err5 || fail=1
# ensure file 'j' was not created
test -e j && fail=1


Exit $fail
