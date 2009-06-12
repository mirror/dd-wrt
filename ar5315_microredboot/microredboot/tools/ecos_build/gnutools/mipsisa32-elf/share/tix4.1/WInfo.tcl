# WInfo.tcl --
#
#	This file implements the command tixWInfo, which return various
#	information about a Tix widget.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

proc tixWInfo {option w} {
    upvar #0 $w data

    case $option {
	tix {
	    # Is this a Tix widget?
	    #
	    return [info exists data(className)]
	}
	compound {
	    # Is this a compound widget?
	    #	Currently this is the same as "tixWinfo tix" because only
	    # Tix compilant compound widgets are supported
	    return [info exists data(className)]
	}
	class {
	    if {[info exists data(className)]} {
		return $data(className)
	    } else {
		return ""
	    }
	}
    }
}
