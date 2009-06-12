# parse_args.tcl -- procedure for pulling in arguments

# parse_args takes in a set of arguments with defaults and examines
# the 'args' in the calling procedure to see what the arguments should
# be set to.  Sets variables in the calling frame to the right values.

proc parse_args { argset } {
    upvar args args

    foreach argument $argset {
	if {[llength $argument] == 1} {
	    # No default specified, so we assume that we should set
	    # the value to 1 if the arg is present and 0 if it's not.
	    # It is assumed that no value is given with the argument.
	    set result [lsearch -exact $args "-$argument"]
	    if {$result != -1} then {
		uplevel 1 [list set $argument 1]
		set args [lreplace $args $result $result]
	    } else {
		uplevel 1 [list set $argument 0]
	    }
	} elseif {[llength $argument] == 2} {
	    # There are two items in the argument.  The second is a
	    # default value to use if the item is not present.
	    # Otherwise, the variable is set to whatever is provided
	    # after the item in the args.
	    set arg [lindex $argument 0]
	    set result [lsearch -exact $args "-[lindex $arg 0]"]
	    if {$result != -1} then {
		uplevel 1 [list set $arg [lindex $args [expr $result+1]]]
		set args [lreplace $args $result [expr $result+1]]
	    } else {
		uplevel 1 [list set $arg [lindex $argument 1]]
	    }
	} else {
	    error "Badly formatted argument \"$argument\" in argument set"
	}
    }
    
    # The remaining args should be checked to see that they match the
    # number of items expected to be passed into the procedure...
}
