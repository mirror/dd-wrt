# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# Env030script -- for use with env030, failchk test.  Sets a hook that
#  causes the program to crash in various places (inside and outside the db
#  library).
# Usage: envscript testdir testfile putget key data hook
# testdir: directory containing the env we are joining.
# testfile: file name for database.
# putget: What to do in the db: put or get.
# key: key to store or get
# data: data to store or get
# hook: what env test hook to set, can be none, outside, no_mutex, latch
#  exc_latch, exc_mutex, failchk


source ./include.tcl
source $test_path/testutils.tcl

set usage "envscript testdir testfile putget key data hook"

# Verify usage
if { $argc != 6 } {
	puts stderr "FAIL:[timestamp] Usage: $usage"
	exit
}

# Initialize arguments
set testdir [ lindex $argv 0 ]
set testfile [ lindex $argv 1 ]
set putget [lindex $argv 2 ]
set key [ lindex $argv 3 ]
set data [ lindex $argv 4 ]
set hook [ lindex $argv 5 ]

set flag1 " -isalive my_isalive "

# Open and register environment.
if {[catch {eval {berkdb_env_noerr} \
    -create -home $testdir -txn $flag1 } dbenv]} {
    	puts "FAIL: opening env returned $dbenv"
}
error_check_good envopen [is_valid_env $dbenv] TRUE

# Open database, put or get, close database.
if {[catch {eval {berkdb_open_noerr} \
    -create -auto_commit -btree -env $dbenv $testfile} db]} {
	puts "FAIL: opening db returned $db"
}
error_check_good dbopen [is_valid_db $db] TRUE

if { $hook != "none" && $hook != "outside" } {
	$dbenv test abort $hook
}

switch $putget {
	PUT {
		set txn [$dbenv txn]
	    	error_check_good db_put [eval {$db put} -txn $txn $key $data] 0
	    	if { $hook == "outside" } {
			exit(1)
	    	}
		error_check_good txn_commit [$txn commit] 0
	}
    	GET {
		set txn [$dbenv txn]
		set ret [eval {$db get} -txn $txn $key]
	    	error_check_good db_get [lindex [lindex $ret 0] 1] $data
	    	if { $hook == "outside" } {
			exit(1)
	    	}
		error_check_good txn_commit [$txn commit] 0
	}
	default {
		puts "FAIL: Unrecognized putget value $putget"
	}
}

error_check_good failchk [$dbenv failchk] 0

error_check_good db_close [$db close] 0

error_check_good env_close [$dbenv close] 0
