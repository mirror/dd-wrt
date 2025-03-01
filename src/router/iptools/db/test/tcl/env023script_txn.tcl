# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# Env023 child process, it changes two db with a given order in one txn.

source ./include.tcl
source $test_path/test.tcl

set order [lindex $argv 0]
set dir [lindex $argv 1]

if { $order != "dba_first" && $order != "dbb_first" } {
	puts "FAIL: unknown order ($order)"
}

puts "Env023: opening env and databases."
set targetenv [berkdb_env -home $testdir -txn_timeout 15000000]

set dba [eval {berkdb_open -env $targetenv -auto_commit "env023dba.db"} ]
error_check_good dbopen [is_valid_db $dba] TRUE

set dbb [eval {berkdb_open -env $targetenv -auto_commit "env023dbb.db"} ]
error_check_good dbopen [is_valid_db $dbb] TRUE

puts "Env023: starting txn to modify databases."
set t [$targetenv txn]
set key 1
set deadlock_err "BDB0068 DB_LOCK_DEADLOCK"

if { $order == "dba_first" } {
	puts "Env023: modifying dba."
	error_check_good filldata [$dba put -txn $t $key $key] 0
	# Wait for 5 seconds to make sure the other env023script_txn process
	# is started even under heavy load.
	tclsleep 5
	puts "Env023: modifying dbb."
	if { [catch {eval $dbb put -txn $t $key $key} result] } {
		if { [is_substr $result $deadlock_err] } {
			puts "Env023: deadlock occurred, abort txn."
			error_check_good txn_abort [$t abort] 0
		} else {
			puts "FAIL: $result"
		}
	} else {
		puts "Env023: committing txn."
		error_check_good txn [$t commit] 0
	}
} else {
	puts "Env023: modifying dbb."
	error_check_good filldata [$dbb put -txn $t $key $key] 0
	# Wait for 5 seconds to make sure the other env023script_txn process
	# is started even under heavy load.
	tclsleep 5
	puts "Env023: modifying dba."
	if { [catch {eval $dba put -txn $t $key $key} result] } {
		if { [is_substr $result $deadlock_err] } {
			puts "Env023: deadlock occurred, abort txn."
			error_check_good txn_abort [$t abort] 0
		} else {
			puts "FAIL: $result"
		}
	} else {
		puts "Env023: committing txn."
		error_check_good txn [$t commit] 0
	}
}

puts "Env023: close db and env."
error_check_good db_close [$dba close] 0
error_check_good db_close [$dbb close] 0
error_check_good env_close [$targetenv close] 0
