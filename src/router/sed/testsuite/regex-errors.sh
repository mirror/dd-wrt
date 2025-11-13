#!/bin/sh
# Exercise regex_compile errors

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
# Invalid backref in address regex
#
cat <<\EOF >exp-err-inv-backref || framework_failure_
sed: -e expression #1, char 4: Invalid back reference
EOF

returns_ 1 sed '/\1/,$p' </dev/null 2>err-inv-backref || fail=1
compare_ exp-err-inv-backref err-inv-backref || fail=1


#
# modifiers on empty regex (BAD_MODIF in regex.c)
#
cat <<\EOF >exp-err-bad-modif || framework_failure_
sed: -e expression #1, char 3: cannot specify modifiers on empty regexp
EOF

returns_ 1 sed '//M,$p' </dev/null 2>err-bad-modif || fail=1
compare_ exp-err-bad-modif err-bad-modif || fail=1


Exit $fail
