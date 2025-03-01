# See the file LICENSE for redistribution information.
#
# Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	env026
# TEST	Test reopening an environment after a panic.
# TEST
# TEST	Repeatedly panic the environment, close & reopen it in order to
# TEST	verify that a process is able to reopen the env and there are no
# TEST	major shmem/mmap "leaks"; malloc leaks will occur, and that's ok.
# TEST
# TEST  Since this test leaks memory, it is meant to be run standalone
# TEST  and should not be added to the automated Tcl test suite.

proc env026 { } {
	source ./include.tcl
	set tnum 026
	# Shmkey could be any value here.
	set shmkey 20

	puts "Env$tnum: Test reopening an environment after a panic."

	# Check that a process can reopen an environment after it panics, with
	# both mmap'd regions and -system_mem shared memory segments.
	set reopenlimit 10
	env026_reopen $reopenlimit $shmkey
	
	# Detect file descriptor limit. Set reopen times to fdlimit + 1.
	if { $is_windows_test == 1 } {
		# In fact, there is no fixed handle limit in Windows. 
		# Windows always allocates a handle in the handle table of the 
		# application's process and returns the handle value.
		# The hard-coded limitation for a user handle is set to
		# 10,000 by default. It is defined in:
		# HKEY_LOCAL_MACHINE\Software\Microsoft\WindowsNT\
		# CurrentVersion\Windows\USERProcessHandleQuota.
		puts "\tEnv$tnum: Use default fd limit:10000"
		set reopenlimit 10000
	} else {
		set fdlimit ""
		# use 'ulimit -n' to get fd limit on linux, freebsd and solaris.
		error_check_good getFDlimit [catch {eval exec \
		    "echo \"ulimit -n\" | bash" } fdlimit] 0
		puts "\tEnv$tnum: fd limit:$fdlimit"
		set reopenlimit $fdlimit
	}
	incr reopenlimit
	env026_reopen $reopenlimit $shmkey

	# Detect SHMALL and SHMMAX, then run subtest with cachesize at
	# (SHMALL * kernel pagesize) or SHMMAX.
	set shmall 0
	set shmmax 0
	set kernel_pgsize 0
	set cache_size 0
	if { $is_linux_test == 1 } {
		error_check_good getSHMALL [catch {eval exec \
		    "cat /proc/sys/kernel/shmall"} shmall] \
		    0
		error_check_good getSHMMAX [catch {eval exec \
		    "cat /proc/sys/kernel/shmmax"} shmmax] \
		    0
		error_check_good getPGSIZE [catch {eval exec \
		    "getconf PAGE_SIZE"} kernel_pgsize]\
		    0
	}
	if { $is_osx_test == 1 } {
		error_check_good getSHMALL [catch {eval exec \
		    "sysctl -n kern.sysv.shmall"} \
		    shmall] 0
		error_check_good getSHMMAX [catch {eval exec \
		    "sysctl -n kern.sysv.shmmax"} \
		    shmmax] 0
		error_check_good getPGSIZE [catch {eval exec \
		    "getconf PAGE_SIZE"} kernel_pgsize]\
		    0
	}
	if { $is_freebsd_test == 1 } {
		error_check_good getSHMALL [catch {eval exec \
		    "sysctl -n kern.ipc.shmall"} \
		    shmall] 0
		error_check_good getSHMMAX [catch {eval exec \
		    "sysctl -n kern.ipc.shmmax"} \
		    shmmax] 0
		error_check_good getPGSIZE [catch {eval exec \
		    "getconf PAGE_SIZE"} kernel_pgsize]\
		    0
	}
	if { $is_sunos_test == 1 } {
		# Cannot get shmall from solaris. Just query shmmax here.
		error_check_good getSHMMAX [catch {eval exec \
	            "prctl -n project.max-shm-memory -i \
		    project default | grep privileged | \
	            awk \"{print \\\$2}\""} \
		    shmmax] 0 
		# Shmmax on solaris is in format of "x.xxGB".
		error_check_good checkSHMMAX [is_substr $shmmax "GB"] 1
		# Convert shmmax, from GB unit to bytes.
		set endpos [expr [string length $shmmax] - \
		    [string length "GB"] - 1]
		set shmmax [string range $shmmax 0 $endpos]
		# Round up the shmmax.
		set shmmax [expr int($shmmax) + 1]
		# Use bc, in case of shmmax is out of Tcl integer range.
		error_check_good computeSHMMAX [catch {eval exec \
		    "echo \"$shmmax * 1024 * 1024 * 1024\" | bc"} shmmax] 0
		error_check_good getPGSIZE [catch {eval exec \
		    "getconf PAGE_SIZE"} kernel_pgsize]\
		    0
	}
	puts "\tEnv$tnum: shmall:$shmall, shmmax:$shmmax,\
	    kernel pgsize:$kernel_pgsize"
	# Choose the bigger one for cache_size.
	set cache_size [expr $shmall * $kernel_pgsize]
	if {$cache_size < $shmmax} {
		set cache_size $shmmax
	}
	# Enlarge cache_size to exceed maximum allowed cache size.
	if { $is_sunos_test == 1 } {
		# In Solaris, there is no specific shmmax so just enlarge
		# cache size to hit its swap space.
		error_check_good enlargeCachesize [catch {eval exec \
		    "echo \"$cache_size * 30\" | bc"} cache_size] 0
	} else {
		error_check_good enlargeCachesize [catch {eval exec \
		    "echo \"$cache_size * 5 / 4\" | bc"} cache_size] 0
	}
	puts "\tEnv$tnum: cache size is set to be $cache_size."
	if { ![catch {env026_reopen 1 $shmkey $cache_size}] } {
		puts "FAIL: large cache size does not lead to a failure."
	} else {
		puts "\tEnv$tnum: Get failure as expected."
	}
}

