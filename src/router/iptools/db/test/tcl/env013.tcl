# See the file LICENSE for redistribution information.
#
# Copyright (c) 2005, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env013
# TEST	Test of basic functionality of fileid_reset.
# TEST
# TEST	Create a database in an env.  Copy it to a new file within
# TEST	the same env.  Reset the file id and make sure it has changed.
proc env013 { {args ""} } {
	source ./include.tcl
	global util_path
	global number_of_slices

	if { [llength $args] > 0 } {
		puts "Env013: Test fileid_reset, with $args."
	} else {
		puts "Env013: Test fileid_reset."
	}

	set testfile A.db
	set dupfile B.db
	set nentries 500
	set filenames "A B C D E"

	foreach lorder { 1234 4321 } {
		puts "\tEnv013.a: Creating env."
		env_cleanup $testdir
		set env [berkdb_env -create -home $testdir -txn]
		error_check_good dbenv [is_valid_env $env] TRUE

		# Open database A, populate and close.
		puts "\tEnv013.b: Creating database with lorder $lorder."
		if { [string match "*-sliced*" $args] == 0 } {
			foreach filename $filenames {
				set db [eval {berkdb_open \
				    -pagesize 8192 -env $env -auto_commit \
				    -btree -create -mode 0644 $testfile \
				    $filename} ]
				error_check_good dbopen [is_valid_db $db] TRUE
				set t [$env txn]
				error_check_good txn [is_valid_txn $t $env] TRUE
				for { set i 0 } { $i < $nentries } { incr i } {
					set key KEY.$i
					set data DATA.$i
					error_check_good db_put \
					    [$db put -txn $t $key $data] 0
				}
				error_check_good txn_commit [$t commit] 0
				error_check_good db_close [$db close] 0
			}
		} else {
			set db [eval {berkdb_open \
			    -pagesize 8192 -env $env -auto_commit \
			    -btree -create -sliced -mode 0644 $testfile } ]
			error_check_good dbopen [is_valid_db $db] TRUE
			for { set i 0 } { $i < $nentries } { incr i } {
				set key KEY.$i
				set data DATA.$i
				error_check_good db_put [$db put $key $data] 0
			}
			error_check_good db_close [$db close] 0
		}

		# Copy database file A to database file B for fileid testing.
		puts "\tEnv013.c: Copy database."
		file copy -force $testdir/$testfile $testdir/$dupfile
		if { [string match "*-sliced*" $args]  } {
			for { set i 0 } { $i < $number_of_slices } { incr i } {
				file copy -force \
				    $testdir/__db.slice00$i/$testfile \
				    $testdir/__db.slice00$i/$dupfile
			}
		}

		# Reset B's fileid and confirm the ID has changed.
		puts "\tEnv013.d: Resetting file id for copied database."
		error_check_good fileid_reset [$env id_reset $dupfile] 0
		set orig_id [getfileid $testdir/$testfile]
		puts "\tEnv013.d: orig: $orig_id"
		set new_id [getfileid $testdir/$dupfile]
		puts "\tEnv013.d: new: $new_id"
		error_check_bad id_changed $orig_id $new_id
		set nodump 0
		if { [string match "*-sliced*" $args]  } {
			set nodump 1
			for { set i 0 } { $i < $number_of_slices } { incr i } {
				set orig_id [getfileid \
				    $testdir/__db.slice00$i/$testfile]
				puts "\tEnv013.d$i: orig: $orig_id"
				set new_id [getfileid \
				    $testdir/__db.slice00$i/$dupfile]
				puts "\tEnv013.d$i: new: $new_id"
				error_check_bad id_changed$i $orig_id $new_id
			}
		}

		# Verify and open B.
		puts "\tEnv013.e: Verify and open database copy."
		if { !$is_hp_test || $number_of_slices == 0 } {
			error_check_good verify [verify_dir $testdir \
			    "\tEnv013.e: " 0 0 $nodump] 0
		}

		set db [eval {berkdb_open} -env $env  \
		    -auto_commit -btree -mode 0644 -rdonly $args $dupfile]
		error_check_good dup_open [is_valid_db $db] TRUE

		# Clean up.
		error_check_good db_close [$db close] 0
		error_check_good env_close [$env close] 0
	}
}

# Get file id number, identified as "uid" in db_stat.
proc getfileid { db } {
	global util_path

	set ret [exec $util_path/db_dump -da $db]
	set uidstart [string first "uid:" $ret]
	set uidend [string first "\tminkey:" $ret]
	set uid [string range $ret $uidstart $uidend]
	set uid [string trimright $uid]
	return $uid
}
