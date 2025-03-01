# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# Multiple process mpool filler.

source ./include.tcl
source $test_path/test.tcl
source $test_path/testutils.tcl

puts "Memp008: Sub-test for MPOOL filling."
set targetenv [berkdb_env -home $testdir]

# Use a marker file to tell child processes when to stop.
set stop_fname "$testdir/memp008.stop"
error_check_good chkfile [file exists $stop_fname] 0

set db [eval {berkdb_open -env $targetenv -auto_commit "memp008.db"} ]
error_check_good dbopen [is_valid_db $db] TRUE

for { set key 0} { $key < 1000 } { incr key } {
	set ret [catch {$db put $key $key} result]
	if { $ret == 1 } {
		puts "$key: $result"
		# The MPOOL might be filled by data.
		if {![is_substr $result "not enough memory"] &&\
		    ![is_substr $result "unable to allocate"] } {
			puts "FAIL: in filling DB, $result"
		}
		break
	}
}

puts "Memp008: Create stop flag file."
set fileid [open $stop_fname "w"]
error_check_good createfile [close $fileid] ""

error_check_good db_close [$db close] 0

puts "Memp008: Sub-test for MPOOL filling finished."
