# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST  env023
# TEST  Test db_deadlock options. For each option, generate a deadlock
# TEST  then call db_deadlock.
proc env023 { } {
	source ./include.tcl
	global EXE

	set envargs " -log -txn -txn_timeout 15000000"
	puts "Env023: test with args:($envargs)."
	
	set deadlock_args "-h $testdir"
	
	set binname db_deadlock
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}
	
	set flaglist [list "" "-a e" "-a m" "-a n" "-a o" "-a W" "-a w" "-a y"]
	foreach flag $flaglist {
		append flag " $deadlock_args"
		env023_test_deadlock $envargs "$binname $flag $std_redirect"
		# While running in verbose mode, these messages are expected:
		# lt-db_deadlock: BDB5102 running at Mon Aug  5 14:53:00 2013
		# lt-db_deadlock: BDB5103 rejected 0 locks
		env023_test_deadlock $envargs "$binname -v $flag $std_redirect"\
		    [list "BDB5102" "BDB5103"]
		# Enable log file.
		env023_test_deadlock $envargs \
		    "$binname -L dl.log $flag $std_redirect"
	}
}

proc env023_test_deadlock { envargs execmd {allowed_msgs ""} } {
	source ./include.tcl
	puts "\tEnv023: Test '$util_path/$execmd'"

	# Set up environment and home folder.
	env_cleanup $testdir
	set env [eval berkdb_env_noerr $envargs -create -home $testdir]

	set dba [eval {berkdb_open -env $env -auto_commit -create -btree\
	    -mode 0644 "env023dba.db"} ]
	error_check_good dbopen [is_valid_db $dba] TRUE

	set dbb [eval {berkdb_open -env $env -auto_commit -create -btree\
	    -mode 0644 "env023dbb.db"} ]
	error_check_good dbopen [is_valid_db $dbb] TRUE

	# Create a deadlock between two child processes.
	set pidlist [env023_gen_deadlock]
	if { $pidlist == "NULL" } {
		puts "FAIL: failed to produce deadlock."
		return
	}

	# Check logs of child processes, make sure no errors occurred.
	puts "\t\tEnv023.a: Checking logs of child processes."
	logcheck $testdir/env023txn1.log
	logcheck $testdir/env023txn2.log

	puts "\t\tEnv023.b: Deadlock is generated."

	if { [is_substr $execmd "-a e"] } {
		# 'db_deadlock -a e' is designed for timeout txn only.
		tclsleep 20
	}

	# Execute db_deadlock
	puts "\t\tEnv023.c: Executing db_deadlock."
	env023_execmd $execmd $allowed_msgs

	puts "\t\tEnv023.d: Wait for child processes to exit."
	watch_procs $pidlist 2 10
	# Wait for a while to make sure child processes are finished.
	tclsleep 10

	# Execute db_deadlock with no deadlock present.
	puts "\t\tEnv023.e: Executing db_deadlock again."
	env023_execmd $execmd $allowed_msgs

	# Wait for a while to make sure db_deadlock exit, so all test files
	# are not hold by any process on Windows.
	tclsleep 2
	puts "\t\tEnv023.f: Cleaning up."
	error_check_good db_close [$dba close] 0
	error_check_good db_close [$dbb close] 0
	error_check_good env_close [$env close] 0
}

proc env023_gen_deadlock {} {
	source ./include.tcl
	set pidlist {}
	
	# Release two child processes to start their transactions: modify 
	# dba/dbb then dbb/dba. Each child process will hold one db and wait
	# for another. That leads to a deadlock. While db_deadlock abort lock
	# operation in one process, 'pipe close' error will be occurred. Add
	# 'ALLOW_PIPE_CLOSE_ERROR' to ignore it.
	set p [exec $tclsh_path $test_path/wrap.tcl env023script_txn.tcl\
	    $testdir/env023txn1.log "ALLOW_PIPE_CLOSE_ERROR" "dba_first"\
	    $testdir &]
	lappend pidlist $p
	
	set p [exec $tclsh_path $test_path/wrap.tcl env023script_txn.tcl\
	    $testdir/env023txn2.log "ALLOW_PIPE_CLOSE_ERROR" "dbb_first"\
	    $testdir &]
	lappend pidlist $p

	tclsleep 10

	# Check pid again to make sure deadlock is produced.
	foreach pid $pidlist {
		if { [file exists $testdir/begin.$pid] == 0 } {
			puts "FAIL: process $pid is not started."
			return "NULL"
		}
		if { [file exists $testdir/end.$pid] != 0 } {
			puts "FAIL: process $pid is finished."
			return "NULL"
		}
	}

	return $pidlist
}

proc env023_execmd { execmd {expected_msgs ""} } {
	source ./include.tcl
	puts "\t\t\tEnv023: $util_path/$execmd"
	set result ""
	if { ![catch {eval exec $util_path/$execmd} result] } {
		return
	}
	# Check for errors.
	set result_lines [split $result "\n"]
	foreach result_line $result_lines {
		set error_unexpected 1
		foreach errstr $expected_msgs {
			if { [is_substr $result_line $errstr] } {
				set error_unexpected 0
				break
			}
		}
		if { $error_unexpected } {
			puts "FAIL: got $result while executing '$execmd'"
			break
		}
	}
}
