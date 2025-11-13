#!/bin/sh
# Test -z/--null-data option

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

# Two lines, differ based on the EOL character.
printf "AB\000CD\nEF\n\000" > in1 || framework_failure_

# 's/^./x/' cmd processed with EOF=\n
printf "xB\000CD\nxF\nx" > exp-s-nl || framework_failure_
# 's/^./x/' cmd processed with EOF=\0
printf "xB\000xD\nEF\n\000" > exp-s-z || framework_failure_

# '=' cmd processed with EOF=\n
printf "1\nAB\000CD\n2\nEF\n3\n\000" > exp-=-nl || framework_failure_

# '=' cmd processed with EOF=\0
printf "1\000AB\0002\000CD\nEF\n\000" > exp-=-z || framework_failure_


# 'l' cmd processed with EOF=\n
cat <<\EOF >exp-l-nl || framework_failure_
AB\000CD$
EF$
\000$
EOF

# 'l' cmd processed with EOF=\0
printf 'AB$\000CD\\nEF\\n$\000' >exp-l-z || framework_failure_

# 'F' cmd with EOL=\n
printf "in1\n" > exp-F-nl || framework_failure_

# 'F' cmd with EOL=\0
printf "in1\000" > exp-F-z || framework_failure_


# Test substitution
sed 's/^./x/' in1 > out-s-nl || fail=1
compare_ exp-s-nl out-s-nl || fail=1

sed -z 's/^./x/' in1 > out-s-z || fail=1
compare_ exp-s-z out-s-z || fail=1



# Test '=' command
sed = in1 > out-=-nl || fail=1
compare_ exp-=-nl out-=-nl || fail=1

sed -z = in1 > out-=-z || fail=1
compare_ exp-=-z out-=-z || fail=1



# Test 'l' command
sed -n l in1 > out-l-nl || fail=1
compare_ exp-l-nl out-l-nl || fail=1

sed -zn l in1 > out-l-z || fail=1
compare_ exp-l-z out-l-z || fail=1


# Test 'F' command
sed -n 1F in1 > out-F-nl || fail=1
compare_ exp-F-nl out-F-nl || fail=1

sed -zn 1F in1 > out-F-z || fail=1
compare_ exp-F-z out-F-z || fail=1


Exit $fail
