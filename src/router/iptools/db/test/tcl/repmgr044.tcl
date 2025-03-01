# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr044
# TEST	repmgr preferred master replication group size test.
# TEST
# TEST	Test preferred master behavior when sites are removed from or added
# TEST	to the replication group.  Also test permanent transfer of preferred
# TEST	mastership to the client site.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr044 { { niter 100 } { tnum "044" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr preferred master repgroup size\
	    test."
	repmgr044_groupsize $method $niter $tnum $args

	puts "Repmgr$tnum ($method): repmgr preferred master transfer test."
	repmgr044_mastertrans $method $niter $tnum $args
}

#
# Perform test cases in which each individual site (client, preferred master)
# is removed from the replication group by the other site and then rejoins.
# Then temporarily add a third site to the replication group and remove it to
# make sure preferred master continues to operate correctly afterwards.
#
proc repmgr044_groupsize { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global databases_in_memory
	global ipversion
	set nsites 3
	set omethod [convert_method $method]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set hoststr [get_hoststr $ipversion]
	set ports [available_ports $nsites]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set client2dir $testdir/CLIENT2DIR

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $client2dir

	# Create error files to capture group size warnings.
	puts "\tRepmgr$tnum.gs.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errfile $testdir/rm44mas.err \
	    -errpfx MASTER -home $masterdir -txn -rep -thread -event"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.gs.b: Start client site."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errfile $testdir/rm44cli.err \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread -event"
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
	puts "\tRepmgr$tnum.gs.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.gs.d: Preferred master client site remove and\
	    rejoin."
	puts "\tRepmgr$tnum.gs.d1: Shut down and remove client, perform master\
	    transactions."
	error_check_good clientenv_close [$clientenv close] 0
	$masterenv repmgr -remove [list $hoststr [lindex $ports 1]]
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.gs.d2: Client rejoins repgroup."
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $clientenv
	# Allow time for extra message cycle needed for gmdb version catch up.
	tclsleep 3

	puts "\tRepmgr$tnum.gs.d3: Master transactions, verify client\
	    contents."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	# Make sure rejoined client behaves like a preferred master client.
	puts "\tRepmgr$tnum.gs.d4: Do client takeover, temporary master\
	    transactions."
	error_check_good masterenv_close [$masterenv close] 0
	await_expected_master $clientenv
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.gs.d5: Restart master, resync and take over."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $masterenv
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.gs.d6: Run/verify transactions at preferred\
	    master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.gs.e: Preferred master site remove and rejoin."
	puts "\tRepmgr$tnum.gs.e1: Close preferred master, client takes over."
	error_check_good masterenv_close [$masterenv close] 0
	await_expected_master $clientenv
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.gs.e2: Remove preferred master site."
	$clientenv repmgr -remove [list $hoststr [lindex $ports 0]]
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.gs.e3: Preferred master rejoins, resyncs and\
	    takes over."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $masterenv
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.gs.e4: Run/verify transactions at preferred\
	    master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.gs.f: Temporarily add third site."
	# This triggers replication group size warnings that we will later
	# verify in error files.  Replication will continue to work.  After
	# removing the third site, verify that preferred master behaves
	# as we expect.
	puts "\tRepmgr$tnum.gs.f1: Start a second client site."
	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $client2dir -txn -rep -thread -event"
	set client2env [eval $cl2_envcmd]
	$client2env repmgr -ack all \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $client2env

	puts "\tRepmgr$tnum.gs.f2: Run/verify transactions at preferred\
	    master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.gs.f3: Remove second client site from repgroup."
	$masterenv repmgr -remove [list $hoststr [lindex $ports 2]]
	await_event $client2env local_site_removed
	error_check_good client2_close [$client2env close] 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.gs.f4: Shut down preferred master, client\
	    takeover."
	error_check_good masterenv_close [$masterenv close] 0
	await_expected_master $clientenv
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.gs.f5: Restart preferred master, resync and\
	    take over."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $masterenv
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.gs.f6: Run/verify transactions at preferred\
	    master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.gs.f7: Verify repgroup size warnings in error\
	    files."
	#
	# We check the errfiles after closing the envs because the close
	# guarantees all messages are flushed to disk.
	#	
	set maserrfile [open $testdir/rm44mas.err r]
	set maserr [read $maserrfile]
	close $maserrfile
	error_check_good errchk [is_substr $maserr "two sites in preferred"] 1
	set clierrfile [open $testdir/rm44cli.err r]
	set clierr [read $clierrfile]
	close $clierrfile
	error_check_good errchk [is_substr $clierr "two sites in preferred"] 1
}

