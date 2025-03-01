# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr043
# TEST	repmgr preferred master transaction retention test.
# TEST
# TEST	Test various cases that create continuous or conflicting sets of
# TEST	transactions across the two sites.  Verify that unique preferred
# TEST	master transactions are never rolled back and that unique temporary
# TEST	master transactions are kept when possible.
# TEST
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr043 { { niter 100 } { tnum "043" } args } {

	source ./include.tcl

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	# When one or both preferred master sites operate independently
	# (e.g. site(s) down, dupmaster), we must reconcile the sets of
	# transactions on each site to guarantee that we do not roll back
	# any preferred master transactions.  We have a continuous set of
	# transactions if only one site had new unique transactions.  But
	# if both sites had new unique transactions we have conflicting sets
	# of transactions and the client/temporary master transactions must
	# be rolled back.

	# Create (mostly) continuous sets of transactions.  Vary the site
	# on which one unique set of data is created and whether one or
	# both sites is shut down before determining how to resolve the
	# unique transactions.
	set txnsite { master client }
	set downopts { onedown bothdown }
	foreach ts $txnsite {
		foreach do $downopts {
			puts "Repmgr$tnum ($method $ts $do): repmgr preferred\
			    master continuous transaction set test."
			repmgr043_continuous $method $niter $tnum $ts $do $args
		}
	}

	# Create conflicting sets of transactions.  Vary the site ordering
	# for the unique sets of transactions and the site on which a
	# larger set of data is created.
	set firstsite { master client }
	set moredata { master client }
	foreach f $firstsite {
		foreach m $moredata {
			puts "Repmgr$tnum ($method $f $m): repmgr preferred\
			    master conflicting transaction set test."
			repmgr043_conflicting $method $niter $tnum $f $m $args
		}
	}

	# Create conflicting, parallel data generations on each site.
	puts "Repmgr$tnum ($method): repmgr\
	    preferred master parallel generation test."
	repmgr043_parallelgen $method $niter $tnum $args

	# Create extra log records before preferred master is restarted.
	# Vary whether these extra log records contain a commit.
	set commitopt { nocommit commit }
	foreach c $commitopt {
		puts "Repmgr$tnum ($method $c): repmgr preferred\
		    master extra log records test."
		repmgr043_extralog $method $niter $tnum $c $args
	}
}

