# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	repmgr046
# TEST	repmgr singleton write forwarding test.
# TEST
# TEST	The initial basic test has several purposes: test general use
# TEST	of repmgr write forwarding API options (config, timeout, stats),
# TEST	test basic forwarded put and del operations, test that expected
# TEST	errors are returned from most simple error scenarios.
# TEST
# TEST	The 3-site test makes sure that write forwarding continues to
# TEST	operate as expected during replication group changes such as
# TEST	a change of master and a former master rejoining as a client.
# TEST
# TEST	The other tests include cases for blobs, subdatabases,
# TEST	various duplicate options, and additional error cases.
# TEST	Run for btree only because access method shouldn't matter.
# TEST
proc repmgr046 { { niter 100 } { tnum "046" } args } {

	source ./include.tcl
	global databases_in_memory

	if { $is_freebsd_test == 1 } {
		puts "Skipping replication manager test on FreeBSD platform."
		return
	}

	set method "btree"
	set args [convert_args $method $args]

	puts "Repmgr$tnum ($method): repmgr write forwarding basic test."
	repmgr046_basic $method $niter $tnum $args

	puts "Repmgr$tnum ($method): repmgr write forwarding 3-site test."
	repmgr046_3site $method $niter $tnum $args

	# Blobs are not expected to work with in-memory databases.
	if {!$databases_in_memory} {
		puts "Repmgr$tnum ($method): repmgr write forwarding blob test."
		repmgr046_blob $method $niter $tnum $args
	}

	puts "Repmgr$tnum ($method): repmgr write forwarding subdatabase\
	    and duplicate test."
	repmgr046_subdb $method $niter $tnum $args

	puts "Repmgr$tnum ($method): repmgr write forwarding dead handle test."
	repmgr046_hdldead $method $niter $tnum $args
}

