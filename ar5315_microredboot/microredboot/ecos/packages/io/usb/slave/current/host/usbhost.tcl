# {{{  Banner						

#===============================================================================
#
#    usbhost.tcl
#
#    Support for USB testing
#
#===============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
#===============================================================================
######DESCRIPTIONBEGIN####
#
# Author(s):	bartv
# Date:		2001-07-04
# Purpose:      To provide higher-level utility commands for performing
#               USB testing, and to iterate through the various test scripts
#               specified on the command line.
#
#####DESCRIPTIONEND####
#===============================================================================
#

# }}}

# {{{  Endpoint data					

# Given the raw endpoint data provided by the C code, turn
# it something more usable from inside Tcl scripts.
namespace eval usbtest {
    array set control {}

    variable bulk_in_endpoints [list]
    array set bulk_in {}
    variable bulk_out_endpoints [list]
    array set bulk_out {}
    
    variable isochronous_in_endpoints [list]
    array set isochronous_in {}
    variable isochronous_out_endpoints [list]
    array set isochronous_out {}

    variable interrupt_in_endpoints [list]
    array set interrupt_in {}
    variable interrupt_out_endpoints [list]
    array set interrupt_out {}

    for { set i 0 } { $i < $usbtest::endpoint_count } { incr i } {
	switch -- $usbtest::endpoint_data($i,type) {
	    "control" {
		set usbtest::control(min_size) $usbtest::endpoint_data($i,min_size)
		set usbtest::control(max_size) $usbtest::endpoint_data($i,max_size)
	    }

	    "bulk" {
		set number $usbtest::endpoint_data($i,number)
		if { "in" == $usbtest::endpoint_data($i,direction) } {
		    lappend usbtest::bulk_in_endpoints $number
		    set usbtest::bulk_in($number,min_size)		$usbtest::endpoint_data($i,min_size)
		    set usbtest::bulk_in($number,max_size)		$usbtest::endpoint_data($i,max_size)
		    set usbtest::bulk_in($number,max_in_padding)	$usbtest::endpoint_data($i,max_in_padding)
		    set usbtest::bulk_in($number,devtab)		$usbtest::endpoint_data($i,devtab)
		} else {
		    lappend usbtest::bulk_out_endpoints $number
		    set usbtest::bulk_out($number,min_size)		$usbtest::endpoint_data($i,min_size)
		    set usbtest::bulk_out($number,max_size)		$usbtest::endpoint_data($i,max_size)
		    set usbtest::bulk_out($number,devtab)		$usbtest::endpoint_data($i,devtab)
		}
	    }

	    "isochronous" {
		set number $usbtest::endpoint_data($i,number)
		if { "in" == $usbtest::endpoint_data($i,direction) } {
		    lappend usbtest::isochronous_in_endpoints $number
		    set usbtest::isochronous_in($number,min_size)	$usbtest::endpoint_data($i,min_size)
		    set usbtest::isochronous_in($number,max_size)	$usbtest::endpoint_data($i,max_size)
		    set usbtest::isochronous_in($number,devtab)		$usbtest::endpoint_data($i,devtab)
		} else {
		    lappend usbtest::isochronous_out_endpoints $number
		    set usbtest::isochronous_out($number,min_size)	$usbtest::endpoint_data($i,min_size)
		    set usbtest::isochronous_out($number,max_size)	$usbtest::endpoint_data($i,max_size)
		    set usbtest::isochronous_out($number,devtab)	$usbtest::endpoint_data($i,devtab)
		}
	    }

	    "interrupt" {
		set number $usbtest::endpoint_data($i,number)
		if { "in" == $usbtest::endpoint_data($i,direction) } {
		    lappend usbtest::interrupt_in_endpoints $number
		    set usbtest::interrupt_in($number,min_size)		$usbtest::endpoint_data($i,min_size)
		    set usbtest::interrupt_in($number,max_size)		$usbtest::endpoint_data($i,max_size)
		    set usbtest::interrupt_in($number,devtab)		$usbtest::endpoint_data($i,devtab)
		} else {
		    lappend usbtest::interrupt_out_endpoints $number
		    set usbtest::interrupt_out($number,min_size)	$usbtest::endpoint_data($i,min_size)
		    set usbtest::interrupt_out($number,max_size)	$usbtest::endpoint_data($i,max_size)
		    set usbtest::interrupt_out($number,devtab)		$usbtest::endpoint_data($i,devtab)
		}
	    }

	    default {
		puts stderr "Internal error: invalid endpoint type $usbtest::endpoint_data($i,type)"
		exit 1
	    }
	}
    }
}

