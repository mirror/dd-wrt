# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	slice003
# TEST	Test associate and secondary index handling in a sliced environment.
proc slice003 { {dbtype btree} {num_slices 3} {nentries 2000} } {
	global dict
	global testdir
	set zero 0
	set one 1
	set dbname "slice.db"
	set secdbname "secon.db"

    	puts "Slice003: Test associate and secondary operations on slices."
	env_cleanup $testdir

	# This test uses no fancy DB_CONFIG settings, just $num_slices.
	slice_db_config $num_slices

	set dbenv [eval {berkdb_env_noerr -create -txn -home $testdir}]
	error_check_good sliced_env_open [is_valid_env $dbenv] TRUE

	puts "\tSlice003.a: Open and fill primary for $dbtype secondary index test."
	set t [$dbenv txn]  
	set pridb [eval {berkdb_open_noerr -create \
	    -sliced -txn $t -env $dbenv -$dbtype $dbname}]
	error_check_good db_open_primary [is_valid_db $pridb] TRUE
    	error_check_good open_txn_commit [$t commit] 0

	# Test that DB_ENV->set_flags is applied to the slices too.
	error_check_good set_wrnosync [$dbenv set_flags -wrnosync on] 0
	error_check_good get_slice0_wrnosync [$dbenv set_flags -wrnosync on] 0

	puts "\tSlice003.b: Opened $dbname, loading $nentries words"
	set did [open $dict]
	for { set n 0 } \
	    { [gets $did str] != -1 && $n < $nentries } \
	    { incr n } {
		set key $str
		gets $did datum
		set keys($n) $key
		set data($n) $datum 

		set ret [eval {$pridb put} {$key $data($n)}]
		error_check_good put($n) $ret 0
	}
	close $did
	set t [$dbenv txn]  

	puts "\tSlice003.c: Create a secondary index and auto-fill with data."
	set secdb [eval {berkdb_open_noerr -create \
	    -sliced -txn $t -env $dbenv -btree $secdbname}]
	error_check_good db_open_secondary [is_valid_db $secdb] TRUE
    	error_check_good open_txn_commit [$t commit] 0

	# Associate the secondary with -create to fill it with the reverse of
	# the primary's data.
	error_check_good db_associate \
	    [$pridb associate -create [callback_n 0] $secdb] 0

	# Check that the auto-fill happened.
	puts "\tSlice003.d: Verify the auto-filled data."
	check_secondaries $pridb [list $secdb] $n keys data "slice003.d.autofill"

	$secdb close
	$pridb close
	$dbenv close
}
