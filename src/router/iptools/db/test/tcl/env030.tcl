# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env030
# TEST	Test failchk.
# TEST
# TEST  Tests that failchk is able to recover the environment after a crash,
# TEST  or returns RUNRECOVERY when appropriate.
# TEST
# TEST  1. Open environment with is_alive function and failchk.
# TEST  2. Open database and insert a record.
# TEST  3. Test that failchk works when another process: exits cleanly,
# TEST    crashes outside the API, crashes inside the API but holds no
# TEST    latches, and crashes inside the API while holding latches.
# TEST  4. Test that failchk returns RUNRECOVER when another process:
# TEST    crashes while holding an exclusive latch, crashes while holding
# TEST    a mutex, and crashes while performing failchk.


proc env030 { {args ""} } {
	source ./include.tcl
	set tnum "030"
	set testfile test.db

	puts "Env$tnum: Test failchk."
	env_cleanup $testdir

	puts "\tEnv$tnum.a1: Open an environment with DB_FAILCHK."
	set dbenv [eval {berkdb_env} \
	    -create -txn -mode 0644 -home $testdir \
	    -msgfile $testdir/msgfile -failchk -isalive my_isalive]
	error_check_good envopen [is_valid_env $dbenv] TRUE

	puts "\tEnv$tnum.a2: Open a database and insert some data."
	set db [eval {berkdb_open} -create -btree -env $dbenv $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	set ret [$db put 0 0]
	error_check_good dbput $ret 0

	set key 1
	set data 1
	set i 1

	puts "\tEnv$tnum.b: Run tests where failchk succeeds."
	set hooks { "none" "outside" "no_mutex" "latch" }
	foreach hook $hooks {
		puts "\t\tEnv$tnum.b$i: Run put process and crash it at $hook."
		set p1 [exec $tclsh_path $test_path/wrap.tcl env030script.tcl \
		    $testdir/env$tnum.log.p1.$i ALLOW_PIPE_CLOSE_ERROR \
		    $testdir $testfile PUT $key $data $hook &]
		watch_procs $p1 1 120
		set ret [$dbenv failchk]
		error_check_good failchk_put $ret 0
		incr i
		
		puts "\t\tEnv$tnum.b$i: Run get process and crash it at $hook."
		set p2 [exec $tclsh_path $test_path/wrap.tcl env030script.tcl \
		    $testdir/env$tnum.log.p2.$i ALLOW_PIPE_CLOSE_ERROR \
		    $testdir $testfile GET $key $data $hook &]

		watch_procs $p2 1 120
		set ret [$dbenv failchk]
		error_check_good failchk_get $ret 0
		incr i
	}

	puts "\tEnv$tnum.c: Close and reopen the environment and database."
	error_check_good db_close [$db close] 0
	error_check_good dbenv_close [$dbenv close] 0
	set dbenv [eval {berkdb_env_noerr} -txn  \
	    -home $testdir -failchk -isalive my_isalive]
	error_check_good envopen [is_valid_env $dbenv] TRUE
	set db [eval {berkdb_open_noerr} -btree -env $dbenv $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	
	puts "\tEnv$tnum.d: Run tests where failchk returns RUNRECOVERY."
	set i 1
	set getData 0
	set hooks { "exc_latch" "exc_mutex" "failchk" }
	foreach hook $hooks {
		puts "\t\tEnv$tnum.d$i: Run put process and crash it at $hook."
		set p3 [exec $tclsh_path $test_path/wrap.tcl env030script.tcl \
		    $testdir/env$tnum.log.p3.$i ALLOW_PIPE_CLOSE_ERROR \
		    $testdir $testfile PUT $key $data $hook &]
		watch_procs $p3 1 120
		set ret [catch { $dbenv failchk } res]
		error_check_good recover [is_substr $res "DB_RUNRECOVERY"] 1
		incr i

		puts "\t\tEnv$tnum.d$i: Reopen with recovery."
		set dbenv [eval {berkdb_env_noerr} -create -txn \
		    -recover -home $testdir -isalive -my_isalive]
		error_check_good envreopen [is_valid_env $dbenv] TRUE
		set db [eval {berkdb_open_noerr} \
		    -create -btree -env $dbenv $testfile]
		error_check_good dbreopen [is_valid_db $db] TRUE
		set ret [eval {$db get} $key]
	    	error_check_good db_get [lindex [lindex $ret 0] 1] $data
		incr i
	}

	error_check_good db_close [$db close] 0
	error_check_good dbenv_close [$dbenv close] 0
}
