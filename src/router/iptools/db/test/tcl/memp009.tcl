# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#

# TEST	memp009
# TEST	Dead files and their buffers are removed when the mutex usage is high.
proc memp009 { } {
	source ./include.tcl

	memp009_body "file-based"

	memp009_body "in-mem named"

	memp009_body "in-mem unnamed"
}

proc memp009_body { flags } {
	source ./include.tcl

	# Setting a small database page size helps drive up the mutex usage
	set db_pagesize 1024
	set mutex_max 400
	set open_dbs {}

	puts "Memp009: $flags dead file tests."

	env_cleanup $testdir

	puts "\tMemp009.a: Create the environment"

	set dbenv [eval {berkdb_env -create -lock -mode 0644 -home $testdir \
	    -mutex_set_max $mutex_max -lock_partitions 1}]
	error_check_good env_open [is_valid_env $dbenv] TRUE

	puts "\tMemp009.b: Create databases"

	set i 0
	set mutex_inuse [mutex_count $dbenv "Mutexes in use"]
	# Simulating the heuristics used in __memp_mf_mark_dead().
	# Each open database handle uses one logical mutex. This is freed when
	# the database handle is freed. In the "in-mem unnamed" case, this causes
	# the $mutex_inuse to drop while we are closing databases. We compensate
	# for that here to make sure the condition always holds in the .d phase.
	while {$mutex_inuse - $i <= $mutex_max - 200 ||
	    [mpoolfile_count] + $i <= $mutex_inuse / 20 } {
		incr i

		set db [ berkdb_open -env $dbenv -create -mode 0644 \
		    -pagesize $db_pagesize -btree [get_dbname $flags $i] ]
		error_check_good db:open:$i [is_valid_db $db] TRUE

		if {$flags == "in-mem unnamed"} {
			lappend open_dbs $db
		} else {
			error_check_good db:close:$i [$db close] 0
		}
		set mutex_inuse [mutex_count $dbenv "Mutexes in use"]
	}
	puts "\t\tMemp009: $i databases created."

	if { $flags != "in-mem unnamed" } {
		puts "\tMemp009.c: Reopen the environment with a new handle"

		error_check_good env_close [$dbenv close] 0
		set dbenv [eval {berkdb_env -lock -home $testdir \
		    -mutex_set_max $mutex_max}]
		error_check_good env_open2 [is_valid_env $dbenv] TRUE
	}

	puts "\tMemp009.d: Make databases dead"

	set old_mpf_count [mpoolfile_count]
	puts "\t\tMemp009: $old_mpf_count open mpoolfile handles."

	for { set j 1 } { $j <= $i } { incr j } {
		if {$flags == "in-mem unnamed"} {
			error_check_good db:dead:close:$j \
			    [ [lindex $open_dbs [expr $j - 1] ] close] 0
		} else {
			error_check_good db:dead:remove:$j [berkdb dbremove \
			    -env $dbenv [get_dbname $flags $j] ] 0
		}
	}
	set new_mpf_count [mpoolfile_count]
	puts -nonewline "\t\tMemp009: $new_mpf_count open mpoolfile handles"
	puts " after making some databases dead."

	error_check_good "The number of open mpoolfile handles does not drop" \
	    [expr $new_mpf_count < $old_mpf_count] 1

	error_check_good env_close2 [$dbenv close] 0
}

proc get_dbname { flags i } {
	if {$flags == "file-based"} {
		return [list "db$i.db"]
	} elseif {$flags == "in-mem named"} {
		return [list "" "db$i.sub"]
	} elseif {$flags == "in-mem unnamed"} {
		return
	}
}

proc mutex_count { env type } {
	set stat [$env mutex_stat]
	foreach pair $stat {
		if { [lindex $pair 0] == $type } {
			return [lindex $pair 1]
		}
	}
	return 0
}

proc mpoolfile_count {} {
	source ./include.tcl
	global util_path
	set pattern {\d+\tmpoolfile handle}
	regexp $pattern [exec $util_path/db_stat -h $testdir -x] stat
	if {[info exists stat] == 0} {
		return 0
	} else {
		return [lindex [split $stat "\t"] 0]
	}
}
