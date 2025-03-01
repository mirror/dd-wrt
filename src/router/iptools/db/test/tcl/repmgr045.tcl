# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr045
# TEST	IPv6/IPv4 interoperability test.
# TEST
# TEST	This test ensures that a single IPv4 site can function as an
# TEST	appointed master, an elected master or a client in a replication
# TEST	group where the other sites use IPv6.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr045 { { niter 100 } { tnum "045" } args } {

	source ./include.tcl
	global ipversion

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	#
	# The use of IPv6 or IPv4 for each test site is governed by the
	# ipversion global variable.  By default, it is set for IPv6.
	# Skip this test if ipversion is set for IPv4 because in that
	# case all sites will use IPv4.
	#
	if { $ipversion == 4 } {
		puts "Skipping repmgr045 IPv6/IPv4 test when only using IPv4."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	#
	# This test creates three sites, each of which is run as the
	# lone IPv4 site in a separate loop iteration.  site1 starts as 
	# master but is then shut down and later rejoins the group as a
	# client.  site2 starts as a client but then wins an election to
	# become master when site1 is shut down.  site3 remains a client
	# for the entire test.
	#
	set v4opts { "site1" "site2" "site3" }
	foreach v4site $v4opts {
		puts "Repmgr$tnum ($method $v4site): \
		    repmgr IPv6/IPv4 interoperability test."
		repmgr045_sub $method $niter $tnum $args $v4site
	}
}

proc repmgr045_sub { method niter tnum largs v4site } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion
	set nsites 3

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	# Set site host strings according to v4site passed in.
	set site1host $hoststr
	set site2host $hoststr
	set site3host $hoststr
	if { $v4site == "site1" } {
		set site1host "127.0.0.1"
	} elseif { $v4site == "site2" } {
		set site2host "127.0.0.1"
	} else {
		set site3host "127.0.0.1"
	}

	set site1dir $testdir/SITE1DIR
	set site2dir $testdir/SITE2DIR
	set site3dir $testdir/SITE3DIR

	file mkdir $site1dir
	file mkdir $site2dir
	file mkdir $site3dir

	puts "\tRepmgr$tnum.a: Start site1 master and two clients."
	set s1_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx SITE1 -home $site1dir -txn -rep -thread"
	set site1env [eval $s1_envcmd]
	$site1env repmgr -ack all \
	    -local [list $site1host [lindex $ports 0]] \
	    -start master

	set s2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx SITE2 -home $site2dir -txn -rep -thread"
	set site2env [eval $s2_envcmd]
	$site2env repmgr -ack all -pri 80 \
	    -local [list $site2host [lindex $ports 1]] \
	    -remote [list $site1host [lindex $ports 0]] \
	    -remote [list $site3host [lindex $ports 2]] \
	    -start client
	await_startup_done $site2env

	set s3_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx SITE3 -home $site3dir -txn -rep -thread"
	set site3env [eval $s3_envcmd]
	$site3env repmgr -ack all -pri 70 \
	    -local [list $site3host [lindex $ports 2]] \
	    -remote [list $site1host [lindex $ports 0]] \
	    -remote [list $site2host [lindex $ports 1]] \
	    -start client
	await_startup_done $site3env

	#
	# Use of -ack all guarantees replication complete before repmgr send
	# function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.b: Run first set of transactions at master."
	set start 0
	eval rep_test $method $site1env NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.c: Verify client database contents."
	rep_verify $site1dir $site1env $site2dir $site2env 1 1 1
	rep_verify $site1dir $site1env $site3dir $site3env 1 1 1

	puts "\tRepmgr$tnum.d: Shut down master, elect site2 master."
	error_check_good master_close [$site1env close] 0
	await_expected_master $site2env
	await_startup_done $site3env

	puts "\tRepmgr$tnum.e: Run second set of transactions at new master."
	eval rep_test $method $site2env NULL $niter $start 0 0 $largs
	incr start $niter

	# Open -recover to clear env region, including startup_done value.
	puts "\tRepmgr$tnum.f: Restart old master to join as client."
	set site1env [eval $s1_envcmd -recover]
	$site1env repmgr -ack all \
	    -local [list $site1host [lindex $ports 0]] \
	    -start client
	await_startup_done $site1env

	puts "\tRepmgr$tnum.g: Verify old and new client database contents."
	rep_verify $site2dir $site2env $site3dir $site3env 1 1 1
	rep_verify $site2dir $site2env $site1dir $site1env 1 1 1

	error_check_good site1_close [$site1env close] 0
	error_check_good site3_close [$site3env close] 0
	error_check_good site2_close [$site2env close] 0
}
