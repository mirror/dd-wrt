# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST	test153
# TEST	Test queue consuming on a database having holes.
# TEST
# TEST  Create a database and then do some put operations. We will commit
# TEST  most of the operations and abort some to create holes(A queue
# TEST  database that is missing some consecutive elements). Then we will
# TEST  consume all records in the database to verify it still works, and
# TEST  we will also verify there are no zombie(see below) extents left.

proc test153 {method {tnum "153"} args} {
	global fixed_len
	source ./include.tcl

	# This test runs only when given the "queue" method and
	# specifies its own extent size.  We skip the regular
	# "queueext" method because the default extent size is not
	# a particularly challenging test.
	if { [is_queue $method] != 1 || [is_queueext $method] == 1 } {
		puts "Skipping test$tnum for method $method."
		return
	}

	# Skip for specified pagesizes. This test uses a fixed pagesize
	# so that we can do computation before opening the database.
	set pgindex [lsearch -exact $args "-pagesize"]
	if { $pgindex != -1 } {
		puts "Test$tnum: Skipping for specific pagesizes"
		return
	}
	set pgsize 1024

	set eindex [lsearch -exact $args "-env"]
	set txnenv 0
	if { $eindex >= 0 }  {
		incr eindex
		set testfile test$tnum.db
		set env [lindex $args $eindex]
		set txnenv [is_txnenv $env]
	}
	# Since we have abort operation, we run the test in a txn env.
	if { $txnenv == 0 } {
		puts "Skipping test$tnum for non-env test or non-txn test"
		return
	}

	# We select the values carefully, so that the 'recpage', 'qext', 'unit'
	# are all prime numbers. Using prime numbers can make the abort
	# operations distribute better.
	set reclen 128
	set qext 31
	set recpage [expr $pgsize / ($reclen + 1)]
	set recext [expr $recpage * $qext]
	# A 'unit' means a group of put operations, and in this group we will
	# commit some and abort the others. For this test, logically we
	# consider 5 operations as a group, and in these 5 operations,
	# there are 4 commits and one abort.
	set unit 5
	# We want to test cases where final record is empty or not, so we
	# test different record counts and abort locations.
	set aposes [list [expr $unit / 2] [expr $unit - 1]]
	set basecnt [expr $recext * $unit]
	set nums [list [expr $basecnt - $recext / 2] $basecnt \
	    [expr $basecnt + $recext / 2]]
	set old_fixed_len $fixed_len
	set fixed_len $reclen
	set largs [convert_args $method $args]
	append largs " -pagesize $pgsize -extent $qext -auto_commit"

	set indx 0
	foreach inorder {0 1} {
		foreach nentries $nums {
			foreach apos $aposes {
				test153_sub "\tTest$tnum.$indx" $method \
				    $nentries $env $testfile $inorder \
				    $unit $apos $largs
				incr indx
		    }
	    }
	}
	set fixed_len $old_fixed_len

}

proc test153_sub { prefix method nentries tenv testfile inorder
    unit apos largs } {
	global fixed_len
	source ./include.tcl
	upvar recext recext

	set testdir [get_home $tenv]
	set dboargs $largs
	if { $inorder == "1" } {
		append dboargs " -inorder"
	}

	# For better coverage, we will make one extent empty.
	set bignum [expr ($nentries - 1) / $recext]
	# The number of the empty extent.
	set empext [berkdb random_int 0 $bignum]
	set omethod [convert_method $method]
	set data [repeat "a" $fixed_len]

	cleanup $testdir $tenv

	puts "$prefix.0: Open the database($dboargs)."
	set db [eval berkdb_open -create -mode 0644 \
	    $omethod $dboargs $testfile] 
	error_check_good dbopen [is_valid_db $db] TRUE

	puts "$prefix.1: Put records($nentries entries,\
	    $unit operations as a unit, abort at operation $apos,\
	    extent $empext empty)."
	set ccnt 0
	for { set i 0 } { $i < $nentries } { incr i } {
		# Calculate current extent number.
		set extnum [expr $i / $recext]
		set t [$tenv txn]
		error_check_good txn [is_valid_txn $t $tenv] TRUE
		set recno [$db put -txn $t -append $data]
		error_check_good dbput $recno [expr $i + 1]
		if { $i % $unit == $apos || $extnum == $empext} {
			error_check_good txn_abort [$t abort] 0
		} else {
			error_check_good txn_commit [$t commit] 0
			incr ccnt
		}
	}
	set cwd [pwd]
	cd $testdir
	set files [glob -nocomplain __dbq.$testfile.*]
	set last [expr [llength $files] - 1]
	# Verify the extent numbers are expected. As there
	# is no 'consume' action, all extents are left, even
	# if the extent is empty(no valid records).
	for { set i 0 } { $i <= $last } {incr i} {
		set indx [lsearch $files __dbq.$testfile.$i]
		error_check_bad check_indx $indx -1
	}
	cd $cwd

	puts "$prefix.2: Consume records."
	set gcnt 0
	while { 1 } {
		set ret [$db get -consume]
		if {[llength $ret] == 0} {
			break
		}
		error_check_good len [llength $ret] 1
		incr gcnt
	}
	error_check_good gcnt $ccnt $gcnt

	puts "$prefix.3: Check remaining files."
	cd $testdir
	set files [glob -nocomplain -directory $testdir __dbq.$testfile.*]
	cd $cwd

	# Since all records are consumed, if there are still extent files
	# left, there should be just one, and it must be the last one.
	# Only the last extent file is allowed to be empty(no valid records), 
	# while others are called zombie extents and we do not allow zombies
	# after a thorough consumption.
	if {[llength $files] == 1} {
		error_check_good check_file [lindex $files 0] \
		    __dbq.$testfile.$last
	} else {
		error_check_good check_files [llength $files] 0
	}

	error_check_good db_close [$db close] 0
}
