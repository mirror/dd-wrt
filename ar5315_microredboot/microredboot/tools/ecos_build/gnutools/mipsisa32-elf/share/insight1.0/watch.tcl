# Watch window for Insight.
# Copyright 1997, 1998, 1999, 2001, 2002 Red Hat
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


# ----------------------------------------------------------------------
# Implements watch windows for gdb. Inherits the VariableWin
# class from variables.tcl. 
# ----------------------------------------------------------------------

class WatchWin {
  inherit VariableWin

  # ------------------------------------------------------------------
  #  CONSTRUCTOR - create new locals window
  # ------------------------------------------------------------------
  constructor {args} {
    set Sizebox 0

    # Only allow one watch window for now...
    if {$init} {
      set init 0
    }
  }

  # ------------------------------------------------------------------
  # METHOD: build_win - build window for watch. This supplants the 
  #         one in VariableWin, so that we can add the entry at the
  #         bottom.
  # ------------------------------------------------------------------
  method build_win {f} {
    global tcl_platform
    #debug "$f"

    set Menu [build_menu_helper Watch]
    $Menu add command -label Remove -underline 0 \
      -command [format {
	%s remove [%s getSelection]
      } $this $this]

    set f [::frame $f.f]
    set treeFrame  [frame $f.top]
    set entryFrame [frame $f.expr]
    VariableWin::build_win $treeFrame
    set Entry [entry $entryFrame.ent -font global/fixed]
    button $entryFrame.but -text "Add Watch" -command [code $this validateEntry]
    pack $f -fill both -expand yes
    grid $entryFrame.ent -row 0 -column 0 -sticky news -padx 2
    grid $entryFrame.but -row 0 -column 1 -padx 2
    grid columnconfigure $entryFrame 0 -weight 1
    grid columnconfigure $entryFrame 1

    if {$tcl_platform(platform) == "windows"} {
      grid columnconfigure $entryFrame 1 -pad 20
      ide_sizebox [namespace tail $this].sizebox
      place [namespace tail $this].sizebox -relx 1 -rely 1 -anchor se
    }

    grid $treeFrame -row 0 -column 0 -sticky news
    grid $entryFrame -row 1 -column 0 -padx 5 -pady 5 -sticky news
    grid columnconfigure $f 0 -weight 1
    grid rowconfigure $f 0 -weight 1
    window_name "Watch Expressions"
    ::update idletasks
    # Binding for the entry
    bind $entryFrame.ent <Return> "$entryFrame.but flash; $entryFrame.but invoke"

  }

  method selectionChanged {entry} {
    VariableWin::selectionChanged $entry

    set state disabled
    set entry [getSelection]
    foreach var $Watched {
      set name [lindex $var 0]
      if {"$name" == "$entry"} {
	set state normal
	break
      }
    }

    $Menu entryconfigure last -state $state
  }

  method validateEntry {} {
    if {!$Running} {
      debug "Getting entry value...."
      set variable [$Entry get]
      debug "Got $variable, going to add"
      set ok [add $variable]
      debug "Added... with ok: $ok"
      
      $Entry delete 0 end
    }
  }

  # ------------------------------------------------------------------
  #  METHOD: clear_file - Clear out state so that a new executable
  #             can be loaded. For WatchWins, this means deleting
  #             the Watched list, in addition to the normal
  #             VariableWin stuff.
  # ------------------------------------------------------------------
  method clear_file {} {
    VariableWin::clear_file
    set Watched {}
  }

  # ------------------------------------------------------------------
  # DESTRUCTOR - delete watch window
  # ------------------------------------------------------------------
  destructor {
    foreach var $Watched {
      $var delete
    }
  }

  method postMenu {X Y} {
#    debug "$x $y"

    set entry [getEntry $X $Y]
    
    # Disable "Remove" if we are not applying this to the parent
    set found 0
    foreach var $Watched {
      set name [lindex $var 0]
      if {"$name" == "$entry"} {
	set found 1
	break
      }
    }

    # Ok, nasty, but a sad reality...
    set noStop [catch {$Popup index "Remove"} i]
    if {!$noStop} {
      $Popup delete $i
    }
    if {$found} {
      $Popup add command -label "Remove" -command "$this remove \{$entry\}"
    }

    VariableWin::postMenu $X $Y
  }

  method remove {entry} {
    global Display Update

    # Remove this entry from the list of watched variables
    set i [lsearch -exact $Watched $entry]
    if {$i == -1} {
      debug "WHAT HAPPENED?"
      return
    }
    set Watched [lreplace $Watched $i $i]    

    set list [$Hlist info children $entry]
    lappend list $entry
    $Hlist delete entry $entry

    $entry delete
  }

  # ------------------------------------------------------------------
  # METHOD: getVariablesBlankPath
  # Overrides VarialbeWin::getVariablesBlankPath. For a Watch Window,
  # this method returns a list of watched variables.
  #
  # ONLY return items that need to be added to the Watch Tree
  # (or use deleteTree)
  # ------------------------------------------------------------------
  method getVariablesBlankPath {} {
#    debug
    set list {}

    set variables [displayedVariables {}]
    foreach var $variables {
      set name [$var name]
      set on($name) 1
    }

    foreach var $Watched {
      set name [$var name]
      if {![info exists on($name)]} {
	lappend list $var
      }
    }

    return $list
  }

  method update {event} {
    global Update Display
    debug "START WATCH UPDATE CALLBACK"
    catch {populate {}} msg
    catch {VariableWin::update dummy} msg
    debug "Did VariableWin::update with return \"$msg\""

    # Make sure all variables are marked as _not_ Openable?
    debug "END WATCH UPDATE CALLBACK"
  }

  method showMe {} {
    debug "Watched: $Watched"
  }

  # ------------------------------------------------------------------
  # METHOD: add - add a variable to the watch window
  # ------------------------------------------------------------------
  method add {name} {
      debug "Trying to add \"$name\" to watch"
 
    # Strip all the junk after the first \n
    set var [split $name \n]
    set var [lindex $var 0]
    set var [split $var =]
    set var [lindex $var 0]

    # Strip out leading/trailing +, -, ;, spaces, commas
    set var [string trim $var +-\;\ \r\n,]

    # Make sure that we have a valid variable
    set err [catch {gdb_cmd "set variable $var"} errTxt]
    if {$err} {
      dbug W "ERROR adding variable: $errTxt"
      ManagedWin::open WarningDlg -transient \
	-over $this -message [list $errTxt] -ignorable "watchvar"
    } else {
      if {[string index $var 0] == "\$"} {
	# We must make a special attempt at verifying convenience
	# variables.. Specifically, these are printed as "void"
	# when they are not defined. So if a user type "$_I_made_tbis_up",
	# gdb responds with the value "void" instead of an error
	catch {gdb_cmd "p $var"} msg
	set msg [split $msg =]
	set msg [string trim [lindex $msg 1] \ \r\n]
	if {$msg == "void"} {
	  return 0
	}
      }

      debug "In add, going to add $name"
      # make one last attempt to get errors
      set err [catch {set foo($name) 1}]
      set err [expr {$err + [catch {expr {$foo($name) + 1}}]}]
      if {!$err} {
	  set var [gdb_variable create -expr $name]
	  set ::Update($this,$var) 1
	  lappend Watched $var
	  update dummy
	  return 1
      }
    }

    return 0
  }

  protected variable Entry
  protected variable Watched {}
  protected variable Menu {}
  protected common init 1
}
