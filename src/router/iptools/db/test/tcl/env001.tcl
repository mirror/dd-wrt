# See the file LICENSE for redistribution information.
#
# Copyright (c) 1999, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env001
# TEST	Test of env remove interface (formerly env_remove).
proc env001 { {args ""} } {
	global errorInfo
	global errorCode
	global number_of_slices

	source ./include.tcl

	set testfile $testdir/env.db
	set t1 $testdir/t1

	puts "Env001: Test of environment remove interface."
	if { [llength $args] > 0 } { 
		puts "Env001: with $args"
	}
	env_cleanup $testdir

	# Try opening without Create flag should error
	puts "\tEnv001.a: Open without create (should fail)."
	catch {set env [berkdb_env_noerr -home $testdir]} ret
	error_check_good env:fail [is_substr $ret "no such file"] 1

	# Now try opening with create
	puts "\tEnv001.b: Open with create."
	set env [berkdb_env -create -mode 0644 -home $testdir]
	error_check_bad env:$testdir $env NULL
	error_check_good env:$testdir [is_substr $env "env"] 1

	# Make sure that close works.
	puts "\tEnv001.c: Verify close."
	error_check_good env:close:$env [$env close] 0

	# Make sure we can reopen.
	puts "\tEnv001.d: Remove on closed environments."
	puts "\t\tEnv001.d.1: Verify re-open."
	set env [berkdb_env -home $testdir]
	error_check_bad env:$testdir $env NULL
	error_check_good env:$testdir [is_substr $env "env"] 1

	# remove environment
	puts "\t\tEnv001.d.2: Close environment."
	error_check_good env:close [$env close] 0
	puts "\t\tEnv001.d.3: Try remove with force (should succeed)."
	error_check_good \
	    envremove [berkdb envremove -force -home $testdir] 0

	# HP-UX doesn't allow a second handle on an open env.
	if { $is_hp_test != 1 } {
		puts "\tEnv001.e: Remove on open environments."
		puts "\t\tEnv001.e.1: Env is open by single proc,\
		    remove no force."
		set env [berkdb_env -create -mode 0644 -home $testdir]
		error_check_bad env:$testdir $env NULL
		error_check_good env:$testdir [is_substr $env "env"] 1
		set stat [catch {berkdb envremove -home $testdir} ret]
		error_check_good env:remove $stat 1
		error_check_good env:close [$env close] 0
	}

	puts \
	    "\t\tEnv001.e.2: Env is open by single proc, remove with force."
	if { $is_hp_test != 1 } {
		set env [berkdb_env_noerr -create -mode 0644 -home $testdir]
		error_check_bad env:$testdir $env NULL
		error_check_good env:$testdir [is_substr $env "env"] 1
		set stat [catch {berkdb envremove -force -home $testdir} ret]
		error_check_good env:remove(force) $ret 0
		#
		# Even though the underlying env is gone, we need to close
		# the handle.
		#
		set stat [catch {$env close} ret]
		error_check_bad env:close_after_remove $stat 0
		error_check_good env:close_after_remove \
		    [is_substr $ret "recovery"] 1
	}

	puts "\t\tEnv001.e.3: Env is open by 2 procs, remove no force."
	# should fail
	set env [berkdb_env -create -mode 0644 -home $testdir]
	error_check_bad env:$testdir $env NULL
	error_check_good env:$testdir [is_substr $env "env"] 1

	set f1 [open |$tclsh_path r+]
	puts $f1 "source $test_path/test.tcl"

	set remote_env [send_cmd $f1 "berkdb_env_noerr -home $testdir"]
	error_check_good remote:env_open [is_valid_env $remote_env] TRUE
	# First close our env, but leave remote open
	error_check_good env:close [$env close] 0
	catch {berkdb envremove -home $testdir} ret
	error_check_good envremove:2procs:noforce [is_substr $errorCode EBUSY] 1
	#
	# even though it failed, $env is no longer valid, so remove it in
	# the remote process
	set remote_close [send_cmd $f1 "$remote_env close"]
	error_check_good remote_close $remote_close 0

	# exit remote process
	set err [catch { close $f1 } result]
	error_check_good close_remote_process $err 0

	puts "\t\tEnv001.e.4: Env is open by 2 procs, remove with force."
	if { $is_hp_test != 1 } {
		set env [berkdb_env_noerr -create -mode 0644 -home $testdir]
		error_check_bad env:$testdir $env NULL
		error_check_good env:$testdir [is_substr $env "env"] 1
		set f1 [open |$tclsh_path r+]
		puts $f1 "source $test_path/test.tcl"

		set remote_env [send_cmd $f1 "berkdb_env -home $testdir"]
		error_check_good remote:env_open [is_valid_env $remote_env] TRUE

		catch {berkdb envremove -force -home $testdir} ret
		error_check_good envremove:2procs:force $ret 0
		#
		# We still need to close our handle.
		#
		set stat [catch {$env close} ret]
		error_check_bad env:close_after_error $stat 0
		error_check_good env:close_after_error \
		    [is_substr $ret recovery] 1

		# Close down remote process
		set err [catch { close $f1 } result]
		error_check_good close_remote_process $err 0
	}

	# Try opening in a different dir
	puts "\tEnv001.f: Try opening env in another directory."
	if { [file exists $testdir/NEWDIR] != 1 } {
		file mkdir $testdir/NEWDIR
	}
	set eflags "-create -home $testdir/NEWDIR -mode 0644"
	set env [eval {berkdb_env} $eflags]
	error_check_bad env:open $env NULL
	error_check_good env:close [$env close] 0
	error_check_good berkdb:envremove \
	    [berkdb envremove -home $testdir/NEWDIR] 0

	puts "\tEnv001.g: Remove environment when Region dir is set."
	set regdir "REGIONDIR"
    	file mkdir $testdir/$regdir
	for {set i 0} {$i < $number_of_slices} {incr i} {
		file mkdir $testdir/__db.slice00$i
		file mkdir $testdir/__db.slice00$i/$regdir
	}
	if {$number_of_slices > 0} {
	    	set container [list "set_region_dir $regdir"]
	        set sliceall [list "set_region_dir $regdir"]
		slice_db_config $number_of_slices $container $sliceall
	}
	set env [berkdb_env -create \
	    -mode 0644 -home $testdir -region_dir $regdir]
    	error_check_good regenv_open [is_valid_env $env] TRUE
    	error_check_good regenv_close [$env close] 0
    	error_check_good reg_exists [file exists $testdir/$regdir/__db.001] 1
	for {set i 0} {$i < $number_of_slices} {incr i} {
	    error_check_good reg_slice_exists \
		    [file exists $testdir/__db.slice00$i/$regdir/__db.001] 1
	}
    	error_check_good regenv_remove \
	    [berkdb envremove -home $testdir -region_dir $regdir] 0
    	error_check_good reg_no_exist [file exists $testdir/$regdir/__db.001] 0
	for {set i 0} {$i < $number_of_slices} {incr i} {
	    error_check_good reg_slice_no_exists \
		    [file exists $testdir/__db.slice00$i/$regdir/__db.001] 0
	}
	puts "\tEnv001 complete."
}
