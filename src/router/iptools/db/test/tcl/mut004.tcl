# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	mut004
# TEST	Try setting various mutex number configurations. Make sure the
# TEST	mutex region has a reasonable size.

proc mut004 { } {
	source ./include.tcl
	env_cleanup $testdir

	puts "Mut004: Configure mutex numbers."

	# Create a transactional environment with a small cache. Get the maximum
	# size of the mutex region.
	puts "\tMut004.a: Create an environment with a small cache."
	set env [berkdb_env -create -mode 0644 -home $testdir -txn \
	    -cachesize {0 1000000 0}]

	set max_size [mutex_stat $env "Mutex region max"]
	$env close

	env_cleanup $testdir

	# Recreate the environment with a bigger cache. The maximum size of the
	# mutex region should grow.
	puts "\tMut004.b: Create an environment with a big cache."
	set env [berkdb_env -create -mode 0644 -home $testdir -txn \
	    -cachesize {0 5000000 0}]

	set init_count [mutex_stat $env "Mutex count"]
	set new_max_size [mutex_stat $env "Mutex region max"]
	error_check_good "The mutex region maximum size doesn't grow" \
	    [expr $new_max_size > $max_size] 1
	$env close

	env_cleanup $testdir

	# Recreate the environment with a small initial mutex number. The initial
	# size of the mutex region should decrease.
	puts "\tMut004.c: Create an environment with a small initial mutex."
	set env [berkdb_env -create -mode 0644 -home $testdir -txn \
	    -cachesize {0 5000000 0} -mutex_set_init 1]

	error_check_good "The mutex count doesn't drop" \
	    [expr [mutex_stat $env "Mutex count"] < $init_count] 1
	error_check_good "The mutex region maximum size should not change" \
	    [expr [mutex_stat $env "Mutex region max"] == $new_max_size] 1
	$env close

	env_cleanup $testdir
}

proc mutex_stat { env key } {
	set stat [$env mutex_stat]
	foreach pair $stat {
		if { [lindex $pair 0] == $key } {
			return [lindex $pair 1]
		}
	}
	return 0
}

