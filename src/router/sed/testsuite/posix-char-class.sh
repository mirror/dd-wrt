#!/bin/sh
# Test character-class definitions in POSIX mode.

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

# NOTE:
# In GNU Extension mode, all text is normalized (e.g. backslash-X combinations).
# In POSIX mode, normalize_text() ensures content of character
# classes is not normalized.
#
# Compare:
#   $ printf "t\t\n" | sed 's/[\t]/X/' | od -a
#   0000000  t   X   nl
#   $ printf "t\t\n" | sed --posix 's/[\t]/X/' | od -a
#   0000000  X   ht  nl
#
# This test unit validates the special handling of character classes
# in posix mode (compile.c:normalize_text() implementation).


. "${srcdir=.}/testsuite/init.sh"; path_prepend_ ./sed
print_ver_ sed

echo X > exp || framework_failure_

# Closing bracket without opening bracket, match as-is
echo ']' | sed --posix 's/]/X/' > out1 || fail=1
compare_ exp out1 || fail=1

# Two opening brackets (same state when opening the second one)
echo '[' | sed --posix 's/[[]/X/' > out2 || fail=1
compare_ exp out2 || fail=1

# Escaping before and after the character class, but not inside it (POSIX MODE)
printf "\tt\t\n" | sed --posix 's/\t[\t]\t/X/' > out3 || fail=1
compare_ exp out3 || fail=1

# Escaping before, inside, and after the character class (GNU MODE)
printf "\t\t\t\n" | sed  's/\t[\t]\t/X/' > out4 || fail=1
compare_ exp out4 || fail=1

# Special characters, but outside a valid character-class syntax
printf "=\n" | sed --posix 's/[.=:.]/X/' > out5 || fail=1
compare_ exp out5 || fail=1

# A valid character class definition
printf "b\n" | sed --posix 's/[[:alpha:]]/X/' > out6 || fail=1
compare_ exp out6 || fail=1



Exit $fail