#
# Create test cases where a unique set of transactions is created on
# one site or the other and we vary whether one or both sites is shut down.
# In most cases this results in a continuous set of transactions such
# that unique temporary master transactions can be retained.
#
proc repmgr043_continuous { method niter tnum txnsite downopt largs } {
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

	puts "\tRepmgr$tnum.ct.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.ct.b: Start client site."
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
	puts "\tRepmgr$tnum.ct.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	# Set up site on which to run unique transactions and site to close.
	if { $txnsite == "master" } {
		set txnenv $masterenv
		set closeenv $clientenv
	} else {
		set txnenv $clientenv
		set closeenv $masterenv
	}

	puts "\tRepmgr$tnum.ct.d: Shut down non-transaction site."
	error_check_good closeenv_close [$closeenv close] 0
	if { $txnsite == "client" } {
		await_expected_master $txnenv
	}

	puts "\tRepmgr$tnum.ct.e: Run unique transactions on $txnsite."
	eval rep_test $method $txnenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.ct.f: Perform easy-to-find final\
	    transaction on $txnsite."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $txnenv $largs $dbname"]
	set t [$txnenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 1 [chop_data $method data$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	if { $downopt == "bothdown" } {
		puts "\tRepmgr$tnum.ct.f1: Shut down $txnsite."
		error_check_good txnenv_close [$txnenv close] 0
	}

	puts "\tRepmgr$tnum.ct.g: Restart one or both sites as needed."
	if { $txnsite == "client" || $downopt == "bothdown" } {
		set masterenv [eval $ma_envcmd]
		$masterenv rep_config {mgrprefmasmaster on}
		$masterenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 0]] -start client
		if { $downopt != "bothdown" } {
			await_startup_done $masterenv
		}
		await_expected_master $masterenv
	}
	if { $txnsite == "master" || $downopt == "bothdown" } {
		set clientenv [eval $cl_envcmd]
		$clientenv rep_config {mgrprefmasclient on}
		$clientenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 1]] \
		    -remote [list $hoststr [lindex $ports 0]] -start client
		await_startup_done $clientenv
	}

	puts "\tRepmgr$tnum.ct.h: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.ct.i: Verify final unique transaction."
	set tmdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $masterenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $tmdb] TRUE
	set ret [lindex [$tmdb get 1] 0]
	if { $downopt == "bothdown" && $txnsite == "client" } {
		# We expect to roll back temporary master unique transactions
		# here because both sites were down and the preferred master
		# is restarted first.  After restarting, the preferred master
		# creates new unique transactions of its own that can't be
		# rolled back and these would be in conflict with the
		# temporary master unique transactions.
		error_check_good tmdb_get1 $ret ""
	} else {
		# There is a continuous set of transactions.  The temporary
		# master unique transactions are retained because there is no
		# danger of rolling back any preferred master transactions.
		error_check_good tmdb_get2 $ret\
		    [list 1 [pad_data $method data$tnum]]
	}
	error_check_good tmdb2_close [$tmdb close] 0

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Create test cases where unique sets of different sizes are created on
# both sites in different orders.  These test cases result in conflicting
# sets of transactions which must always be resolved by keeping the
# preferred master unique transactions and rolling back the temporary
# master unique transactions.  This test verifies that the sizes of the
# unique data sets are immaterial in preferred master mode.  The cases
# where the first site is the preferred master also test the next_gen_lsn
# comparison in the lsnhist_match code.
#
proc repmgr043_conflicting { method niter tnum firstsite moredata largs } {
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
	set big_iter [expr $niter * 2]

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.cf.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.cf.b: Start client site."
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
	puts "\tRepmgr$tnum.cf.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	# Set up site order for running unique transactions and amount of
	# data at each site.
	set firstiter $niter
	set seconditer $big_iter
	if { $firstsite == "master" } {
		set firstenv $masterenv
		set secondenv $clientenv
		if { $moredata == "master" } {
			set firstiter $big_iter
			set seconditer $niter
		}
	} else {
		set firstenv $clientenv
		set secondenv $masterenv
		if { $moredata == "client" } {
			set firstiter $big_iter
			set seconditer $niter
		}
	}

	puts "\tRepmgr$tnum.cf.d: Shut down second site."
	error_check_good secondenv_close [$secondenv close] 0
	await_expected_master $firstenv

	puts "\tRepmgr$tnum.cf.e: Run transactions at first site."
	eval rep_test $method $firstenv NULL $firstiter $start 0 0 $largs
	# Avoid duplicates later by incrementing the larger possible value.
	incr start $big_iter

	puts "\tRepmgr$tnum.cf.f: Perform easy-to-find final transaction\
	    on first site."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $firstenv $largs $dbname"]
	set t [$firstenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 1 [chop_data $method data$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	puts "\tRepmgr$tnum.cf.g: Shut down first site, start second site."
	error_check_good firstenv_close [$firstenv close] 0
	if { $firstsite == "master" } {
		set clientenv [eval $cl_envcmd]
		$clientenv rep_config {mgrprefmasclient on}
		$clientenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 1]] \
		    -remote [list $hoststr [lindex $ports 0]] -start client
		await_expected_master $clientenv
		set secondenv $clientenv
	} else {
		set masterenv [eval $ma_envcmd]
		$masterenv rep_config {mgrprefmasmaster on}
		$masterenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 0]] -start client
		await_expected_master $masterenv
		set secondenv $masterenv
	}

	puts "\tRepmgr$tnum.cf.h: Run transactions at second site."
	eval rep_test $method $secondenv NULL $seconditer $start 0 0 $largs
	# Avoid duplicates later by incrementing the larger possible value.
	incr start $big_iter

	puts "\tRepmgr$tnum.cf.i: Perform easy-to-find final transaction\
	    on second site."
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $secondenv $largs $dbname"]
	set t [$secondenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 2 [chop_data $method data2$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	puts "\tRepmgr$tnum.cf.j: Restart first site."
	if { $firstsite == "master" } {
		set masterenv [eval $ma_envcmd]
		$masterenv rep_config {mgrprefmasmaster on}
		$masterenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 0]] -start client
		await_expected_master $masterenv
		set firstenv $masterenv
	} else {
		set clientenv [eval $cl_envcmd]
		$clientenv rep_config {mgrprefmasclient on}
		$clientenv repmgr -ack all \
		    -local [list $hoststr [lindex $ports 1]] \
		    -remote [list $hoststr [lindex $ports 0]] -start client
		await_startup_done $clientenv
		set firstenv $clientenv
	}

	puts "\tRepmgr$tnum.cf.k: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.cf.l: Verify preferred master unique transactions\
	    retained."
	set tmdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $masterenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $tmdb] TRUE
	set ret1 [lindex [$tmdb get 1] 0]
	set ret2 [lindex [$tmdb get 2] 0]
	# Verify that the preferred master data unique data was retained
	# in all cases, regardless of the order.
	if { $firstsite == "master" } {
		error_check_good tmdb_get1 $ret1 \
		    [list 1 [pad_data $method data$tnum]]
		error_check_good tmdb_get2 $ret2 ""

	} else {
		error_check_good tmdb_get3 $ret1 ""
		error_check_good tmdb_get4 $ret2 \
		    [list 2 [pad_data $method data2$tnum]]
	}
	error_check_good tmdb2_close [$tmdb close] 0

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Create a situation where each site runs independently ("whack-a-mole")
# with the temporary master reaching a higher generation than the preferred
# master.  Then restart the preferred master and make sure that its data
# is kept and the temporary master data is rolled back.  This tests the
# timestamp comparison in the lsnhist_match code.
#
proc repmgr043_parallelgen { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
	global databases_in_memory
	global ipversion
	set nsites 2
	set omethod [convert_method $method]
	set small_iter [expr $niter / 2]

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

	puts "\tRepmgr$tnum.pg.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.pg.b: Start client site."
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
	puts "\tRepmgr$tnum.pg.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.pg.d: Perform unique transaction\
	    on preferred master."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	set t [$masterenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 1 [chop_data $method data$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	puts "\tRepmgr$tnum.pg.e: Close both sites, restart preferred master."
	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv
	eval rep_test $method $masterenv NULL $small_iter $start 0 0 $largs
	incr start $small_iter

	puts "\tRepmgr$tnum.pg.f: Close preferred master, restart client as\
	    temporary master."
	error_check_good masterenv_close [$masterenv close] 0
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $clientenv
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.pg.g: Perform unique transaction\
	    on temporary master."
	set tmdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]
	set t [$clientenv txn]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 2 [chop_data $method data2$tnum]] 0
	error_check_good txn_commit [$t commit] 0
	error_check_good tmdb_close [$tmdb close] 0

	puts "\tRepmgr$tnum.pg.h: Close and restart temporary master."
	# Increment gen again to show that preferred master transactions
	# are kept after multiple new temporary master generations.
	error_check_good client_close [$clientenv close] 0
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $clientenv
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter
	set cdupm1 [stat_field $clientenv \
	    rep_stat "Duplicate master conditions"]

	puts "\tRepmgr$tnum.pg.i: Restart preferred master."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.pg.j: Verify no dupmasters on client."
	# The preferred master startup sequence should have forced the
	# temporary master to restart as a client to avoid a dupmaster.
	set cdupm2 [stat_field $clientenv \
	    rep_stat "Duplicate master conditions"]
	error_check_good no_cli_dupm [expr {$cdupm1 == $cdupm2}] 1

	puts "\tRepmgr$tnum.pg.k: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.pg.l: Verify preferred master unique transaction."
	set tmdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $masterenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $tmdb] TRUE
	set ret1 [lindex [$tmdb get 1] 0]
	set ret2 [lindex [$tmdb get 2] 0]
	error_check_good tmdb_get1 $ret1 [list 1 [pad_data $method data$tnum]]
	# The temporary master unique transaction should have been rolled back.
	error_check_good tmdb_get2 $ret2 ""
	error_check_good tmdb2_close [$tmdb close] 0

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Create test cases where there are extra log records on the preferred
# master site before it restarts repmgr.  If these extra log records
# contain a commit we must roll back temporary master transactions.  If the
# extra log records do not contain a commit we can retain temporary master
# transactions.  This tests the lsnhist_match find_commit logic.
#
proc repmgr043_extralog { method niter tnum commitopt largs } {
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
	# Extra fast connection retry timeout for prompt connection on
	# preferred master restart.
	set connretry 500000

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.el.a: Start preferred master site."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -timeout [list connection_retry $connretry] \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.el.b: Start client site."
	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrprefmasclient on}
	$clientenv repmgr -ack all \
	    -timeout [list connection_retry $connretry] \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] -start client
	await_startup_done $clientenv

	#
	# Use of -ack all guarantees that replication is complete before the
	# repmgr send function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.el.c: Run/verify transactions at preferred master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.el.d: Shut down preferred master site."
	error_check_good prefmas_close [$masterenv close] 0
	await_expected_master $clientenv

	puts "\tRepmgr$tnum.el.e: Run transactions at temporary master."
	eval rep_test $method $clientenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.el.f: Perform easy-to-find final temporary\
	    master transaction."
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

	puts "\tRepmgr$tnum.el.g: Open preferred master environment."
	set masterenv [eval $ma_envcmd]
	puts "\tRepmgr$tnum.el.h: Create extra preferred master log records."
	set t [$masterenv txn]
	# Do not use auto_commit because we don't always want a commit.
	set tmdb [eval "berkdb_open_noerr -create $omethod -txn $t \
	    -env $masterenv $largs $dbname"]
	error_check_good db_put \
	    [eval $tmdb put -txn $t 2 [chop_data $method data2$tnum]] 0
	if { $commitopt == "commit" } {
		error_check_good xtxn_commit [$t commit] 0
	} else {
		error_check_good xtxn_abort [$t abort] 0
	}
	error_check_good tmdb_close [$tmdb close] 0
	puts "\tRepmgr$tnum.el.i: Start repmgr on preferred master."
	$masterenv rep_config {mgrprefmasmaster on}
	$masterenv repmgr -ack all \
	    -timeout [list connection_retry $connretry] \
	    -local [list $hoststr [lindex $ports 0]] -start client
	await_expected_master $masterenv

	puts "\tRepmgr$tnum.el.j: Run/verify transactions at preferred master."
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	puts "\tRepmgr$tnum.el.k: Verify expected unique transaction was\
	    retained."
	set tmdb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
	    -env $masterenv $largs $dbname"]
	error_check_good reptest_db [is_valid_db $tmdb] TRUE
	set ret1 [lindex [$tmdb get 1] 0]
	set ret2 [lindex [$tmdb get 2] 0]
	if { $commitopt == "commit" } {
		# If we committed before preferred master restart, verify that
		# we rolled back the temporary master transaction and kept
		# the preferred master transaction.
		error_check_good tmdb_get1 $ret1 ""
		error_check_good tmdb_get2 $ret2 \
		    [list 2 [pad_data $method data2$tnum]]
	} else {
		# If there was no commit before preferred master restart,
		# verify that we kept the temporary master transaction and
		# the preferred master transaction was rolled back.
		error_check_good tmdb_get3 $ret1 \
		    [list 1 [pad_data $method data$tnum]]
		error_check_good tmdb_get4 $ret2 ""
	}
	error_check_good tmdb2_close [$tmdb close] 0

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}
