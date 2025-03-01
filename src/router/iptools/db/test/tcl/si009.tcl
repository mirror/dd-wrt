# See the file LICENSE for redistribution information.
#
# Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
#
# $Id: si009.tcl,v fe390f089fce 2014/01/10 21:03:56 carol $
#
# TEST	si009
# TEST	Secondary index put/delete with lorder test
# TEST
# TEST  This test is the same as si001 except we use 
# TEST  partial put to update entries.
proc si009 { methods {nentries 10} {tnum "009"} args } {
	source ./include.tcl
	global dict nsecondaries
	global default_pagesize
	global fixed_len

	# Primary method/args.
	set pmethod [lindex $methods 0]
	set pargs [convert_args $pmethod $args]
	if [big_endian] {
		set nativeargs " -lorder 4321"
		set swappedargs " -lorder 1234"
	} else {
		set swappedargs " -lorder 4321"
		set nativeargs " -lorder 1234"
	}
	set argtypes "{$nativeargs} {$swappedargs}"
	set pomethod [convert_method $pmethod]

	# Renumbering recno databases can't be used as primaries.
	if { [is_rrecno $pmethod] == 1 } {
		puts "Skipping si$tnum for method $pmethod"
		return
	}

	# Method/args for all the secondaries.  If only one method
	# was specified, assume the same method (for btree or hash)
	# and a standard number of secondaries.  If primary is not
	# btree or hash, force secondaries to be one btree, one hash.
	set methods [lrange $methods 1 end]
	if { [llength $methods] == 0 } {
		for { set i 0 } { $i < $nsecondaries } { incr i } {
			if { [is_btree $pmethod] || [is_hash $pmethod] } {
				lappend methods $pmethod
			} else {
				if { [expr $i % 2] == 0 } {
					lappend methods "-btree"
				} else {
					lappend methods "-hash"
				}
			}
		}
	}

	set argses [convert_argses $methods $args]
	set omethods [convert_methods $methods]

	set mutexargs " -mutex_set_max 10000 "
	if { $default_pagesize <= 2048 } {
		set mutexargs "-mutex_set_max 40000 "
	}
	# If we are given an env, use it.  Otherwise, open one.
	set eindex [lsearch -exact $args "-env"]
	if { $eindex == -1 } {
		env_cleanup $testdir
		set cacheargs " -cachesize {0 4194304 1} "
		set env [eval {berkdb_env} -create \
		    $cacheargs $mutexargs -home $testdir]
		error_check_good env_open [is_valid_env $env] TRUE
	} else {
		incr eindex
		set env [lindex $args $eindex]
		set envflags [$env get_open_flags]
		if { [lsearch -exact $envflags "-thread"] != -1 &&\
			[is_queue $pmethod] == 1 } {
			puts "Skipping si$tnum for threaded env"
			return
		}
		set testdir [get_home $env]
	}

	set pname "primary$tnum.db"
	set snamebase "secondary$tnum"

	foreach pbyteargs $argtypes {
		foreach sbyteargs $argtypes {
			if { $pbyteargs == $nativeargs } {
				puts "Si$tnum: Using native\
				    byteorder $nativeargs for primary."
			} else {
				puts "Si$tnum: Using swapped\
				    byteorder $swappedargs for primary."
			}
			if { $sbyteargs == $nativeargs } {
				puts "Si$tnum: Using native\
				    byteorder $nativeargs for secondaries."
			} else {
				puts "Si$tnum: Using swapped\
				    byteorder $swappedargs for secondaries."
			}

			puts "si$tnum\
			    \{\[ list $pmethod $methods \]\} $nentries"
			cleanup $testdir $env

			# Open primary.
			set pdb [eval {berkdb_open -create -env} $env \
			    $pomethod $pargs $pbyteargs $pname]
			error_check_good primary_open [is_valid_db $pdb] TRUE

			# Open and associate the secondaries
			set sdbs {}
			for { set i 0 } \
			    { $i < [llength $omethods] } { incr i } {
				set sdb [eval {berkdb_open -create -env} $env \
				    [lindex $omethods $i] [lindex $argses $i] \
				    $sbyteargs $snamebase.$i.db]
				error_check_good\
				    second_open($i) [is_valid_db $sdb] TRUE

				error_check_good db_associate($i) \
				    [$pdb associate [callback_n $i] $sdb] 0
				lappend sdbs $sdb
			}

			puts "\tSi$tnum.a: Put loop"
			set did [open $dict]
			for { set n 0 } \
			    { [gets $did str] != -1 && $n < $nentries } \
			    { incr n } {
				if { [is_record_based $pmethod] == 1 } {
					set key [expr $n + 1]
					set datum $str
				} else {
					set key $str
					gets $did datum
				}
				set keys($n) $key
				set data($n) [pad_data $pmethod $datum]

				set ret [eval {$pdb put}\
				    {$key [chop_data $pmethod $datum]}]
				error_check_good put($n) $ret 0
			}
			close $did

			check_secondaries\
			    $pdb $sdbs $nentries keys data "Si$tnum.a"

			puts "\tSi$tnum.b: Partial put loop" 
			for { set n 0 } { $n < $nentries } { incr n } {
				set orig($n) $data($n)
			}

			if { [is_fixed_length $pmethod] == 0 } {
				puts "\tSi$tnum.b1:\
				    Partial put: extend from beginning"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n).$keys($n)
					set dlen [string length $data($n)]
					set ret [$pdb put -partial \
					    [list 0 $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
					set data($n) [pad_data $pmethod $newd]
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b2:\
				    Partial put: shrink from beginning"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n)
					set dlen [string length $data($n)]
					set ret [$pdb put -partial \
					    [list 0 $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
					set data($n) [pad_data $pmethod $newd]
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b3:\
				    Partial put: extend from middle"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n).$keys($n)
					set dlen [string length $data($n)]
					set doff [expr $dlen / 2]
					set ret [$pdb put -partial \
					    [list $doff $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
					set data($n) [pad_data $pmethod \
					    [string range $data($n) 0 \
					    [expr $doff - 1]]$newd]
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b4:\
				    Partial put: shrink from middle"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n)
					set dlen [string length $data($n)]
					set doff [expr $dlen / 2]
					set ret [$pdb put -partial \
					    [list $doff $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
					set data($n) [pad_data $pmethod \
					    [string range $data($n) 0 \
					    [expr $doff - 1]]$newd]
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b5:\
				    Partial put: extend from end"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n)
					set dlen [string length $data($n)]
					set doff $dlen
					set ret [$pdb put -partial \
					    [list $doff $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
					set data($n) [pad_data $pmethod \
					    $data($n)$newd]
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b6: Partial put: corner cases"

				puts "\t\tSi$tnum.b6:\
				    Put nothing from beginning"
				set ret [$pdb put -partial {0 0} $keys(0) ""]
				error_check_good partial_put(0) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6:\
				    Put nothing from middle"
				set doff [expr [string length $data(0)] / 2]
				set ret [$pdb put -partial [list $doff 0] \
				    $keys(0) ""]
				error_check_good partial_put(1) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6:\
				    Put nothing from end"
				set doff [expr [string length $data(0)] - 1]
				set ret [$pdb put -partial [list $doff 0] \
				    $keys(0) ""]
				error_check_good partial_put(2) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6:\
				    Remove one character from beginning"
				set ret [$pdb put -partial {0 1} $keys(0) ""]
				error_check_good partial_put(3) $ret 0
				set data(0) [string range $data(0) 1 end]
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6:\
				    Remove one character from middle"
				set doff [expr [string length $data(1)] / 2]
				set ret [$pdb put -partial [list $doff 1] \
				    $keys(1) ""]
				error_check_good partial_put(4) $ret 0
				set data(1) [string replace $data(1) \
				    $doff $doff]
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6:\
				    Remove one character from end"
				set doff [expr [string length $data(2)] - 1]
				set ret [$pdb put -partial [list $doff 1] \
				    $keys(2) ""]
				error_check_good partial_put(5) $ret 0
				set data(2) [string range $data(2) 0 end-1]
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b6: Empty data"
				set dlen [string length $data(3)]
				set ret [$pdb put -partial [list 0 $dlen] \
				    $keys(3) ""]
				error_check_good partial_put(6) $ret 0
				set data(3) {}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"
			} else {
				puts "\tSi$tnum.b1:\
				    Partial put from beginning"
				for { set n 0 } { $n < $nentries } { incr n } {
					set newd $orig($n)
					set dlen [string length $data($n)]
					set ret [$pdb put -partial \
					    [list 0 $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b2:\
				    Partial put from middle"
				for { set n 0 } { $n < $nentries } { incr n } {
					set tmp_len [string length $data($n)]
					set doff [expr $dlen / 2]
					set dlen [expr $fixed_len - $doff]
					set newd [string range $data($n) \
					    $doff [expr $tmp_len - 1]]
					set ret [$pdb put -partial \
					    [list $doff $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b3:\
				    Partial put from end"
				for { set n 0 } { $n < $nentries } { incr n } {
					set tmp_len [string length $data($n)]
					set doff $tmp_len
					set dlen 0
					set newd ""
					set ret [$pdb put -partial \
					    [list $doff $dlen] $keys($n) \
					    [chop_data $pmethod $newd]]
					error_check_good partial_put($n) $ret 0
				}
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\tSi$tnum.b4: Partial put: corner cases"

				puts "\t\tSi$tnum.b4:\
				    Put nothing from beginning"
				set ret [$pdb put -partial {0 0} $keys(0) ""]
				error_check_good partial_put(0) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b4:\
				    Put nothing from middle"
				set doff [expr [string length $data(0)] / 2]
				set ret [$pdb put -partial [list $doff 0] \
				    $keys(0) ""]
				error_check_good partial_put(1) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"

				puts "\t\tSi$tnum.b4:\
				    Put nothing from end"
				set doff [expr [string length $data(0)] - 1]
				set ret [$pdb put -partial [list $doff 0] \
				    $keys(0) ""]
				error_check_good partial_put(2) $ret 0
				check_secondaries\
				    $pdb $sdbs $nentries keys data "Si$tnum.b"
			}

			puts "\tSi$tnum.c: Closing/disassociating primary"
			error_check_good primary_close [$pdb close] 0
			foreach sdb $sdbs {
				error_check_good secondary_close [$sdb close] 0
			}
		}
	}
	# If this test made the last env, close it.
	if { $eindex == -1 } {
		error_check_good env_close [$env close] 0
	}
}
