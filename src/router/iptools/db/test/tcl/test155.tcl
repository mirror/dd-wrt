# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	test155
# TEST	Make sure DB_AFTER works properly after DBcursor->next(DB_MULTIPLE_KEY)
# TEST	has returned the last item.
proc test155 { method args } {
	source ./include.tcl

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	puts "Test155: Test of DB_AFTER after DB_MULTIPLE_KEY returns the last item."
	if { [is_rrecno $method] == 0 &&\
	    [is_btree $method] == 0 &&\
	    [is_hash $method] == 0 } {
		puts "Test155: skipping for method $method."
		return
	}
	if { [lsearch -exact $args "-compress"] != -1 } {
		puts "Test155: skipping for compressed database with method $method."
		return
	}

	set txn ""
	set dupflags ""
	
	if { [is_btree $method] == 1 || [is_hash $method] == 1 } {
		set dupflags " -dup"
	}

	puts "\tTest155.a: Create $method database."
	set txnenv 0
	set eindex [lsearch -exact $args "-env"]
	#
	# If we are using an env, then testfile should just be the db name.
	# Otherwise it is the test directory and the name.
	if { $eindex == -1 } {
		set testfile $testdir/test155.db
		set env NULL
	} else {
		set testfile test155.db
		incr eindex
		set env [lindex $args $eindex]
		set txnenv [is_txnenv $env]
		if { $txnenv == 1 } {
			append args " -auto_commit "
		}
		set testdir [get_home $env]
	}
	cleanup $testdir $env

	set oflags "-create -mode 0644 $args $omethod"
	set db [eval {berkdb_open} $oflags $dupflags $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE

	# Insert one item
	puts "\tTest155.b: insert one key/data pair."
	if { $txnenv == 1 } {
		set t [$env txn]
		error_check_good txn [is_valid_txn $t $env] TRUE
		set txn "-txn $t"
	}
	set ret [eval {$db put} $txn {1 "data"}]
	error_check_good dbput $ret 0
	if { $txnenv == 1 } {
		error_check_good txn [$t commit] 0
	}

	# Open cursor to database.
	if { $txnenv == 1 } {
		set t [$env txn]
		error_check_good txn [is_valid_txn $t $env] TRUE
		set txn "-txn $t"
	}
	set dbc [eval {$db cursor} $txn]
	error_check_good db_cursor [is_valid_cursor $dbc $db] TRUE

	puts "\tTest155.c: get the pair with multi_key."
	set d [$dbc get -next -multi_key 65536]

	puts "\tTest155.d: put with DB_AFTER."
	set ret [$dbc put -after "data"]

	if { [is_rrecno $method] == 1 } {
		error_check_good dbcput:after $ret 2
	} else {
		# It's OK for Hash database to return DB_NOTFOUND.
		if { [is_hash $method] == 1 && [is_substr $ret "DB_NOTFOUND"] == 1 } {
			set ret 0
		}
		error_check_good dbcput:after $ret 0
	}

	error_check_good dbc_close [$dbc close] 0
	if { $txnenv == 1 } {
		error_check_good txn [$t commit] 0
	}
	error_check_good db_close [$db close] 0
}