#
# It is possible that a hardware failure or other circumstances could make it
# impossible to continue with the same preferred master site.  This case tests
# the sequence of operations needed to turn the client site into the new
# preferred master site, retaining the repgroup data stored on it.  Then a
# different site can be started up as the new client.
#
proc repmgr044_mastertrans { method niter tnum largs } {
	global testdir
	global rep_verbose

	global verbose_type
	global databases_in_memory
	global ipversion
	set nsites 3
	set omethod [convert_method $method]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set hoststr [get_hoststr $ipversion]
	set ports [available_ports $nsites]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set client2dir $testdir/CLIENT2DIR

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $client2dir

	puts "\tRepmgr$tnum.mx.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errfile $testdir/rm44mas.err \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.mx.b: Start client site."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errfile $testdir/rm44cli.err \
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
	puts "\tRepmgr$tnum.mx.c: Run/verify transactions at preferred\
	    master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.mx.d: Perform a final transaction on preferred\
	    master."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}
	set orig_mdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	set t [$masterenv txn]
	error_check_good db_put \
	    [eval $orig_mdb put -txn $t 1 [chop_data $method data$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good omdb_close [$orig_mdb close] 0

	puts "\tRepmgr$tnum.mx.e: Shut down and remove original preferred\
	    master."
	error_check_good masterenv_close [$masterenv close] 0
	await_expected_master $clientenv
	$clientenv repmgr -remove [list $hoststr [lindex $ports 0]]
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.mx.f: Restart client as new preferred master\
	    site."
	error_check_good client_close [$clientenv close] 0
	set clientenv [eval $cl_envcmd -recover]
	$clientenv rep_config {mgrprefmasmaster on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] -start client
	await_expected_master $clientenv
	# On some slower platforms, it takes the repmgr startup in
	# the election thread a bit longer to finish and release its
	# resources, leading to a deadlock without this pause.
	tclsleep 1
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.mx.g: Make a third site the new preferred\
	    master client."
	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $client2dir -txn -rep -thread"
	set client2env [eval $cl2_envcmd]
	$client2env rep_config {mgrprefmasclient on}
	$client2env repmgr -ack all \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 1]] -start client
	await_startup_done $client2env
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.mx.h: Verify new preferred master client takeover."
	error_check_good client_close [$clientenv close] 0
	await_expected_master $client2env
	eval rep_test $method $client2env NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.mx.i: Verify new preferred master resync and take\
	    over."
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasmaster on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] -start client
	await_startup_done $clientenv
	await_expected_master $clientenv

	puts "\tRepmgr$tnum.mx.j: Run/verify transactions at new\
	    preferred master."
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $clientdir $clientenv $client2dir $client2env 1 1 1

	puts "\tRepmgr$tnum.mx.k: Verify original master transactions\
	    survived."
	set orig_mdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $clientenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $orig_mdb] TRUE
	set ret [lindex [$orig_mdb get 1] 0]
	error_check_good omdb_get $ret [list 1 [pad_data $method data$tnum]]
	error_check_good omdb2_close [$orig_mdb close] 0

	error_check_good client2_close [$client2env close] 0
	error_check_good client_close [$clientenv close] 0
}
