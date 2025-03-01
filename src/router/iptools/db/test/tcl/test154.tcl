# See the file LICENSE for redistribution information.
#
# Copyright (c) 2006, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	test154
# TEST	Test of multi-version concurrency control.
# TEST
# TEST	Test basic snapshot functionality, adding volume and
# TEST  multiple iterations to generate freeze/thaw behavior.

proc test154 { method {tnum "154"} args } {
	source ./include.tcl

	# This test needs its own env.
	set eindex [lsearch -exact $args "-env"]
	if { $eindex != -1 } {
		incr eindex
		set env [lindex $args $eindex]
		puts "Test$tnum skipping for env $env"
		return
	}

	# MVCC is not allowed with queue methods.
	if { [is_queue $method] == 1 } {
		puts "Test$tnum skipping for method $method"
		return
	}

	puts "\tTest$tnum ($method): MVCC with snapshot isolation:\
	    Freeze and thaw buffers."

	set args [convert_args $method $args]
	set omethod [convert_method $method]
	set encargs ""
	set args [split_encargs $args encargs]
	set pageargs ""
	split_pageargs $args pageargs
	set filename "test$tnum.db"

	# When native pagesize is small, we need to adjust the 
	# default numbers of locks and mutexes.
	set mutexargs ""
	set max_locks 2000
	set max_objects 2000
	set native_pagesize [get_native_pagesize]
	if { $native_pagesize < 2048 } { 
		set mutexargs "-mutex_set_max 40000"
		set max_locks 5000
		set max_objects 5000
	}

	# Increase cache size.
	set cachesize [expr 2 * 1024 * 1024]
	
	# Create transactional env.  Specifying -multiversion makes
	# all databases opened within the env -multiversion.
	env_cleanup $testdir
	puts "\tTest$tnum.a: Creating txn env."
	set env [eval {berkdb_env -cachesize "0 $cachesize 1"}\
	    -lock_max_locks $max_locks -lock_max_objects $max_objects\
	    -create -txn -multiversion $pageargs $encargs -home $testdir]
	error_check_good env_open [is_valid_env $env] TRUE

	# Open database.
	puts "\tTest$tnum.b: Creating -multiversion db."
	set db [eval {berkdb_open} \
	    -create -auto_commit -env $env $omethod $args $filename]
	error_check_good db_open [is_valid_db $db] TRUE

	# Go through a loop of starting a snapshot txn, making many
	# entries in a writer txn, and verifying that the snapshot 
	# does not see the writer's changes.  
	set iter 5
	set nentries 1000
	set did [open $dict]
	for { set i 0 } { $i < $iter } { incr i } {
		puts "\tTest$tnum.c1: Iteration $i: start snapshot transaction."
		set snap_count 0
		set t0 [$env txn -snapshot]
		set snapshot_txn "-txn $t0"	
		set snapshot_cursor [eval {$db cursor} $snapshot_txn]
	
		# Read through the db with a snapshot cursor to 
		# find out how many entries it sees.
		for { set dbt [$snapshot_cursor get -first] }\
		    { [llength $dbt] != 0 } { set dbt [$snapshot_cursor get -next] } { 
			incr snap_count
		}
		set before_writes $snap_count

		puts "\tTest$tnum.c2: Iteration $i: start writer transaction."
		set t1 [$env txn]
		set writer_txn "-txn $t1"
	
		# Enter some data using writer transaction. 
		set write_count 0
		while { [gets $did str] != -1 && $write_count < $nentries } {
                        if { [is_record_based $method] == 1 } {
                                set key [expr $i * $nentries + $write_count + 1]
                        } else {
                                set key $str
                        }

                        set ret [eval \
                            {$db put} $writer_txn {$key [chop_data $method $str]}]
                        error_check_good put $ret 0
                        incr write_count
		}

		# Snapshot txn counts its view again.
		set snap_count 0
		for { set dbt [$snapshot_cursor get -first] }\
		    { [llength $dbt] != 0 } { set dbt [$snapshot_cursor get -next] } { 
			incr snap_count
		}
		set after_writes $snap_count

		puts "\tTest$tnum.c3: Iteration $i: check data."
		# On each iteration, the snapshot will find more entries, but
		# not within an iteration.  
		error_check_good total_count $snap_count [expr $i * $write_count]
		error_check_good snap_count_unchanged $before_writes $after_writes

		error_check_good snapshot_txn_commit [$t0 commit] 0
		error_check_good writer_txn_commit [$t1 commit] 0
		
	}

	# Clean up.
	error_check_good db_close [$db close] 0
	error_check_good env_close [$env close] 0
}
