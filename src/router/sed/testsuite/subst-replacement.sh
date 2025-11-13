#!/bin/sh
# Test Substitute replacements, e.g. 's/(.)/\U\1/'

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

#
# Backslash followed by unrecognized letter,
# use letter as-is.
echo a > in-rpl1 || framework_failure_
echo Q > exp-rpl1 || framework_failure_
sed -E 's/(.)/\Q/' in-rpl1 > out-rpl1 || fail=1
compare_ exp-rpl1 out-rpl1 || fail=1

#
# numbered backreferences
#
echo 123456789 > in-rpl2 || framework_failure_
for i in 1 2 3 4 5 6 7 8 9 ;
do
    echo $i > exp-rpl2-$i || framework_failure_
    sed -E "s/(.)(.)(.)(.)(.)(.)(.)(.)(.)/\\$i/" in-rpl2 > out-rpl2-$i || fail=1
    compare_ exp-rpl2-$i out-rpl2-$i || fail=1
done

# \0 matches entire pattern (TODO: is this documented?)
# output should be the same as the input.
sed -E 's/(.)(.)(.)(.)(.)(.)(.)(.)(.)/\0/' in-rpl2 > out-rpl2-0 || fail=1
compare_ in-rpl2 out-rpl2-0 || fail=1

# Unescaped '&' matches entire pattern
# output should be the same as the input.
sed -E 's/(.)(.)(.)(.)(.)(.)(.)(.)(.)/&/' in-rpl2 > out-rpl2-amp || fail=1
compare_ in-rpl2 out-rpl2-amp || fail=1


#
# gnu extension: \U \u \L \l \E
#
echo abCde > in-rpl3 || framework_failure_

# \U - all uppercase
echo ABCde > exp-rpl3-U || framework_failure_
sed -E 's/(.)(.)(.)/\U\1\2\3/' in-rpl3 > out-rpl3-U || fail=1
compare_ exp-rpl3-U out-rpl3-U || fail=1

# \u - next-char uppercase
echo AbCde > exp-rpl3-u || framework_failure_
sed -E 's/(.)(.)(.)/\u\1\2\3/' in-rpl3 > out-rpl3-u || fail=1
compare_ exp-rpl3-u out-rpl3-u || fail=1

# \L - all lowercase
echo abcde > exp-rpl3-L || framework_failure_
sed -E 's/(.)(.)(.)/\L\1\2\3/' in-rpl3 > out-rpl3-L || fail=1
compare_ exp-rpl3-L out-rpl3-L || fail=1

# \l - next-char lowercase
echo abCde > exp-rpl3-l || framework_failure_
sed -E 's/(.)(.)(.)/\l\1\2\3/' in-rpl3 > out-rpl3-l || fail=1
compare_ exp-rpl3-l out-rpl3-l || fail=1

# \E - stop \U \u \L \l processing
echo AbCde > exp-rpl3-E1 || framework_failure_
sed -E 's/(.)(.)(.)/\U\1\E\2\3/' in-rpl3 > out-rpl3-E1 || fail=1
compare_ exp-rpl3-E1 out-rpl3-E1 || fail=1

echo abCde > exp-rpl3-E2 || framework_failure_
sed -E 's/(.)(.)(.)/\L\1\2\E\3/' in-rpl3 > out-rpl3-E2 || fail=1
compare_ exp-rpl3-E2 out-rpl3-E2 || fail=1


Exit $fail
