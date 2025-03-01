# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	bigfile003
# TEST	1. Create two databases.  One will hold a very large (5 GB)
# TEST	blob and the other a relatively small one (5 MB) to test some 
# TEST	functionality that is punishingly slow on the 5 GB blob.
# TEST	2. Add empty blobs.
# TEST	3. Append data into the blobs by database stream.
# TEST	4. Verify the blob size and data. For txn env, verify it with
# TEST	txn commit/abort.
# TEST	5. Verify getting the blob by database/cursor get method returns
# TEST	the error DB_BUFFER_SMALL.
# TEST	6. Run verify_dir and a regular db_dump on both databases.
# TEST	7. Run db_dump -r and -R on the small blob only.
# TEST
# TEST	This test requires a platform that supports 5 GB files and
# TEST	64-bit integers.
proc bigfile003 { args } {
	source ./include.tcl
	global databases_in_memory
	global is_fat32
	global tcl_platform

	if { $is_fat32 } {
		puts "Skipping Bigfile003 for FAT32 file system."
		return
	}
	if { $databases_in_memory } {
		puts "Skipping Bigfile003 for in-memory database."
		return
	}
	if { $tcl_platform(pointerSize) != 8 } {
		puts "Skipping bigfile003 for system\
		    that does not support 64-bit integers."
		return
	}
	
	# We need about 10 GB of free space to run this test
	# successfully. 
	set space_available [diskfree-k "$testdir"]
	if { [expr $space_available < 11000000] } {
		puts "Skipping bigfile003, not enough disk space."
		return
	}

	# args passed to the test will be ignored.
	foreach method { btree rbtree hash heap } {
		foreach envtype { none regular txn } {
			bigfile003_sub $method $envtype
		}
	}
}

