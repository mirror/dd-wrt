# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr042
# TEST	repmgr preferred master client startup test.
# TEST
# TEST	Test various preferred master client start up and shut down cases.
# TEST	Verify replication group continued operation without a client.
# TEST	Verify client site's startup as the temporary master and the
# TEST	ability of the preferred master site to resync and take over
# TEST	afterwards.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr042 { { niter 100 } { tnum "042" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr preferred master client startup\
	    test."
	repmgr042_sub $method $niter $tnum $args

}

proc repmgr042_sub { method niter tnum largs } {
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

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.a: Primordial start of client as temporary\
	    master (error)."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	catch {$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client } res
	error_check_good starttoofew [is_substr $res \
	    "Too few remote sites"] 1
	error_check_good badclientenv_close [$clientenv close] 0
	env_cleanup $clientdir

	puts "\tRepmgr$tnum.b: Start preferred master and client sites."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $clientenv

	#
	# Use of -ack all guarantees that replication is complete before the
	# repmgr send function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.d: Shut down client, run more transactions on\
	    master."
	set cdupm1 [stat_field $clientenv \
	    rep_stat "Duplicate master conditions"]
	error_check_good clientenv_close [$clientenv close] 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.e: Restart client, verify no dupmasters."
	set mdupm1 [stat_field $masterenv \
	    rep_stat "Duplicate master conditions"]
	set clientenv [eval $cl_envcmd -recover]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $clientenv
	# This test case can have a false positive if the client doesn't
	# find the master in time, starts as master and then generates a
	# dupmaster that is resolved fortuitously.  Make sure there was no
	# dupmaster to assure that the preferred master client startup
	# occurred as expected.
	set mdupm2 [stat_field $masterenv \
	    rep_stat "Duplicate master conditions"]
	set cdupm2 [stat_field $clientenv \
	    rep_stat "Duplicate master conditions"]
	error_check_good no_mas_dupm [expr {$mdupm1 == $mdupm2}] 1
	error_check_good no_cli_dupm [expr {$cdupm1 == $cdupm2}] 1


	puts "\tRepmgr$tnum.f: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.g: Shut down both sites."
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.h: Restart client to become temporary master\
	    (non-primordial)."
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $clientenv

	puts "\tRepmgr$tnum.i: Run transactions at temporary master."
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.j: Restart preferred master, resync and take over."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $masterenv
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.k: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}
