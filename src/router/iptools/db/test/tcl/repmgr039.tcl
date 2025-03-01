# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr039
# TEST	repmgr duplicate master test.
# TEST
# TEST	This test verifies repmgr's automatic dupmaster resolution.  It
# TEST	uses the repmgr test hook to prevent sending heartbeats and
# TEST	2SITE_STRICT=off to enable the client to become a master in
# TEST	parallel with the already-established master.  After rescinding
# TEST	the test hook, it makes sure repmgr performs its dupmaster resolution
# TEST	process resulting in the expected winner.
# TEST	
# TEST	This test runs in the following configurations:
# TEST	    Default elections where master generation helps determine winner
# TEST	    The undocumented DB_REP_CONF_ELECT_LOGLENGTH election option
# TEST	    A Preferred Master replication group
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr039 { { niter 100 } { tnum "039" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	#
	# Run for the default case where master generation takes precedence
	# over log length for the election winner, and for the undocumented
	# option to base the election winner on log length without considering
	# the master generation.  Also run to test dupmaster operation in
	# preferred master mode.
	#
	# Add more data to one site or the other during the dupmaster.
	#
	set electopts { mastergen loglength prefmas }
	set moredataopts { master client }
	foreach e $electopts {
		foreach m $moredataopts {
			puts "Repmgr$tnum ($method $e $m): repmgr duplicate\
			    master test."
			repmgr039_sub $method $niter $tnum $e $m $args
		}
	}
}

proc repmgr039_sub { method niter tnum electopt moredataopt largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 2

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set hoststr [get_hoststr $ipversion]
	set ports [available_ports $nsites]

	# Heartbeat timeout values.
	set hbsend 500000
	set hbmon 1100000
	# Extra fast connection retry timeout for prompt dupmaster resolution.
	set connretry 500000
	set big_iter [expr $niter * 2]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	# Open a master.
	puts "\tRepmgr$tnum.a: Start master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread -event"
	set masterenv [eval $ma_envcmd]
	set role master
	if { $electopt == "loglength" } {
		$masterenv rep_config {electloglength on}
	}
	if { $electopt == "prefmas" } {
		# Both preferred master sites (master and client) must use
		# the -client option to start to allow the preferred master
		# startup sequence in the code to control which site becomes
		# master.
		set role client
		$masterenv rep_config {mgrprefmasmaster on}
	}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list heartbeat_send $hbsend] \
	    -timeout [list heartbeat_monitor $hbmon] \
	    -timeout [list connection_retry $connretry] \
	    -start $role
	if { $electopt != "prefmas" } {
		$masterenv rep_config {mgr2sitestrict off}
	}
	await_expected_master $masterenv

	# Open a client
	puts "\tRepmgr$tnum.b: Start client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread -event"
	set clientenv [eval $cl_envcmd]
	if { $electopt == "loglength" } {
		$clientenv rep_config {electloglength on}
	}
	if { $electopt == "prefmas" } {
		$clientenv rep_config {mgrprefmasclient on}
	}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list heartbeat_send $hbsend] \
	    -timeout [list heartbeat_monitor $hbmon] \
	    -timeout [list connection_retry $connretry] \
	    -start client
	if { $electopt != "prefmas" } {
		$clientenv rep_config {mgr2sitestrict off}
	}
	await_startup_done $clientenv

	#
	# Use of -ack all guarantees that replication is complete before the
	# repmgr send function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.c: Run first set of transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	# Set up expected winner and loser after the dupmaster.
	if { ($electopt == "loglength" && $moredataopt == "master") ||
	    $electopt == "prefmas" } {
		# For loglength, the master should win when it has more data.
		# For preferred master, the master's data is always retained.
		set winenv $masterenv
		set windir $masterenv
		set loseenv $clientenv
		set losedir $clientdir
	} else {
		# For mastergen, client always wins regardless of data size.
		# For loglength, the client should win when it has more data.
		set winenv $clientenv
		set windir $clientdir
		set loseenv $masterenv
		set losedir $masterdir
	}
	# Set up amount of data at each site during dupmaster.
	if { $moredataopt == "master" } {
		set m_iter $big_iter
		set c_iter $niter
	} else {
		set m_iter $niter
		set c_iter $big_iter
	}

	puts "\tRepmgr$tnum.d: Enable test hook to prevent heartbeats."
	$masterenv test abort repmgr_heartbeat
	$clientenv test abort repmgr_heartbeat
	#
	# Make sure client site also becomes a master.  This indicates
	# that we have the needed dupmaster condition.
	#
	await_expected_master $clientenv

	puts "\tRepmgr$tnum.e: Run transactions at each site, more on\
	    $moredataopt."
	eval rep_test $method $masterenv NULL $m_iter $start 0 0 $largs
	eval rep_test $method $clientenv NULL $c_iter $start 0 0 $largs
	incr start $big_iter

	if { $electopt == "prefmas" } {
		# Restart temporary master a varying number of times to test
		# the preferred master site's ability to catch up with multiple
		# temporary master generations.
		set num_restarts [berkdb random_int 0 3]
		puts "\tRepmgr$tnum.e1: Perform $num_restarts additional\
		    temporary master restart(s)."
		for { set i 0 } { $i < $num_restarts } { incr i } {
			error_check_good client_close [$clientenv close] 0
			set clientenv [eval $cl_envcmd]
			$clientenv rep_config {mgrprefmasclient on}
			$clientenv test abort repmgr_heartbeat
			$clientenv repmgr -ack all \
			    -local [list $hoststr [lindex $ports 1]] \
			    -remote [list $hoststr [lindex $ports 0]] \
			    -timeout [list heartbeat_send $hbsend] \
			    -timeout [list heartbeat_monitor $hbmon] \
			    -timeout [list connection_retry $connretry] \
			    -start client
			await_expected_master $clientenv
		}
		set loseenv $clientenv
	} else {
		# Depending on thread ordering, some reconnection and
		# dupmaster scenarios can have initial elections that don't
		# count both votes because one site still needs to update its
		# gen.  When this happens, the wrong site can win the election
		# with only its own vote unless we turn on 2site_strict.
		$masterenv rep_config {mgr2sitestrict on}
		$clientenv rep_config {mgr2sitestrict on}
	}

	puts "\tRepmgr$tnum.f: Rescind test hook to prevent heartbeats."
	$masterenv test abort none
	$clientenv test abort none
	#
	# Pause to allow time to for dupmaster to be noticed on both sites and
	# for the resulting election to occur.
	#
	tclsleep 3

	# Check for expected winner after the dupmaster resolution.
	await_expected_master $winenv
	await_startup_done $loseenv

	puts "\tRepmgr$tnum.g: Run final set of transactions at winner."
	eval rep_test $method $winenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.h: Verify dupmaster event on each site."
	# Needed to process some messages to see the dupmaster event.
	error_check_good dupmaster_event2 \
	    [is_event_present $masterenv dupmaster] 1
	error_check_good dupmaster_event \
	    [is_event_present $clientenv dupmaster] 1

	puts "\tRepmgr$tnum.i: Verify loser's database contents."
	rep_verify $windir $winenv $losedir $loseenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}
