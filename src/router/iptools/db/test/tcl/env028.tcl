# See the file LICENSE for redistribution information.
#
# Copyright (c) 1999, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env028
# TEST	Test of SET_REGION_DIR.
# TEST	With an environment path specified using -home, and then again
# TEST	with it specified by the environment variable DB_HOME:
# TEST	1) Make sure that the set_region_dir option is respected
# TEST		a) as a relative pathname.
# TEST		b) as an absolute pathname.
# TEST	2) Make sure that the SET_REGION_DIR db_config argument is respected,
# TEST		again as relative and absolute pathnames.
# TEST	3) Make sure that if -both- db_config and a file are present,
# TEST		only the file is respected (see doc/env/naming.html).
# TEST	4) Check that setting the region dir is incompatible with
# TEST  	DB_PRIVATE and DB_SYSTEM_MEM
# TEST	5) Check that MVCC freezer files go into the region dir.        
proc env028 { } {
	#   env028 is essentially just a small driver that runs
	# env028_body twice;  once, it
	# supplies a "home" argument to use with environment opens,
	# and the second time it sets DB_HOME instead.
	#   Note that env028_body itself calls env028_run_test to run
	# the body of the actual test and check for the presence
	# of logs.  The nesting, I hope, makes this test's structure simpler.

	global env
	source ./include.tcl

	puts "Env028: set_region_dir test."

	puts "\tEnv028: Running with -home argument to berkdb_env."
	env028_body "-home $testdir"

	puts "\tEnv028: Running with environment variable DB_HOME set."
	set env(DB_HOME) $testdir
	env028_body "-use_environ"

	unset env(DB_HOME)

	puts "\tEnv028: Running with both DB_HOME and -home set."
	# Should respect -only- -home, so we give it a bogus
	# environment variable setting.
	set env(DB_HOME) $testdir/bogus_home
	env028_body "-use_environ -home $testdir"
	unset env(DB_HOME)

	puts "\tEnv028: Cannot set Region dir with DB_PRIVATE."
	env_cleanup $testdir
	set regdir "REGIONDIR"
	file mkdir $testdir/$regdir
	set ret [catch {berkdb_env_noerr -create -home $testdir \
	    -region_dir $regdir -private} msg] 
	set expected_err "region directory cannot be set"
	error_check_good system_mem_err [is_substr $msg $expected_err] 1

	puts "\tEnv028: Cannot set Region dir with DB_SYSTEM_MEM."
	env_cleanup $testdir
	file mkdir $testdir/$regdir
	set ret [catch {berkdb_env_noerr -create -home $testdir \
	    -region_dir $regdir -system_mem} msg]
    	error_check_good system_mem_err [is_substr $msg $expected_err] 1

	puts "\tEnv028: MVCC freezer files go in the Region dir."
	env_cleanup $testdir
	file mkdir $testdir/$regdir

	# Create an environment that supports MVCC
	set dbenv [eval {berkdb_env -home $testdir -create -mode 0644\
	    -cachesize {0 70000 1} -multiversion -txn \
	    -region_dir $regdir} ]
	error_check_good dbenv_open [is_valid_env $dbenv] TRUE

	# Create a DB with txn support.
	set db [eval {berkdb_open -env $dbenv -auto_commit -create \
	    -btree -mode 0644 "freeze.db"} ]

	# Write data until some frozen buckets appear.
	puts "\tEnv028: Writing data until some freezer files appear."
	set k 0
	set data 0
	set t [$dbenv txn -snapshot]
	set found 0

	while { $k < 10000 } {
		set ret [catch {$db put -txn $t $k $data }]
	   	# Check for frozen buckets.
		set file_list [glob -nocomplain \
	    	    -directory $testdir/$regdir __db.freezer*K]
		if { [llength $file_list] > 0 } {
			puts "\tEnv028: Freezer file found in\
	    		     Region dir."
			$t abort
			set found 1
			break
		}
		incr k
	}
	error_check_good found_freezer $found 1
	error_check_good db_close [$db close] 0
	error_check_good env_close [$dbenv close] 0
}

proc env028_body { home_arg } {
	source ./include.tcl

	env_cleanup $testdir
	set regdir "REGIONDIR"

	file mkdir $testdir/$regdir

	# Set up full path to $regdir for when we test absolute paths.
	set curdir [pwd]
	cd $testdir/$regdir
	set fullregdir [pwd]
	cd $curdir

	env028_make_config $regdir

	# Run the meat of the test.
	env028_run_test a 1 "relative path, config file" $home_arg \
		$testdir/$regdir

	env_cleanup $testdir

	file mkdir $fullregdir
	env028_make_config $fullregdir

	# Run the test again
	env028_run_test a 2 "absolute path, config file" $home_arg \
		$fullregdir

	env_cleanup $testdir

	# Now we try without a config file, but instead with db_config
	# relative paths
	file mkdir $testdir/$regdir
	env028_run_test b 1 "relative path, db_config" "$home_arg \
		-region_dir $regdir -data_dir ." \
		$testdir/$regdir

	env_cleanup $testdir

	# absolute
	file mkdir $fullregdir
	env028_run_test b 2 "absolute path, db_config" "$home_arg \
		-region_dir $fullregdir -data_dir ." \
		$fullregdir

	env_cleanup $testdir

	# Now, set db_config -and- have a # DB_CONFIG file, and make
	# sure only the latter is honored.

	file mkdir $testdir/$regdir
	env028_make_config $regdir

	# note that we supply a -nonexistent- region dir to the
	# commandline.
	env028_run_test c 1 "relative path, both db_config and file" \
		"$home_arg -region_dir $testdir/bogus \
		-data_dir ." $testdir/$regdir
	env_cleanup $testdir

	file mkdir $fullregdir
	env028_make_config $fullregdir

	# note that we supply a -nonexistent- region dir to db_config
	env028_run_test c 2 "relative path, both db_config and file" \
		"$home_arg -region_dir $fullregdir/bogus \
		-data_dir ." $fullregdir
}

proc env028_run_test { major minor msg env_args reg_path} {
	global testdir
	set testfile "env028.db"

	puts "\t\tEnv028.$major.$minor: $msg"

	# Create an environment then close it
	set dbenv [eval {berkdb_env -create } $env_args]
	error_check_good env_open [is_valid_env $dbenv] TRUE
	error_check_good env_close [$dbenv close] 0

	# Now make sure the region file is where we want it to be.
	error_check_good region_exists \
		[file exists $reg_path/__db.001] 1
}

proc env028_make_config { regdir } {
	global testdir

	set cid [open $testdir/DB_CONFIG w]
	puts $cid "set_data_dir ."
	puts $cid "set_region_dir $regdir"
	close $cid
}
