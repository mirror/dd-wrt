# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env027
# TEST	Test recovery of the truncate of a hash db. 
# TEST  
# TEST	This is a one-off test of the fix for #23772 --
# TEST	the conditions for reproducing it are so specific 
# TEST	that the test does not generalize well to other 
# TEST	access methods.

proc env027 { } {
	source ./include.tcl
	env_cleanup $testdir

	set testfile env027.db

	puts "\tEnv027.a: Fill."
	# Open the env with thread status support to enable __memp_fget's
	# diagnostic code to check for page locks.
	set env [eval {berkdb_env_noerr -create -mode 0666  -home $testdir \
		-log -txn -thread -thread_count { 100 } -recover } ]
	error_check_good env_open [is_valid_env $env] TRUE
	set data [repeat "abcd " 20]
	set db [berkdb_open_noerr -create -auto_commit -env $env -hash $testfile]
	error_check_good db_open [is_valid_db $db] TRUE

	# This pattern of inserts (0x9a->0xfe, skipping a few in the middle,
	# then 0->0x39) extends the hash db a couple of times, leaving two
	# not-yet-touched pages which need special handling in recovery.
	for { set k 0x9a } { $k <= 0xfe } { incr k } {
		if { $k == 0x9c || $k == 0xcd || $k == 0xd1 ||
		     ($k >= 0xd3 && $k <= 0xd5) } {
		     	continue
		}
		set err [eval {$db put} {[binary format i $k] $data} ]
	}
	for { set k 0x0 } { $k <= 0x39 } { incr k } {
		set err [eval {$db put} {[binary format i $k] $data} ]
	}
	
	puts "\tEnv027.b: Recover $testdir before truncates."
	set env2 [eval {berkdb_env -create -mode 0666  -home $testdir \
		-log -txn -thread -thread_count { 100 } -recover } ] 
	error_check_good run_recovery_number1 [is_valid_env $env] TRUE

	set db2 [berkdb_open -env $env2 -auto_commit $testfile]
	error_check_good re-db_open [is_valid_db $db2] TRUE
	set ret [$db2 truncate]
	puts "\tEnv027.c: First truncate returned $ret pages."
	set ret [$db2 truncate]
	puts "\tEnv027.d: Second truncate returned $ret pages."

	# Clean up the leftover handles from before the truncate, 
	# then clean up the recovered env's handles.
	catch {$db close} ignore
	catch {$env close} ignore
	$db2 close 
	$env2 close

	puts "\tEnv027.e: Recovering $testdir after truncates."
	set env [eval {berkdb_env -create -mode 0666  -home $testdir -log -txn \
		-thread -thread_count { 100 } -recover } ] 
	error_check_good run_recovery_number2 [is_valid_env $env] TRUE
	$env close
}
