# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	memp008
# TEST  Test for MPOOL multi-process operation.
# TEST
# TEST  This test stress tests MPOOL by creating frozen buckets and 
# TEST  then resizing.

proc memp008 { } {
	source ./include.tcl

	puts "Memp008: Test MPOOL resizing and fsync."
	env_cleanup $testdir

	# Some code in mp_resize.c is targeted for those buckets in MVCC chain.
	# So we create a ENV with MVCC support here.
	set env [eval {berkdb_env -home $testdir -create -mode 0644\
	    -cachesize {0 100000 10} -multiversion -txn} ]
	error_check_good dbenv [is_valid_env $env] TRUE

	# This line will be omit until [#21769] is fixed.
	# memp008_frozen_buffer_test $env

	puts "\tMemp008.a: Create, fill and modify DB with MVCC support."

	set db [eval {berkdb_open -env $env -auto_commit -create -btree\
	    -mode 0644 "memp008.db"} ]
	error_check_good dbopen [is_valid_db $db] TRUE

	# Modify keys with different values in transaction to fill MVCC chain.
	for { set i 0 } { $i < 10 } { incr i } {
		for { set key 0 } { $key <= 100 } { incr key } {
			set t [$env txn]
			error_check_good filldata\
			    [$db put -txn $t $key [ expr $key + $i ] ] 0
			error_check_good txn [$t commit] 0
		}
	}

	set pidlist {}

	# Generate process to fsync.
	puts "\tMemp008.b: Spawn process for fsyncing MPOOL."
	set p [exec $tclsh_path $test_path/wrap.tcl \
	    memp008fsync.tcl $testdir/memp008.fsync.log &]
	lappend pidlist $p
	
	puts "\tMemp008.c: Resizing env cache while fsyncing mpool."
	memp008_resize_mpool $env

	puts "\tMemp008.d: Spawn process for filling MPOOL."
	set p [exec $tclsh_path $test_path/wrap.tcl \
	    memp008fill.tcl $testdir/memp008.filling.log &]
	lappend pidlist $p

	puts "\tMemp008.e: Wait for child processes to exit."
	watch_procs $pidlist 1

	puts "\tMemp008.f: Checking logs of child processes."
	logcheck $testdir/memp008.resize.log
	logcheck $testdir/memp008.fsync.log

	puts "\tMemp008.g: Cleaning up."
	error_check_good db_close [$db close] 0
	error_check_good env_close [$env close] 0
}

proc memp008_frozen_buffer_test { env } {
	source ./include.tcl

	# Create a DB with txn support.
	set t_db [eval {berkdb_open -env $env -auto_commit -create -btree\
	    -mode 0644 "testbh.db"} ]

	# Write data until some frozen buckets appear.
	puts "Memp008: Writing data until some frozen BHP appear..."
	set k 0
	set data 0
	while { 1 } {
		set t [$env txn]
		$t_db put -txn $t $k $data
		incr k
		$t commit

		# Check for frozen buckets.
		set ret 0
		set file_list [glob -nocomplain "$testdir/__db.freezer.*K"]
		if { [llength $file_list] > 2 } {
			puts "Memp008: Found more than two frozen buckets."
			break
		}

		if { $k > 500 } {
			set k 0
			incr data
			if { $data > 10 } {
				puts "FAIL: no frozen BHP appear."
				break
			}
		}
	}

	memp008_resize_mpool $env

	$t_db close

	memp008_resize_mpool $env
}

# Continuously vary the size of the cache between
# 60000 and 100000 until we run out of memory or
# we've looped more than the maximum allowed times.
proc memp008_resize_mpool { env } {
	set max_size 100000
	set min_size 60000
	set size $max_size
	set inc_step -10000
	for { set i 0 } { $i < 100 } { incr i } {
		set ret 0
		catch {eval "$env resize_cache {0 $size}"} ret
		if { $ret != 0 } {
			error_check_good resize_mp\
			    [is_substr $ret "not enough memory"] 1
			puts "FAIL: Not enough memory, loop count:$i"
			break
		}
		set size [expr $size + $inc_step]
		if { $size < $min_size } {
			set inc_step 10000
		} elseif { $size > $max_size } {
			set inc_step -10000
 		}
	}
}