proc repmgr046_basic { method niter tnum largs } {
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
	set dmlsleep 1

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.a: Start a master and a client."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list write_forward 3000000] \
	    -start client
	await_startup_done $clientenv

	puts "\tRepmgr$tnum.b: Check expected config, timeout and stat values."
	# Check rep_set_config flags.
	error_check_good wfmas [$masterenv rep_get_config mgrforwardwrites] 1
	error_check_good wfcli [$clientenv rep_get_config mgrforwardwrites] 1
	# Check write forwarding timeout from rep_set_timeout.  Master has
	# default value of 5 seconds.
	error_check_good mdefwftimeout \
	    [$masterenv rep_get_timeout write_forward] 5000000
	error_check_good csetwftimeout \
	    [$clientenv rep_get_timeout write_forward] 3000000
	# Check write forwarding repmgr stats, 0 values expected.
	error_check_good mrcvstat [stat_field $masterenv repmgr_stat \
	    "Forwarded write operations received"] 0
	error_check_good cforstat [stat_field $clientenv repmgr_stat \
	    "Write operations forwarded"] 0

	#
	# Use of -ack all guarantees that replication is complete before the
	# repmgr send function returns and rep_test finishes.
	#
	puts "\tRepmgr$tnum.c: Run transactions at master."
	set start 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	#
	# Basic write forwarding DML test.  Perform some inserts and an update
	# and a delete, then verify that the expected data is replicated back
	# to the client.
	#
	puts "\tRepmgr$tnum.d: Perform simple DMLs to be forwarded on client."
	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}

	set mdmldb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	set cdmldb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]

	set key1 1
	set key2 2
	set data2 222
	set key3 3
	set data3 3333.33
	set key4 4
	# Insert key1 and key2.
	error_check_good cdmldb_put1 \
	    [eval $cdmldb put $key1 [chop_data $method data$key1]] 0
	error_check_good cdmldb_put2 [eval $cdmldb put $key2 $data3] 0
	# Now delete key1.
	error_check_good cdmldb_del1 [eval $cdmldb del $key1] 0
	# Update key2.
	error_check_good cdmldb_putupd2 [eval $cdmldb put $key2 $data2] 0
	# Insert key3.
	error_check_good cdmldb_put3 [eval $cdmldb put $key3 $data3] 0
	# Now delete a key that's not there, which should just succeed.
	error_check_good cdmldb_del4 [eval $cdmldb del $key4] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	# Verify key1 is gone.
	set ret [lindex [$cdmldb get $key1] 0]
	error_check_good cdmldb_get1 $ret ""
	# Verify key2 has updated value.
	set ret [lindex [$cdmldb get $key2] 0]
	error_check_good cdmldb_get2 $ret [list $key2 $data2]
	# Verify key3 is there.
	set ret [lindex [$cdmldb get $key3] 0]
	error_check_good cdmldb_get3 $ret [list $key3 $data3]

	# Make sure stats reflect the 6 write operations above.
	error_check_good mrcvstat [stat_field $masterenv repmgr_stat \
	    "Forwarded write operations received"] 6
	error_check_good cforstat [stat_field $clientenv repmgr_stat \
	    "Write operations forwarded"] 6

	puts "\tRepmgr$tnum.e: EACCES if write forwarding disabled on master."
	set key 4
	$masterenv rep_config {mgrforwardwrites off}
	catch { $cdmldb put $key [chop_data $method data$key] } res
	error_check_good mwfoffp [is_substr $res "permission denied"] 1
	catch { $cdmldb del $key } res
	error_check_good mwfoffd [is_substr $res "permission denied"] 1
	# Reenable write forwarding on master and make sure it works.
	$masterenv rep_config {mgrforwardwrites on}
	error_check_good cdmldb_put$key \
	    [eval $cdmldb put $key [chop_data $method data$key]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	set ret [lindex [$cdmldb get $key] 0]
	error_check_good cdmldb_get$key $ret \
	    [list $key [pad_data $method data$key]]

	puts "\tRepmgr$tnum.f: EACCES if write forwarding disabled on client."
	set key 5
	$clientenv rep_config {mgrforwardwrites off}
	catch { $cdmldb put $key [chop_data $method data$key] } res
	error_check_good cwfoffp [is_substr $res "permission denied"] 1
	catch { $cdmldb del $key } res
	error_check_good cwfoffd [is_substr $res "permission denied"] 1
	# Reenable write forwarding on client and make sure it works.
	$clientenv rep_config {mgrforwardwrites on}
	error_check_good cdmldb_put$key \
	    [eval $cdmldb put $key [chop_data $method data$key]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	set ret [lindex [$cdmldb get $key] 0]
	error_check_good cdmldb_get$key $ret \
	    [list $key [pad_data $method data$key]]

	puts "\tRepmgr$tnum.g: EACCES if non-NULL transaction."
	set key 6
	set t [$clientenv txn]
	catch { $cdmldb put -txn $t $key [chop_data $method data$key] } res
	error_check_good ctxnp [is_substr $res "permission denied"] 1
	catch { $cdmldb del -txn $t $key } res
	error_check_good ctxnd [is_substr $res "permission denied"] 1
	error_check_good txn_abort [$t abort] 0

	puts "\tRepmgr$tnum.h: EACCES if cursor put or del."
	set key 7
	set c [$cdmldb cursor]
	catch { $c put $key [chop_data $method data$key] } res
	error_check_good cursput [is_substr $res "permission denied"] 1
	error_check_good cdmldb_put$key \
	    [eval $cdmldb put $key [chop_data $method data$key]] 0
	catch { $c del $key } res
	error_check_good cursdel [is_substr $res "permission denied"] 1
	$c close

	puts "\tRepmgr$tnum.i: DB_TIMEOUT if operation takes too long."
	set key 8
	# Set a tiny write forwarding timeout.
	$clientenv repmgr -timeout {write_forward 3}
	catch { $cdmldb put $key [chop_data $method data$key] } res
	error_check_good ctimeoutp [is_substr $res "timed out"] 1
	catch { $cdmldb del $key } res
	error_check_good ctimeoutd [is_substr $res "timed out"] 1
	# Restore reasonable timeout and make sure write forwarding works.
	$clientenv repmgr -timeout {write_forward 3000000}
	error_check_good cdmldb_put$key \
	    [eval $cdmldb put $key [chop_data $method data$key]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	set ret [lindex [$cdmldb get $key] 0]
	error_check_good cdmldb_get$key $ret \
	    [list $key [pad_data $method data$key]]

	error_check_good cdmldb_close [$cdmldb close] 0
	error_check_good mdmldb_close [$mdmldb close] 0

	puts "\tRepmgr$tnum.j: EACCES if no open master database handle."
	set cdmldb [eval "berkdb_open_noerr $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]
	set key 9
	catch { $cdmldb put $key [chop_data $method data$key] } res
	error_check_good cnomashdlp [is_substr $res "permission denied"] 1
	catch { $cdmldb del $key } res
	error_check_good cnomashdld [is_substr $res "permission denied"] 1
	# Open master database handle and make sure write forwarding works.
	set mdmldb [eval "berkdb_open_noerr $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	error_check_good cdmldb_put$key \
	    [eval $cdmldb put $key [chop_data $method data$key]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	set ret [lindex [$cdmldb get $key] 0]
	error_check_good cdmldb_get$key $ret \
	    [list $key [pad_data $method data$key]]

	puts "\tRepmgr$tnum.k: EACCES for unsupported bulk put or del."
	set key 10
	catch { $cdmldb put -multiple $key [chop_data $method data$key] } res
	error_check_good cnobulkp [is_substr $res "permission denied"] 1
	catch { $cdmldb del -multiple_key $key } res
	error_check_good cnobulkd [is_substr $res "permission denied"] 1

	error_check_good mdmldb_close [$mdmldb close] 0
	error_check_good cdmldb_close [$cdmldb close] 0

	puts "\tRepmgr$tnum.l: Verify master and client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0

	puts "\tRepmgr$tnum.m: EACCES if no master site in repgroup."
	# Use of default 2SITE_STRICT ensures client won't elect itself master.
	set key 11
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all -pri 0 \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -timeout [list write_forward 3000000] \
	    -start client
	set cdmldb [eval "berkdb_open_noerr $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]
	catch { $cdmldb put $key [chop_data $method data$key] } res
	error_check_good cnomasp [is_substr $res "permission denied"] 1
	catch { $cdmldb del $key } res
	error_check_good cnomasd [is_substr $res "permission denied"] 1
	error_check_good cdmldb_close [$cdmldb close] 0
	error_check_good client_close [$clientenv close] 0
}

#
# Ensure that write forwarding continues working in a 3-site replication
# group that goes through a change of master.  Also ensure that the
# original master restarted as a client can forward writes to the new
# master.
#
proc repmgr046_3site { method niter tnum largs } {
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
	set dmlsleep 1

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2

	puts "\tRepmgr$tnum.3s.a: Start a master and two clients."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list write_forward 4000000] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all -pri 50 \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 rep_config {mgrforwardwrites on}
	$clientenv2 repmgr -ack all -pri 30 \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv2

	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}

	set mdb3s [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	set cdb3s [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]
	set c2db3s [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv2 $largs $dbname"]

	puts "\tRepmgr$tnum.3s.b: Do a DML on each client, verify replication."
	set key1 1
	set key2 2
	error_check_good cdb3s_put1 \
	    [eval $cdb3s put $key1 [chop_data $method data$key1]] 0
	error_check_good c2db3s_put2 \
	    [eval $c2db3s put $key2 [chop_data $method data$key2]] 0
	# Allow time for DML to be replicated back to clients.
	tclsleep $dmlsleep
	# Verify key1 and key2 are present on all sites.
	set ret [lindex [$mdb3s get $key1] 0]
	error_check_good mdb3s_get1 $ret \
	    [list $key1 [pad_data $method data$key1]]
	set ret [lindex [$mdb3s get $key2] 0]
	error_check_good mdb3s_get2 $ret \
	    [list $key2 [pad_data $method data$key2]]
	set ret [lindex [$cdb3s get $key1] 0]
	error_check_good cdb3s_get1 $ret \
	    [list $key1 [pad_data $method data$key1]]
	set ret [lindex [$cdb3s get $key2] 0]
	error_check_good cdb3s_get2 $ret \
	    [list $key2 [pad_data $method data$key2]]
	set ret [lindex [$c2db3s get $key1] 0]
	error_check_good c2db3s_get1 $ret \
	    [list $key1 [pad_data $method data$key1]]
	set ret [lindex [$c2db3s get $key2] 0]
	error_check_good c2db3s_get2 $ret \
	    [list $key2 [pad_data $method data$key2]]

	# Do not close client handles - this test needs to make sure
	# they are still usable after the change of master because an
	# application exclusively using write forwarding shouldn't need
	# to track changes of master.
	error_check_good mdb3s_close [$mdb3s close] 0

	puts "\tRepmgr$tnum.3s.c: Shut down master, client takes over."
	error_check_good master_close [$masterenv close] 0
	await_expected_master $clientenv
	await_startup_done $clientenv2

	puts "\tRepmgr$tnum.3s.d: Verify client2 can do DML with new master."
	set key3 3
	error_check_good c2db3s_put3 \
	    [eval $c2db3s put $key3 [chop_data $method data$key3]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	# Verify key3 is present on both sites.
	set ret [lindex [$cdb3s get $key3] 0]
	error_check_good cdb3s_get3 $ret \
	    [list $key3 [pad_data $method data$key3]]
	set ret [lindex [$c2db3s get $key3] 0]
	error_check_good c2db3s_get3 $ret \
	    [list $key3 [pad_data $method data$key3]]

	puts "\tRepmgr$tnum.3s.e: Restart original master to rejoin as client."
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list write_forward 4000000] \
	    -start client
	await_startup_done $masterenv
	set mdb3s [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]

	puts "\tRepmgr$tnum.3s.f: Verify master rejoining as client can do DML."
	set key4 4
	error_check_good mdb3s_put4 \
	    [eval $mdb3s put $key4 [chop_data $method data$key4]] 0
	# Allow time for DML to be replicated back to clients.
	tclsleep $dmlsleep
	# Verify key4 is present on all sites.
	set ret [lindex [$mdb3s get $key4] 0]
	error_check_good mdb3s_get4 $ret \
	    [list $key4 [pad_data $method data$key4]]
	set ret [lindex [$cdb3s get $key4] 0]
	error_check_good cdb3s_get4 $ret \
	    [list $key4 [pad_data $method data$key4]]
	set ret [lindex [$c2db3s get $key4] 0]
	error_check_good c2db3s_get4 $ret \
	    [list $key4 [pad_data $method data$key4]]

	error_check_good c2db3s_close [$c2db3s close] 0
	error_check_good cdb3s_close [$cdb3s close] 0
	error_check_good mdb3s_close [$mdb3s close] 0

	puts "\tRepmgr$tnum.3s.g: Verify master and client database contents."
	rep_verify $clientdir $clientenv $masterdir $masterenv 1 1 1
	rep_verify $clientdir $clientenv $clientdir2 $clientenv2 1 1 1

	error_check_good masterenv_close [$masterenv close] 0
	error_check_good client2_close [$clientenv2 close] 0
	error_check_good client_close [$clientenv close] 0
}

#
# Note that blobs are not supported with in-memory databases, so this
# test case should only be run with on-disk database files.
#
proc repmgr046_blob { method niter tnum largs } {
	global testdir
	global rep_verbose
	global verbose_type
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
	set dmlsleep 1

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.bl.a: Start a master and a client."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -timeout [list write_forward 4000000] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all -pri 50 \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	set dbname  "test.db"

	puts "\tRepmgr$tnum.3s.b: Test write forwarding blobs."
	set mdbblob [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv -blob_threshold 10 $largs $dbname"]
	set cdbblob [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv -blob_threshold 10 $largs $dbname"]

	set keyb1 1
	set keyb2 2
	set keyb3 3
	set b2_data [string repeat "a" 100]
	set b3_data [string repeat "b" 1000]

	error_check_good cdbblob_putb1 \
	    [eval $cdbblob put $keyb1 "under"] 0
	error_check_good cdbblob_putb2 [eval $cdbblob put $keyb2 $b2_data] 0
	error_check_good cdbblob_putb3 [eval $cdbblob put $keyb3 $b3_data] 0

	# Allow time for DML to be replicated back to clients.
	tclsleep $dmlsleep
	set ret [lindex [$cdbblob get $keyb1] 0]
	error_check_good cdbblob_getb1 $ret [list $keyb1 "under"]
	set ret [lindex [$cdbblob get $keyb2] 0]
	error_check_good cdbblob_getb2 $ret [list $keyb2 $b2_data]
	set ret [lindex [$cdbblob get $keyb3] 0]
	error_check_good cdbblob_getb3 $ret [list $keyb3 $b3_data]
	set ret [lindex [$mdbblob get $keyb1] 0]
	error_check_good mdbblob_getb1 $ret [list $keyb1 "under"]
	set ret [lindex [$mdbblob get $keyb2] 0]
	error_check_good mdbblob_getb2 $ret [list $keyb2 $b2_data]
	set ret [lindex [$mdbblob get $keyb3] 0]
	error_check_good mdbblob_getb3 $ret [list $keyb3 $b3_data]

	error_check_good cdbblob_close [$cdbblob close] 0
	error_check_good mdbblob_close [$mdbblob close] 0

	puts "\tRepmgr$tnum.bl.c: Verify master and client database contents."
	rep_verify $masterdir $masterenv $clientdir $clientenv 1 1 1

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

# Test write forwarding to subdatabases and some cases using duplicate values.
proc repmgr046_subdb { method niter tnum largs } {
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
	set dmlsleep 1

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	puts "\tRepmgr$tnum.sdb.a: Start a master and a client."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all -pri 50 \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	if {$databases_in_memory} {
		#
		# Create separate in-memory databases oddtest.db and
		# eventest.db because there can't be in-memory subdatabases.
		#
		set odbname { "" "oddtest.db" }
		set edbname { "" "eventest.db" }
		set modb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
		    -auto_commit -env $masterenv $largs $odbname"]
		set medb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
		    -auto_commit -env $masterenv $largs $edbname"]
		set codb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
		    -auto_commit -env $clientenv $largs $odbname"]
		set cedb [eval "berkdb_open_noerr -create -mode 0644 $omethod \
		    -auto_commit -env $clientenv $largs $edbname"]
	} else {
		#
		# Create a single database file test.db that contains the
		# two subdatabases oddsubdb and evensubdb.
		#
		set dbname "test.db"
		set osub "oddsubdb"
		set esub "evensubdb"	
		set modb [eval "berkdb_open_noerr -create $omethod \
		    -auto_commit -env $masterenv $largs $dbname $osub"]
		set medb [eval "berkdb_open_noerr -create $omethod \
		    -auto_commit -env $masterenv $largs $dbname $esub"]
		set codb [eval "berkdb_open_noerr -create $omethod \
		    -auto_commit -env $clientenv $largs $dbname $osub"]
		set cedb [eval "berkdb_open_noerr -create $omethod \
		    -auto_commit -env $clientenv $largs $dbname $esub"]
	}
	error_check_good modb_open [is_valid_db $modb] TRUE
	error_check_good medb_open [is_valid_db $medb] TRUE
	error_check_good codb_open [is_valid_db $codb] TRUE
	error_check_good cedb_open [is_valid_db $cedb] TRUE

	puts "\tRepmgr$tnum.sdb.b: Test client DMLs to subdatabases."
	set key1 1
	set key2 2
	set key3 3
	set key4 4
	error_check_good codb_put1 \
	    [eval $codb put $key1 [chop_data $method data$key1]] 0
	error_check_good cedb_put2 \
	    [eval $cedb put $key2 [chop_data $method data$key2]] 0
	# Put key3 in wrong subdb, give key4 wrong data.
	error_check_good cedb_put3 \
	    [eval $cedb put $key3 [chop_data $method data$key3]] 0
	error_check_good cedb_put4wrongdata \
	    [eval $cedb put $key4 [chop_data $method data$key3]] 0
	# Correct key3 and key4.
	error_check_good cedb_del1 [eval $cedb del $key3] 0
	error_check_good codb_put3 \
	    [eval $codb put $key3 [chop_data $method data$key3]] 0
	error_check_good cedb_put4 \
	    [eval $cedb put $key4 [chop_data $method data$key4]] 0
	# Allow time for DML to be replicated back to clients.
	tclsleep $dmlsleep
	# Verify keys are present in the correct database.
	set ret [lindex [$codb get $key1] 0]
	error_check_good codb_get1 $ret \
	    [list $key1 [pad_data $method data$key1]]
	set ret [lindex [$cedb get $key2] 0]
	error_check_good cedb_get2 $ret \
	    [list $key2 [pad_data $method data$key2]]
	set ret [lindex [$codb get $key3] 0]
	error_check_good codb_get3 $ret \
	    [list $key3 [pad_data $method data$key3]]
	set ret [lindex [$cedb get $key4] 0]
	error_check_good cedb_get4 $ret \
	    [list $key4 [pad_data $method data$key4]]
	# Verify keys are not present in the other database.
	set ret [lindex [$cedb get $key1] 0]
	error_check_good cedb_badget1 $ret ""
	set ret [lindex [$codb get $key2] 0]
	error_check_good codb_badget2 $ret ""
	set ret [lindex [$cedb get $key3] 0]
	error_check_good cedb_badget3 $ret ""
	set ret [lindex [$codb get $key4] 0]
	error_check_good codb_badget4 $ret ""

	error_check_good cedb_close [$cedb close] 0
	error_check_good codb_close [$codb close] 0
	error_check_good medb_close [$medb close] 0
	error_check_good modb_close [$modb close] 0

	puts "\tRepmgr$tnum.sdb.c: Test client DMLs with duplicates."
	if {$databases_in_memory} {
		set dupdbname { "" "duptest.db" }
	} else {
		set dupdbname  "duptest.db"
	}

	# Need to sort duplicates for a put -nodupdata to work.
	set mdupdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv -dup -dupsort $largs $dupdbname"]
	set cdupdb [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv -dup -dupsort $largs $dupdbname"]

	# Insert multiple values for key1 and key2.
	error_check_good cdupdb_put1 \
	    [eval $cdupdb put $key1 [chop_data $method data$key1]] 0
	error_check_good cdupdb_put1dup1 \
	    [eval $cdupdb put $key1 [chop_data $method datadup1$key1]] 0
	error_check_good cdupdb_put1dup2 \
	    [eval $cdupdb put $key1 [chop_data $method datadup2$key1]] 0
	error_check_good cdupdb_put2 \
	    [eval $cdupdb put $key2 [chop_data $method data$key2]] 0
	error_check_good cdupdb_put1dup2 \
	    [eval $cdupdb put $key2 [chop_data $method datadup1$key2]] 0
	# Make sure putting truly duplicate value returns KEYEXIST.
	catch { $cdupdb put -nodupdata $key2 \
	    [chop_data $method datadup1$key2] } res
	error_check_good cdupdb_dupdat [is_substr $res "pair already exists"] 1
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	# Verify all dup values are there.
	set ret [lindex [$cdupdb get -get_both $key1 data$key1] 0]
	error_check_good cdupdb_get1 $ret [list $key1 data$key1]
	set ret [lindex [$cdupdb get -get_both $key1 datadup1$key1] 0]
	error_check_good cdupdb_get1dup1 $ret [list $key1 datadup1$key1]
	set ret [lindex [$cdupdb get -get_both $key1 datadup2$key1] 0]
	error_check_good cdupdb_get1dup2 $ret [list $key1 datadup2$key1]
	set ret [lindex [$cdupdb get -get_both $key2 data$key2] 0]
	error_check_good cdupdb_get2 $ret [list $key2 data$key2]
	set ret [lindex [$cdupdb get -get_both $key2 datadup1$key2] 0]
	error_check_good cdupdb_get2dup1 $ret [list $key2 datadup1$key2]

	# Now delete key1, which deletes all of its duplicate values.
	error_check_good cdupdb_del1 [eval $cdupdb del $key1] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep
	# Verify key1 is gone.
	set ret [lindex [$cdupdb get $key1] 0]
	error_check_good cdupdb_get1 $ret ""

	error_check_good cdupdb_close [$cdupdb close] 0
	error_check_good mdupdb_close [$mdupdb close] 0

	puts "\tRepmgr$tnum.sdb.d: Verify master and client database contents."
	if {$databases_in_memory} {
		rep_verify $clientdir $clientenv $masterdir $masterenv 1 1 1 \
		    oddtest.db
		rep_verify $clientdir $clientenv $masterdir $masterenv 1 1 1 \
		    eventest.db
	} else {
		rep_verify $clientdir $clientenv $masterdir $masterenv 1 1 1 \
		    duptest.db
		rep_verify $clientdir $clientenv $masterdir $masterenv 1 1 1
	}

	error_check_good client_close [$clientenv close] 0
	error_check_good masterenv_close [$masterenv close] 0
}

#
# Test write forwarding in a scenario that generates a HANDLE_DEAD
# error because some client transactions were rolled back.
#
proc repmgr046_hdldead { method niter tnum largs } {
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
	set dmlsleep 1

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR
	set clientdir2 $testdir/CLIENTDIR2

	file mkdir $masterdir
	file mkdir $clientdir
	file mkdir $clientdir2

	#
	# Using noelections mode because we need tight control over which
	# site gets appointed master later in the test, otherwise a
	# different site might be elected.
	#
	puts "\tRepmgr$tnum.hd.a: Start a master and two clients."
	set ma_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx MASTER -home $masterdir -txn -rep -thread"
	set masterenv [eval $ma_envcmd]
	$masterenv rep_config {mgrelections off}
	$masterenv rep_config {mgrforwardwrites on}
	$masterenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 0]] \
	    -start master

	set cl_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT -home $clientdir -txn -rep -thread"
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrelections off}
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv

	set cl2_envcmd "berkdb_env_noerr -create $verbargs \
	    -errpfx CLIENT2 -home $clientdir2 -txn -rep -thread"
	set clientenv2 [eval $cl2_envcmd]
	$clientenv2 rep_config {mgrelections off}
	$clientenv2 rep_config {mgrforwardwrites on}
	$clientenv2 repmgr -ack all \
	    -local [list $hoststr [lindex $ports 2]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start client
	await_startup_done $clientenv2

	if {$databases_in_memory} {
		set dbname { "" "test.db" }
	} else {
		set dbname  "test.db"
	}

	set mdbhd [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $masterenv $largs $dbname"]
	set c2dbhd [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv2 $largs $dbname"]

	puts "\tRepmgr$tnum.hd.b: Do first DML on client2."
	set key1 1
	set key2 2
	error_check_good c2dbhd_put1 \
	    [eval $c2dbhd put $key1 [chop_data $method data$key1]] 0
	# Allow time for DML to be replicated back to clients.
	tclsleep $dmlsleep

	puts "\tRepmgr$tnum.hd.c: Close client and do more master DMLs."
	set start 0
	error_check_good client_close [$clientenv close] 0
	eval rep_test $method $masterenv NULL $niter $start 0 0 $largs
	incr start $niter

	puts "\tRepmgr$tnum.hd.d: Close master, restart client as master."
	error_check_good mdbhd_close [$mdbhd close] 0
	error_check_good master_close [$masterenv close] 0
	set clientenv [eval $cl_envcmd]
	$clientenv rep_config {mgrelections off}
	$clientenv rep_config {mgrforwardwrites on}
	$clientenv repmgr -ack all \
	    -local [list $hoststr [lindex $ports 1]] \
	    -remote [list $hoststr [lindex $ports 0]] \
	    -start master
	await_expected_master $clientenv
	#
	# This client sync makes client2 roll back its most recent master
	# transactions.
	#
	await_startup_done $clientenv2
	set cdbhd [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv $largs $dbname"]

	puts "\tRepmgr$tnum.hd.e: HANDLE_DEAD for forwarded put using old\
	    handle."
	# Allow time for repmgr connections to get reestablished.
	tclsleep 2
	catch { $c2dbhd put $key2 [chop_data $method data$key2] } res
	error_check_good c2dbhd_hdead [is_substr $res "no longer valid"] 1

	puts "\tRepmgr$tnum.hd.f: Close/reopen db handle, put succeeds."
	error_check_good c2dbhd_close [$c2dbhd close] 0
	set c2dbhd [eval "berkdb_open_noerr -create $omethod -auto_commit \
	    -env $clientenv2 $largs $dbname"]
	error_check_good c2dbhd_put2 \
	    [eval $c2dbhd put $key2 [chop_data $method data$key2]] 0
	# Allow time for DML to be replicated back to client.
	tclsleep $dmlsleep

	puts "\tRepmgr$tnum.hd.g: Verify both forwarded DMLs."
	set ret [lindex [$c2dbhd get $key1] 0]
	error_check_good c2dbhd_get1 $ret \
	    [list $key1 [pad_data $method data$key1]]
	set ret [lindex [$c2dbhd get $key2] 0]
	error_check_good c2dbhd_get2 $ret \
	    [list $key2 [pad_data $method data$key2]]

	error_check_good c2dbhd_close [$c2dbhd close] 0
	error_check_good cdbhd_close [$cdbhd close] 0

	puts "\tRepmgr$tnum.hd.h: Verify database contents."
	rep_verify $clientdir $clientenv $clientdir2 $clientenv2 1 1 1

	error_check_good client2_close [$clientenv2 close] 0
	error_check_good client_close [$clientenv close] 0
}