# Env026_reopen tests that a process can reopen environment after a panic,
# without needed to start a new process. Usually it runs for a few iterations,
# but a "leak" test would run for hundreds or thousands of iterations, in order
# to reach file descriptor and shared memory limits. Some places to find them are:
# Oracle Enterprise Linux: limit or ulimit; /proc/sys/kernel/shmmni 
# Solaris: prctl -n process.max-file-descriptor | project.max-shm-ids $$
proc env026_reopen { { reopenlimit 10 } { shmkey 0 } {cache_size 0}} {
	source ./include.tcl

	set tnum 026
	set testfile TESTFILE
	set key KEY_REOPEN
	set data DATA_REOPEN

	env_cleanup $testdir
	set envopen [list -create -home $testdir -txn -register -recover ]
	lappend envopen -errfile "$testdir/errfile"
	if { $cache_size != 0} {
		set GB [expr 1024 * 1024 * 1024]
		set gbytes [expr int($cache_size / $GB)]
		set bytes [expr $cache_size % $GB]
		# Cache number could be any integer, but each cache 
		# should be less than 4GB.
		set cachenum [expr $gbytes + 1]
		lappend envopen -cachesize "$gbytes $bytes $cachenum"
		puts "\tEnv$tnum: cache parameter:$gbytes $bytes $cachenum"
	}
	set shmmesg ""
	if { $shmkey != 0 } {
		lappend envopen -system_mem -shm_key $shmkey
		set shmmesg " with a shared memory key of $shmkey"
	}
	puts "\tEnv$tnum: Reopen panic'ed env $reopenlimit times$shmmesg."
	env_cleanup $testdir
	for {set reopen 0} {$reopen < $reopenlimit} {incr reopen} {
		set env [ berkdb_env {*}$envopen -errpfx "ENV026 #$reopen" ]
		# Verify that the open of the environment ran recovery by
		# checking that no txns have been created.
		error_check_good "Env$tnum #$reopen: detect-recovery" \
		    [getstats [$env txn_stat] {Number txns begun}] 0
		set txn [$env txn]
		error_check_good \
		    "Env$tnum: #$reopen txn" [is_valid_txn $txn $env] TRUE

		# The db open needs to be the "_noerr" version; the plain
		# version overrides the -errfile specification on the env.
		set db [eval {berkdb_open_noerr -env $env -create -mode 0644} \
		    -auto_commit {-btree $testfile}  ]
		error_check_good \
		    "Env$tnum: #$reopen db open" [is_valid_db $db] TRUE
		set ret [eval {$db put}  $key $data]
		error_check_good "Env$tnum: #$reopen put($key,$data)" $ret 0
		set dbc [eval {$db cursor} -txn $txn]
		error_check_good "Env$tnum: #$reopen db cursor" \
		    [is_valid_cursor $dbc $db] TRUE
		set ret [ catch {$env set_flags -panic on} res ]
		# This intentionally does not close the cursor, db, or txn.
		# We want to test that a slight faulty app doesn't crash.
		if {[catch {eval [$env close]} ret] == 0} {
			puts "Env$tnum: #$reopen close didn't panic: $ret"
		}

		if {$reopen > 0 && $reopen % 20 == 0} {
			puts "\t\tEnv$tnum: reopen times:$reopen "
		}
	}
	set env [ berkdb_env_noerr {*}$envopen ]
	error_check_good "Env$tnum final recovery check" \
	    [getstats [$env txn_stat] {Number txns begun}] 0
	puts "\tEnv$tnum: #$reopen Each reopen after a panic succeeded."
}
