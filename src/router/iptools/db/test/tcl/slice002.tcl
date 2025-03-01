# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	slice002
# TEST	Test transactional operations on sliced databases.
# TEST  Test that transactions behave as expected when:
# TEST  1. Auto commit is used.
# TEST  2. Transactions cover only a single operation each.
# TEST  3. Transactions cover multiple operations on the same key.
# TEST  4. Transactions cover multiple operations on different keys.
# TEST  5. Transactions cover multiple operations on different slices.
# TEST  Do this when both committing and aborting the transaction.
proc slice002 { } {
	source ./include.tcl
	
    	puts "Slice002: Test transactional operations on slices."
	# Make the local directory paths
	set num_slices 2
	
	puts "\tSlice002.a: Open the environment."
	slice_db_config $num_slices
	set dbenv [berkdb_env_noerr -create -txn -home $testdir]
	error_check_good sliced_env_open [is_valid_env $dbenv] TRUE

	set methods {-btree -hash}
	set dbname "sliced.db"

	foreach meth $methods {
	puts "\tSlice002: Create a database $meth."
		set t [$dbenv txn]
		set txn " -txn $t "
		set db [eval {berkdb_open_noerr -create \
		    -sliced} $txn {-env $dbenv $meth $dbname}]
		error_check_good db_open [is_valid_db $db] TRUE
		error_check_good open_txn_commit [$t commit] 0
		set key0 0
		set data0 1
		set key1 1
		set data1 2
		puts "\tSlice002: Insert some data."
		error_check_good put_key0 [$db put $key0 $data0] 0
		error_check_good put_key1 [$db put $key1 $data1] 0
		test_txn $testdir $dbenv $db

		puts "\tSlice002: Clean up the database."
		error_check_good db_close [$db close] 0
		error_check_good dbenv_remove [$dbenv dbremove $dbname] 0
	}
	error_check_good dbenv_close [$dbenv close] 0
	env_cleanup $testdir
}

proc test_txn { testdir dbenv db } {
	puts "\t\tSlice002: Test transactional behavior on the sliced database."
	# Have the transaction cover a single operation, several
	# operations on the same key, several operations on different
	# keys in the same slice, and several operations over different
	# slices (will always fail in slices version 1).
	set txn_types { single same_key multi_key multi_slice }
	foreach txn_type $txn_types {
		# Test with resolving the transaction abort and commit
		# unless the transaction type is multi_slice, in that
		# case only test abort since the operation will fail.
		set resolve_types {abort}
		if { $txn_type != "multi_slice"} {
			lappend resolve_types "commit"
		}
		# Only test autocommit if using single operation
		# transactions.
		if { $txn_type eq "single" } {
			lappend resolve_types "auto"
		}
		# The key values used for each transaction type is as 
		# follows:
		# Single operation, and multi operation same key
		# put: 2
		# get: 2
		# del: 2
		# Multi operation, different keys on same slice
		# put: 2
		# get: 0
		# del: 2
		# Multi operation, different slices
		# put: 2
		# get: 2
		# del: 1
		set put_key 2
		set put_data 2
		set get_key 2
		set del_key 2
		if { $txn_type eq "multi_slice" } {
			set del_key 1
		}
		if { $txn_type eq "multi_key" } {
			set get_key 0
		}
		foreach res_type $resolve_types {
puts "\t\tSlice002: Using txn type $txn_type, and resolve type $res_type"
			set t ""
			set txn ""
			if { $res_type != "auto" } {
				puts "\t\t\tSlice002: Create an explicit txn."
				set t [$dbenv txn]
				set txn " -txn $t "
			} else {
				puts "\t\t\tSlice002: Use autocommit."
			}
			puts "\t\t\tSlice002: Do a put."
			error_check_good slice_put \
			    [eval {$db put} $txn {$put_key $put_data}] 0
			if { $txn_type eq "single" && $res_type != "auto"} {
				puts "\t\t\tSlice002: Commit the put txn."
				# Commit the put, since the get and del
				# may depend on it.
				error_check_good put_commit [$t commit] 0
				set t [$dbenv txn]
				set txn " -txn $t "
			}
			puts "\t\t\tSlice002: Do a get."
			set result {{2 2}}
			if { $txn_type eq "multi_key" } {
				set result {{0 1}}
			}
			error_check_good slice_get \
			    [eval {$db get} $txn {$get_key}] $result
			if { $txn_type eq "single" && $res_type != "auto" } {
			puts "\t\t\tSlice002: Resolve the get txn with $res_type."
				error_check_good \
				    get_$res_type [$t $res_type] 0
				set t [$dbenv txn]
				set txn "-txn $t "
			}
			puts "\t\t\tSlice002: Do a del."
			set ret [catch {eval \
			    {$db del} $txn {$del_key} } msg]
			if { $txn_type eq "multi_slice" } {
	puts "\t\t\tSlice002: Fail because the txn touched more than one slice."
				error_check_good slice_error $ret 1
				set err_msg \
	    "*A transaction tried to access a second slice*"
				error_check_good err_msg \
	    			    [string match $err_msg $msg] 1
			} else {
				error_check_good slice_del $ret 0
			}
			if { $res_type != "auto" } {
			puts "\t\t\tSlice002: Resolve the txn with $res_type."
				error_check_good \
				    del_$res_type [$t $res_type] 0
			}

			# Repeat the operations with a cursor if not doing
			# single operation transactions or same key
			if { $txn_type eq "single" \
			    || $txn_type eq "same_key" } {
				continue
			}
	       	puts "\t\t\tSlice002: Create a cursor and get, put, del."
			set t [$dbenv txn]
			set txn " -txn $t "
			error_check_good put_key2 \
			    [eval {$db put} $txn {$put_key $put_data}] 0
			error_check_good txn_commit [$t commit] 0
			set t [$dbenv txn]
			set txn " -txn $t "
			set dbc [eval {$db cursor} $txn]
			error_check_good cursor_get \
			    [$dbc get -first] {{0 1}}
			error_check_good cursor_get \
			    [$dbc get -next] {{2 2}}
			error_check_good cursor_put \
			    [$dbc put -current $put_data] 0
			error_check_good cursor_del [$dbc del] 0
			if { $txn_type eq "multi_slice" } {
				set ret [catch {eval $dbc get -next} msg]
				error_check_good cursor_err $ret 1
				set err_msg \
	    "*A transaction tried to access a second slice*"
				error_check_good cursor_msg \
	    			    [string match $err_msg $msg] 1
				
			}
			puts "\t\t\tSlice002: Resolve cursor with $res_type."
			error_check_good cursor_close [$dbc close] 0
			error_check_good cursor_$res_type [$t $res_type] 0
		}
	}
}