# }}}
# {{{  Constants					

# The C code expects to receive certain data as simple numbers,
# corresponding to #define's in common.c and elsewhere. Strictly
# speaking it would be better to pass strings to the C code and
# have it do the translation, thus ensuring that these constants
# exist in only one place.

namespace eval usbtest {

    variable _USB_DIR_IN        0x0080
    variable _DATA_NONE		     0
    variable _DATA_BYTE_FILL	     1
    variable _DATA_WORD_FILL	     2
    variable _DATA_BYTE_GEN	     3
    variable _DATA_WORD_GEN	     4
    variable _IO_MECHANISM_USB	     1
    variable _IO_MECHANISM_DEV	     2
}

# It is also desirable to have some constants corresponding
# to common random number generators.
namespace eval usbtest {
    variable MULTIPLIER        1103515245
    variable INCREMENT              12345
}

# }}}
# {{{  Argument processing                              

# ----------------------------------------------------------------------------
# Given a list of arguments of the form "xyzzy=123" or "xyzzy 123", and
# an array arguments containing entries such as arguments(xyzzy) and
# already filled in with default values, update the array using the
# actual arguments
namespace eval usbtest {

    proc process_arguments { list array_ref } {
	upvar $array_ref array
	array set defined_args [list]

	set listlen [llength $list]
	for { set index 0 } { $index < $listlen } { incr index } {
	    set arg [lindex $list $index]
	    set found 0
	    foreach name [array names array] {
		set len [string length $name]
		if { [string equal -length $len $name $arg] } {
		    # Partial match found.
		    if { [string equal $name $arg] } {
			# Exact match found, The value must be the next arg.
			if { [info exists defined_args($name)] } {
			    error "Argument $name should be specified only once"
			}
			incr index
			if { $index >= $listlen } {
			    error "Missing value after argument $name"
			}
			set array($name) [lindex $list $index]
			set found 1
			break
		    }

		    # Not an exact match. Try looking for x=y
		    incr len
		    if { [string equal -length $len "$name=" $arg] } {
			if { [info exists defined_args($name)] } {
			    error "Argument $name should be specified only once"
			}
			set array($name) [string range $arg $len end]
			set found 1
			break
		    }
		}
	    }
	    if { ! $found } {
		error "Invalid argument $arg"
	    }
	}
    }
}

# }}}
# {{{  Starting and ending tests			

# This section deals with starting tests, or cleaning up when the
# tests cannot actually proceed. Also there is some validation,
# for example to make sure that no endpoint number is used for
# multiple tests.

namespace eval usbtest {
    variable results
    variable _tests_submitted 0
    variable _control_endpoint_in_use 0
    variable _in_endpoints_in_use [list]
    variable _out_endpoints_in_use [list]
    
    proc reset { } {
	if { 0 != $usbtest::_tests_submitted } {
	    usbtest::_cancel
	    set usbtest::_tests_submitted 0
	    
	}
	set usbtest::_in_endpoints_in_use [list]
	set usbtest::_out_endpoints_in_use [list]
    }

    proc use_endpoint { endpoint direction } {
	if { 0 == $endpoint } {
	    if { $usbtest::_control_endpoint_in_use } {
		error "Attempt to run multiple tests on the control endpoint"
	    }
	    set usbtest::_control_endpoint_in_use 1
	} else {
	    switch -- $direction {
		"in" {
		    if { -1 != [lsearch -exact $usbtest::_in_endpoints_in_use $endpoint] } {
			error "Attempt to run multiple IN tests on endpoint $endpoint"
		    }
		    lappend usbtest::_in_endpoints_in_use $endpoint
		}

		"out" {
		    if { -1 != [lsearch -exact $usbtest::_out_endpoints_in_use $endpoint] } {
			error "Attempt to run multiple OUT tests on endpoint $endpoint"
		    }
		    lappend usbtest::_out_endpoints_in_use $endpoint
		}

		default {
		    error "Invalid direction passed to usbtest::use_endpoint"
		}
	    }
	}
    }

    proc test_submitted { } {
	incr usbtest::_tests_submitted
    }
    
