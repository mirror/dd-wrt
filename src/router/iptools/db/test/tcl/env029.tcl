# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env029
# TEST	Test the behavior of the slice specific functions and flags.
# TEST  There are two versions of this test, one that tests the
# TEST  functions when BDB was built with slices enabled, and one
# TEST  that tests them when they are not enabled.
proc env029 { } {
	source ./include.tcl
	env_cleanup $testdir
	if { [berkdb slice_enabled ] } {
		puts "Env029: Testing slice functions with slices enabled."
		env029_slice_enabled
	} else {
		puts "Env029: Testing slice functions with slices not enabled."
		env029_slice_not_enabled
	}
}

proc env029_slice_callback { key } {
    return [expr $key % 2]
}

proc env029_slice_enabled { } {
	source ./include.tcl

	set num_slices 2
	set err_msg "This environment was not configured with slices"

	puts "\tEnv029.a: Open a non-sliced environment."
	set dbenv [eval {berkdb_env_noerr -create -txn -home $testdir }]
	error_check_good env_open [is_valid_env $dbenv] TRUE
    
	puts "\tEnv029.b: Test get_slice_count on a non-slice env."
	error_check_good slice_count_0 [$dbenv get_slice_count] 0

	puts "\tEnv029.c: Test get_slices on a non-slice env."
	set ret [catch {$dbenv get_slices} msg]
	error_check_good get_slices_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.d Open a sliced database in a non-sliced environment."
	set ret [catch {berkdb_open_noerr \
	    -create -env $dbenv -sliced -btree test.db} msg]
	error_check_good db_open_slice_msg [is_substr $msg $err_msg] 1

	error_check_good env_close [$dbenv close] 0
	env_cleanup $testdir

	puts "\tEnv029.e: Open a sliced environment."
	set container {}
	set slice_all {}
	set slice0 "slice0"
	set slice1 "slice1"
    	set slice [list "0 home $slice0" "1 home $slice1"]
	slice_db_config $num_slices $container $slice_all $slice
	set dbenv [eval {berkdb_env_noerr -create -txn -home $testdir}]
	error_check_good sliced_env_open [is_valid_env $dbenv] TRUE

	puts "\tEnv029.f: Test get_slice_count on a sliced env."
	error_check_good get_slice_count [$dbenv get_slice_count] 2
	
	puts "\tEnv029.g: Test get_slices on a sliced env."
	set dbenvs [$dbenv get_slices]
	error_check_good num_slice_env [llength $dbenvs ] 2
	# Slice envss are of the form [container_name].slice[number]
    	set dbenv0 [lindex $dbenvs 0]
    	error_check_good valid_env0 [string match $dbenv.slice* $dbenv0] 1
	set env_home [$dbenv0 get_home]
	error_check_good env_home0 [string equal $env_home $testdir/$slice0] 1
	set dbenv1 [lindex $dbenvs 1]
 	error_check_good valid_env1 [string match $dbenv.slice* $dbenv1] 1
	set env_home [$dbenv1 get_home]
	error_check_good env_home1 [string equal $env_home $testdir/$slice1] 1

	puts "\tEnv029.h: Open a non-slice database in a sliced env."
	set db [eval {berkdb_open_noerr \
	    -create -env $dbenv -btree noslice.db}]
	error_check_good db_open [is_valid_db $db] TRUE

	set err_msg "This database was not opened for sliced access"
    
	puts "\tEnv029.i Test get_slices for the non-slice database."
	set ret [catch {$db get_slices} msg]
	error_check_good db_get_slices_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.j Test slice_lookup for the non-slice database."
	set ret [catch {$db slice_lookup 1} msg]
	error_check_good db_get_slices_msg [is_substr $msg $err_msg] 1

	error_check_good non_slice_db_close [$db close] 0

	puts "\tEnv029.k: Open a sliced database in a sliced env."
    	set txn [$dbenv txn]
	set db [eval {berkdb_open -create -sliced -slice_callback \
	    env029_slice_callback -txn $txn -env $dbenv -btree slice.db}]
	error_check_good db_open [is_valid_db $db] TRUE
    	error_check_good open_txn_commit [$txn commit] 0

	puts "\tEnv029.l: Test get_slices for a sliced database."
	set dbs [$db get_slices]
	error_check_good num_slice_db [llength $dbs] 2
    	# Slice databases are of the form [container_name].slice[number]
	set db0 [lindex $dbs 0]
	error_check_good valid_db0 [string match $db.slice* $db0] 1
	set db1 [lindex $dbs 1]
	error_check_good valid_db1 [string match $db.slice* $db1] 1

	puts "\tEnv029.m: Test slice_lookup for a sliced database."
	set key0 0
	set data0 1
	set key1 1
	set data1 2
   	set txn [$dbenv txn]
	error_check_good put_key0 [$db put -txn $txn $key0 $data0] 0
	set txn_stats [$dbenv txn_stat]
	set txn_index [lsearch $txn_stats "*Active slice txn ID*"]
	error_check_good txn_slices [expr $txn_index > 0 ] 1
    	error_check_good txn_commit [$txn commit] 0
	error_check_good put_key1 [$db put $key1 $data1] 0
	set db0 [$db slice_lookup $key0]
	set db1 [$db slice_lookup $key1]
	error_check_good valid_db_slice0 [string match $db.slice* $db0] 1
	error_check_good valid_db_slice1 [string match $db.slice* $db1] 1
	set data [$db0 get $key0]
    	error_check_good slice0_data0 $data [list [list $key0 $data0]]
	set data [$db1 get $key1]
    	error_check_good slice1_data1 $data [list [list $key1 $data1]]

	set slices [$dbenv get_slices]

	puts "\tEnv029.n: Test DBENV->set_flags applies to the slices too."
	# This just tests with DB_TXN_WRITENOSYNC, selected at random.
	error_check_good set_wrnosync [$dbenv set_flags -wrnosync on] 0
	error_check_match get_slice0_wrnosync \
		[[lindex $slices 0] get_flags] "-wrnosync"
	error_check_match get_slice1_wrnosync \
		[[lindex $slices 1] get_flags] "-wrnosync"

	puts "\tEnv029.o: Test DBENV->set_{err,msg}pfx applies to the slices too."
	set old_errpfx [$dbenv get_errpfx]
	error_check_good set_errpfx [$dbenv errpfx "slice-err"] 0
	error_check_good set_msgpfx [$dbenv msgpfx "slice-msg"] 0
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_match get_slice${i}_errpfx \
			[[lindex $slices $i] get_errpfx] "slice-err"
		error_check_match get_slice${i}_msgpfx \
			[[lindex $slices $i] get_msgpfx] "slice-msg"
	}
	# Change one slice, verify that the other is not changed.
	error_check_good set_slice1_msgpfx [[lindex $slices 1] msgpfx "slice1-msg"] 0
	error_check_match get_slice0_msgpfx \
		[[lindex $slices 0] get_msgpfx] "slice-msg"
	error_check_match get_slice1_msgpfx \
		[[lindex $slices 1] get_msgpfx] "slice1-msg"

	error_check_good restore_errpfx [$dbenv errpfx $old_errpfx] 0


	error_check_good sliced_db_close [$db close] 0
   	set txn [$dbenv txn]
	error_check_good remove_slice [$dbenv dbremove -txn $txn "slice.db"] 0
   	error_check_good remove_commit [$txn commit] 0
	error_check_good sliced_env_close [$dbenv close] 0
	file delete $testdir/DB_CONFIG
}

