# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST	env025
# TEST	Test db_recover with all allowed option combinations.
proc env025 { } {
	source ./include.tcl
	global has_crypto

	set encrypt 0
	if { $has_crypto == 1 } {
		lappend encrypt 1
	}

	# Test with -P -c -e -f -t and -v.
	foreach e $encrypt {
		foreach catastrophic { 1 0 } {
			foreach retain_env { 1 0 } {
				foreach show_percent { 1 0 } {
					foreach use_timestamp { 1 0 } {
						foreach verbose { 1 0 } {
							env025_subtest \
							    $e \
							    $catastrophic \
							    $retain_env \
							    $show_percent \
							    $use_timestamp \
							    $verbose
						}
					}
				}
			}
		}
	}

	set binname db_recover
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname ".exe"
	}

	# Print version.
 	puts "\tEnv025: $binname -V $std_redirect"
	set ret [catch {eval exec $util_path/$binname -V $std_redirect} r]
	error_check_good db_recover($r) $ret 0
}

proc env025_subtest { encrypt catastrophic retain_env show_percent \
    use_timestamp verbose } {
	source ./include.tcl

	puts "Env025: Test with options: (encrypt:$encrypt\
	    catastrophic:$catastrophic\
	    retain_env:$retain_env\
	    show_percent:$show_percent\
	    use_timestamp:$use_timestamp\
	    verbose:$verbose)"

	set passwd "passwd"
	set envargs ""
	set recover_args ""
	if { $catastrophic } {
		append recover_args " -c"
	}
	if { $retain_env } {
		append recover_args " -e"
	}
	if { $show_percent } {
		append recover_args " -f"
	}
	if { $verbose } {
		append recover_args " -v"
	}

	append recover_args " -h $testdir"
	if { $encrypt } {
		append recover_args " -P $passwd"
		append envargs " -encryptaes $passwd"
	}

	env_cleanup $testdir

	set env [eval berkdb_env $envargs -create -txn -home $testdir]
	error_check_good env [is_valid_env $env] TRUE

	set method "-btree"
	set db [eval {berkdb_open -env $env -create $method -mode 0644 \
	    -auto_commit "env025.db"}]
	error_check_good dbopen [is_valid_db $db] TRUE

	set txn [$env txn]
	error_check_good db_fill [populate $db $method $txn 10 0 0] 0
	error_check_good txn_commit [$txn commit] 0
	error_check_good checkpoint [$env txn_checkpoint] 0

	if { $use_timestamp } {
		tclsleep 1
		set timestamp [clock format [clock seconds] \
		    -format %Y%m%d%H%M.%S]
  		append recover_args " -t $timestamp"
	}

	error_check_good db_close [$db close] 0
	error_check_good env_close [$env close] 0

	set binname db_recover
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname ".exe"
	}
  	puts "\tEnv025: $binname $recover_args"
	set ret [catch {exec $util_path/$binname $recover_args} r]
	error_check_bad db_recover($r) $ret 0

	if { $retain_env } {
		# The environment should be retained.
		set env [eval berkdb_env_noerr -home $testdir $envargs]
		error_check_good env_close [$env close] 0
	}
}