    proc start { timeout } {
	set result 0
	if { 0 == $usbtest::_tests_submitted } {
	    error "Attempt to start tests when no tests are scheduled to run."
	} elseif { ! [string is integer -strict $timeout] } {
	    error "Invalid timeout specified, it should be a simple number."
	} else {
	    set usbtest::results [list]
	    set result [usbtest::_run $timeout]
	    set usbtest::_tests_submitted 0
	    set usbtest::_control_endpoint_in_use 0
	    array unset _in_endpoints_in_use
	    array unset _out_endpoints_in_use
	}
	return $result
    }
}

# }}}
# {{{  Bulk tests					

# Prepare to run a bulk test.
#
# This test requires rather a lot of parameters, many of which
# will have sensible defaults.

namespace eval usbtest {

    proc bulktest { endpoint direction number_packets args } {

	
	# Parameters to be passed to the C code. Most are
	# held in an array indexed by the option name,
	# facilitating command-line parsing.
	set arguments(format)        "none"
	set arguments(data1)         0
	set arguments(data*)         1
	set arguments(data+)         0
	set arguments(data1*)        1
	set arguments(data1+)        0
	set arguments(data**)        1
	set arguments(data*+)        0
	set arguments(data+*)        1
	set arguments(data++)        0
	set arguments(mechanism)     "usb"
	set arguments(txsize1)       32
	set arguments(txsize>=)      0
	set arguments(txsize<=)      -1
	set arguments(txsize*)       1
	set arguments(txsize/)       1
	set arguments(txsize+)       0
	set arguments(rxsize1)       0
	set arguments(rxsize>=)      0
	set arguments(rxsize<=)      -1
	set arguments(rxsize*)       1
	set arguments(rxsize/)       1
	set arguments(rxsize+)       0
	set arguments(txdelay1)      0
	set arguments(txdelay>=)     0
	set arguments(txdelay<=)     1000000000
	set arguments(txdelay*)      1
	set arguments(txdelay/)      1
	set arguments(txdelay+)      0
	set arguments(rxdelay1)      0
	set arguments(rxdelay>=)     0
	set arguments(rxdelay<=)     1000000000
	set arguments(rxdelay*)      1
	set arguments(rxdelay/)      1
	set arguments(rxdelay+)      0
	
	set endpoint_param			""

	# Target limits
	set target_min_size			0
	set target_max_size			0
	set target_padding			0
	set target_devtab			""
	
	# Start by validating the endpoint and direction arguments.
	# Also check that the specified endpoint is not yet in use.
	if { ![string is integer -strict $endpoint] } {
	    error "Invalid endpoint argument \"$endpoint\": should be a number"
	}
	if { ($endpoint < 1) || ($endpoint > 15) } {
	    error "Invalid bulk endpoint argument \"$endpoint\": should be between 1 and 15"
	}
	switch -- $direction {
	    "in" -
	    "In" -
	    "IN" {
		set direction "in"
		if { -1 == [lsearch -exact $usbtest::bulk_in_endpoints $endpoint] } {
		    error "Invalid bulk endpoint argument \"$endpoint\": the target does not list that as a bulk IN endpoint"
		}
		set target_min_size	 $usbtest::bulk_in($endpoint,min_size)
		set target_max_size	 $usbtest::bulk_in($endpoint,max_size)
		set target_padding       $usbtest::bulk_in($endpoint,max_in_padding)
		set target_devtab	 $usbtest::bulk_in($endpoint,devtab);
		set endpoint_param [expr $endpoint | $usbtest::_USB_DIR_IN]
	    }

	    "out" -
	    "Out" -
	    "OUT" {
		set direction "out"
		if { -1 == [lsearch -exact $usbtest::bulk_out_endpoints $endpoint] } {
		    error "Invalid bulk endpoint argument \"$endpoint\": the target does not list that as a bulk OUT endpoint"
		}
		set target_min_size	 $usbtest::bulk_out($endpoint,min_size)
		set target_max_size	 $usbtest::bulk_out($endpoint,max_size)
		set target_devtab	 $usbtest::bulk_out($endpoint,devtab);
		set target_padding       0; # Not applicable
		set endpoint_param       $endpoint
	    }

	    default {
		error "Invalid direction argument \"$direction\": should be \"in\" or \"out\""
	    }
	}

	# Now parse any remaining arguments
	usbtest::process_arguments $args arguments
	
	# Convert two of the arguments from strings to numbers, for the
	# convenience of the C code
	switch -- $arguments(format) {
	    "none"      { set arguments(format) $usbtest::_DATA_NONE }
	    "bytefill"  { set arguments(format) $usbtest::_DATA_BYTE_FILL }
	    "wordfill"  { set arguments(format) $usbtest::_DATA_WORD_FILL }
	    "byteseq"   { set arguments(format) $usbtest::_DATA_BYTE_GEN }
	    "wordseq"   { set arguments(format) $usbtest::_DATA_WORD_GEN }

	    default {
		error "Invalid data format argument \"$arguments(data)\"\n    \
		       Should be \"none\", \"bytefill\", \"wordfill\", \"byteseq\" or \"wordseq\""
	    }
	}
	switch -- $arguments(mechanism) {
	    "usb"       { set arguments(mechanism) $usbtest::_IO_MECHANISM_USB }
	    "devtab"    { set arguments(mechanism) $usbtest::_IO_MECHANISM_DEV }

	    default {
		error "Invalid mechanism argument \"$arguments(mechanism)\"\n    \
		       Should be \"usb\" or \"devtab\""
	    }
	}

	puts "validating fields"
	# Validate the remaining fields
	foreach field [list data1 data* data+ data1* data1+ data** data*+ data+* data++ \
	                    txsize1 txsize>= txsize<= txsize* txsize/ txsize+           \
	                    rxsize1 rxsize>= rxsize<= rxsize* rxsize/ rxsize+           \
	                    txdelay1 txdelay>= txdelay<= txdelay* txdelay/ txdelay+     \
	                    rxdelay1 rxdelay>= rxdelay<= rxdelay* rxdelay/ rxdelay+] {
            if { ![string is integer -strict $arguments($field)] } {
		error "Invalid value \"$arguments($field)\" for argument $field, should be an integer."
	    }
        }
	
	if { $arguments(txsize>=) < $target_min_size } {
	    set arguments(txsize>=) $target_min_size
	}
	if { (-1 == $arguments(txsize<=) ) || ($arguments(txsize<=) > $target_max_size) } {
	    set arguments(txsize<=) $target_max_size
	}
	if { $arguments(rxsize<=) == -1 } {
	    set arguments(rxsize<=) $target_max_size
	}
	if { $arguments(txsize1) < $arguments(txsize>=) } {
	    set arguments(txsize1) $arguments(txsize>=)
	}
	# Make sure the endpoint is not already in use
	usbtest::use_endpoint $endpoint $direction

	puts "Submitting test"
	# Now submit the test. This is handled by C code.
	usbtest::_test_bulk             \
		$number_packets		\
		$endpoint_param		\
		$arguments(txsize1)     \
	        $arguments(txsize>=)    \
	        $arguments(txsize<=)    \
	        $arguments(txsize*)     \
	        $arguments(txsize/)     \
	        $arguments(txsize+)     \
	        $arguments(rxsize1)     \
	        $arguments(rxsize>=)    \
	        $arguments(rxsize<=)    \
	        $arguments(rxsize*)     \
	        $arguments(rxsize/)     \
	        $arguments(rxsize+)     \
	        $target_padding         \
	        $arguments(txdelay1)    \
	        $arguments(txdelay>=)   \
	        $arguments(txdelay<=)   \
	        $arguments(txdelay*)    \
	        $arguments(txdelay/)    \
	        $arguments(txdelay+)    \
	        $arguments(rxdelay1)    \
	        $arguments(rxdelay>=)   \
	        $arguments(rxdelay<=)   \
	        $arguments(rxdelay*)    \
	        $arguments(rxdelay/)    \
	        $arguments(rxdelay+)    \
	        $arguments(mechanism)   \
	        $arguments(format)      \
	        $arguments(data1)       \
	        $arguments(data*)       \
	        $arguments(data+)       \
	        $arguments(data1*)      \
	        $arguments(data1+)      \
	        $arguments(data**)      \
	        $arguments(data*+)      \
	        $arguments(data+*)      \
	        $arguments(data++)

	test_submitted
    }
}

# }}}
# {{{  Execute the specified test script 		

# Interpret the arguments as a test script plus auxiliary data.
set script [lindex $::argv 0]
set ::argv [lrange $::argv 1 end]

set result [catch {
    set path [file join [pwd] $script]
    if { ![file exists $path] } {
	set path "$path.tcl"
	if { ![file exists $path] } {
	    set path [file join $usbtest::USBAUXDIR $script]
	    if { ![file exists $path] } {
		set path "$path.tcl"
		if { ![file exists $path] } {
		    error "Error: unknown test script $script"
		}
	    }
	}
    }
    
    source $path
    
} message]

if { 0 != $result } {
    puts $message
}

# }}}
