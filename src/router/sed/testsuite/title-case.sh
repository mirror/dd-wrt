#!/bin/sh
# Test case insensitive matching for titlecase and similarly odd chars.

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

require_el_iso88597_locale_

a='\323' # SIGMA
b='\362' # stigma
c='\363' # sigma

printf "$a\\n$b\\n$c\\n" >in || framework_failure_
for chr in "$a" "$b" "$c"; do
   printf '/()\\1'"$chr"/Ip >prog || fail=1
   LC_ALL=el_GR.iso88597 sed -r -n -f prog in >out || fail=1
   compare_ in out || fail=1
done

Exit $fail
