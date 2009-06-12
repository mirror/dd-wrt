# Local variable window for Insight.
# Copyright 1997, 1998, 1999, 2001 Red Hat
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (GPL) as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.


class LocalsWin {
    inherit VariableWin

    # ------------------------------------------------------------------
    #  CONSTRUCTOR - create new locals window
    # ------------------------------------------------------------------
    constructor {args} {
	update dummy
    }

    # ------------------------------------------------------------------
    # DESTRUCTOR - delete locals window
    # ------------------------------------------------------------------
    destructor {
    }

    method build_win {f} {
	global tcl_platform
	build_menu_helper Variable
	if {$tcl_platform(platform) == "windows"} {
	    frame $f.f
	    VariableWin::build_win $f.f
	    pack $f.f -expand yes -fill both -side top
	    frame $f.stat
	    pack $f.stat -side bottom -fill x
	} else {
	    VariableWin::build_win $f
	}
    }


    # ------------------------------------------------------------------
    # METHOD: reconfig
    # Overrides VarialbeWin::reconfig method.  Have to make sure the locals
    #  will get redrawn after everything is destroyed...
    # ------------------------------------------------------------------
    method reconfig {} {
	VariableWin::reconfig
	populate {}
    }

    # ------------------------------------------------------------------
    # METHOD: getVariablesBlankPath
    # Overrides VarialbeWin::getVariablesBlankPath. For a Locals Window,
    # this method returns a list of local variables.
    # ------------------------------------------------------------------
    method getVariablesBlankPath {} {
	global Update
	debug

	return [$_frame variables]
    }

    method update {event} {
	global Update Display

	debug "START LOCALS UPDATE CALLBACK"
	# Check that a context switch has not occured
	if {[context_switch]} {
	    debug "CONTEXT SWITCH"

	    # our context has changed... repopulate with new variables
	    # destroy the old tree and create a new one
	    #
	    # We need to be more intelligent about saving window state
	    # when browsing the stack or stepping into new frames, but
	    # for now, we'll have to settle for just getting this working.
	    deleteTree
	    set ChangeList {}
	    
	    # context_switch will have already created the new frame for
	    # us, so all we need to do is shove stuff into the window.
	    debug "_frame=$_frame"
	    if {$_frame != ""} {
		debug "vars=[$_frame variables]"
	    }
	    if {$_frame != "" && [$_frame variables] != ""} {
		populate {}
	    }
	}

	# Erase old variables
	if {$_frame != ""} {
	    foreach var [$_frame old] {
		$Hlist delete entry $var
		$_frame deleteOld
		unset Update($this,$var)
	    }

	    # Add new variables
	    foreach var [$_frame new] {
		set Update($this,$var) 1
		$Hlist add $var                 \
		    -itemtype text              \
		    -text [label $var]
		if {[$var numChildren] > 0} {
		    # Make sure we get this labeled as openable
		    $Tree setmode $var open
		}
	    }
	}

	# Update variables in window
	VariableWin::update dummy

	debug "END LOCALS UPDATE CALLBACK"
    }
}

