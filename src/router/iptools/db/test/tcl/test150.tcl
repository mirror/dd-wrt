# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST	test150
# TEST	Test db_verify and db_log_verify with all allowed options.
proc test150 { method {tnum "150"} args } {
	source ./include.tcl
	global encrypt
	global passwd
	global EXE

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	# db_verify and db_log_verify do not support partition callback yet.
	set ptcbindex [lsearch -exact $args "-partition_callback"]
	if { $ptcbindex != -1 } {
		puts "Test$tnum: skip partition callback mode."
		return
	}

	# verify_args contains arguments used in db_verify.
	set verify_args ""
	# log_verify_args contains arguments used in db_log_verify.
	set log_verify_args ""

	# Set up environment and home folder.
	set env NULL
	set secenv 0
	set txnenv 0
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
		append verify_args "-h $testdir "
		append log_verify_args "-h $testdir "
	} else {
		set testfile $testdir/test$tnum.db
		set testfile2 $testdir/test$tnum.2.db
	}

	# Append password to args.
	if { $encrypt != 0 || $secenv != 0 } {
		append verify_args " -P $passwd"
		append log_verify_args " -P $passwd"
	}

	set allow_subdb 1
	if { [is_partitioned $args] == 1 || [is_queue $method] == 1 || \
	    [is_heap $method] == 1} {
		set allow_subdb 0
	}

	cleanup $testdir $env

	# Create db and fill it with data.
	set db [eval {berkdb_open -create -mode 0644} $args $omethod $testfile]
	error_check_good db_open [is_valid_db $db] TRUE

	set txn ""
	if { $txnenv == 1 } { 
		set txn [$env txn]
	}
	error_check_good db_fill [populate $db $method $txn 10 0 0] 0
	if { $txnenv == 1 } { 
		error_check_good txn_commit [$txn commit] 0
	}
	error_check_good db_close [$db close] 0

	if { $allow_subdb == 1 } {
		# Create db with a given name in another file.
		set dbname "test$tnum"
		set db [eval {berkdb_open -create -mode 0644} $args\
		    $omethod $testfile2 $dbname]
		error_check_good db_open [is_valid_db $db] TRUE
	
		if { $txnenv == 1 } { 
			set txn [$env txn]
		}
		error_check_good db_fill [populate $db $method $txn 10 0 0] 0
		if { $txnenv == 1 } { 
			error_check_good txn_commit [$txn commit] 0
		}
		error_check_good db_close [$db close] 0
	}

	puts "Test$tnum: $method ($args) testing db_verify."

	set binname db_verify
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}

	# Verify DB file.
	test150_execmd "$binname $verify_args $testfile $std_redirect"

	# Try again with quiet mode on.
	test150_execmd "$binname -q $verify_args $testfile $std_redirect"

	# Try again with no-locking mode on.
	test150_execmd "$binname -N $verify_args $testfile $std_redirect"

	# Try again with no-order check.
	test150_execmd "$binname -o $verify_args $testfile $std_redirect"

	# Try again with UNREF mode on.
	test150_execmd "$binname -u $verify_args $testfile $std_redirect"

	# Check usage info is contained in error message.
	set execmd "$util_path/$binname $std_redirect"
	puts "\tTest$tnum: $execmd"
	catch {eval exec [split $execmd " "]} result
	error_check_good db_verify [is_substr $result "usage:"] 1

	# Print version info.
	test150_execmd "$binname -V $std_redirect"

	# Continue test if ENV is log enabled.
	if { $env == "NULL" || ![is_logenv $env] } {
		return
	}

	puts "Test$tnum: $method ($args) testing db_log_verify."

	set binname db_log_verify
	if { $is_windows_test } {
		append binname $EXE
	}

	# Verify DB log file.
	test150_execmd "$binname $log_verify_args $std_redirect"

	# This one should be blocked until SR[#22136] is fixed.
#	if { $allow_subdb == 1 } {
#		# Verify DB with specified database file and database name.
#		test150_execmd "$binname $log_verify_args -D $dbname -d\
#		    $testfile2 $std_redirect"
#	}

	# Test with specified start LSN.
	set start_lsn "1/0"
	test150_execmd "$binname $log_verify_args -b $start_lsn $std_redirect"

	# Test with specified end LSN.
	set end_lsn "2/0"
	test150_execmd "$binname $log_verify_args -e $end_lsn $std_redirect"

	# Test with specified start timestamp
	set start_t 1350000000 
	set end_t 1450000000 
	test150_execmd "$binname $log_verify_args -s $start_t -z $end_t\
	    $std_redirect"

	# Test with specified cachesize.
	set cachesize 5
	test150_execmd "$binname -C $cachesize $log_verify_args $std_redirect"

	# Test with continue on error flag.
	test150_execmd "$binname -c $log_verify_args $std_redirect"

	# Show version number only.
	test150_execmd "$binname $log_verify_args -V $std_redirect"

	# Test with specified home folder.
	set tmphome "$testdir/temphome"
	if { ![file exists $tmphome] } {
		file mkdir $testdir/temphome
	}
	test150_execmd "$binname -H $tmphome $log_verify_args $std_redirect"

	# Test without acquiring shared region mutexes while running.
	test150_execmd "$binname -N $log_verify_args $std_redirect"

	# Check usage info is contained in error message.
	set execmd "$util_path/$binname -xxx $std_redirect"
	puts "\tTest$tnum: $execmd"
	catch {eval exec [split $execmd " "]} result
	error_check_good db_log_verify [is_substr $result "usage:"] 1

	# Print version.
	test150_execmd "$binname -V $std_redirect"
}

proc test150_execmd { execmd } {
	source ./include.tcl
	puts "\tTest150: $util_path/$execmd"
	if { [catch {eval exec $util_path/$execmd} result] } {
		puts "FAIL: got $result while executing '$execmd'"
	}
}
