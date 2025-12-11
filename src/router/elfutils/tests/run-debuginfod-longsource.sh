#!/usr/bin/env bash
#
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

. $srcdir/debuginfod-subr.sh

# for test case debugging
set -x
unset VALGRIND_CMD

DB=${PWD}/.debuginfod_tmp.sqlite
tempfiles $DB
export DEBUGINFOD_CACHE_PATH=${PWD}/.client_cache

# Set up directories for scanning
mkdir Z
cp -rvp ${abs_srcdir}/debuginfod-tars/bighello.tar Z

# This variable is essential and ensures no time-race for claiming ports occurs
# set base to a unique multiple of 100 not used in any other 'run-debuginfod-*' test
base=14200
get_ports

# We use -t0 and -g0 here to turn off time-based scanning & grooming.
# For testing purposes, we just sic SIGUSR1 at the process.

env LD_LIBRARY_PATH=$ldpath DEBUGINFOD_URLS= ${abs_builddir}/../debuginfod/debuginfod $VERBOSE \
    -Ztar -p $PORT1 -d $DB -t0 -g0 -v ./Z > vlog$PORT1 2>&1 &
PID1=$!
tempfiles vlog$PORT1
errfiles vlog$PORT1
# Server must become ready
wait_ready $PORT1 'ready' 1
# And initial scan should be done
wait_ready $PORT1 'thread_work_total{role="traverse"}' 1

kill -USR1 $PID1 # run another index pass to make sure the srcdef/srcref stuff is fully located

# Wait till both files are in the index.
wait_ready $PORT1 'thread_work_total{role="traverse"}' 2
wait_ready $PORT1 'thread_work_pending{role="scan"}' 0
wait_ready $PORT1 'thread_busy{role="scan"}' 0

export DEBUGINFOD_URLS=http://127.0.0.1:$PORT1

########################################################################

# Build-id for a.out in said tarball
BUILDID=7fc69cb0e8fb9d4b57e594271b9941b67410aaaa

# Download short & long files
testrun ${abs_top_builddir}/debuginfod/debuginfod-find -vvv source $BUILDID /tmp/bighello-sources/bighello.c
testrun ${abs_top_builddir}/debuginfod/debuginfod-find -vvv source $BUILDID /tmp/bighello-sources/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/moremoremoremoremoremoremoremore/bighello.h

exit 0
