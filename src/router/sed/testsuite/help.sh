#!/bin/sh

# Test --help screen

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

# Help screen should include the contact email address
sed --help | grep E-mail > /dev/null 2>&1 || fail=1


# With explicit --help - show usage then email at the bottom.
# With missing parameters - show the usage without the email.
# Ensure these are identical (except for the email).
sed --help \
    | sed '1s/ [^ ]* / sed /; /^E-mail/,$d' > help-out1

sed 2>&1 \
    | sed '1s/ [^ ]* / sed /' > help-out2

compare help-out1 help-out2 || fail=1


Exit $fail
