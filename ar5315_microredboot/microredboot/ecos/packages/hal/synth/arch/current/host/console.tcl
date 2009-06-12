# {{{  Banner                                                   

# ============================================================================
# 
#      console.tcl
# 
#      Console output support for the eCos synthetic target I/O auxiliary
# 
# ============================================================================
# ####COPYRIGHTBEGIN####
#                                                                           
#  ----------------------------------------------------------------------------
#  Copyright (C) 2002 Bart Veer
# 
#  This file is part of the eCos host tools.
# 
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the Free 
#  Software Foundation; either version 2 of the License, or (at your option) 
#  any later version.
#  
#  This program is distributed in the hope that it will be useful, but WITHOUT 
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
#  more details.
#  
#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
#  ----------------------------------------------------------------------------
#                                                                           
# ####COPYRIGHTEND####
# ============================================================================
# #####DESCRIPTIONBEGIN####
# 
#  Author(s):   bartv
#  Contact(s):  bartv
#  Date:        2002/08/07
#  Version:     0.01
#  Description:
#      Implementation of the console device. This script should only ever
#      be run from inside the ecosynth auxiliary.
# 
# ####DESCRIPTIONEND####
# ============================================================================

# }}}

# The console device is pretty simple. There can only ever be one
# instance of the console, and it does not take any initialization
# data from the target or from command-line arguments. It does
# look for entries in the target definition file, to set up
# colours for console output, and to install additional regexp-based
# filters. The only type of request that can go to the console device
# is to write some text.

namespace eval ::console {

    variable _pending_output ""
    variable filter_count 0
    array set filters [list]
    
    proc instantiate { id name data } {
	# There is only console so no name is expected, and the target
	# cannot provide any initialization data.
	if { ("" != $name) || ("" != $data) } {
	    synth::report_error "The target has passed invalid data when instantiating the console device.\n"
	    return ""
	}
	
	# There are no command line arguments to be processed and consumed.

	# Look for and consume target definition entries related to the console.
	# These are only actually applicable when running in GUI mode, but
	# should always be consumed.
	set console_appearance ""

	if { [synth::tdf_has_device "console"] } {
	    if { [synth::tdf_has_option "console" "appearance"] } {
		set console_appearance [synth::tdf_get_option "console" "appearance"]
	    }

	    if { [synth::tdf_has_option "console" "filter"] } {
		set tdf_filters [synth::tdf_get_options "console" "filter"]
		foreach filter $tdf_filters {
		    if { 2 > [llength $filter] } {
			set msg "Invalid entry in target definition file $synth::target_definition\n"
			append msg "  Device console, option filter takes at least two arguments, a name and a regular expression.\n"
			synth::report_error $msg
		    } else {
			# Attempt some minimal validation of the regexp
			set name [lindex $filter 0]
			set regexp [lindex $filter 1]
			set error ""
			if { [catch { regexp -- $regexp "Hello world\n" } error] } {
			    set msg "Invalid entry in target definition file $synth::target_definition\n"
			    append msg "  Device console, filter $name, invalid regular expression\n    $error\n"
			    synth::report_error $msg
			} else {
			    set ::console::filters($::console::filter_count,name)       $name
			    set ::console::filters($::console::filter_count,regexp)     $regexp
			    set ::console::filters($::console::filter_count,appearance) [lrange $filter 2 end]
			    incr ::console::filter_count
			}
		    }
		}
	    }
	}

	# If the GUI is enabled then set up a filter for the console, and
	# any additional filters specified in the target definition file
	# for e.g. trace output.
	if { $synth::flag_gui } {
	    if { [synth::filter_exists "console" ] } {
		synth::report_warning "The console device script [info script] cannot create a filter for \"console\".\nThis filter already exists.\n"
	    } elseif { "" == $console_appearance } {
		synth::filter_add "console"
	    } else {
		array set parsed_options [list]
		set message ""
		if { ![synth::filter_parse_options $console_appearance parsed_options message] } {
		    synth::report_error \
		        "Invalid entry in target definition file $synth::target_definition\
		         \n  synth_device \"console\", entry \"appearance\"\n$message"
		} else {
		    synth::filter_add_parsed "console" parsed_options
		}
	    }

	    for { set i 0 } { $i < $::console::filter_count } { incr i } {
		set name $::console::filters($i,name)
		set appearance $::console::filters($i,appearance)
		array unset parsed_options
		array set parsed_options [list]

		if { [synth::filter_exists $name] } {
		    synth::report_warning "The console device script [info script] cannot create a filter for \"$name\".\nThis filter already exists.\n"
		} else {
		    set message ""
		    if { ![synth::filter_parse_options $appearance parsed_options message] } {
			synth::report_error \
				"Invalid entry in target definition file $synth::target_definition\
				\n  synth_device \"console\", entry filter $name\n$message"
		    } else {
			synth::filter_add_parsed $name parsed_options
		    }
		}
	    }
	}

	# An instantiation function should return a handler for further requests
	# to this device instance.
	return console::handle_request
    }
    
    proc handle_request { id reqcode arg1 arg2 reqdata reqlen reply_len } {
	# Unfortunately the main eCos diagnostic code assumes it is
	# talking to a tty in raw mode, since typically the output
	# will go via the gdb output window. Hence it will have inserted
	# carriage returns which are best filtered out here.
	set reqdata [string map {"\r" " "} $reqdata]

	# The output should be processed one line at a time, to make it
	# easier to write the regexp filters.
	append console::_pending_output $reqdata
	while { -1 != [string first "\n" $console::_pending_output] } {
	    set regexp_matched 0
	    set index [string first "\n" $console::_pending_output]
	    set line [string range $console::_pending_output 0 $index]
	    set ::console::_pending_output [string range $console::_pending_output [expr $index + 1] end]

	    for { set i 0 } { !$regexp_matched && ($i < $console::filter_count) } { incr i } {
		if { [regexp -- $console::filters($i,regexp) $line] } {
		    synth::output $line $console::filters($i,name)
		    set regexp_matched 1
		}
	    }
	    if { ! $regexp_matched } {
		synth::output $line "console"
	    }
	}
    }

    # Deal with the case where eCos has exited after sending only part
    # of a line, which is still pending. In practice this has no
    # effect at present because the data is still buffered inside
    # eCos.
    proc _flush { arg_list } {
	if { "" != $console::_pending_output } {
	    synth::output "$console::_pending_output\n" "console"
	}
    }
    synth::hook_add "ecos_exit" console::_flush

}

return console::instantiate
