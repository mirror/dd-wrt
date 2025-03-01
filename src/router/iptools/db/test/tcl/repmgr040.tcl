# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr040
# TEST	repmgr preferred master basic configuration test.
# TEST
# TEST	This test verifies repmgr's preferred master mode, including
# TEST	basic operation and configuration errors.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr040 { { niter 100 } { tnum "040" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr preferred master basic test."
	repmgr040_sub $method $niter $tnum $args
}

proc repmgr040_sub { method niter tnum largs } {
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
	set otherdir $testdir/OTHERDIR

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $otherdir

	#
	# Open environments without -errpfx so that full error text is
	# available to be checked.
	#

	# Open preferred master site.
	puts "\tRepmgr$tnum.a: Start preferred master."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	# Test that preferred master site can only be started as client.
	catch {$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start master} res
	error_check_good startmaster [is_substr $res \
	    "preferred master site must be started"] 1
	catch {$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start elect} res
	error_check_good startelect [is_substr $res \
	    "preferred master site must be started"] 1
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list heartbeat_send 500000] \
	    -timeout [list heartbeat_monitor 1500000] \
	    -timeout [list election_retry 2000000] -start client

	# Open preferred master client site.
	puts "\tRepmgr$tnum.b: Start client."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	# Test that elections and 2sitestrict get turned back on after start.
	$clientenv rep_config {mgrelections off}
	$clientenv rep_config {mgr2sitestrict off}
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.c: Check automatic preferred master configuration."
	error_check_good mpri [$masterenv rep_get_priority] 200
	error_check_good cpri [$clientenv rep_get_priority] 75
	error_check_good chbm \
	    [$clientenv rep_get_timeout heartbeat_monitor] 2000000
	error_check_good chbs \
	    [$clientenv rep_get_timeout heartbeat_send] 750000
	error_check_good celr \
	    [$clientenv rep_get_timeout election_retry] 1000000
	error_check_good m2site [$masterenv rep_get_config mgr2sitestrict] 1
	error_check_good melect [$masterenv rep_get_config mgrelections] 1
	error_check_good c2site [$clientenv rep_get_config mgr2sitestrict] 1
	error_check_good celect [$clientenv rep_get_config mgrelections] 1
	# Make sure user-set timeouts were preserved.
	error_check_good mhbm \
	    [$masterenv rep_get_timeout heartbeat_monitor] 1500000
	error_check_good mhbs \
	    [$masterenv rep_get_timeout heartbeat_send] 500000
	error_check_good melr \
	    [$masterenv rep_get_timeout election_retry] 2000000

	puts "\tRepmgr$tnum.d: Test configuration errors in preferred master\
	    environment."
	# Test setting heartbeat timeouts to 0.
	catch {$masterenv repmgr -timeout {heartbeat_send 0}} res
	error_check_good mhbs0 [is_substr $res "turn off heartbeat timeout"] 1
	catch {$clientenv repmgr -timeout {heartbeat_monitor 0}} res
	error_check_good chbm0 [is_substr $res "turn off heartbeat timeout"] 1
	# Test changing priority.
	catch {$masterenv repmgr -pri 250} res
	error_check_good mpnc [is_substr $res "cannot change priority"] 1
	# Test invalid configuration options.
	catch {$masterenv rep_config {mgr2sitestrict off}} res
	error_check_good m2siteoff [is_substr $res \
	    "disable 2SITE_STRICT in preferred"] 1
	catch {$clientenv rep_config {mgrelections off}} res
	error_check_good celectoff [is_substr $res \
	    "disable elections in preferred"] 1
	catch {$clientenv rep_config {lease on}} res
	error_check_good cleaseon [is_substr $res \
	    "enable leases in preferred"] 1
	# Test creating in-memory database.
	set dbname { "" "test.db" }
	catch { set mdb [eval "berkdb_open_noerr -create -btree -auto_commit \
	    -env $masterenv $largs $dbname"] } res
	error_check_good inmemdb [is_substr $res \
	    "In-memory databases are not supported in Replication Manager"] 1
	# Test changing preferred master after starting repmgr.
	catch {$clientenv rep_config {mgrprefmasmaster on}} res
	error_check_good pmchg1 [is_substr $res \
	    "preferred master must be configured"] 1
	catch {$clientenv rep_config {mgrprefmasclient off}} res
	error_check_good pmchg2 [is_substr $res \
	    "preferred master must be configured"] 1

	#
	# Use of -ack all guarantees that replication is complete before the
	# repmgr send function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.e: Run transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.f: Verify client's database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.g: Test configuration errors in diverse\
	    environments."
	puts "\tRepmgr$tnum.g1: In-memory replication files environment."
	set inmemrep_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $otherdir -txn -rep -thread -rep_inmem_files"
	set otherenv [eval $inmemrep_envcmd]
	catch {$otherenv rep_config {mgrprefmasmaster on}} res
	error_check_good inmemrep [is_substr $res \
	    "mode cannot be used with in-memory replication files"] 1
	error_check_good otherenv_close1 [$otherenv close] 0
	env_cleanup $otherdir

	puts "\tRepmgr$tnum.g2: Master leases environment."
	set lease_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $otherdir -txn -rep -thread"
	set otherenv [eval $lease_envcmd]
	$otherenv rep_config {lease on}
	catch {$otherenv rep_config {mgrprefmasmaster on}} res
	error_check_good leases [is_substr $res \
	    "mode cannot be used with master leases"] 1
	error_check_good otherenv_close2 [$otherenv close] 0
	env_cleanup $otherdir

	puts "\tRepmgr$tnum.g3: Private environment."
	# Test turning on preferred master in existing private environment.
	set privenv_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $otherdir -txn -rep -thread -private"
	set otherenv [eval $privenv_envcmd]
	catch {$otherenv rep_config {mgrprefmasmaster on}} res
	error_check_good privenv [is_substr $res \
	    "mode cannot be used with a private environment"] 1
	error_check_good otherenv_close3 [$otherenv close] 0
	env_cleanup $otherdir
	# Test opening new private environment with preferred master already
	# configured (different error.)
	set privenv2_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $otherdir -txn -rep -thread -private \
	    -rep_config {mgrprefmasmaster on}"
	catch {set otherenv [eval $privenv2_envcmd]} res
	error_check_good privenv [is_substr $res \
	    "DB_PRIVATE is not supported in Replication Manager preferred"] 1
	env_cleanup $otherdir

	puts "\tRepmgr$tnum.g4: In-memory logs environment."
	set inmemlog_envcmd "berkdb_env_noerr -create $verbargs \
	    -home $otherdir -txn -rep -thread -log_inmemory"
	set otherenv [eval $inmemlog_envcmd]
	catch {$otherenv rep_config {mgrprefmasmaster on}} res
	error_check_good inmemlogenv [is_substr $res \
	    "mode cannot be used with in-memory log files"] 1
	error_check_good otherenv_close4 [$otherenv close] 0
	env_cleanup $otherdir
}
