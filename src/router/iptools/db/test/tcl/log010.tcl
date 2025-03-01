# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	log010
# TEST	Test of DB_LOG_NOSYNC
# TEST
# TEST	Configure a database with txn_nosync and log_nosync. 
# TEST	Set a very small log size so we'll easily create more 
# TEST  than one.  Verify that we have a small number of writes,
# TEST  and no log syncs at all.
proc log010 { { iter 500 } } {
	source ./include.tcl
	set testfile log010.db

	puts "Log010: DB_LOG_NOSYNC"
	env_cleanup $testdir

	# Make the configuration file to set log nosync.
	set cid [open $testdir/DB_CONFIG w]
	puts $cid "set_flags db_txn_nosync"
        puts $cid "log_set_config db_log_nosync"
        close $cid

	# Make the logs small so we will create more than one.
	# This will test whether switching to a new log file
	# generates a sync.
	set log_max 32768 

	# Open/create the region.
	puts "\tLog010.a: Create env and database with log nosync."
	set e [berkdb_env -create -home $testdir -txn -log_max $log_max]
	error_check_good env_open [is_valid_env $e] TRUE

	# Open/create database.
	set db [berkdb_open -env $e -auto_commit -create -btree $testfile]
	error_check_good db_open [is_valid_db $db] TRUE

	# Put some data.
	puts "\tLog010.b: Put $iter data items."
	for { set i 1 } { $i < $iter } { incr i } {
		$db put key$i data$i
	}
	set stat [$e log_stat]
	puts "\tLog010.c: Check log stats"
	foreach i $stat {
		set txt [lindex $i 0]
		if { [string equal $txt {Times log written}] == 1 } {
			set writes [lindex $i 1]
		}
		if { [string equal $txt {Times log flushed to disk}] == 1 } {
			set syncs [lindex $i 1]
		}

	}
	
	# We expect a small number of writes, and zero syncs.
	set max_writes [expr $iter / 50]
	error_check_good writes [expr $writes < $max_writes] 1 
	error_check_good syncs $syncs 0

	error_check_good db_close [$db close] 0
	error_check_good env_close [$e close] 0
}
