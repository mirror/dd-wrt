#! /usr/bin/env bash
# Copyright (C) 2024 Mark J. Wielaard
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

# Make sure the profile.sh or profile.d/debuginfod.sh works even with
# set -e (any command error is an error) and set -o pipefail (any error
# in a pipe fails the whole pipe command).

set -e
set -o pipefail

source ${abs_top_builddir}/config/profile.sh

type fish 2>/dev/null || (echo "no fish installed"; exit 77)
fish ${abs_top_builddir}/config/profile.fish
