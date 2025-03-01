# See the file LICENSE for redistribution information.
#
# Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	test156
# TEST	Test byte order conversions.
proc test156 { method {nentries 10000} args } {
	global datastr
	global pad_datastr
	global passwd
	source ./include.tcl

	set args [convert_args $method $args]
	set omethod [convert_method $method]

	set txnenv 0
	set eindex [lsearch -exact $args "-env"]
	#
	# If we are using an env, then testfile should just be the db name.
	# Otherwise it is the test directory and the name.
	if { $eindex == -1 } {
		set testfile $testdir/test156.db
		set env NULL
		set home_arg ""
		set conv_args ""
	} else {
		set testfile test156.db
		incr eindex
		set env [lindex $args $eindex]
		set txnenv [is_txnenv $env]
		if { $txnenv == 1 } {
			append args " -auto_commit "
			#
			# If we are using txns and running with the
			# default, set the default down a bit.
			#
			if { $nentries == 10000 } {
				set nentries 100
			}
		}
		set testdir [get_home $env]
		set home_arg "-h $testdir"
		set conv_args "-env $env"
		if { [$env get_encrypt_flags] == "-encryptaes" } {
			set home_arg "$home_arg -P $passwd"
		}
	}

	set pindex [lsearch -exact $args "-partition"]
	if { $pindex != -1 } {
		incr pindex
		set part [lindex $args $pindex]
		set conv_args "$conv_args -partition {$part}"
	}
	set pindex [lsearch -exact $args "-partition_callback"]
	if { $pindex != -1 } {
		incr pindex
		set npart [lindex $args $pindex]
		incr pindex
		set pcallback [lindex $args $pindex]
		set conv_args "$conv_args -partition_callback $npart $pcallback"
	}

	set pwdindex [lsearch -exact $args "-encryptaes"]
	if { $pwdindex != -1 } {
		incr pwdindex
		set pwd [lindex $args $pwdindex]
		set conv_args "$conv_args -P $pwd"
		set home_arg "$home_arg -P $pwd"
	}

	# Create the database and open the dictionary
	puts "Test156: $method ($args) $nentries key <fixed data> pairs"
	cleanup $testdir $env
	set db [eval {berkdb_open \
	     -create -mode 0644} $args {$omethod $testfile}]
	error_check_good dbopen [is_valid_db $db] TRUE
	set did [open $dict]

	set pflags ""
	set gflags ""
	set txn ""
	set count 0
	set lorder [$db get_lorder]

	# Here is the loop where we put and get each key/data pair

	if { [is_record_based $method] == 1 } {
		append gflags "-recno"
	}
	set pad_datastr [pad_data $method $datastr]
	puts "\tTest156.a: put/get loop with native order $lorder"
	while { [gets $did str] != -1 && $count < $nentries } {
		if { [is_record_based $method] == 1 } {
			set key [expr $count + 1]
		} else {
			set key $str
		}
		if { $txnenv == 1 } {
			set t [$env txn]
			error_check_good txn [is_valid_txn $t $env] TRUE
			set txn "-txn $t"
		}
		set ret [eval {$db put} $txn $pflags {$key [chop_data $method $datastr]}]
		error_check_good put $ret 0
		if { $txnenv == 1 } {
			error_check_good txn [$t commit] 0
		}

		set ret [eval {$db get} $gflags {$key}]

		error_check_good get $ret [list [list $key [pad_data $method $datastr]]]
		incr count
	}
	close $did
	error_check_good db_close [$db close] 0

	puts "\tTest156.b: dump the database"
	set base_dump [exec $util_path/db_dump {*}$home_arg -k -p $testfile]

	# Now, convert the database to a different byte order
	convert_test $testfile [toggle_byteorder $lorder] $conv_args \
	    $args $home_arg $base_dump

	# Now, convert the database back to native byte order
	convert_test $testfile $lorder $conv_args \
	    $args $home_arg $base_dump
}

proc toggle_byteorder {lorder} {
	if {$lorder == 1234} {
		return 4321
	} else {
		return 1234
	}
}

proc get_byteorder {testfile db_args} {
	set db [berkdb_open {*}$db_args -rdonly -unknown $testfile]
	error_check_good dbopen_byteorder [is_valid_db $db] TRUE
	set lorder [$db get_lorder]
	error_check_good db_close_byteorder [$db close] 0
	return $lorder
}

proc convert_test {testfile lorder conv_args db_args home_args base_dump} {
	source ./include.tcl

	puts "\tTest156.c: convert the database to order $lorder"
	error_check_good convert \
	    [berkdb convert {*}$conv_args -order $lorder $testfile] 0

	# check that the byte order is indeed converted
	error_check_good convert_order \
	    [get_byteorder $testfile $db_args] $lorder
	# check partition files
	foreach pfile [glob -nocomplain $testdir/__dbp*] {
		error_check_good convert_partorder \
		    [get_byteorder $pfile ""] $lorder
	}

	puts "\tTest156.d: compare the converted database with the original"
	error_check_good convert_dump \
	    [exec $util_path/db_dump {*}$home_args -k -p $testfile] $base_dump
}
