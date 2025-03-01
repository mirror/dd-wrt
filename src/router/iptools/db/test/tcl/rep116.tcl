# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	rep116
# TEST	Test of instant internal initialization, where internal init is started
# TEST	instantly if a file delete is found while walking back through the
# TEST	logs during the verify step.
# TEST
# TEST	One master, one client.
# TEST	Generate several log files.
# TEST	Remove old master and client log files.
# TEST	Create a network partition between the master and client,
# TEST	and restart the client as a master.
# TEST	Delete a database or blob file on the client, then close the client and
# TEST	have it rejoin the master.  Assert that the deleted file is present on
# TEST	the client.
#
proc rep116 { method { niter 200 } { tnum "116" } args } {

	source ./include.tcl
	global databases_in_memory
	global env_private
	global repfiles_in_memory

	if { $checking_valid_methods } {
		return "ALL"
	}

	set args [convert_args $method $args]
	set saved_args $args

	set msg "and on-disk replication files"
	if { $repfiles_in_memory } {
		set msg "and in-memory replication files"
	}

	set msg2 ""
	if { $env_private } {
		set msg2 "with private env"
	}

	if { $databases_in_memory } {
		puts "Skipping rep$tnum for in-memory databases."
		return
	}

	# Delete database 1, database 2, a blob file, or nothing
	set del_opt { "db1" "db2" "blob" "none" }

	foreach r $test_recopts {
		foreach o $del_opt {
			set envargs ""
			set args $saved_args
			puts "Rep$tnum ($method $envargs $r deleting $o \
			    $args): Test of internal initialization $msg \
			    $msg2."
			rep116_sub $method $niter $tnum $envargs $r $o $args
		}
	}
}

