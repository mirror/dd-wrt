#!/bin/sh
# Exercise some panic stops

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
# failure to create temp file
#

# inplace with an unwritable directory
mkdir a || framework_failure_
touch a/a || framework_failure_
chmod a-w a || framework_failure_

# Expected error message, with actual filename/errno trimmed
cat <<\EOF >exp-err-temp || framework_failure_
sed: couldn't open temporary file
EOF

# TODO: why exit-code 4 (currently hard-coded)
returns_ 4 sed -i = a/a 2>err-temp || fail=1

# trim the filename/errno message (using sed itself...)
sed -i 's/file.*$/file/' err-temp || framework_failure_
compare_ exp-err-temp err-temp || fail=1

# restore writability, to ensure it can be deleted
chmod a+w a || framework_failure_


#
# no input files (with inplace)
#

# Expected error message
cat <<\EOF> exp-err-no-files || framework_failure_
sed: no input files
EOF

# /dev/null to ensure it doesn't hang if panic is not invoked
returns_ 4 sed -i = </dev/null 2>err-no-files || fail=1
compare_ exp-err-no-files err-no-files || fail=1


#
# Not a regular file (with inplace)
#
cat <<\EOF >exp-err-not-reg-file || framework_failure_
sed: couldn't edit f: not a regular file
EOF

mkfifo f || framework_failure_

# NOTE: the file-mode check is not performed until the first line is read.
#       an empty/blocking fifo will hang forever.
printf a > f &

# TODO: add a timeout in case of bug leading to a blocking fifo?
returns_ 4 sed -i = f 2>err-not-reg-file || fail=1
compare_ exp-err-not-reg-file err-not-reg-file || fail=1


#
# inplace on a terminal device
# (if available)
#

#NOTE: device name is replaced later
cat <<\EOF >exp-err-tty || framework_failure_
sed: couldn't edit X: is a terminal
EOF

ttydev=no-such-file
type tty >/dev/null 2>&1 && ttydev=$(tty 2>/dev/null)
if test -w "$ttydev" && test -r "$ttydev" ; then
    returns_ 4 sed -i = "$ttydev" 2>err-tty || fail=1

    # remove the actual terminal device name (using sed itself...)
    sed -i 's/edit.*:/edit X:/' err-tty || framework_failure_

    compare_ exp-err-tty err-tty || fail=1
fi



Exit $fail
