# Variable display window for Insight.
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
# Implements variable windows for gdb. LocalsWin and WatchWin both
# inherit from this class. You need only override the method 
# 'getVariablesBlankPath' and a few other things...
# ----------------------------------------------------------------------

class VariableWin {
    inherit EmbeddedWin GDBWin
    protected variable Sizebox 1

    # ------------------------------------------------------------------
    #  CONSTRUCTOR - create new watch window
    # ------------------------------------------------------------------
    constructor {args} {
	#
	#  Create a window with the same name as this object
	#
	gdbtk_busy
	set _queue [Queue \#auto]
	build_win $itk_interior
	gdbtk_idle

	add_hook gdb_no_inferior_hook "$this no_inferior"
	add_hook gdb_clear_file_hook [code $this clear_file]
        # FIXME: This is too harsh.  We must add to varobj a method
        # to re-parse the expressions and compute new types so we can
	# keep the contents of the window whenever possible.
	add_hook file_changed_hook [code $this clear_file]
    }

    # ------------------------------------------------------------------
    #  METHOD:  build_win - build the watch window
    # ------------------------------------------------------------------
    method build_win {f} {
	global tcl_platform Display
	#    debug
	set width [font measure global/fixed "W"]
	# Choose the default width to be...
	set width [expr {40 * $width}]
	if {$tcl_platform(platform) == "windows"} {
	    set scrollmode both
	} else {
	    set scrollmode auto
	}

	debug "tree=$f.tree"
	set Tree [tixTree $f.tree        \
		      -opencmd  "$this open"  \
		      -closecmd "$this close" \
		      -ignoreinvoke 1         \
		      -width $width           \
		      -browsecmd [list $this selectionChanged] \
		      -scrollbar $scrollmode \
		      -sizebox $Sizebox]
	if {![pref get gdb/mode]} {
	    $Tree configure -command [list $this editEntry]
	}
	set Hlist [$Tree subwidget hlist]

        # FIXME: probably should use columns instead.
        $Hlist configure -header 1 

	set l [expr {$EntryLength - $Length - [string length "Name"]}]
	# Ok, this is as hack as it gets
	set blank "                                                                                                                                                             "
      $Hlist header create 0 -itemtype text -headerbackground $::Colors(bg) \
	    -text "Name[string range $blank 0 $l]Value"

	# Configure the look of the tree
	set width [font measure global/fixed $LengthString]
	$Hlist configure -indent $width \
	  -bg $::Colors(textbg) -fg $::Colors(textfg) \
	  -selectforeground $::Colors(textfg) -selectbackground $::Colors(textbg) \
	  -selectborderwidth 0 -separator . -font global/fixed

	# Get display styles
	set normal_fg    [$Hlist cget -fg]
	set highlight_fg $::Colors(sfg)
	set disabled_fg  red
	set NormalTextStyle [tixDisplayStyle text -refwindow $Hlist \
			       -bg $::Colors(textbg) -font global/fixed]
        set HighlightTextStyle [tixDisplayStyle text -refwindow $Hlist \
				  -bg $::Colors(hbg) -font global/fixed]
	set DisabledTextStyle [tixDisplayStyle text -refwindow $Hlist \
				   -bg green -fg red -font global/fixed]

	if {[catch {gdb_cmd "show output-radix"} msg]} {
	    set Radix 10
	} else {
	    regexp {[0-9]+} $msg Radix
	}


	# Update the tree display
	update dummy
	pack $Tree -expand yes -fill both

	# Create the popup menu for this widget
	bind $Hlist <3> "$this postMenu %X %Y"
	bind $Hlist <KeyPress-space> [code $this toggleView]

	# Do not use the tixPopup widget... 
	set Popup [menu $f.menu -tearoff 0]
	set disabled_foreground red
	$Popup configure -disabledforeground $disabled_foreground
	set ViewMenu [menu $Popup.view]

	# Populate the view menu
	$ViewMenu add radiobutton -label "Hex" -variable Display($this) \
	    -value hexadecimal
	$ViewMenu add radiobutton -label "Decimal" -variable Display($this) \
	    -value decimal
	$ViewMenu add radiobutton -label "Binary" -variable Display($this) \
	    -value binary
	$ViewMenu add radiobutton -label "Octal" -variable Display($this) \
	    -value octal
	$ViewMenu add radiobutton -label "Natural" -variable Display($this) \
	    -value natural

	$Popup add command -label "dummy" -state disabled
	$Popup add separator
	$Popup add cascade -label "Format" -menu $ViewMenu
	#    $Popup add checkbutton -label "Auto Update"
	#    $Popup add command -label "Update Now"
	if {![pref get gdb/mode]} {
	    $Popup add command -label "Edit"
	}

	# Make sure to update menu info.
	selectionChanged ""

	window_name "Local Variables" "Locals"
    }

    # ------------------------------------------------------------------
    #  DESTRUCTOR - destroy window containing widget
    # ------------------------------------------------------------------
    destructor {
	#    debug
	# Make sure to clean up the frame
	catch {destroy $_frame}
	
	# Delete the display styles used with this window
	destroy $NormalTextStyle
	destroy $HighlightTextStyle
	destroy $DisabledTextStyle

	# Remove this window and all hooks
	remove_hook gdb_no_inferior_hook "$this no_inferior"
	remove_hook gdb_clear_file_hook [code $this clear_file]
	remove_hook file_changed_hook [code $this clear_file]
    }

    # ------------------------------------------------------------------
    #  METHOD:  clear_file - Clear out state and prepare for loading
    #              a new executable.
    # ------------------------------------------------------------------
    method clear_file {} {
	no_inferior
    }

    # ------------------------------------------------------------------
    #  METHOD:  reconfig - used when preferences change
    # ------------------------------------------------------------------
    method reconfig {} {
	#    debug
	foreach win [winfo children $itk_interior] { 
	    destroy $win
	}

	build_win $itk_interior
    }

    # ------------------------------------------------------------------
    #  METHOD:  build_menu_helper - Create the menu for a subclass.
    # ------------------------------------------------------------------
    method build_menu_helper {first} {
	global Display
	menu [namespace tail $this].mmenu

	[namespace tail $this].mmenu add cascade -label $first -underline 0 -menu [namespace tail $this].mmenu.var

	menu [namespace tail $this].mmenu.var
	if {![pref get gdb/mode]} {
	    [namespace tail $this].mmenu.var add command -label Edit -underline 0 -state disabled \
		-command [format {
		    %s editEntry [%s getSelection]
		} $this $this]
	}
	[namespace tail $this].mmenu.var add cascade -label Format -underline 0 -state disabled \
	    -menu [namespace tail $this].mmenu.var.format

	menu [namespace tail $this].mmenu.var.format
	foreach label {Hex Decimal Binary Octal Natural} fmt {hexadecimal decimal binary octal natural} {
	    [namespace tail $this].mmenu.var.format add radiobutton \
		-label $label -underline 0 \
		-value $fmt -variable Display($this) \
		-command [format {
		    %s setDisplay [%s getSelection] %s
		} $this $this $fmt]
	}

	#    [namespace tail $this].mmenu add cascade -label Update -underline 0 -menu [namespace tail $this].mmenu.update
	#    menu [namespace tail $this].mmenu.update

	# The -variable is set when a selection is made in the tree.
	#    [namespace tail $this].mmenu.update add checkbutton -label "Auto Update" -underline 0 \
	    #      -command [format {
	#	%s toggleUpdate [%s getSelection]
	#      } $this $this]
	#    [namespace tail $this].mmenu.update add command -label "Update Now" -underline 0 \
	    #      -accelerator "Ctrl+U" -command [format {
	#	%s updateNow [%s getSelection]
	#      } $this $this]

	set top [winfo toplevel [namespace tail $this]]
	$top configure -menu [namespace tail $this].mmenu
	bind_plain_key $top Control-u [format {
	    if {!$Running} {
		if {[%s getSelection] != ""} {
		    %s updateNow [%s getSelection]
		}
	    }
	} $this $this $this]

	return [namespace tail $this].mmenu.var
    }

    # Return the current selection, or the empty string if none.
    method getSelection {} {
	return [$Hlist info selection]
    }

    # This is called when a selection is made.  It updates the main
    # menu.
    method selectionChanged {variable} {
	global Display

	if {$Running} {
	    # Clear the selection, too
	    $Hlist selection clear
	    return
	}

	# if something is being edited, cancel it
	if {[info exists EditEntry]} {
	    UnEdit
	}

	if {$variable == ""} {
	    set state disabled
	} else {
	    set state normal
	}

	foreach menu [list [namespace tail $this].mmenu.var [namespace tail $this].mmenu.var.format ] {
	    set i [$menu index last]
	    while {$i >= 0} {
		if {[$menu type $i] != "cascade"} {
		    $menu entryconfigure $i -state $state
		}
		incr i -1
	    }
	}

	if {$variable != "" && [$variable editable]} {
	    set state normal
	} else {
	    set state disabled
	}

	if {$variable != ""} {
	    set Display($this) [$variable format]
	}

	foreach label {Hex Decimal Binary Octal Natural} {
	    [namespace tail $this].mmenu.var.format entryconfigure $label
	    if {$label != "Hex"} {
		[namespace tail $this].mmenu.var.format entryconfigure $label -state $state
	    }
	}
	#    [namespace tail $this].mmenu.update entryconfigure 0 -variable Update($this,$name)
    }

    method updateNow {variable} {
	# debug "$variable"

	if {!$Running} {
	    set text [label $variable]
	    $Hlist entryconfigure $variable -itemtype text -text $text
	}
    }

    method getEntry {x y} {
	set realY [expr {$y - [winfo rooty $Hlist]}]

	# Get the tree entry which we are over
	return [$Hlist nearest $realY]
    }

    method editEntry {variable} {
	if {!$Running} {
	    if {$variable != "" && [$variable editable]} {
		edit $variable
	    }
	}
    }

    method postMenu {X Y} {
	global Update Display
	#    debug

	# Quicky for menu posting problems.. How to unpost and post??

	if {[winfo ismapped $Popup] || $Running} {
	    return
	}

	set variable [getEntry $X $Y]
	if {[string length $variable] > 0} {
	  # First things first: highlight the variable we just selected
	  $Hlist selection set $variable

	    # Configure menu items
	    # the title is always first..
	    #set labelIndex [$Popup index "dummy"]
	    set viewIndex [$Popup index "Format"]
	    #      set autoIndex [$Popup index "Auto Update"]
	    #      set updateIndex [$Popup index "Update Now"]
	    set noEdit [catch {$Popup index "Edit"} editIndex]

	    # Retitle and set update commands
	    $Popup entryconfigure 0 -label "[$variable name]"
	    #      $Popup entryconfigure $autoIndex -command "$this toggleUpdate \{$entry\}" \
		-variable Update($this,$entry) 
	    #      $Popup entryconfigure $updateIndex -command "$this updateNow \{$entry\}"

	    # Edit pane
	    if {$variable != "" && [$variable editable]} {
		if {!$noEdit} {
		    $Popup delete $editIndex
		}
		if {![pref get gdb/mode]} {
		    $Popup  add command -label Edit -command "$this edit \{$variable\}"
		}
	    } else {
		if {!$noEdit} {
		    $Popup delete $editIndex
		}
	    }

	    # Set view menu
	    set Display($this) [$variable format]
	    foreach i {0 1 2 3 4} fmt {hexadecimal decimal binary octal natural} {
		debug "configuring entry $i ([$ViewMenu entrycget $i -label]) to $fmt"
		$ViewMenu entryconfigure $i \
		    -command "$this setDisplay \{$variable\} $fmt"
	    }

	    if {$::tcl_platform(platform) == "windows"} {
	      # Don't ask me why this works, but it does work around
	      # a Win98/2000 Tcl bug with deleting entries from popups...
	      set no [$Popup index end]
	      for { set k 1 } { $k < $no } { incr k } {
		$Popup insert 1 command 
	      }
	      $Popup delete 1 [expr {$no - 1}]
	    }

	    tk_popup $Popup $X $Y
	}
    }

    # ------------------------------------------------------------------
    # METHOD edit -- edit a variable
    # ------------------------------------------------------------------
    method edit {variable} {
	global Update

	# disable menus
	selectionChanged ""
        debug "editing \"$variable\""

	set fg   [$Hlist cget -foreground]
	set bg   [$Hlist cget -background]

	if {$Editing == ""} {
	    # Must create the frame
	    set Editing [frame $Hlist.frame -bg $bg -bd 0 -relief flat]
	    set lbl [::label $Editing.lbl -fg $fg -bg $bg -font global/fixed]
	    set ent [entry $Editing.ent -bg $::Colors(bg) -fg $::Colors(fg) -font global/fixed]
	    pack $lbl $ent -side left
	}

	if {[info exists EditEntry]} {
	    # We already are editing something... So reinstall it first
	    # I guess we discard any changes?
	    UnEdit
	}

	# Update the label/entry widgets for this instance
	set Update($this,$variable) 1
	set EditEntry $variable
	set label [label $variable 1];	# do not append value
	$Editing.lbl configure -text "$label  "
	$Editing.ent delete 0 end

	# Strip the pointer type, text, etc, from pointers, and such
	set err [catch {$variable value} text]
	if {$err} {return}
	if {[$variable format] == "natural"} {
	    # Natural formats must be stripped. They often contain
	    # things like strings and characters after them.
	    set index [string first \  $text]
	    if {$index != -1} {
		set text [string range $text 0 [expr {$index - 1}]]
	    }
	}
	$Editing.ent insert 0 $text

	# Find out what the previous entry is
	set previous [getPrevious $variable]

	$Hlist delete entry $variable

	set cmd [format { \
			      %s add {%s} %s -itemtype window -window %s \
			  } $Hlist $variable $previous $Editing]
	eval $cmd

	if {[$variable numChildren] > 0} {
	    $Tree setmode $variable open
	}

	# Set focus to entry
	focus $Editing.ent
	$Editing.ent selection to end

	# Setup key bindings
	bind $Editing.ent <Return> "$this changeValue"
	bind $Hlist <Return> "$this changeValue"
	bind $Editing.ent <Escape> "$this UnEdit"
	bind $Hlist <Escape> "$this UnEdit"
    }

    method getPrevious {variable} {
	set prev [$Hlist info prev $variable]
	set parent [$Hlist info parent $variable]

	if {$prev != ""} {
	    # A problem occurs with PREV if its parent is not the same as the entry's
	    # parent. For example, consider these variables in the window:
	    # + foo        struct {...}
	    # - bar        struct {...}
	    #     a        1
	    #     b        2
	    # local        0
	    # if you attempt to edit "local", previous will be set at "bar.b", not
	    # "struct bar"...
	    if {[$Hlist info parent $prev] != $parent} {
		# This is the problem!
		# Find this object's sibling in that parent and place it there.
		set children [$Hlist info children $parent]
		set p {}
		foreach child $children {
		    if {$child == $variable} {
			break
		    }
		    set p $child
		}

		if {$p == {}} {
		    # This is the topmost child
		    set previous "-before [lindex $children 1]"
		} else {
		    set previous "-after $p"
		}
	    } else {
		set previous "-after \{$prev\}"
	    }
	} else {
	    # this is the first!
	    set previous "-at 0"
	}
	
	if {$prev == "$parent"} {
	    # This is the topmost-member of a sub-grouping..
	    set previous "-at 0"
	}

	return $previous
    }

    method UnEdit {} {
	set previous [getPrevious $EditEntry]
	
	$Hlist delete entry $EditEntry
	set cmd [format {\
			     %s add {%s} %s -itemtype text -text {%s} \
			 } $Hlist $EditEntry $previous [label $EditEntry]]
	eval $cmd
	if {[$EditEntry numChildren] > 0} {
	    $Tree setmode $EditEntry open
	}
	
	# Unbind
	bind $Hlist <Return> {}
	bind $Hlist <Escape> {}
	if {$Editing != ""} {
	    bind $Editing.ent <Return> {}
	    bind $Editing.ent <Escape> {}
	}
	
	unset EditEntry
	selectionChanged ""
    }

    method changeValue {} {
	# Get the old value
	set new [string trim [$Editing.ent get] \ \r\n]
	if {$new == ""} {
	    UnEdit
	    return
	}

	if {[catch {$EditEntry value $new} errTxt]} {
	    tk_messageBox -icon error -type ok -message $errTxt \
		-title "Error in Expression" -parent [winfo toplevel $itk_interior]
	    focus $Editing.ent
	    $Editing.ent selection to end
	} else {
	    UnEdit

            # We may have changed a register or something else that is 
            # being displayed in another window
            gdbtk_update
	    
	    # Get rid of entry... and replace it with new value
	    focus $Tree
	}
    }


    # ------------------------------------------------------------------
    #  METHOD:  toggleView: Toggle open/close the current selection.
    # ------------------------------------------------------------------  
    method toggleView {} {

	set v [getSelection]
	set mode [$Tree getmode $v]

	# In the tixTree widget, "open" means "openable", not that it is open...

	debug "mode=$mode"
	switch $mode {
	    open {
		$Tree setmode $v close
		open $v
	    }

	    close {
		$Tree setmode $v open
		close $v
	    }

	    default {
		dbug E "What happened?"
	    }
	}
    }

    method toggleUpdate {variable} {
	global Update
      debug $variable
	if {$Update($this,$variable)} {
	  debug NORMAL
	    # Must update value
	    $Hlist entryconfigure $variable \
		-style $NormalTextStyle    \
		-text [label $variable]
	} else {
	  debug DISABLED
	    $Hlist entryconfigure $variable \
		-style $DisabledTextStyle
	}
	::update
    }

    method setDisplay {variable format} {
	debug "$variable $format"
	if {!$Running} {
	    $variable format $format
	    set ::Display($this) $format
	    $Hlist entryconfigure $variable -text [label $variable]
	}
    }
    
    # ------------------------------------------------------------------
    # METHOD:   label - used to label the entries in the tree
    # ------------------------------------------------------------------
    method label {variable {noValue 0}} {
	# Ok, this is as hack as it gets
	set blank "                                                                                                                                                             "
	# Use protected data Length to determine how big variable
	# name should be. This should clean the display up a little
	set name [$variable name]
	set indent [llength [split $variable .]]
	set indent [expr {$indent * $Length}]
	set len [string length $name]
	set l [expr {$EntryLength - $len - $indent}]
	set label "$name[string range $blank 0 $l]"
	#debug "label=$label $noValue"
	if {$noValue} {
	    return $label
	}

	set err [catch {$variable value} value]
	set value [string trim $value \ \r\t\n]
	#debug "err=$err value=$value"

	# Insert the variable's type for things like ptrs, etc.
	set type [$variable type]
	if {!$err} {
	    if {$value == "{...}"} {
		set val " $type $value"
	    } elseif {[string first * $type] != -1} {
		set val " ($type) $value"
	    } elseif {[string first \[ $type] != -1} {
		set val " $type"
	    } else {
		set val " $value"
	    }
	} else {
	    set val " $value"
	}

	return "$label $val"
    }

    # ------------------------------------------------------------------
    # METHOD:   open - used to open an entry in the variable tree
    # ------------------------------------------------------------------
    method open {path} {
	global Update
	# We must lookup all the variables for this struct
	#    debug "$path"

	# Cancel any edits
	if {[info exists EditEntry]} {
	    UnEdit
	}

	if {!$Running} {
	    # Do not open disabled paths
	    if {$Update($this,$path)} {
		cursor watch
		populate $path
		cursor {}
	    }
	} else {
	    $Tree setmode $path open
	}
    }

    # ------------------------------------------------------------------
    # METHOD:   close - used to close an entry in the variable tree
    # ------------------------------------------------------------------
    method close {path} {
	global Update
	debug "$path"
	# Close the path and destroy all the entry widgets

	# Cancel any edits
	if {[info exists EditEntry]} {
	    UnEdit
	}

	if {!$Running} {
	    # Only update when we we are not disabled
	    if {$Update($this,$path)} {

		# Delete the offspring of this entry
		$Hlist delete offspring $path
	    }
	} else {
	    $Tree setmode $path close
	}
    }

    method isVariable {var} {

	set err [catch {gdb_cmd "output $var"} msg]
	if {$err 
	    || [regexp -nocase "no symbol|syntax error" $msg]} {
	    return 0
	}

	return 1
    }

    # OVERRIDE THIS METHOD
    method getVariablesBlankPath {} {
	dbug -W "You forgot to override getVariablesBlankPath!!"
	return {}
    }

    method cmd {cmd} {
	eval $cmd
    }
    
    # ------------------------------------------------------------------
    # METHOD:   populate - populate an entry in the tree
    # ------------------------------------------------------------------
    method populate {parent} {
	global Update
	debug "$parent"

	if {[string length $parent] == 0} {
	    set variables [getVariablesBlankPath]
	} else {
	    set variables [$parent children]
	}

	debug "variables=$variables"
	eval $_queue push $variables
	for {set variable [$_queue pop]} {$variable != ""} {set variable [$_queue pop]} {
	    debug "inserting variable: $variable"
	    set Update($this,$variable) 1

	    $Hlist add $variable          \
		-itemtype text              \
		-text [label $variable]
	    if {[$variable numChildren] > 0} {
		# Make sure we get this labeled as openable
		$Tree setmode $variable open
	    }

	    # Special case: If we see "public" with no value or type, then we
	    # have one of our special c++/java children. Open it automagically
	    # for the user.
	    if {[string compare [$variable name] "public"] == 0
		&& [$variable type] == "" && [$variable value] == ""} {
		eval $_queue push [$variable children]
		$Tree setmode $variable close
	    }
	}

	debug "done with populate"
    }

    # Get all current locals
    proc getLocals {} {

	set vars {}
	set err [catch {gdb_get_args} v]
	if {!$err} {
	    set vars [concat $vars $v]
	}

	set err [catch {gdb_get_locals} v]
	if {!$err} {
	    set vars [concat $vars $v]
	}

	debug "--getLocals:\n$vars\n--getLocals"
	return [lsort $vars]
    }

    method context_switch {} {
	set err [catch {gdb_selected_frame} current_frame]
	debug "1: err=$err; _frame=\"$_frame\"; current_frame=\"$current_frame\""
	if {$err && $_frame != ""} {
	    # No current frame
	    debug "no current frame"
	    catch {destroy $_frame}
	    set _frame {}
	    return 1
	} elseif {$current_frame == "" && $_frame == ""} {
	    debug "2"
	    return 0
	} elseif {$_frame == "" || $current_frame != [$_frame address]} {
	    # We've changed frames. If we knew something about
	    # the stack layout, we could be more intelligent about
	    # destroying variables, but we don't know that here (yet).
	    debug "switching to frame at $current_frame"

	    # Destroy the old frame and create the new one
	    catch {destroy $_frame}
	    set _frame [Frame ::\#auto $current_frame]
	    debug "created new frame: $_frame at [$_frame address]"
	    return 1
	}

	# Nothing changed
	debug "3"
	return 0
    }

    # ------------------------------------------------------------------
    # METHOD:   update
    # OVERRIDE THIS METHOD and call it from there
    # ------------------------------------------------------------------
    method update {event} {
	global Update
	debug

	# First, reset color on label to normal
	foreach w $ChangeList {
	    catch {
		$Hlist entryconfigure $w -style $NormalTextStyle
	    }
	}

	# Tell toplevel variables to update themselves. This will
	# give us a list of all the variables in the table that
	# have changed values.
	set ChangeList {}
	set variables [$Hlist info children {}]
	foreach var $variables {
	    # debug "VARIABLE: $var ($Update($this,$var))"
            set numchild [$var numChildren]
	    set UpdatedList [$var update]
            # FIXME: For now, we can only infer that the type has changed
            # if the variable is not a scalar; the varobj code will have to
            # give us an indication that this happened.
            if {([lindex $UpdatedList 0] == $var)
                && ($numchild > 0)} {
              debug "Type changed."
              # We must fix the tree entry to correspond to the new type
              $Hlist delete offsprings $var
              $Hlist entryconfigure $var -text [label $var]
              if {[$var numChildren] > 0} {
                $Tree setmode $var open
              } else {
                $Tree setmode $var none
              }
            } else {
	      set ChangeList [concat $ChangeList $UpdatedList]
	      # debug "ChangeList=$ChangeList"
            }
	}

	foreach var $ChangeList {
	  debug "$var HIGHLIGHT"
	    $Hlist entryconfigure $var \
		-style  $HighlightTextStyle   \
		-text [label $var]
	}
    }

    method idle {event} {
	# Re-enable the UI
	enable_ui
    }

    # RECURSION!!
    method displayedVariables {top} {
	#    debug
	set variableList {}
	set variables [$Hlist info children $top]
	foreach var $variables {
	    set mode [$Tree getmode $var]
	    if {$mode == "close"} {
		set moreVars [displayedVariables $var]
		lappend variableList [join $moreVars]
	    }
	    lappend variableList $var
	}

	return [join $variableList]
    }

    method deleteTree {} {
	global Update
	debug
#	set variables [displayedVariables {}]

	# Delete all HList entries
	$Hlist delete all

	# Delete the variable objects
#	foreach i [array names Variables] {
#	    $Variables($i) delete
#	    unset Variables($i)
#	    catch {unset Update($this,$i)}
#	}
    }

    # ------------------------------------------------------------------
    # METHOD:   enable_ui
    #           Enable all ui elements.
    # ------------------------------------------------------------------
    method enable_ui {} {
	
	# Clear fencepost
	set Running 0
	cursor {}
    }

    # ------------------------------------------------------------------
    #   PUBLIC METHOD:  busy - BusyEvent handler
    #           Disable all ui elements that could affect gdb's state
    # ------------------------------------------------------------------
    method busy {event} {

	# Set fencepost
	set Running 1

	# Cancel any edits
	if {[info exists EditEntry]} {
	    UnEdit
	}

	# Change cursor
	cursor watch
    }

    # ------------------------------------------------------------------
    # METHOD:   no_inferior
    #           Reset this object.
    # ------------------------------------------------------------------
    method no_inferior {} {

	# Clear out the Hlist
	deleteTree

	# Clear fencepost
	set Running 0
	set _frame {}
	cursor {}
    }

    # ------------------------------------------------------------------
    #  METHOD:  cursor - change the toplevel's cursor
    # ------------------------------------------------------------------
    method cursor {what} {
	[winfo toplevel [namespace tail $this]] configure -cursor $what
	::update idletasks
    }

    #
    # PUBLIC DATA
    #

    #
    #  PROTECTED DATA
    #

    # the tixTree widget for this class
    protected variable Tree  {}

    # the hlist of this widget
    protected variable Hlist {}

    # entry widgets which need to have their color changed back to black
    # when idle (used in conjunction with update)
    protected variable ChangeList {}

    protected variable ViewMenu
    protected variable Popup

    # These are for setting the indent level to an number of characters.
    # This will help clean the tree a little
    common EntryLength 15
    common Length 1
    common LengthString " "

    # These should be common... but deletion?
    # Display styles for HList
    protected variable HighlightTextStyle
    protected variable NormalTextStyle
    protected variable DisabledTextStyle
    
    protected variable Radix

    # Frame object for the selected frame
    protected variable _frame {}

    protected variable Editing {}
    protected variable EditEntry

    # Fencepost for enable/disable_ui and idle/busy hooks.
    protected variable Running 0

    # little queue for convenience
    protected variable _queue {}
}
