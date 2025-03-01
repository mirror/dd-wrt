# See the file LICENSE for redistribution information.
#
# Copyright (c) 2012, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr004
# TEST	Test the repmgr incoming queue limit.
# TEST
# TEST  Test that setting the repmgr incoming queue limit works.
# TEST  We create a master and a client, and set a small client
# TEST  incoming queue limit.  We verify this limit works on the
# TEST  client side for full and abbreviated internal init and 
# TEST  for regular processing.  In addition to the default case, 
# TEST  we will also test cases using bulk transfer and blob
# TEST  databases.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr004 { { niter 1000 } { tnum "004" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]
	
	foreach blob {0 1} {
		foreach bulk {0 1} {
			set opts ""
			if {$bulk} {
				append opts "-bulk "
			}
			if {$blob} {
				append opts "-blob "
			}
			puts "Repmgr$tnum ($method $opts):\
			    repmgr incoming queue limit test."
			repmgr004_sub $method $niter $tnum $bulk $blob $args
		}
	}
}

proc repmgr004_sub { method niter tnum use_bulk use_blob largs } {
	source ./include.tcl
	global rep_verbose
	global verbose_type
	global overflowword1
	global overflowword2
	global databases_in_memory
	global gigabyte
	global ipversion

	# Avoid using pure digit strings since pure digit strings can make 
	# 'Tcl_GetIntFromObj' run very slowly in '_CopyObjBytes', see 
	# lang/tcl/tcl_internal.c for information about '_CopyObjBytes'.
	set overflowword1 "abcdefghijklmn"
	set overflowword2 "opqrstuvwxyz"
	set nsites 2
	set gigabyte [expr 1024 * 1048576]
	set uint32max [expr 4 * $gigabyte - 1]
	# Defaults for incoming queue limit.
	set defgbytes 0
	set defbytes [expr 100 * 1048576]
	set smallgbytes 0
	set smallbytes [expr 10 * 1024]
	set tinygbytes 0
	set tinybytes 16
	# Short set_request times to encourage rerequests.
	set req_min 400
	set req_max 12800

	set times 1
	set test_proc "rep_test"
	set overflow 0 
	set blobargs ""
	if {$use_bulk || $use_blob} {
		set test_proc "rep_test_bulk"
		# The average word length in test/tcl/wordlist is 8 bytes.
		# The proc 'rep_test_bulk' will repeat the word 100 times
		# as the data.  So if we are not using overflow, the average 
		# data size per item is about 800 bytes.  Considering
		# the padding, fill factor and data header, to store this
		# key-data pair, we need about 1 kilobyte.
		#
		# When using bulk transfer, the bulk buffer is 1MB.  This
		# is hard coded in base replication currently.
		#
		# To drop incoming messages, we need at least 3 messages 
		# (1 in processing, 1 in the queue, and 1 to be dropped).
		# As 3 messages mean 3 megabytes, and as 3MB/1KB=3000,
		# we need to put at least 3000 key-data pairs each time.
		# As we have 4 steps, we need at least 12000 pairs, but the 
		# wordlist file has only 10001 distinct words.  Thus
		# for bulk transfer, we make the data overflow, so every data
		# will be 1MB as least, and we only need a few (4 is selected 
		# here) items in the wordlist file.
		#
		# For blob, as we want to put records bigger than the blob
		# threshold, using overflow is expected.
		#
		# To make sure there will be messages dropped, we will do the
		# put loop several iterations for bulk transfer.  So we set
		# times to be more than one.
		set overflow 1
		if {$use_bulk} {
			set niter 8
			set times 6
		} else {
			set niter 2
			set times 1
		}

		# For blobs, we select a threshold of 32KB.
		# This is equal to the default log buffer size, so we can
		# avoid creating too many small log records.
		if {$use_blob} {
			set blobargs "-blob_threshold 32768"
		}
	}

	# Small number of transactions to trigger rerequests.
	set small_iter 10

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " $verbargs -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	# Use a big cache size on the master so that the master can do 
	# operations fast.
	puts "\tRepmgr$tnum.a: Start the master."
	set mcacheargs "-cachesize {0 104857600 1}"
	set ma_envcmd "berkdb_env -create $verbargs \
	    -errpfx MASTER -home $masterdir \
	    -txn -rep -thread -event $blobargs $mcacheargs"
	set masterenv [eval $ma_envcmd]
	repmgr004_verify_config $masterenv $defgbytes $defbytes

	# Create the database here, because we will use the handle to
	# find the page size later.
	if { $databases_in_memory == 1 } {
		set testfile { "" "test.db" }
	} else {
		set testfile "test.db"
	}
	set oargs [convert_args $method $largs]
	set omethod [convert_method $method]
	set repdb [eval {berkdb_open_noerr -env $masterenv -auto_commit \
	    -create -mode 0644} $oargs $omethod $testfile]
	error_check_good reptest_db [is_valid_db $repdb] TRUE

	if {$use_bulk} {
		$masterenv rep_config {bulk on}
	}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	puts "\tRepmgr$tnum.a.1: Run some transactions on the master."
	set start 0
	if {$use_blob || $use_bulk} {
	        eval $test_proc $method $masterenv $repdb \
		    $niter $start $start $overflow $largs
	} else {
	        eval $test_proc $method $masterenv $repdb \
		    $niter $start $start 0 $largs
	}
	incr start $niter

	# Use 4 pages as the incoming queue limit.
	# Setting it too small can make the process very very slow,
	# while setting it too big will not drop messages.  For 
	# larger pagesizes use 2 pages.
	set initpgsz [stat_field $repdb stat "Page size"]
	if { [expr $initpgsz > 8192] == 1 } { 
		set initdbsz [expr $initpgsz * 2] 
	} else {
		set initdbsz [expr $initpgsz * 2]
	}
	set initdbgsz 0

	set cl_envcmd "berkdb_env -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread -event"
	# Test the internal init here.
	# When we open the client without "-recover", we are testing the full
	# internal init, and when we open the client with "-recover", we are
	# testing the abbreviated internal init.
	foreach opt {"" "-recover"} {
		set istr "b"
		set msg ""
		set inqgbytes $initdbgsz
		set inqbytes $initdbsz
		if {$opt == "-recover"} {
			set istr "c"
			set msg " with $opt"
			set inqgbytes $smallgbytes
			set inqbytes $smallbytes
		}
		puts "\tRepmgr$tnum.$istr: Start the client$msg."

		set clientenv [eval $cl_envcmd $opt]
		$clientenv rep_request $req_min $req_max
		$clientenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 1]] \
		    -remote [list $hoststr [lindex $ports 0]] \
		    -inqueue [list $inqgbytes $inqbytes] \
		    -start client
		repmgr004_verify_config $clientenv $inqgbytes $inqbytes

		# Sleep a while so that the master will send enough messages
		# to the client to fill the incoming queue.
		puts "\tRepmgr$tnum.$istr.1:\
		    Pause 20 seconds to fill the queue."
		tclsleep 20

		# We do the check before rep_flush since we don't want to
		# count the messages generated by rep_flush.
		puts "\tRepmgr$tnum.$istr.2: Check full queue drops messages."
		set c_drop [stat_field $clientenv \
		    repmgr_stat "Incoming messages discarded"]
		set c_event [$clientenv event_count incoming_queue_full]
		# puts "c_drop:$c_drop c_event:$c_event"
		# When we open the client without -recover, it is the time
		# the environment is created.  And when we open the client
		# with -recover, it clears the stat values.  So we just 
		# check if the value is >0 here.
		error_check_good check_cdrop [expr $c_drop > 0] 1
		error_check_good check_cevent [expr $c_event > 0] 1
		error_check_good check_ctwo [expr $c_drop >= $c_event] 1

		# Wait up to 200 seconds for the client to finish start up.
		repmgr004_await_startup_flush $masterenv $clientenv 200

		# Wait until the incoming queue is empty.
		await_condition {[stat_field $clientenv repmgr_stat \
		    "Incoming messages size (gbytes)"] == 0 && \
		    [stat_field $clientenv repmgr_stat \
		    "Incoming messages size (bytes)"] == 0} 100
		# Verify the incoming queue full event is turned on.
		set onoff [$clientenv repmgr_get_inqueue_fullevent]
		error_check_good event_onoff $onoff 1

		# Get the latest values for later check.
		set c_drop [stat_field $clientenv \
		    repmgr_stat "Incoming messages discarded"]
		set c_event [$clientenv event_count incoming_queue_full]
		# puts "new_c_drop:$c_drop new_c_event:$c_event"

		puts "\tRepmgr$tnum.$istr.3:\
		    Close the client and run transactions on the master."
		$masterenv repmgr -ack none
		error_check_good client_close [$clientenv close] 0
		for {set i 0} {$i < $times} {incr i} {
			if {$use_bulk || $use_blob} {
			        eval $test_proc $method $masterenv $repdb \
				    $niter $start $start $overflow $largs
			} else {
			        eval $test_proc $method $masterenv $repdb \
				    $niter $start $start 0 $largs
			}
			incr start $niter
		}
		$masterenv repmgr -ack all
	}

	# Test big gap between the master and the client.
	puts "\tRepmgr$tnum.d: Start the client again without -recover."
	set clientenv [eval $cl_envcmd]
	$clientenv rep_request $req_min $req_max
	$clientenv repmgr -ack all -start client

	# Sleep a while to make sure the connection is established between
	# the master and the client.
	tclsleep 3

	#
	# We need a few more transactions to clear out the congested input
	# queues and predictably process all expected rerequests.  We can't
	# use repmgr heartbeats for automatic rerequests because they could
	# cause an election when the queues are full.  Base replication
	# rerequests are triggered only by master activity.
	#
	# We do not use rep_flush here since rep_flush does not
	# make the messages on the client drop as reliably as we expect.
	#
	puts "\tRepmgr$tnum.d.1:\
	    Trigger rerequests with some more small transactions on the master."
	eval rep_test $method $masterenv $repdb \
	    $small_iter $start $start 0 $largs
	incr start $small_iter

	# Wait until the incoming queue is empty.
	await_condition {[stat_field $clientenv repmgr_stat \
	    "Incoming messages size (gbytes)"] == 0 && \
	    [stat_field $clientenv repmgr_stat \
	    "Incoming messages size (bytes)"] == 0} 100
	# Verify the incoming queue full event is turned on.
	set onoff [$clientenv repmgr_get_inqueue_fullevent]
	error_check_good event_onoff $onoff 1

	puts "\tRepmgr$tnum.d.2: Check full queue drops messages."
	set c_drop2 [stat_field $clientenv \
	    repmgr_stat "Incoming messages discarded"]
	set c_event2 [$clientenv event_count incoming_queue_full]
	# puts "c_drop2:$c_drop2 c_event2:$c_event2"
	error_check_good check_cdrop2 [expr $c_drop2 > $c_drop] 1
	error_check_good check_cevent2 [expr $c_event2 > 0] 1
	error_check_good check_ctwo2 [expr [expr $c_drop2 - $c_drop] >= \
	    $c_event2] 1

	# For regular processing, it is not reliable to get messages dropped
	# on SunOS, so we skip the regular processing on SunOS.
	if {$is_sunos_test} {
		puts "\tRepmgr$tnum.e:\
		    Skip tiny incoming queue test for SunOS."
		puts "\tRepmgr$tnum.e.1:\
		    Set the incoming queue size to be unlimited."
		# Pass 0 to set the incoming queue size to be unlimited.
		$clientenv repmgr -inqueue [list 0 0]
		repmgr004_verify_config $clientenv $uint32max \
		    [expr $gigabyte - 1]
		set c_drop3 $c_drop2
		set c_event3 $c_event2
	} else {
		puts "\tRepmgr$tnum.e: Test tiny incoming queue on the client."
		# Set the queue size even smaller to make sure there are 
		# messages dropped during regular replication processing.
		$clientenv repmgr -inqueue [list $tinygbytes $tinybytes]
		repmgr004_verify_config $clientenv $tinygbytes $tinybytes

		# Run transactions so that the client falls behind.
		puts "\tRepmgr$tnum.e.1:\
		    Run another set of transactions on the master."
		$masterenv repmgr -ack none
		for {set i 0} {$i < $times} {incr i} {
			if {$use_bulk || $use_blob} {
			        eval $test_proc $method $masterenv $repdb \
				    $niter $start $start $overflow $largs
			} else {
			        eval $test_proc $method $masterenv $repdb \
				    $niter $start $start 0 $largs
			}
			incr start $niter
		}
		$masterenv repmgr -ack all

		# Wait until the incoming queue is empty.
		await_condition {[stat_field $clientenv repmgr_stat \
		    "Incoming messages size (gbytes)"] == 0 && \
		    [stat_field $clientenv repmgr_stat \
		    "Incoming messages size (bytes)"] == 0} 100
		# Verify the incoming queue full event is turned on.
		set onoff [$clientenv repmgr_get_inqueue_fullevent]
		error_check_good event_onoff $onoff 1

		puts "\tRepmgr$tnum.e.2:\
		    Set the incoming queue size to be unlimited."
		# Pass 0 to set the incoming queue size to be unlimited.
		$clientenv repmgr -inqueue [list 0 0]
		repmgr004_verify_config $clientenv $uint32max \
		    [expr $gigabyte - 1]

		puts "\tRepmgr$tnum.e.3: Check full queue drops messages."
		set c_drop3 [stat_field $clientenv \
		    repmgr_stat "Incoming messages discarded"]
		set c_event3 [$clientenv event_count incoming_queue_full]
		#puts "c_drop3:$c_drop3 c_event3:$c_event3"
		error_check_good check_cdrop3 [expr $c_drop3 > $c_drop2] 1
		error_check_good check_cevent3 [expr $c_event3 > $c_event2] 1
		error_check_good check_ctwo3 [expr [expr $c_drop3 - \
		    $c_drop2] >= [expr $c_event3 - $c_event2]] 1

		puts "\tRepmgr$tnum.e.4:\
		    Clear queues with a few more transactions."
		eval rep_test $method $masterenv \
		    $repdb $small_iter $start 0 0 $largs
		incr start $small_iter

		# Wait until the incoming queue is empty.
		await_condition {[stat_field $clientenv repmgr_stat \
		    "Incoming messages size (gbytes)"] == 0 && \
		    [stat_field $clientenv repmgr_stat \
		    "Incoming messages size (bytes)"] == 0} 100
		# Verify the incoming queue full event is turned on.
		set onoff [$clientenv repmgr_get_inqueue_fullevent]
		error_check_good event_onoff $onoff 1

    	}

	puts "\tRepmgr$tnum.f: Run more transactions on the master."
	$masterenv repmgr -ack none
	for {set i 0} {$i < $times} {incr i} {
		if {$use_bulk || $use_blob} {
		        eval $test_proc $method $masterenv $repdb \
			    $niter $start $start $overflow $largs
		} else {
		        eval $test_proc $method $masterenv $repdb \
			    $niter $start $start 0 $largs
		}
		incr start $niter
	}
	$masterenv repmgr -ack all

	# Flush last LSN from the master until the client receives
	# all the log records and stores them in the log files.
	# We will wait up to 200 seconds.
	set mfile [stat_field $masterenv log_stat "Current log file number"]
	set moffset [stat_field $masterenv log_stat "Current log file offset"]
	set cfile [stat_field $clientenv log_stat "Current log file number"]
	set coffset [stat_field $clientenv log_stat "Current log file offset"]
	set i 0
	set maxcnt 200
	while {$cfile != $mfile || $coffset != $moffset} {
		$masterenv rep_flush
		tclsleep 1
		incr i
		if {$i >= $maxcnt} {
			break;
		}
		set cfile [stat_field \
		    $clientenv log_stat "Current log file number"]
		set coffset [stat_field \
		    $clientenv log_stat "Current log file offset"]
	}

	puts "\tRepmgr$tnum.f.1: Verify no more message discards."
	set c_drop4 [stat_field $clientenv \
	    repmgr_stat "Incoming messages discarded"]
	set c_event4 [$clientenv event_count incoming_queue_full]
	# puts "c_drop4:$c_drop4 c_event4:$c_event4"
	error_check_good c_nodrop4 [expr $c_drop4 == $c_drop3] 1
	error_check_good c_noevent4 [expr $c_event4 == $c_event3] 1
	# Verify the incoming queue full event is turned on.
	set onoff [$clientenv repmgr_get_inqueue_fullevent]
	error_check_good event_onoff $onoff 1

	# The client storing all the log records does not mean all
	# these log records have been applied.  The final ones may
	# be in the process of applying.  Wait 5 seconds to make 
	# sure they are all applied.
	tclsleep 5

	puts "\tRepmgr$tnum.g: Verify client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good repdb_close [$repdb close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

