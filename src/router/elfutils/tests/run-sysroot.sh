#! /usr/bin/env bash
# Copyright (C) 2024 Red Hat, Inc.
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

. $srcdir/test-subr.sh

tmpdir="$(mktemp -d)"
trap "rm -rf -- ${tmpdir}" EXIT

tar xjf "${abs_srcdir}/testfile-sysroot.tar.bz2" -C "${tmpdir}"

# check that stack supports --sysroot option
testrun "${abs_top_builddir}"/src/stack --core "${tmpdir}/core.bash" \
	--sysroot "${tmpdir}/sysroot" >"${tmpdir}/stack.out"

# Remove 2 stack frames with symbol names contained in .gnu_debugdata.
# Whether or not these names appear in the output depends on if elfutils
# was built with LZMA support.
sed -i '4,5d' "${tmpdir}/stack.out"

tempfiles cat.out

# check that we are able to get fully symbolized backtrace
testrun_compare cat "${tmpdir}/stack.out" <<\EOF
PID 431185 - core
TID 431185:
#0  0x0000ffff8ebe5a8c kill
#3  0x0000aaaae562b2fc execute_command
#4  0x0000aaaae561cbb4 reader_loop
#5  0x0000aaaae5611bf0 main
#6  0x0000ffff8ebd09dc __libc_start_call_main
#7  0x0000ffff8ebd0ab0 __libc_start_main@@GLIBC_2.34
#8  0x0000aaaae56127f0 _start
EOF

HAVE_OPENAT2=$(grep '^#define HAVE_OPENAT2_RESOLVE_IN_ROOT' \
                    ${abs_builddir}/../config.h | awk '{print $3}')

if [[ "$HAVE_OPENAT2" = 1 ]]; then
    # Change the layout of files in sysroot to test symlink escape scenario
    rm -f "${tmpdir}/sysroot/bin"
    mkdir "${tmpdir}/sysroot/bin"
    mv "${tmpdir}/sysroot/usr/bin/bash" "${tmpdir}/sysroot/bin/bash"
    ln -s /bin/bash "${tmpdir}/sysroot/usr/bin/bash"

    # Check that stack with --sysroot generates correct backtrace even if target
    # binary is actually absolute symlink pointing outside of sysroot directory
    testrun "${abs_top_builddir}"/src/stack --core "${tmpdir}/core.bash" \
	    --sysroot "${tmpdir}/sysroot" >"${tmpdir}/stack.out"

    # Remove 2 stack frames with symbol names contained in .gnu_debugdata.
    # Whether or not these names appear in the output depends on if elfutils
    # was built with LZMA support.
    sed -i '4,5d' "${tmpdir}/stack.out"

    # Check that we are able to get fully symbolized backtrace
    testrun_compare cat "${tmpdir}/stack.out" <<\EOF
PID 431185 - core
TID 431185:
#0  0x0000ffff8ebe5a8c kill
#3  0x0000aaaae562b2fc execute_command
#4  0x0000aaaae561cbb4 reader_loop
#5  0x0000aaaae5611bf0 main
#6  0x0000ffff8ebd09dc __libc_start_call_main
#7  0x0000ffff8ebd0ab0 __libc_start_main@@GLIBC_2.34
#8  0x0000aaaae56127f0 _start
EOF
fi

exit_cleanup
