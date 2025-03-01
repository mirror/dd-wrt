# See the file LICENSE for redistribution information.
#
# Copyright (c) 2013, 2017 Oracle and/or its affiliates. All rights reserved.
#
# $Id$
#
# TEST  env024
# TEST  Test db_hotbackup with all allowed option combinations.
proc env024 { } {
	source ./include.tcl
	global EXE
	global has_crypto

	set encrypt 0 
	if { $has_crypto == 1 } {
		lappend encrypt 1
	}

	# Test with -P -c -v and -D.
	foreach e $encrypt {
		foreach chkpt { 1 0 } {
			foreach verbose { 1 0 } {
				foreach configfile { 1 0 } {
					env024_subtest $e $chkpt \
					    $verbose $configfile
				}
			}
		}
	}

	# Test with -V
	set binname db_hotbackup
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}
	puts "Env024: Print version info."
	env024_execmd "$binname -V"
}

proc env024_subtest { encrypt chkpt verbose configfile } {
	source ./include.tcl
	global passwd
	global EXE

	puts "Env024: Test with options: (encrypt:$encrypt chkpt:$chkpt\
	    verbose:$verbose DB_CONFIG:$configfile)"

	set envargs " -log -txn"
	set envhome "$testdir/envhome"
	set backupdir "$testdir/backup"
	set backup_args "-h $envhome -b $backupdir"
	
	set binname db_hotbackup
	set std_redirect "> /dev/null"
	if { $is_windows_test } {
		set std_redirect "> /nul"
		append binname $EXE
	}

	if { $encrypt } {
		append backup_args " -P $passwd"
		append envargs " -encryptaes $passwd"
	}
	if { $chkpt } {
		append backup_args " -c"
	}
	if { $verbose } {
		append backup_args " -v"
	}
	if { $configfile } {
		append backup_args " -D"
	}

	foreach logdir { 0 1 } {
		foreach datadir { 0 1 } {
			# '-d' and '-c' could be specified at the same time.
			if { [is_substr $backup_args "-c"] && $datadir } {
				puts "\tEnv024: skip '-d' while backup_args\
				    contains '-c'"
				continue
			}

			env_cleanup $testdir
			file mkdir $envhome
			set additional_envargs ""
			set additional_bkupargs ""
			set additional_msg "logdir:$logdir datadir:$datadir"
			set config_file_content ""
			if { $logdir } {
				# test with '-l', use individual log directory.
				set logdir_path "$envhome/logs"
				file mkdir $logdir_path
				append additional_envargs \
				    " -log_dir logs"
				append additional_bkupargs " -l logs"
				append config_file_content "set_lg_dir logs\n"
			}
			if { $datadir } {
				# Test with '-d', use individual data directory.
				set datadir_path "$envhome/data"
				file mkdir $datadir_path
				append additional_envargs \
				    " -data_dir data"
				append additional_bkupargs " -d data"
				append config_file_content "set_data_dir data\n"
			}
			if { $configfile } {
				# Reset args if use DB_CONFIG file.
				set additional_envargs ""
				set additional_bkupargs ""
				# Write DB_CONFIG to disk.
				set fileid [open "$envhome/DB_CONFIG" w]
				puts -nonewline $fileid $config_file_content
				close $fileid
			}

			puts "\tEnv024: test with directory options:\
			    $additional_msg"
			# Prepare a target env.
			set env [env024_prepare_env $envhome "$envargs \
			    $additional_envargs -create -home $envhome"]
			env024_execmd "$binname $backup_args \
			    $additional_bkupargs"
			puts "\t\tEnv024: update it again with '-u'."
			# Back it up again with '-u' to update current backup.
			env024_execmd "$binname -u $backup_args \
			    $additional_bkupargs"
			error_check_good env_close [$env close] 0
		}
	}
}

# Set up a env with some data in target directory with given args.
proc env024_prepare_env { envhome envargs } {
	source ./include.tcl

	set env [eval berkdb_env_noerr $envargs]
	set method "btree"
	set db [eval {berkdb_open -env $env -create "-$method" -mode 0644 "db.db"}]
	error_check_good db_fill [populate $db $method "" 10 0 0] 0

	error_check_good db_close [$db close] 0
	return $env
}

proc env024_execmd { execmd } {
	source ./include.tcl

	set result ""
	if { ![catch {eval exec $util_path/$execmd} result] } {
		return
	}
	puts "FAIL: got $result while executing '$execmd'"
}