proc repmgr004_verify_config {testenv gbytes bytes} {
	global gigabyte
	set rdpercent 85
	set ovfl [expr $gigabyte * 4]

	# In Tcl-C layer, we use Tcl_NewLongObj to represent
	# values of u_int32_t to the user.  But on systems where
	# 'long' has the same size with 'u_int32_t', values >= 2G
	# will be represented as minus numbers.  So we need to
	# do special processing for minus numbers.

	# Verify the incoming queue limit values.
	set inqmax [$testenv repmgr_get_inqueue_max]
	set inqmaxgbytes [lindex $inqmax 0]
	set inqmaxbytes [lindex $inqmax 1]
	error_check_good check_inqmaxbytes $inqmaxbytes $bytes
	if {$inqmaxgbytes >= 0} {
		error_check_good check_inqmaxgbytes $inqmaxgbytes $gbytes
	} else {
		error_check_good check_inqmaxgbytes $inqmaxgbytes \
		    [expr $gbytes - $ovfl]
	}
       
	# Verify the red zone values.
	# The red zone is 85% of the incoming queue limit.
	set redzone_expect \
	    [expr ($gbytes * $gigabyte + $bytes) * $rdpercent / 100]
	set inqredzone [$testenv repmgr_get_inqueue_redzone]
	set redzone_gbytes [lindex $inqredzone 0]
	if {$redzone_gbytes < 0} {
		set redzone_gbytes [expr $ovfl + $redzone_gbytes]
	}
	set redzone_bytes [lindex $inqredzone 1]
	error_check_good check_redzone_bytes [expr $redzone_bytes >= 0 && \
	    $redzone_bytes < $gigabyte] 1
	set redzone_cur [expr $redzone_gbytes * $gigabyte + $redzone_bytes]
	error_check_good check_redzone $redzone_expect $redzone_cur
}

# Wait until start up is complete on the client side.
# We use rep_flush here instead of running small transactions on 
# the master to avoid creating new log records.  We will wait up to
# $maxcnt seconds for the client to complete the start up.
proc repmgr004_await_startup_flush  {masterenv clientenv maxcnt} {
	set i 0
	while {[stat_field $clientenv rep_stat "Startup complete"] == 0} {
		$masterenv rep_flush
		tclsleep 1
		incr i
		if {$i >= $maxcnt} {
			error_check_bad startup_complete [stat_field \
			    $clientenv rep_stat "Startup complete"] 0
			break
		}
	}
}
