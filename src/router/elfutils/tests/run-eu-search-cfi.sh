#! /bin/sh
# Data race test for parallelized next_cfi
# Copyright (C) 2015, 2018 Red Hat, Inc.
# This file is part of elfutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# elfutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. $srcdir/thread-safety-subr.sh

check_thread_safety_enabled

cfi_test_files=("testfile11" "testfile12"
                "testfilearm" "testfileaarch64"
                "testfileppc32" "testfileppc64")

testfiles "${cfi_test_files[@]}"

for file in "${cfi_test_files[@]}"; do
    testrun ${abs_builddir}/eu_search_cfi $file
done
