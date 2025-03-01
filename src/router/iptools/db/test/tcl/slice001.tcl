# See the file LICENSE for redistribution information.
#
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$
#
# TEST	slice001
# TEST	Test of the various environment set_*_dir settings in DB_CONFIG
# TEST	for sliced environments.  The following configs are tested for both
# TEST	relative and absolute paths.
# TEST	1. home
# TEST	2. set_create_dir
# TEST	3. add_data_dir
# TEST	4. set_metadata_dir
# TEST	5. set_blob_dir
# TEST	6. set_tmp_dir
# TEST	7. set_region_dir
# TEST	8. set_lg_dir
proc slice001 { } {
	source ./include.tcl
	
    	puts "Slice001: Test DB_CONFIG directory setting with slices."
	env_cleanup $testdir

	# Make the local directory paths
	set num_slices 2
	set slicedir "slice"
	set zero 0
    	set one 1
	set local_dirs [list "log" "metadata" \
	    "data" "data" "blob" "tmp" "region"]
	set db_configs [list "set_lg_dir " "set_metadata_dir " \
	    "add_data_dir " "set_create_dir " "set_blob_dir " \
	    "set_tmp_dir " "set_region_dir "]
	foreach subdir $local_dirs {
		make_dirs $num_slices $testdir $slicedir $subdir
	}
	set curdir [pwd]
	cd $testdir
	set abs_home [pwd]
	cd $curdir
	
	puts "\tSlice001.a: Open the environment\
	    with \"home\" applied to all slices (error)."
	slice_db_config $num_slices {} [list "home $slicedir"]
	set err_msg "\"slice all home\" is not permitted"
	set ret [catch {berkdb_env_noerr -create -txn -home $testdir} msg]
	error_check_good dbenv_home_all [is_substr $msg $err_msg] 1

	puts "\tSlice001.b: Open the environment\
	    with absolute directories applied to all slices (error)."
	set slice_all {}
	foreach db_config $db_configs local_dir $local_dirs {
		lappend slice_all \
		    "$db_config $abs_home/$slicedir$zero/$local_dir"
	}
	slice_db_config $num_slices {} $slice_all
	set err_msg \
	    "\"slice all\" configurations may not include absolute paths"
	set ret [catch {berkdb_env_noerr -create -txn -home $testdir} msg]
	error_check_good dbenv_home_all [is_substr $msg $err_msg] 1

	puts "\tSlice001.c: Open the environment with local paths."
	set slice_all {}
	set container {}
    	set slice [list "0 home $slicedir$zero" "1 home $slicedir$one"]
	foreach db_config $db_configs local_dir $local_dirs {
		lappend slice_all "$db_config $local_dir"
		lappend container "$db_config $local_dir"
	}
	lappend slice_all "set_blob_threshold 1 0"
	lappend container "set_blob_threshold 1 0"
	slice_db_config $num_slices $container $slice_all $slice

	test_dirs $testdir $slicedir $num_slices
	
	puts "\tSlice001.d: Open the environment with absolute paths."
	foreach subdir $local_dirs {
		make_dirs $num_slices $testdir $slicedir $subdir
	}
	set slice_all {}
	set container {}
    	set slice [list "0 home $abs_home/$slicedir$zero" \
	    "1 home $abs_home/$slicedir$one"]
	foreach db_config $db_configs local_dir $local_dirs {
	    lappend container "$db_config $abs_home/$local_dir"
	    for {set i 0} {$i < $num_slices} {incr i} {
		    lappend slice \
			"$i $db_config $abs_home/$slicedir$i/$local_dir"
	    }
	}
	lappend slice_all "set_blob_threshold 1 0"
	lappend container "set_blob_threshold 1 0"
	slice_db_config $num_slices $container $slice_all $slice
	test_dirs $testdir $slicedir $num_slices
}

proc make_dirs { num_slices testdir slicedir subdir} {
	file mkdir $testdir/$subdir
	for {set i 0} {$i < $num_slices} {incr i} {
	    file mkdir $testdir/$slicedir$i
	    file mkdir $testdir/$slicedir$i/$subdir
	}
}

proc test_dirs { testdir slicedir num_slices } {
	set zero 0
	set one 1
	set dbname "slice.db"
	set dbenv [eval {berkdb_env_noerr -create -txn -home $testdir}]
	error_check_good sliced_env_open [is_valid_env $dbenv] TRUE

	puts "\t\tSlice001: Create a database and insert data."
	set t [$dbenv txn]  
	set db [eval {berkdb_open_noerr -create \
	    -sliced -txn $t -env $dbenv -btree $dbname}]
	error_check_good db_open [is_valid_db $db] TRUE
    	error_check_good open_txn_commit [$t commit] 0
	set key0 0
	set data0 1
	set key1 1
	set data1 2
	error_check_good put_key0 [$db put $key0 $data0] 0
	error_check_good put_key1 [$db put $key1 $data1] 0

	puts "\t\tSlice001: Test the database is in the correct dirs."
	error_check_good container_db [file exists "$testdir/data/$dbname"] 1
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good slice_db \
		    [file exists "$testdir/$slicedir$i/data/$dbname"] 1
	}
	
	puts "\t\tSlice001: Test the blob files are in the correct dirs."
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good "slice_blob_meta.db$i" [file exists \
		    "$testdir/$slicedir$i/blob/__db1/__db_blob_meta.db"] 1
	}
	
	puts "\t\tSlice001: Test the log files are in the correct dirs."
	error_check_good container_log \
	    [file exists "$testdir/log/log.0000000001"] 1
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good slice_log [file exists \
		    "$testdir/$slicedir$i/log/log.0000000001"] 1
	}

	puts "\t\tSlice001: Test the region files are in the correct dirs."
	error_check_good container_reg \
	    [file exists "$testdir/region/__db.001"] 1
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good slice_reg [file exists \
		    "$testdir/$slicedir$i/region/__db.001"] 1
	}

	error_check_good db_close [$db close] 0

	puts "\t\tSlice001: Test whether the fileid metadata is verified."
	# Exchange the database fragment in slice 0 with the one in slice 1 in
	# order to force the bad metadata DB_SLICE_CORRUPT error.
	file rename $testdir/$slicedir$zero/data/$dbname \
		    $testdir/$slicedir$zero
	file rename $testdir/$slicedir$one/data/$dbname \
		    $testdir/$slicedir$zero/data
	file rename $testdir/$slicedir$zero/$dbname \
		    $testdir/$slicedir$one/data/$dbname
	catch {set db [eval berkdb_open_noerr -env $dbenv $dbname] } res
	error_check_match db_open_fileid_invalid $res "*BDB0788*"

	puts "\t\tSlice001: Remove the database even though it is corrupt."
	error_check_good dbenv_remove [$dbenv dbremove $dbname] 0

	puts "\t\tSlice001: Test the database has been deleted."
	error_check_good container_db_gone \
	    [file exists "$testdir/create/$dbname"] 0
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good slice_db_gone \
		    [file exists "$testdir/$slicedir$i/create/$dbname"] 0
	}
	
	puts "\t\tSlice001: Test the blob files are deleted."
	for {set i 0} {$i < $num_slices} {incr i} {
		error_check_good slice_blob_gone [file exists \
		    "$testdir/$slicedir$i/blob/__db1/__db_blob_meta.db"] 0
	}

	error_check_good dbenv_close [$dbenv close] 0
	env_cleanup $testdir
}
