#!/bin/sh
# Test --follow-symlinks option

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
# 'F' with/out follow-symlinks
#
echo dummy > a || framework_failure_
ln -s a la1 || framework_failure_
ln -s la1 la2 || framework_failure_

echo a > exp-a || framework_failure_
echo la1 > exp-la1 || framework_failure_

# Sanity-check: the real file
sed 'F;Q' a > out-a || fail=1
compare_ exp-a out-a || fail=1

# Without follow-symlinks
sed -n 'F' la1 > out-la1 || fail=1
compare_ exp-la1 out-la1 || fail=1

# With follow-symlinks
sed -n --follow-symlinks 'F' la1 > out-la1-flw || fail=1
compare_ exp-a out-la1-flw || fail=1

# With follow-symlinks and two levels of indirections
sed -n --follow-symlinks 'F' la2 > out-la2-flw || fail=1
compare_ exp-a out-la2-flw || fail=1

# Two symlinks input
# (implementation note: utils.c:follow_symlinks() uses a static buffer
#  which will be non-empty on the second invocation)
printf "%s\n" a a > exp-two-symlinks || framework_failure_
sed --follow-symlinks -n 'F' la1 la2 > out-two-symlinks || fail=1
compare_ exp-two-symlinks out-two-symlinks || fail=1

# non-existing input with --follow-symlink
# implementation note: readlink called before open, thus "couldn't readlink"
cat <<\EOF >exp-stat || framework_failure_
sed: couldn't readlink badfile:
EOF
returns_ 4 sed --follow-symlinks 'F' badfile >/dev/null 2>err-stat || fail=1

# trim the filename/errno message (using sed itself...)
sed -i 's/badfile:.*$/badfile:/' err-stat || framework_failure_
compare_ exp-stat err-stat || fail=1


# symlinks with absolute path
ln -s "$PWD/a" la-abs || framework_failure_
echo "$PWD/a" > exp-la-abs || framework_failure_
sed -n --follow-symlinks 'F' la-abs > out-la-abs || fail=1
compare_ exp-la-abs out-la-abs || fail=1

# symlink loop
ln -s la-loop la-loop || framework_failure_
sed --follow-symlinks -i s/a/b/ la-loop && fail=1

Exit $fail
