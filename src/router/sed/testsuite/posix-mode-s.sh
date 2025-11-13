#!/bin/sh
# Ensure GNU extensions are rejected in posix mode

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

cat <<\EOF >exp-err || framework_failure_
sed: -e expression #1, char 7: unknown option to `s'
EOF

# substitution command options (
# TODO: conditionally test sSxX in perl mode
for opt in i I m M ;
do
    # These options should fail in strict POSIX mode
    returns_ 1 sed --posix "s/a/b/$opt" </dev/null 2>err || fail=1
    compare_ exp-err err || fail=1

    # These options are allowed otherwise
    sed "s/a/b/$opt" </dev/null || fail=1

    # POSIXLY_CORRECT alone does not disable them
    POSIXLY_CORRECT=y sed "s/a/b/$opt" </dev/null || fail=1
done


# test s//e (execute pattern-space as shell)
printf "A\n" > in1        || framework_failure_

printf "hello\n" >exp-gnu-e || framework_failure_
sed 's/./printf hello/e' in1 > out-gnu-e || fail=1
compare exp-gnu-e out-gnu-e || fail=1


# s///e rejected in POSIX mode
cat <<\EOF >exp-err-psx-e || framework_failure_
sed: -e expression #1, char 10: unknown option to `s'
EOF
returns_ 1 sed --posix 's/./echo/e' in1 2>err-posix-e || fail=1
compare_ exp-err-psx-e err-posix-e || fail=1


# substitution special commands (e.g \l \L \U \u \E).
# see compile.c:setup_replacement()
printf "a\n" > exp-gnu    || framework_failure_
printf "lA\n" > exp-posix || framework_failure_

# gnu-extension: turn the next character to lowercase
sed 's/./\l&/' in1 > out-gnu || fail=1
compare_ exp-gnu out-gnu || fail=1

# posix: '\l' is just 'l'
sed --posix 's/./\l&/' in1 > out-posix || fail=1
compare_ exp-posix out-posix || fail=1


Exit $fail