proc bigfile003_sub { method envtype } {
	source ./include.tcl
	global alphabet

	cleanup $testdir NULL

	#
	# Set up the args and env if needed.
	# It doesn't matter what blob threshold value we choose, since the
	# test will create an empty blob and append data into blob by
	# database stream.
	#
	set args ""
	set bflags "-blob_threshold 100"
	set txnenv 0
	if { $envtype == "none" } {
		set testfile $testdir/bigfile003.db
		set testfile2 $testdir/bigfile003.2.db
		set env NULL
		append bflags " -blob_dir $testdir/__db_bl"
		#
		# Use a 50MB cache. That should be
		# manageable and will help performance.
		#
		append args " -cachesize {0 50000000 0}"
	} else {
		set testfile bigfile003.db
		set testfile2 bigfile003.2.db
		set txnargs ""
		if { $envtype == "txn" } {
			append args " -auto_commit "
			set txnargs "-txn"
			set txnenv 1
		}
		#
		# Use a 50MB cache. That should be
		# manageable and will help performance.
		#
		set env [eval {berkdb_env_noerr -cachesize {0 50000000 0}} \
		    -create -home $testdir $txnargs]
		error_check_good is_valid_env [is_valid_env $env] TRUE
		append args " -env $env"
	}

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	# No need to print the cachesize argument.
	set msg $args
	if { $env == "NULL" } {
		set indx [lsearch -exact $args "-cachesize"]
		set msg [lreplace $args $indx [expr $indx + 1]]
	}
	puts "Bigfile003: ($method $msg) Database stream test with 5 GB blob."

	puts "\tBigfile003.a: Create the blob databases."
	set db [eval {berkdb_open -create -mode 0644} \
	    $bflags $args $omethod $testfile]
	error_check_good dbopen [is_valid_db $db] TRUE
	set db2 [eval {berkdb_open -create -mode 0644} \
	    $bflags $args $omethod $testfile2]
	error_check_good dbopen [is_valid_db $db2] TRUE

	puts "\tBigfile003.b: Create empty blobs in each db."
	set txn ""
	if { $txnenv == 1 } {
		set t [$env txn]
		error_check_good txn [is_valid_txn $t $env] TRUE
		set txn "-txn $t"
	}
	set key 1
	if { [is_heap $omethod] == 1 } {
		set ret [catch {eval {$db put} \
		    $txn -append -blob {""}} key]
		set ret [catch {eval {$db2 put} \
		    $txn -append -blob {""}} key]
	} else {
		set ret [eval {$db put} $txn -blob {$key ""}]
		set ret [eval {$db2 put} $txn -blob {$key ""}]
	}
	error_check_good db_put $ret 0
	if { $txnenv == 1 } {
		error_check_good txn_commit [$t commit] 0
	}

	# Verify the blobs are empty.
	if { $txnenv == 1 } {
		set t [$env txn]
		error_check_good txn [is_valid_txn $t $env] TRUE
		set txn "-txn $t"
	}
	set dbc [eval {$db cursor} $txn]
	error_check_good cursor_open [is_valid_cursor $dbc $db] TRUE
	set ret [catch {eval {$dbc get} -set {$key}} res]
	error_check_good cursor_get $ret 0
	error_check_good cmp_data [string length [lindex [lindex $res 0] 1]] 0
	set dbc2 [eval {$db2 cursor} $txn]
	error_check_good cursor2_open [is_valid_cursor $dbc2 $db2] TRUE
	set ret [catch {eval {$dbc2 get} -set {$key}} res]
	error_check_good cursor2_get $ret 0
	error_check_good cmp2_data [string length [lindex [lindex $res 0] 1]] 0

	# Open the database stream.
	set dbs [$dbc dbstream]
	error_check_good dbstream_open [is_valid_dbstream $dbs $dbc] TRUE
	error_check_good dbstream_size [$dbs size] 0
	set dbs2 [$dbc2 dbstream]
	error_check_good dbstream2_open [is_valid_dbstream $dbs2 $dbc2] TRUE
	error_check_good dbstream2_size [$dbs2 size] 0

	puts "\tBigfile003.c: Append data to blobs with dbstream."
	flush stdout

	# Append 1 MB data into the big blob until it gets to 5GB.
	set basestr [repeat [repeat $alphabet 40] 1024]
	set largeblobsize [ expr 5 * 1024 ]
	fillblob $basestr $dbs $largeblobsize
	puts "\tBigfile003.c1: Large blob is complete."

	# Now the small blob file. 
	set smallblobsize 5
	fillblob $basestr $dbs2 $smallblobsize
	puts "\tBigfile003.c2: Small blob is complete."

	# If the txn is aborted, the blobs should still be empty.
	if { $txnenv == 1 } {
		# Close database streams and cursors before aborting.
		error_check_good dbstream_close [$dbs close] 0
		error_check_good cursor_close [$dbc close] 0
		error_check_good dbstream2_close [$dbs2 close] 0
		error_check_good cursor2_close [$dbc2 close] 0

		puts "\tBigfile003.c3: Abort the txn."
		error_check_good txn_abort [$t abort] 0

		# Open a new txn.
		set t [$env txn]
		error_check_good txn [is_valid_txn $t $env] TRUE
		set txn "-txn $t"

		puts "\tBigfile003.c4: Verify the blob is still empty."
		# Reopen both cursors and streams while we are here. 
		set dbc [eval {$db cursor} $txn]
		set dbc2 [eval {$db2 cursor} $txn]
		error_check_good cursor_open [is_valid_cursor $dbc $db] TRUE
		error_check_good cursor_open [is_valid_cursor $dbc2 $db2] TRUE

		set ret [catch {eval {$dbc get} -set {$key}} res]
		error_check_good cursor_get $ret 0
		error_check_good cmp_data \
		    [string length [lindex [lindex $res 0] 1]] 0

		set ret [catch {eval {$dbc2 get} -set {$key}} res]
		error_check_good cursor2_get $ret 0
		error_check_good cmp_data \
		    [string length [lindex [lindex $res 0] 1]] 0

		set dbs [$dbc dbstream]
		error_check_good dbstream_open \
		    [is_valid_dbstream $dbs $dbc] TRUE
		error_check_good dbstream_size [$dbs size] 0

		set dbs2 [$dbc2 dbstream]
		error_check_good dbstream2_open \
		    [is_valid_dbstream $dbs2 $dbc2] TRUE
		error_check_good dbstream2_size [$dbs2 size] 0

		puts "\tBigfile003.c5: Reappend 5 GB to the large blob."
		fillblob $basestr $dbs $largeblobsize
		puts "\tBigfile003.c5: Done."
		puts "\tBigfile003.c5: Reappend 5 MB to the small blob."
		fillblob $basestr $dbs2 $smallblobsize
		puts "\tBigfile003.c5: Done."
	}

	# Close the database stream and cursor.
	error_check_good dbstream_close [$dbs close] 0
	error_check_good cursor_close [$dbc close] 0
	error_check_good dbstream2_close [$dbs2 close] 0
	error_check_good cursor2_close [$dbc2 close] 0

	if { $txnenv == 1 } {
		puts "\tBigfile003.c6: Commit the txn."
		error_check_good txn_commit [$t commit] 0
	}

	puts "\tBigfile003.d1: Get blob by cursor get.  Should\
	    return DB_BUFFER_SMALL."
	# We test the large blob only, first with database get ...
	set ret [catch {eval {$db get $key}} res]
	error_check_bad db_get $ret 0
	error_check_good db_get [is_substr $res DB_BUFFER_SMALL] 1

	# ... and then with cursor get.
	set dbc [$db cursor]
	error_check_good cursor_open [is_valid_cursor $dbc $db] TRUE
	set ret [catch {eval {$dbc get} -set {$key}} res]
	error_check_bad cursor_get $ret 0
	error_check_good cursor_get [is_substr $res DB_BUFFER_SMALL] 1

	puts "\tBigfile003.d2: Getting the blob with -partial succeeds."
	set len [string length $basestr]
	set ret [eval {$db get -partial [list 0 $len]} $key]
	error_check_bad db_get [llength $ret] 0
	set data [lindex [lindex $ret 0] 1]
	error_check_good data_length [string compare $data $basestr] 0

	# Close the cursors and databases.  We haven't reopened 
	# the second cursor, so we don't need to close it.
	error_check_good cursor_close [$dbc close] 0
	error_check_good db_close [$db close] 0
	error_check_good db2_close [$db2 close] 0

	# Close the env if opened.
	if { $env != "NULL" } {
		error_check_good env_close  [$env close] 0
	}

	# Run verify_dir with nodump -- we do the dump by hand 
	# later in the test.
	puts "\tBigfile003.e: run verify_dir."
	error_check_good verify_dir \
	    [verify_dir $testdir "\tBigfile003.e: " 0 0 1 50000000] 0

	# Calling the standard salvage_dir proc creates very large 
	# dump files that can cause problems on some test platforms.  
	# Therefore we test the dump here, and we also test the 
	# -r and -R options on the smaller blob only. 

	puts "\tBigfile003.f: dump the database with various options."
	set dumpfile $testdir/bigfile003.db-dump
	set dumpfile2 $testdir/bigfile003.2.db-dump
	set salvagefile2 $testdir/bigfile003.2.db-salvage
	set aggsalvagefile2 $testdir/bigfile003.2.db-aggsalvage
	set utilflag "-b $testdir/__db_bl"

	# First do an ordinary db_dump.
	puts "\tBigfile003.f1: ([timestamp]) Dump 5 GB blob."
	set rval [catch {eval {exec $util_path/db_dump} $utilflag \
	    -f $dumpfile $testdir/bigfile003.db} res]
	error_check_good ordinary_dump $rval 0
	puts "\tBigfile003.f1: ([timestamp]) Dump complete."
	puts "\tBigfile003.f1: Dump 5 MB blob."
	set rval [catch {eval {exec $util_path/db_dump} $utilflag \
	    -f $dumpfile2 $testdir/bigfile003.2.db} res]
	error_check_good ordinary_dump2 $rval 0
	puts "\tBigfile003.f1: ([timestamp]) Dump complete."

	# Remove the dump files immediately to reuse the memory.
	fileremove -f $dumpfile
	fileremove -f $dumpfile2

	# Now the regular salvage.
	puts "\tBigfile003.f2: Dump -r on 5 MB blob."
	set rval [catch {eval {exec $util_path/db_dump} $utilflag -r \
	    -f $salvagefile2 $testdir/bigfile003.2.db} res]
	error_check_good salvage_dump $rval 0
	fileremove -f $salvagefile2

	# Finally the aggressive salvage.
	# We can't avoid occasional verify failures in aggressive
	# salvage.  Make sure it's the expected failure.
	puts "\tBigfile003.f3: Dump -R on 5 MB blob."
	set rval [catch {eval {exec $util_path/db_dump} $utilflag -R \
	    -f $aggsalvagefile2 $testdir/bigfile003.2.db} res]
	if { $rval == 1 } {
		error_check_good agg_failure \
		    [is_substr $res "DB_VERIFY_BAD"] 1
	} else {
		error_check_good aggressive_salvage $rval 0
	}
	fileremove -f $aggsalvagefile2
}

proc fillblob { basestr dbs megabytes } {

	set offset 0
	set delta [string length $basestr]
	set size 0
	set gb 0

	for { set mb 1 } { $mb <= $megabytes } { incr mb } {
		error_check_good dbstream_write \
		    [$dbs write -offset $offset $basestr] 0
		incr size $delta
		error_check_good dbstream_size [$dbs size] $size
		error_check_good dbstream_read \
		    [string compare $basestr \
		    [$dbs read -offset $offset -size $delta]] 0
		incr offset $delta
		if { [expr $mb % 1024] == 0 } { 
			incr gb
			puts "\t\tBigfile003: $gb GB added to blob"
		}
	}
}
