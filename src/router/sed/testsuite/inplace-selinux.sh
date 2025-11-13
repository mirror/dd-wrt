#!/bin/sh

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
require_selinux_

sed --version | grep "with SELinux" > /dev/null \
  || skip_ "sed built without SELinux support"
sed --version | grep "^SELinux is enabled" > /dev/null \
  || skip_ "sed reports SELinux is disabled on this system"

touch a || framework_failure_
chcon -u system_u a || skip_ "chcon doesn't work"
chcon -u user_u a || skip_ "chcon doesn't work"

# Create the first file and symlink pointing at it.
echo "Hello World" > inplace-selinux-file || framework_failure_
ln -s ./inplace-selinux-file inplace-selinux-link || framework_failure_

chcon -h -u system_u inplace-selinux-file || framework_failure_
chcon -h -u user_u inplace-selinux-link || framework_failure_


# Create the second file and symlink pointing at it.
# These will be used with the --follow-symlink option.
echo "Hello World" > inplace-selinux-file2 || framework_failure_
ln -s ./inplace-selinux-file2 inplace-selinux-link2 || framework_failure_

chcon -h -u system_u inplace-selinux-file2 || framework_failure_
chcon -h -u user_u inplace-selinux-link2 || framework_failure_

# Modify prepared files inplace via the symlinks
sed -i -e "s~Hello~Hi~" inplace-selinux-link || fail=1
sed -i --follow-symlinks -e "s~Hello~Hi~" inplace-selinux-link2 || fail=1

# Check selinux context - the first file should be created with the context
# of the symlink...
ls -Z inplace-selinux-link | grep user_u: || fail=1
# ...the second file should use the context of the file itself.
ls -Z inplace-selinux-file2 | grep system_u: || fail=1

Exit $fail
