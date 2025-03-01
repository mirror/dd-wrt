# See the file LICENSE for redistribution information.
#
# Copyright (c) 2007, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr010
# TEST	Acknowledgement policy and timeout test. 
# TEST
# TEST	Verify that "quorum" acknowledgement policy succeeds with fewer than 
# TEST	nsites running. Verify that "all" acknowledgement policy results in 
# TEST	ack failures with fewer than nsites running.  Make sure the presence
# TEST	of more views than participants doesn't cause incorrect ack behavior.
# TEST	Make sure unelectable master requires more acks for "quorum" policy.
# TEST	Test that an unelectable client joining the group doesn't cause
# TEST	PERM_FAILs.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr010 { { niter 100 } { tnum "010" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	set viewopts { noview view }
	foreach v $viewopts {
		puts "Repmgr$tnum ($method $v): repmgr ack policy test."
		repmgr010_sub $method $niter $tnum $v $args
	}

	puts "Repmgr$tnum.ju ($method): repmgr join unelectable test."
	repmgr010_joinunelect $method $niter $tnum $args
}

proc repmgr010_sub { method niter tnum viewopt largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion

	if { $viewopt == "view" } {
		set nsites 7
		set viewstr " and views"
	} else {
		set nsites 3
		set viewstr ""
	}

	set small_iter [expr $niter / 10]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2
	if { $viewopt == "view" } {
		set viewdir1 $testdir/VIEWDIR1
		set viewdir2 $testdir/VIEWDIR2
		set viewdir3 $testdir/VIEWDIR3
		set viewdir4 $testdir/VIEWDIR4
		file mkdir $viewdir1
		file mkdir $viewdir2
		file mkdir $viewdir3
		file mkdir $viewdir4
	}

	puts "\tRepmgr$tnum.a: Start master, clients$viewstr, acks quorum."
	# Open a master.
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack quorum \
	    -timeout {ack 5000000} \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	# Open first client
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack quorum \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 2]] \
	    -start client
	await_startup_done $clientenv

	# Open second client
	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 repmgr -ack quorum \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 1]] \
	    -start client
	await_startup_done $clientenv2

	# Open views.
	if { $viewopt == "view" } {
		set viewcb ""
		set viewenv1 [repmgr010_create_view VIEW1 $viewdir1 \
		    $verbargs [lindex $ports 3] [lindex $ports 0]]
		set viewenv2 [repmgr010_create_view VIEW2 $viewdir2 \
		    $verbargs [lindex $ports 4] [lindex $ports 0]]
		set viewenv3 [repmgr010_create_view VIEW3 $viewdir3 \
		    $verbargs [lindex $ports 5] [lindex $ports 0]]
		set viewenv4 [repmgr010_create_view VIEW4 $viewdir4 \
		    $verbargs [lindex $ports 6] [lindex $ports 0]]
	}

	puts "\tRepmgr$tnum.b: Run first set of transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	#
	# Special verification needed for quorum ack policy. Wait
	# longer than ack timeout (default 1 second) then check for 
	# ack failures (perm_failed events). Quorum only guarantees
	# that transactions replicated to one site or the other, so
	# test for this condition instead of both sites.
	#
	puts "\tRepmgr$tnum.c: Verify both client databases, no ack failures."
	error_check_good quorum_perm_failed1 \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] 0
	catch {rep_verify\
	    $masterdir $masterenv $clientdir $clientenv 1 1 1} ver1
	catch {rep_verify\
	    $masterdir $masterenv $clientdir2 $clientenv2 1 1 1} ver2
	error_check_good onesite [expr [string length $ver1] == 0 || \
	    [string length $ver2] == 0] 1

	puts "\tRepmgr$tnum.d: Shut down first client."
	error_check_good client_close [$clientenv close] 0

	puts "\tRepmgr$tnum.e: Run second set of transactions at master."
	eval rep_test $method $masterenv NULL $small_iter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.f: Verify client$viewstr, no ack failures."
	error_check_good quorum_perm_failed2 \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] 0
	rep_verify $masterdir $masterenv $clientdir2 $clientenv2 1 1 1
	if { $viewopt == "view" } {
		rep_verify $masterdir $masterenv $viewdir1 $viewenv1 1 1 1
		rep_verify $masterdir $masterenv $viewdir2 $viewenv2 1 1 1
		rep_verify $masterdir $masterenv $viewdir3 $viewenv3 1 1 1
		rep_verify $masterdir $masterenv $viewdir4 $viewenv4 1 1 1
	}

	#
	# Test that an unelectable master impacts the number of client
	# acks required for the quorum policy.  In a repgroup with an
	# unelectable master and two electable clients, acks from both
	# clients are required for durability.
	#
	puts "\tRepmgr$tnum.g: Make master unelectable."
	$masterenv repmgr -pri 0

	puts "\tRepmgr$tnum.h: Run more transactions, verify ack failures."
	# One electable client is no longer enough for durability.
	set unelect_perm_failed \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"]
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	error_check_good unelect_perm_fails [expr \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] \
	    > $unelect_perm_failed] 1

	puts "\tRepmgr$tnum.i: Restart client."
	set clientenv [eval $cl_envcmd -recover]
	$clientenv repmgr -ack quorum \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -remote [list $hoststr [lindex $ports 2]] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.j: Run more transactions, verify no ack failures."
	# Now with both clients up, we have enough acks for durability.
	set unelect_perm_failed \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"]
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	error_check_good unelect_no_perm_fails [expr \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] \
	    == $unelect_perm_failed] 1

	puts "\tRepmgr$tnum.k: Make master electable again."
	$masterenv repmgr -pri 100

	puts "\tRepmgr$tnum.l: Adjust all sites to ack policy all."
	$masterenv repmgr -ack all
	$clientenv repmgr -ack all
	$clientenv2 repmgr -ack all
	if { $viewopt == "view" } {
		$viewenv1 repmgr -ack all
		$viewenv2 repmgr -ack all
		$viewenv3 repmgr -ack all
		$viewenv4 repmgr -ack all
	}

	puts "\tRepmgr$tnum.m: Shut down first client."
	error_check_good client_close [$clientenv close] 0
	set init_perm_failed \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"]

	#
	# Use of -ack all guarantees replication complete before repmgr send
	# function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.n: Run more transactions at master."
	eval rep_test $method $masterenv NULL $small_iter $start 0 0 $largs

	puts "\tRepmgr$tnum.o: Verify client$viewstr, some ack failures."
	rep_verify $masterdir $masterenv $clientdir2 $clientenv2 1 1 1
	error_check_good all_perm_failed [expr \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] \
	    > $init_perm_failed] 1
	if { $viewopt == "view" } {
		rep_verify $masterdir $masterenv $viewdir1 $viewenv1 1 1 1
		rep_verify $masterdir $masterenv $viewdir2 $viewenv2 1 1 1
		rep_verify $masterdir $masterenv $viewdir3 $viewenv3 1 1 1
		rep_verify $masterdir $masterenv $viewdir4 $viewenv4 1 1 1

		error_check_good v4_close [$viewenv4 close] 0
		error_check_good v3_close [$viewenv3 close] 0
		error_check_good v2_close [$viewenv2 close] 0
		error_check_good v1_close [$viewenv1 close] 0
	}
	error_check_good client2_close [$clientenv2 close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Test that an unelectable client joining the replication group doesn't
# generate PERM_FAILs before it is connected.
#
proc repmgr010_joinunelect { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global ipversion

	set nsites 3

	set small_iter [expr $niter / 10]

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	env_cleanup $testdir
	set ports [available_ports $nsites]
	set hoststr [get_hoststr $ipversion]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2
	puts "\tRepmgr$tnum.ju.a: Start master and client, acks allpeers."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv repmgr -ack allpeers \
	    -timeout {ack 5000000} \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv repmgr -ack allpeers \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.ju.b: Start unelectable client, enable test hook."
	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	#
	# The heartbeat test hook also prevents connection attempts.  In
	# this test, it keeps the unelectable client in a state where it
	# has joined the group but it cannot establish its regular repmgr
	# connections until the test hook is rescinded.  During this time,
	# the master knows from the join operation that this site is not a
	# peer, so its presence should not cause any PERM_FAILs.
	#
	$masterenv test abort repmgr_heartbeat
	$clientenv test abort repmgr_heartbeat
	$clientenv2 test abort repmgr_heartbeat
	$clientenv2 repmgr -pri 0 -ack allpeers \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	# Defer await_startup_done until test hook is turned off.

	set unelect_perm_failed \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"]

	puts "\tRepmgr$tnum.ju.c: Run first set of transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $small_iter $start 0 0 $largs
	incr start $small_iter

	puts "\tRepmgr$tnum.ju.d: Disable test hook."
	$masterenv test abort none
	$clientenv test abort none
	$clientenv2 test abort none
	await_startup_done $clientenv2

	puts "\tRepmgr$tnum.ju.e: Run second set of transactions at master."
	eval rep_test $method $masterenv NULL $small_iter $start 0 0 $largs
	incr start $small_iter

	puts "\tRepmgr$tnum.ju.f: Verify client databases, no ack failures."
	error_check_good unelect_perm_fails2 [expr \
	    [stat_field $masterenv repmgr_stat "Acknowledgement failures"] \
	    == $unelect_perm_failed] 1
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1
	rep_verify $masterdir $masterenv $clientdir2 $clientenv2 1 1 1

	error_check_good client2_close [$clientenv2 close] 0
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

proc repmgr010_create_view { vprefix vdir verbargs lport rport } {
	global ipversion

	set hoststr [get_hoststr $ipversion]
	set venv NULL
	set viewcb ""
	set v_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx $vprefix -rep_view \[list $viewcb \] \
	    -home $vdir -txn -rep -thread"
	set venv [eval $v_envcmd]
	$venv repmgr -ack quorum \
	    -local [list $hoststr $lport] \
	    -remote [list $hoststr $rport] \
	    -start client
	await_startup_done $venv
	return $venv
}
