#!/bin/sh
# sed would access uninitialized memory for certain regexes.
# Before sed 4.6 these would result in "Conditional jump or move depends on
# uninitialised value(s)" and "Invalid read of size 1"
# by valgrind from regexp.c:286

# Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

require_valgrind_

# 40 characters ensures valgrind detects the bug
# (with less than 25 - it does not).
z=0000000000000000000000000000000000000000

printf '%s\n' $z $z > in || framework_failure_
printf '%s\n' $z $z > exp || framework_failure_

# Before sed-4.6, this would fail with:
# [...]
# ==13131== Conditional jump or move depends on uninitialised value(s)
# ==13131==    at 0x4C3002B: memchr (vg_replace_strmem.c:883)
# ==13131==    by 0x1120BD: match_regex (regexp.c:286)
# ==13131==    by 0x110736: do_subst (execute.c:1101)
# ==13131==    by 0x1115D3: execute_program (execute.c:1591)
# ==13131==    by 0x111A4C: process_files (execute.c:1774)
# ==13131==    by 0x112E1C: main (sed.c:405)
# ==13131==
# ==13131== Invalid read of size 1
# ==13131==    at 0x4C30027: memchr (vg_replace_strmem.c:883)
# ==13131==    by 0x1120BD: match_regex (regexp.c:286)
# ==13131==    by 0x110736: do_subst (execute.c:1101)
# ==13131==    by 0x1115D3: execute_program (execute.c:1591)
# ==13131==    by 0x111A4C: process_files (execute.c:1774)
# ==13131==    by 0x112E1C: main (sed.c:405)
# ==13131==  Address 0x55ec765 is 0 bytes after a block of size 101 alloc'd
# ==13131==    at 0x4C2DDCF: realloc (vg_replace_malloc.c:785)
# ==13131==    by 0x113BA2: ck_realloc (utils.c:418)
# ==13131==    by 0x10E682: resize_line (execute.c:154)
# ==13131==    by 0x10E6F0: str_append (execute.c:165)
# ==13131==    by 0x110779: do_subst (execute.c:1106)
# ==13131==    by 0x1115D3: execute_program (execute.c:1591)
# ==13131==    by 0x111A4C: process_files (execute.c:1774)
# ==13131==    by 0x112E1C: main (sed.c:405)
valgrind --quiet --error-exitcode=1 \
  sed -e 'N; s/$//m2' in > out 2> err || fail=1

# Work around a bug in CentOS 5.10's valgrind
# FIXME: remove in 2018 or when CentOS 5 is no longer officially supported
grep 'valgrind: .*Assertion.*failed' err-no-posix > /dev/null \
  && skip_ 'you seem to have a buggy version of valgrind'

# Remove any valgrind-added diagnostics from stderr.
sed -i '/^==/d' err

compare exp out || fail=1
compare /dev/null err || fail=1

echo "valgrind report:"
echo "=================================="
cat err
echo "=================================="

exit $fail