proc rep116_sub { method niter tnum envargs recargs del_opts largs } {
	global testdir
	global util_path
	global env_private
	global repfiles_in_memory
	global rep_verbose
	global verbose_type

	set verbargs ""
	if { $rep_verbose == 1 } {
		set verbargs " -verbose {$verbose_type on} "
	}

	set repmemargs ""
	if { $repfiles_in_memory } {
		set repmemargs "-rep_inmem_files "
	}

	set privargs ""
	if { $env_private } {
		set privargs " -private "
	}

	set blobargs ""
	set num 10
    	if { [can_support_blobs $method $largs] == 1 } {
		set blobargs "-blob_threshold 100"
	} else {
		if { $del_opts == "blob" } {
puts "\tRep$tnum: Skipping blob file delete test, blobs not supported."
		return
		}
		set num 50	
	}

	env_cleanup $testdir

	replsetup $testdir/MSGQUEUEDIR

	set masterdir $testdir/MASTERDIR
	set clientdir $testdir/CLIENTDIR

	file mkdir $masterdir
	file mkdir $clientdir

	# Log size is small so we quickly create more than one.
	# The documentation says that the log file must be at least
	# four times the size of the in-memory log buffer.
	set pagesize 4096
	append largs " -pagesize $pagesize "
	set log_max [expr $pagesize * 8]

	set file1 "test1.db"
	set file2 "test2.db"

	# Open a master.
	puts "\tRep$tnum.a: Opening the master and client."
	repladd 1
	set ma_envcmd "berkdb_env_noerr -create -txn $repmemargs \
	    $privargs -log_max $log_max $envargs $verbargs \
	    -errpfx MASTER -home $masterdir \
	    $blobargs -rep_transport \[list 1 replsend\]"
	set masterenv [eval $ma_envcmd $recargs -rep_master]

	# Open a client
	repladd 2
	set cl_envcmd "berkdb_env_noerr -create -txn $repmemargs \
	    $privargs -log_max $log_max $envargs $verbargs \
	    -errpfx CLIENT -home $clientdir \
	    $blobargs -rep_transport \[list 2 replsend\]"
	set clientenv [eval $cl_envcmd $recargs -rep_client]

	# Bring the clients online by processing the startup messages.
	set envlist "{$masterenv 1} {$clientenv 2}"
	process_msgs $envlist

	puts "\tRep$tnum.b: Creating database 1 and database 2"
	# Create two databases.  db1 does not support blobs, db2 may support
	# blobs if the environment supports them.  Replicate the new dbs.
	set omethod [convert_method $method]
	set oargs [convert_args $method $largs]
	set oflags " -create -auto_commit -blob_threshold 0 -env \
	    $masterenv $omethod "
	set db1 [eval {berkdb_open_noerr} $oflags $oargs $file1]
	error_check_good db1open [is_valid_db $db1] TRUE
	eval rep_test $method $masterenv $db1 $num 0 0 0 $largs
	process_msgs $envlist

	set oflags " -env $masterenv $omethod $blobargs -create -auto_commit "
	set db2 [eval {berkdb_open_noerr} $oflags $oargs $file2]
	error_check_good db2open [is_valid_db $db2] TRUE
	eval rep_test $method $masterenv $db2 $num 0 0 0 $largs
	process_msgs $envlist

	# Clobber replication's 30-second anti-archive timer, which will have
	# been started by client sync-up internal init, so that we can do a
	# log_archive in a moment.
	#
	$masterenv test force noarchive_timeout

	puts "\tRep$tnum.c: Run db_archive on master."
	$masterenv log_flush
	set res [eval exec $util_path/db_archive -l -h $masterdir]
	error_check_bad log.1.present [lsearch -exact $res log.0000000001] -1
	set res [eval exec $util_path/db_archive -d -h $masterdir]
	set res [eval exec $util_path/db_archive -l -h $masterdir]
	error_check_good log.1.gone [lsearch -exact $res log.0000000001] -1
	process_msgs $envlist

	puts "\tRep$tnum.d: Run db_archive on client."
	$clientenv log_flush
	set res [eval exec $util_path/db_archive -l -h $clientdir]
	error_check_bad log.1.present [lsearch -exact $res log.0000000001] -1
	set res [eval exec $util_path/db_archive -d -h $clientdir]
	set res [eval exec $util_path/db_archive -l -h $clientdir]
	error_check_good log.1.gone [lsearch -exact $res log.0000000001] -1
	process_msgs $envlist

	puts "\tRep$tnum.e: Close the client and reopen as a master."
	error_check_good clientenv_close [$clientenv close] 0
	set clientenv [eval $cl_envcmd $recargs -rep_master]

	# Perform the delete on the client.
	if { $del_opts == "db1" } {
		puts "\tRep$tnum.f: Client deletes database 1."
		error_check_good remove_1 \
		    [eval {$clientenv dbremove -auto_commit} $file1] 0
	} elseif { $del_opts == "db2" } {
		puts "\tRep$tnum.f: Client deletes database 2."
		error_check_good remove_2 \
		    [eval {$clientenv dbremove -auto_commit} $file2] 0
	} elseif { $del_opts == "blob" } {
		puts "\tRep$tnum.f: Client deletes a blob record."
		# Blobs are inserted at the end, so delete the last record
		set oflags " -env $clientenv $omethod -auto_commit "
		set db2_cli [eval {berkdb_open_noerr} $oflags $oargs $file2]
		set txn [eval $clientenv txn]
		set dbc [eval $db2_cli cursor -txn $txn]
		set ret [eval $dbc get -last]
		error_check_good blob_del [eval $dbc del] 0
		error_check_good cursor_close [$dbc close] 0
		error_check_good txn_commit [$txn commit] 0
		error_check_good close_db2_cli [$db2_cli close] 0
	} elseif { $del_opts == "none" } {
		puts "\tRep$tnum.f: Client does nothing."
	}
	eval rep_test $method $masterenv $db1 10 0 0 0 $largs
	$clientenv log_flush
	$masterenv log_flush

	puts "\tRep$tnum.g: Close the client and master and reopen."
	error_check_good clientenv_close [$clientenv close] 0
	error_check_good db1_close [$db1 close] 0
	error_check_good db2_close [$db2 close] 0
	error_check_good masterenv_close [$masterenv close] 0
	# Clear any messages in the message queues
	replclear 1
	replclear 2
	
	set clientenv [eval $cl_envcmd -recover -rep_client]
	set masterenv [eval $ma_envcmd -recover -rep_master]
	set envlist "{$masterenv 1} {$clientenv 2}"
	process_msgs $envlist

	puts "\tRep$tnum.g: Verify the databases and files exist."
	# Verify the databases are all still there and the same
	rep_verify $masterdir $masterenv $clientdir $clientenv 0 1 0 $file1
	rep_verify $masterdir $masterenv $clientdir $clientenv 0 1 0 $file2

	error_check_good masterenv_close [$masterenv close] 0
	error_check_good clientenv_close [$clientenv close] 0
	replclose $testdir/MSGQUEUEDIR
}

