#!/usr/bin/env bash
#
# Copyright (C) 2022-2024 Red Hat, Inc.
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

# for test case debugging, uncomment:
set -x
unset VALGRIND_CMD
# VALGRIND_CMD="valgrind --enable-debuginfod=no"

type curl 2>/dev/null || { echo "need curl"; exit 77; }
type jq 2>/dev/null || { echo "need jq"; exit 77; }

pkg-config json-c libcurl || { echo "one or more libraries are missing (libjson-c, libcurl)"; exit 77; }

DB=${PWD}/.debuginfod_tmp.sqlite
export DEBUGINFOD_CACHE_PATH=${PWD}/.client_cache
tempfiles $DB ${DB}_2

# This variable is essential and ensures no time-race for claiming ports occurs
# set base to a unique multiple of 100 not used in any other 'run-debuginfod-*' test
base=13100
get_ports
mkdir R D
cp -rvp ${abs_srcdir}/debuginfod-rpms/rhel7 R
cp -rvp ${abs_srcdir}/debuginfod-debs/*deb D

env LD_LIBRARY_PATH=$ldpath DEBUGINFOD_URLS= ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod $VERBOSE -R \
    -d $DB -p $PORT1 -t0 -g0 R > vlog$PORT1 2>&1 &
PID1=$!
tempfiles vlog$PORT1
errfiles vlog$PORT1

wait_ready $PORT1 'ready' 1
wait_ready $PORT1 'thread_work_total{role="traverse"}' 1
wait_ready $PORT1 'thread_work_pending{role="scan"}' 0
wait_ready $PORT1 'thread_busy{role="scan"}' 0

env LD_LIBRARY_PATH=$ldpath DEBUGINFOD_URLS="http://127.0.0.1:$PORT1 https://bad/url.web" ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod $VERBOSE -U \
    -d ${DB}_2 -p $PORT2 -t0 -g0 --cors D > vlog$PORT2 2>&1 &
PID2=$!
tempfiles vlog$PORT2
errfiles vlog$PORT2

wait_ready $PORT2 'ready' 1
wait_ready $PORT2 'thread_work_total{role="traverse"}' 1
wait_ready $PORT2 'thread_work_pending{role="scan"}' 0
wait_ready $PORT2 'thread_busy{role="scan"}' 0

# have clients contact the new server
export DEBUGINFOD_URLS=http://127.0.0.1:$PORT2

tempfiles json.txt
# Check that we find correct number of files, both via local and federated links
RESULTJ=`env LD_LIBRARY_PATH=$ldpath ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod-find metadata glob "/u?r/bin/*"`
echo $RESULTJ
N_FOUND=`echo $RESULTJ | jq '.results | length'`
test $N_FOUND -eq 1
RESULTJ=`env LD_LIBRARY_PATH=$ldpath ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod-find metadata glob "/usr/lo?al/bin/*"`
echo $RESULTJ
N_FOUND=`echo $RESULTJ | jq '.results | length'`
test $N_FOUND -eq 2


# Query via the webapi as well
curl http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*'
# no --cors on $PORT1's debuginfod
test "`curl -s -i http://127.0.0.1:$PORT1'/metadata?key=glob&value=/usr/bin/*hi*' | grep -i access.control.allow.origin: || true`" == ""
curl -s -i http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*' | grep -i access.control.allow.origin:
test `curl -s http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*' | jq '.results[0].buildid == "f17a29b5a25bd4960531d82aa6b07c8abe84fa66"'` = 'true'
test `curl -s http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*' | jq '.results[0].file == "/usr/bin/hithere"'` = 'true'
test `curl -s http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*' | jq '.results[0].archive | test(".*hithere.*deb")'` = 'true'
# Note we query the upstream server too, since the downstream will have an incomplete result due to the badurl
test `curl -s http://127.0.0.1:$PORT1'/metadata?key=glob&value=/usr/bin/*hi*' | jq '.complete == true'` = 'true'
test `curl -s http://127.0.0.1:$PORT2'/metadata?key=glob&value=/usr/bin/*hi*' | jq '.complete == false'` = 'true'

# An empty array is returned on server error or if the file DNE
RESULTJ=`env LD_LIBRARY_PATH=$ldpath ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod-find metadata file "/this/isnt/there"`
echo $RESULTJ
test `echo $RESULTJ | jq ".results == [ ]" ` = 'true'

kill $PID1
kill $PID2
wait $PID1
wait $PID2
PID1=0
PID2=0

# check it's still in cache
RESULTJ=`env LD_LIBRARY_PATH=$ldpath ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod-find metadata file "/usr/bin/hithere"`
echo $RESULTJ
test `echo $RESULTJ | jq ".results == [ ]" ` = 'true'

# invalidate cache, retry previously successful query to now-dead servers
echo 0 > $DEBUGINFOD_CACHE_PATH/metadata_retention_s
RESULTJ=`env LD_LIBRARY_PATH=$ldpath ${VALGRIND_CMD} ${abs_builddir}/../debuginfod/debuginfod-find metadata glob "/u?r/bin/*"`
echo $RESULTJ
test `echo $RESULTJ | jq ".results == [ ]" ` = 'true'
test `echo $RESULTJ | jq ".complete == false" ` = 'true'

exit 0
