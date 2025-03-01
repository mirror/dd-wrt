# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST	test151
# TEST	Test db_dump and db_load with all allowed options.
proc test151 { method {tnum "151"} args } {
	source ./include.tcl
	global encrypt
	global passwd
	global databases_in_memory
	global repfiles_in_memory
	global EXE

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	# db_dump and db_load do not support partition callback yet.
	set ptcbindex [lsearch -exact $args "-partition_callback"]
	if { $ptcbindex != -1 } {
		puts "Test$tnum: skip partition callback mode."
		return
	}

	# dump_args contains arguments used with db_dump.
	set dump_args ""
	# load/loadr_args contains arguments used in db_load and db_load -r.
	set load_args ""
	set loadr_args ""

	# Set up environment and home folder.
	set env NULL
	set secenv 0
	set txnenv 0
	set extent 0
	set chksum 0
	set dbremove_args ""
	set eindex [lsearch -exact $args "-chksum"]
	if { $eindex != -1 && $databases_in_memory == 0 &&\
	    $repfiles_in_memory == 0} {
		set chksum 1
	}
	set eindex [lsearch -exact $args "-extent"]
	if { $eindex != -1 } {
		set extent 1
	}
	set eindex [lsearch -exact $args "-env"]
	if { $eindex != -1 } {
		incr eindex
		set env [lindex $args $eindex]
		set testdir [get_home $env]
		set secenv [is_secenv $env]
		set txnenv [is_txnenv $env]
		if { $txnenv == 1 } {
			append args " -auto_commit "
		}
		set testfile test$tnum.db
		set testfile2 test$tnum.2.db
		set testfile3 test$tnum.3.db
		set badfile wordlist
		append dump_args "-h $testdir"
		append loadr_args "-h $testdir"
		append dbremove_args "-env $env"
	} else {
		set testfile $testdir/test$tnum.db
		set testfile2 $testdir/test$tnum.2.db
		set testfile3 $testdir/test$tnum.3.db
		set badfile $testdir/wordlist
	}

	set eindex [lsearch -regexp $args "-encrypt.*"]
	if { $eindex != -1 } {
		set encrypt_flag [lindex $args $eindex]
		append dbremove_args " $encrypt_flag"
		if { $encrypt_flag != "-encrypt" } {
			incr eindex
			append dbremove_args " " [lindex $args $eindex]	
		}
	}

	# Under these circumstances db_dump will generate
	# a misleading error message.  Just skip testing
	# db_dump under run_repmethod. 
	if { $env != "NULL" && [is_repenv $env] == 1 } {
		puts "Test$tnum: skip test in rep environment."
		return
	}

	set dump_file "$testdir/test$tnum.dump"
	append load_args "-f $dump_file"

	# Set up passwords.
	if { $encrypt != 0 || $secenv != 0 } {
		append dump_args " -P $passwd"
		# Can not use chksum option when using a encrypted env.
		set chksum 0
	}

	set minkey 5
	if { [is_btree $method] == 1 } {
		append args " -minkey $minkey"
	}

	puts "Test$tnum: $method ($args) Test of db_dump."

	cleanup $testdir $env

	# Create db and fill it with data.
	puts "Test$tnum: Preparing $testfile."
	set db [eval {berkdb_open -create -mode 0644 } $args\
	    $omethod $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	set txn ""
	if { $txnenv == 1 } { 
		set txn [$env txn]
	}
	error_check_good db_fill [populate $db $method $txn 10 0 0] 0
	if { $txnenv == 1 } { 
		error_check_good txn_commit [$txn commit] 0
	}

	set stat [$db stat]
	set pgsize [get_pagesize $stat]
	error_check_bad get_pgsize $pgsize -1
	error_check_good db_close [$db close] 0

	set subdb 1
	if { [is_queue $method] == 1 || [is_heap $method] == 1 ||\
	    [is_partitioned $args] == 1} {
		set subdb 0
	}

	if { $subdb != 0 } {
		# Create a subdatabase, then fill it.
		puts "Test$tnum: Preparing $testfile2."
		set dbname "test$tnum"
		set db [eval {berkdb_open -create -mode 0644 }\
		    $args $omethod $testfile2 $dbname]
		error_check_good dbopen [is_valid_db $db] TRUE
		if { $txnenv == 1 } { 
			set txn [$env txn]
		}
		error_check_good db_fill [populate $db $method $txn 10 0 0] 0
		if { $txnenv == 1 } { 
			error_check_good txn_commit [$txn commit] 0
		}
		error_check_good db_close [$db close] 0
	}

	if { [is_hash $method] == 1 } {
		# Create a database with custom hash function, then fill it.
		puts "Test$tnum: Preparing $testfile3."
		set db [eval {berkdb_open -create -mode 0644 -hashproc test151_hash }\
		    $args $omethod $testfile3]
		error_check_good dbopen [is_valid_db $db] TRUE
		if { $txnenv == 1 } { 
			set txn [$env txn]
		}
		error_check_good db_fill [populate $db $method $txn 10 0 0] 0
		if { $txnenv == 1 } { 
			error_check_good txn_commit [$txn commit] 0
		}
		error_check_good db_close [$db close] 0
	}

	puts "Test$tnum: testing db_dump."

	set binname db_dump
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}

	# For flag -R, error with DB_VERIFY_BAD is allowed.
	test151_execmd "$binname -R $dump_args $testfile $std_redirect"\
	    [list "DB_VERIFY_BAD"]

	if { $subdb != 0} {
		# List databases stored in file.
		test151_execmd "$binname -l $dump_args $testfile2 $std_redirect"
	}

	# For hash database with custom hash functions, -v v flag returns
	# DB_VERIFY_BAD.
	if { [is_hash $method] == 1 } {
		test151_execmd "$binname -v v $dump_args $testfile3 $std_redirect"\
		    [list "DB_VERIFY_BAD"]
		# Remove the database so the general verification doesn't fail
		error_check_good dbremove [eval {berkdb dbremove} $dbremove_args\
		    $testfile3] 0
	}

	# wordlist is not a valid database file, error with DB_VERIFY_BAD is allowed.
	file copy $test_path/wordlist $testdir/wordlist
	test151_execmd "$binname -v o $dump_args $badfile $std_redirect"\
	    [list "DB_VERIFY_BAD"]

	# All remaining options.
	set flaglist [list "-d a" "-d h" "-d r" "-f $dump_file" "-N" "-p" "-r"\
	    "-k" "" "-v o" "-v v"]

	foreach flag $flaglist {
		test151_execmd "$binname $flag $dump_args\
		    $testfile $std_redirect"
		# Omit -r with a specified database name --
		# you cannot specify a database name when
		# attempting to salvage a possibly corrupt
		# database. 
		if { $flag == "-r" } {
			continue
		}
		if { $subdb != 0} {
			# Test flags with specified database name.
			test151_execmd "$binname $flag $dump_args\
			    -s $dbname $testfile2 $std_redirect"
		}
	}

	# Print version.
	test151_execmd "$binname -V $std_redirect"

	# Check usage info is contained in error message.
	set execmd "$util_path/$binname $std_redirect"
	puts "\tTest$tnum: $execmd"
	catch {eval exec [split $execmd " "]} result
	error_check_good db_dump [is_substr $result "usage:"] 1

	# Test db_load with dump file.
	set binname db_load
	if { $is_windows_test } {
		append binname $EXE
	}

	# Relative path of testfile3, which is the target file of db_load.
	# We need to put it in another folder in case of conflict with 
	# current environment.
	set loaddir $testdir/dbload
	file mkdir $loaddir
	set testfile3 $loaddir/test$tnum.3.db

	set flaglist [list "-c chksum=0" "-c db_pagesize=$pgsize"]
	# Omit page params for heap mode DB.
	if { [is_heap $method] == 1 } {
		set flaglist [list ""]
	}
	if { $chksum == 1 } {
		lappend flaglist "-c chksum=1"
	}
	if { $env != "NULL" } {
		append load_args " -h $loaddir"
		set testfile3 test$tnum.3.db
	}
	if { [big_endian] == 1 } {
		lappend flaglist "-c db_lorder=4321"
	} else {
		lappend flaglist "-c db_lorder=1234"
	}
	if { $extent == 1 } {
		lappend flaglist "-c extentsize=65536"
	}
	if { [is_queue $method] != 1 && [is_heap $method] != 1 &&\
	    $subdb == 1 } {
		lappend flaglist "-c database=test151"
		lappend flaglist "-c subdatabase=test151"
	}
       	if { [is_queue $method] == 1 } {
		lappend flaglist "-t queue"
	}
	if { [is_compressed $args] != 1 && [is_partitioned $args] != 1 } {
		if { [is_btree $method] == 1 || [is_hash $method] == 1 } {
			lappend flaglist "-c duplicates=1"
			lappend flaglist "-c duplicates=0"
			lappend flaglist "-c dupsort=1"
			lappend flaglist "-c dupsort=0"
		}
	}
	if { [is_btree $method] == 1 } {
		lappend flaglist "-c bt_minkey=$minkey"
		if { [is_partitioned $args] == 0 &&\
		    [is_compressed $args] == 0} {
			lappend flaglist "-c recnum=1"
			lappend flaglist "-c recnum=0"
		}
		lappend flaglist "-t btree"
	}
	if { [is_hash $method] } {
		lappend flaglist "-c h_ffactor=40"
		lappend flaglist "-c h_ffactor=60"
		lappend flaglist "-c h_ffactor=80"
		lappend flaglist "-c h_nelem=100"
		lappend flaglist "-c h_nelem=1000"
		lappend flaglist "-c h_nelem=10000"
		lappend flaglist "-t hash"
	}
	if { [is_queue $method] == 1 || [is_recno $method] == 1 } {
		lappend flaglist "-c keys=0"
		lappend flaglist "-c re_pad=."
		lappend flaglist "-c re_pad=%"
	}
	if { [is_recno $method] == 1 } {
		lappend flaglist "-c re_len=512"
		lappend flaglist "-c re_len=1024"
		lappend flaglist "-c re_len=2048"
		lappend flaglist "-c renumber=1"
		lappend flaglist "-c renumber=0"
		lappend flaglist "-t recno"
	}
	# Prepare a suitable DB file for testing load -r.
	lappend flaglist ""
	foreach flag $flaglist {
		# Clean up.
		env_cleanup $loaddir 
		test151_execmd "$binname $load_args $flag\
		    $testfile3 $std_redirect"
	}
	# For flag -r, db_load will reset lsn/fileid of existing db file.
	# This will lead to verification error in run_reptest.
	# Skip it in rep_env.
	if { $env != "NULL" && [is_txnenv $env] == 1 &&\
	    [is_repenv $env] == 0 } {
		test151_execmd "$binname -r lsn $loadr_args\
		    test$tnum.db $std_redirect"
		test151_execmd "$binname -r fileid $loadr_args\
		    test$tnum.db $std_redirect"
	}
	# Clean up.
	env_cleanup $loaddir 
	# For flag -n, error with 'key already exists' is allowed.
	test151_execmd "$binname $load_args -n $testfile3 $std_redirect"\
	    [list "key already exists"]
	# Clean up.
	env_cleanup $loaddir 

	# Print version.
	test151_execmd "$binname -V $std_redirect"

	# Check usage info is contained in error message.
	set execmd "$util_path/$binname $std_redirect"
	puts "\tTest$tnum: $execmd"
	catch {eval exec [split $execmd " "]} result
	error_check_good db_load [is_substr $result "usage:"] 1
}

proc test151_execmd { execmd {allowed_errs ""} } {
	source ./include.tcl
	puts "\tTest151: $util_path/$execmd"
	set result ""
	if { ![catch {eval exec $util_path/$execmd} result] } {
		return
	}
	# Check whether allowed errors occurred.
	foreach errstr $allowed_errs {
		if { [is_substr $result $errstr] } {
			return
		}
	}
	puts "FAIL: got $result while executing '$execmd'"
}

# Simple hash comparison.
proc test151_hash { a } {
	return 1
}
