# See the file LICENSE for redistribution information.
#
# Copyright (c) 2010, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id$

source ./include.tcl
source $test_path/test.tcl
source $test_path/testutils.tcl
source $test_path/reputils.tcl

set hoststr [lindex $argv 0]
set dirS [lindex $argv 1]
set portS [lindex $argv 2]
set rvS [lindex $argv 3]
set errpfxS [lindex $argv 4]

proc in_sync_state { d } {
	global util_path
	set stat [exec $util_path/db_stat -N -r -R A -h $d]
	puts "stat is $stat"
	set in_page [is_substr $stat "SYNC_PAGE"]
	puts "value is $in_page"
	return $in_page
}

puts "Start site $errpfxS"
set envS [berkdb env -create -errpfx $errpfxS -home $dirS -txn -rep -thread \
	      -recover -verbose [list rep $rvS]]
$envS repmgr -local [list $hoststr $portS] -start elect

puts "Wait until it gets into SYNC_PAGES state"
while {![in_sync_state $dirS]} {
	tclsleep 1
}

# Make sure there is time for the entire internal init file to be written
# out to disk.  If the final group membership database section is not yet
# there, it causes DB_REP_UNAVAIL failures when this site is restarted in
# the main process.
tclsleep 1
