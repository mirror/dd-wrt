# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr041
# TEST	repmgr preferred master basic resync and take over test.
# TEST
# TEST	Creates a preferred master replication group and shuts down the master
# TEST	site so that the client site takes over as temporary master.  Then
# TEST	it restarts the preferred master site, which synchronizes with the
# TEST	temporary master and takes over as preferred master again.  Verifies
# TEST	that temporary master transactions are retained.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr041 { { niter 100 } { tnum "041" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr preferred master basic resync and\
	    take over test."
	repmgr041_sub $method $niter $tnum $args
}

proc repmgr041_sub { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global databases_in_memory
	global ipversion
	set nsites 2
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

	file mkdir $masterdir
	file mkdir $clientdir

	# Primordial startup is the very first time a site starts up.
	# Non-preferred master repgroups require the first site to start as
	# master with the group creator flag.  But we don't allow users to
	# start preferred master sites as master because the code should
	# control this.  So on primordial start, the code internally makes
	# the preferred master site take the master/group creator path.
	puts "\tRepmgr$tnum.a: Preferred master site primordial startup."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.b: Client site primordial startup."
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

	puts "\tRepmgr$tnum.d: Shut down master and wait for client takeover."
	error_check_good masterenv_close [$masterenv close] 0
	await_expected_master $clientenv

	puts "\tRepmgr$tnum.e: Run transactions at temporary master."
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.f: Perform easy-to-find final transaction on\
	    temporary master."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]
	set t [$clientenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 1 [chop_data $method data$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	puts "\tRepmgr$tnum.g: Restart master, resync and take over."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $masterenv
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.h: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.i: Verify final temporary master transaction."
	set tmdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $masterenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $tmdb] TRUE
	set ret [lindex [$tmdb get 1] 0]
	error_check_good tmdb_get $ret [list 1 [pad_data $method data$tnum]]
	error_check_good tmdb2_close [$tmdb close] 0

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.j: Restart both sites (non-primordial startup)."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client

	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.k: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}
