# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# Process for fsyncing MPOOL.

source ./include.tcl
source $test_path/test.tcl
source $test_path/testutils.tcl

puts "Memp008: Sub-test for MPOOL fsync."
set targetenv [berkdb_env -home $testdir]
set stop_fname "$testdir/memp008.stop"

set targetmp [$targetenv mpool -create -pagesize 512\
    -mode 0644 "memp008fsync.mem"]

# Will keep fsyncing MPOOL.
while { ![file exists $stop_fname] } {
	$targetmp fsync
}

$targetmp close
error_check_good envclose [$targetenv close] 0
puts "Memp008: Sub-test for MPOOL fsync finished."