proc env029_slice_not_enabled { } {
	source ./include.tcl

	set err_msg "Library build did not include slice support"
	set num_slice 2
	slice_db_config $num_slice

	puts "\tEnv029.a: Open a sliced environment."
	set ret [catch {berkdb_env_noerr -create -txn -home $testdir} msg]
	error_check_good env_open_msg [is_substr $msg $err_msg] 1
	file delete $testdir/DB_CONFIG

	puts "\tEnv029.b: Open a non-sliced environment."
	set dbenv [eval {berkdb_env_noerr -create -txn -home $testdir }]
	error_check_good env_open [is_valid_env $dbenv] TRUE

	puts "\tEnv029.c: Test get_slice_count."
	set ret [catch {$dbenv get_slice_count} msg]
	error_check_good slice_count_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.d: Test get_slices."
	set ret [catch {$dbenv get_slices} msg]
	error_check_good get_slices_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.e Open a database with DB_SLICE."
	set ret [catch {berkdb_open_noerr \
	    -create -env $dbenv -sliced -btree test.db} msg]
	error_check_good db_open_slice_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.f Open a database with a slice callback."
	set ret [catch {berkdb_open_noerr -create -env \
	    $dbenv -btree -slice_callback env029_slice_callback test.db} msg]
	error_check_good db_open_slice_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.g Open a database without DB_SLICE."
	set db [eval {berkdb_open_noerr -create -env $dbenv -btree test.db}]
	error_check_good db_open [is_valid_db $db] TRUE

	puts "\tEnv029.h Test get_slices for the database."
	set ret [catch {$db get_slices} msg]
	error_check_good db_get_slices_msg [is_substr $msg $err_msg] 1

	puts "\tEnv029.i Test slice_lookup."
	set ret [catch {$db slice_lookup 1} msg]
	error_check_good db_get_slices_msg [is_substr $msg $err_msg] 1

	error_check_good db_close [$db close] 0
	error_check_good env_close [$dbenv close] 0
}
