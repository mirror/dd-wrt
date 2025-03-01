# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST	env022
# TEST	Test db_archive and db_checkpoint with all allowed options.
proc env022 { } {
	source ./include.tcl
	global passwd
	global has_crypto

	set cases "nopasswd"
	if { $has_crypto == 1 } {
		lappend cases "passwd"
	}
	foreach case $cases {
		# Set up environment and home folder.
		env_cleanup $testdir
		if { $case == "nopasswd" } {
			puts "Env022.a: Test without password."
			set env [eval berkdb_env -create -home $testdir\
			    -log -txn]
		}
		if { $case == "passwd" } {
			puts "Env022.b: Test with password."
			set env [eval berkdb_env -create -home $testdir\
			    -log -txn -encryptaes $passwd]
		}
		error_check_good env_open [is_valid_env $env] TRUE
	
		env022_subtest $env
	
		error_check_good env_close [$env close] 0
	}
}

proc env022_subtest { env } {
	source ./include.tcl
	global passwd
	global EXE

	# archive_args contains arguments used in db_archive.
	set archive_args ""
	# chkpt_args contains arguments used with db_checkpoint.
	# We use -1 to force an immediate checkpoint, since
	# db_checkpoint normally waits for some log activity.
	set chkpt_args "-1 "

	set secenv 0
	set testdir [get_home $env]
	set secenv [is_secenv $env]
	set txnenv [is_txnenv $env]
	set testfile env022.db
	append archive_args "-h $testdir"
	append chkpt_args "-h $testdir"

	# Set up passwords.
	if { $secenv != 0 } {
		append archive_args " -P $passwd"
		append chkpt_args " -P $passwd"
	}

	puts "\tEnv022: Test of db_archive."

	# Create db and fill it with data.
	puts "\tEnv022: Preparing $testfile."
	set method "-btree"
	set db [eval {berkdb_open -create -mode 0644 } $method $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	error_check_good db_fill [populate $db $method "" 1000 0 0] 0
	error_check_good db_close [$db close] 0

	puts "\tEnv022: testing db_archive."

	set binname db_archive
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}

	# All remaining db_archive options.
	set flaglist [list "-a" "-d" "-l" "-s" ""]
	
	foreach flag $flaglist {
		if { $flag == "" } {
			env022_execmd "$binname $archive_args $std_redirect"
			continue
		}
		env022_execmd "$binname $flag $archive_args $std_redirect"
		# Test in verbose mode.
		env022_execmd "$binname $flag -v $archive_args $std_redirect"
	}

	# Print version number.
	env022_execmd "$binname -V $std_redirect"

	puts "\tEnv022: testing db_checkpoint."

	# Test db_checkpoint.
	set binname db_checkpoint
	if { $is_windows_test } {
		append binname $EXE
	}

	# All remaining db_checkpoint options.
	set flaglist [list "-k 512" "-L $testdir/chkpt.tmp" "-p 1" ""]
	
	foreach flag $flaglist {
		if { $flag == "" } {
			env022_execmd "$binname $chkpt_args $std_redirect"
			continue
		}
		env022_execmd "$binname $flag $chkpt_args $std_redirect"
		# Test in verbose mode.
		env022_execmd "$binname $flag -v $chkpt_args $std_redirect"\
		    [list "checkpoint begin" "checkpoint complete"]
	}

	# Print version number.
	env022_execmd "$binname -V $std_redirect"

	# Check usage info is contained in error message.
	set execmd "$util_path/$binname $std_redirect"
	puts "\tEnv022: $execmd"
	catch {eval exec [split $execmd " "]} result
	error_check_good db_load [is_substr $result "usage:"] 1
}

proc env022_execmd { execmd {expected_msgs ""} } {
	source ./include.tcl
	puts "\tEnv022: $util_path/$execmd"
	set result ""
	if { ![catch {eval exec $util_path/$execmd} result] } {
		return
	}
	# Check for errors.
	foreach errstr $expected_msgs {
		if { [is_substr $result $errstr] } {
			return
		}
	}
	puts "FAIL: got $result while executing '$execmd'"
}
