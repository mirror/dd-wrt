# OBSOLETE: Please see gdbmenubar, gdbtoolbar, srcmenubar and srctoolbar
#
# Menu, toolbar, and status window for GDBtk.
# Copyright 1997, 1998, 1999 Cygnus Solutions
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


# Implements a menu, toolbar, and status window for GDB
# This class has methods for adding buttons & menus, and 
# a collection of methods for the standard GDB menu sets
# and button sets.  It does not actually add any buttons or
# menus on its own, however.

class oldGDBToolBar {
  inherit itk::Widget

  # ------------------------------------------------------------------
  #  CONSTRUCTOR - create new console window
  # ------------------------------------------------------------------
  constructor {src} {
    set source $src
    _load_images
    _load_src_images

    build_win
    add_hook gdb_idle_hook "$this enable_ui 1"
    add_hook gdb_busy_hook "$this enable_ui 0"
    add_hook gdb_no_inferior_hook "$this enable_ui 2"
    add_hook gdb_set_hook "$this set_hook"
  }

  # ------------------------------------------------------------------
  #  METHOD:  build_win - build the main toolbar window
  # ------------------------------------------------------------------
  public method build_win {} {

    set OtherMenus {}
    set ControlMenus {}
    set OtherButtons {}
    set ControlButtons {}

    set Menu [menu $itk_interior.m -tearoff 0]
    if {! [create_menu_items]} {
      destroy $Menu
      set Menu {}
    } else {
      [winfo toplevel $itk_interior] configure -menu $Menu
    }

    # Make a subframe so that the menu can't accidentally conflict
    # with a name created by some subclass.
    set ButtonFrame [frame $itk_interior.t]
    create_buttons

    if {! [llength $button_list]} {
      destroy $ButtonFrame
    } else {
      eval standard_toolbar $ButtonFrame $button_list
      pack $ButtonFrame $itk_interior -fill both -expand true
    }
  }

  # ------------------------------------------------------------------
  #  DESTRUCTOR - destroy window containing widget
  # ------------------------------------------------------------------
  destructor {
    remove_hook gdb_idle_hook "$this enable_ui 1"
    remove_hook gdb_busy_hook "$this enable_ui 0"
    remove_hook gdb_no_inferior_hook "$this enable_ui 2"
    remove_hook gdb_set_hook "$this set_hook"
    #destroy $this
  }

  # ------------------------------------------------------------------
  #  METHOD:  reconfig - used when preferences change
  # ------------------------------------------------------------------
  public method reconfig {} {
    debug "toolbar::reconfig"
    _load_images 1
  }

  public method _set_stepi {} {
  }

  # ------------------------------------------------------------------
  #  METHOD:  create_buttons - Add some buttons to the toolbar.  Returns
  #                         list of buttons in form acceptable to
  #                         standard_toolbar.
  # ------------------------------------------------------------------
  public method create_buttons {} {
    _load_images
    create_buttons
  }

  method add_label {name text balloon args} {
    set lname $ButtonFrame.$name
    eval label $lname -text \$text $args
    balloon register $lname $balloon
    lappend button_list $lname    
  }
 

  # ------------------------------------------------------------------
  #  METHOD:  create_button - Creates all the bookkeeping for a button,
  #           without actually inserting it in the toolbar.
  # ------------------------------------------------------------------
  method create_button {name class command balloon args} {
    set bname $ButtonFrame.$name
    set Buttons($name) $bname

    eval button $bname -command \$command $args
    balloon register $bname $balloon
    foreach elem $class {
      switch $elem {
	None {}
	default { 
	  lappend ${elem}Buttons $bname
	}
      }
    }

    return $bname
  }

  # ------------------------------------------------------------------
  #  METHOD:  add_button - Creates a button, and inserts it at the end
  #           of the button list.  Call this when the toolbar is being
  #           set up, but has not yet been made.
  # ------------------------------------------------------------------
  method add_button {name class command balloon args} {
    
    lappend button_list [eval create_button \$name \$class \$command \$balloon $args]
    
  }

  # ------------------------------------------------------------------
  #  METHOD:  insert_button - Inserts button "name" before button "before".
  #           the toolbar must be made, and the buttons must have been created
  #           before you run this.
  # ------------------------------------------------------------------
  method insert_button {name before} {

    if {[string first "-" $name] == 0} {
      set name [string range $name 1 end]
      set add_sep 1
    } else {
      set add_sep 0
    }

    if {![info exists Buttons($name)] || ![info exists Buttons($before)]} {
      error "insert_buttons called with non-existant button"
    }

    set before_col [gridCGet $Buttons($before) -column]
    set before_row [gridCGet $Buttons($before) -row]

    set slaves [grid slaves $ButtonFrame]

    set incr [expr 1 + $add_sep]
    foreach slave $slaves {
      set slave_col [gridCGet $slave -column]
      if {$slave_col >= $before_col} {
	grid configure $slave -column [expr $slave_col + $incr]
      }
    }
    if {$add_sep} {
      grid $Buttons(-$name) -column $before_col -row $before_row
    }

    # Now grid our button.  Have to put in the pady since this button
    # may not have been originally inserted by the libgui toolbar
    # proc.

    grid $Buttons($name) -column [expr $before_col + $add_sep] \
      -row $before_row -pady 2
    
  }

  method remove_button {name} {

    if {[string first "-" $name] == 0} {
      set name [string range $name 1 end]
      set remove_sep 1
    } else {
      set remove_sep 0
    }

    if {![info exists Buttons($name)] } {
      error "remove_buttons called with non-existant button $name"
    }

    set name_col [gridCGet $Buttons($name) -column]
    set name_row [gridCGet $Buttons($name) -row]
    
    grid remove $Buttons($name)
    if {$remove_sep} {
      set Buttons(-$name) [grid slaves $ButtonFrame \
			     -column [expr $name_col - 1] \
			    -row $name_row]
      grid remove $Buttons(-$name)
    }

    set slaves [grid slaves $ButtonFrame -row $name_row]

    foreach slave $slaves {
      set slave_col [gridCGet $slave -column]
      if {$slave_col > $name_col} {
	grid configure $slave -column [expr $slave_col - 1]
      }
    }    
  }

  method add_button_separator {} {
    lappend button_list -
  }
  
  method button_right_justify {} {
    lappend button_list --
  }

  method swap_button_lists {in_list out_list} {
    # Now swap out the buttons...
    set first_out [lindex $out_list 0]
    if {[info exists Buttons($first_out)] && [grid info $Buttons($first_out)] != ""} {
      foreach button $in_list {
	insert_button $button $first_out
      }
      foreach button $out_list {
	remove_button $button
      }
    } elseif {[info exists Buttons($first_out)]} {
      debug "Error in swap_button_list - $first_out not gridded..."
    } else {
      debug "Button $first_out is not in button list"
    }
  }

  ############################################################
  # The next set of commands control the menubar associated with the
  # toolbar.  Currently, only sequential addition of submenu's and menu
  # entries is allowed.  Here's what you do.  First, create a submenu
  # with the "new_menu" command.  This submenu is the targeted menu. 
  # Subsequent calls to add_menu_separator, and add_menu_command add
  # separators and commands to the end of this submenu.
  # If you need to edit a submenu, call clear_menu and then add all the
  # items again.
  #
  # Each menu command also has a class list.  Transitions between states
  #  of gdb will enable and disable different classes of menus.
  #
  # FIXME - support insert_command, and also cascade menus, whenever
  # we need it...
  # FIXME - The toolbar and the Menubar support are glommed together in
  # one class for historical reasons, but there is no good reason for this.
  ############################################################

  # ------------------------------------------------------------------
  #  METHOD:  create_menu_items - Add some menu items to the menubar.
  #                               Returns 1 if any items added.
  # 
  # num = number of last menu entry
  # ------------------------------------------------------------------
  method create_menu_items {} {
    # Empty - This is overridden in child classes.
  }

  # ------------------------------------------------------------------
  #  METHOD:  new_menu - Add a new cascade menu to the Toolbar's main menu.
  #                      Also target this menu for subsequent add_menu_command
  #                      calls.
  #
  #  name - the token for the new menu
  #  label - The label used for the label
  #  underline - the index of the underlined character for this menu item.
  #
  #  RETURNS: then item number of the menu.
  # ------------------------------------------------------------------
  method new_menu {name label underline} {
    set current_menu $Menu.$name
    set menu_list($name) [$Menu add cascade -menu  $current_menu \
			     -label $label -underline $underline]
    menu $current_menu -tearoff 0

    set item_number -1
    return $current_menu
  }

  # ------------------------------------------------------------------
  #  METHOD:  menu_exists - Report whether a menu keyed by NAME exists.
  # 
  #  name - the token for the menu sought
  #
  #  RETURNS: 1 if the menu exists, 0 otherwise.
  # ------------------------------------------------------------------
  method menu_exists {name} {
    return [info exists menu_list($name)]

  }

  # ------------------------------------------------------------------
  #  METHOD:  clear_menu - Deletes the items from one of the cascade menus
  #                        in the Toolbar's main menu.  Also makes this menu
  #                        the target menu.
  # 
  #  name - the token for the new menu
  #
  #  RETURNS: then item number of the menu, or "" if the menu is not found.
  # ------------------------------------------------------------------
  method clear_menu {name} {
    if {[info exists menu_list($name)]} {
      set current_menu [$Menu entrycget $menu_list($name) -menu]
      $current_menu delete 0 end
      set item_number -1
      return $current_menu
    } else {
      return ""
    }
  }


  # ------------------------------------------------------------------
  #  METHOD:  add_menu_separator - Adds a menu separator to the currently
  #                        targeted submenu of the Toolbar's main menu.
  # 
  # ------------------------------------------------------------------
  method add_menu_separator {} {
    incr item_number
    $current_menu add separator
  }

  # ------------------------------------------------------------------
  #  METHOD:  add_menu_command - Adds a menu command item to the currently
  #                        targeted submenu of the Toolbar's main menu.
  #
  #  class - The class of the command, used for disabling entries.
  #  label - The text for the command.
  #  command - The command for the menu entry
  #  args  - Passed to the menu entry creation command (eval'ed) 
  # ------------------------------------------------------------------
  method add_menu_command {class label command args} {

    eval $current_menu add command -label \$label -command \$command \
	  $args
      
    incr item_number

    switch $class {
      None {}
      default {
        foreach elem $class {
	  lappend menu_classes($elem) [list $current_menu $item_number]
	}
      }
    }
  }

    
  # ------------------------------------------------------------------
  #  METHOD:  _load_images - Load standard images.  Private method.
  # ------------------------------------------------------------------
  public method _load_images { {reconfig 0} } {
    global gdb_ImageDir
    if {!$reconfig && $_loaded_images} {
      return
    }
    set _loaded_images 1

    lappend imgs console reg stack vmake vars watch memory bp
    foreach name $imgs {
      image create photo ${name}_img -file [file join $gdb_ImageDir ${name}.gif]
    }
  }


  # ------------------------------------------------------------------
  #  METHOD:  _load_src_images - Load standard images.  Private method.
  # ------------------------------------------------------------------
  method _load_src_images { {reconf 0} } {
    global gdb_ImageDir

    if {!$reconf && $_loaded_src_images} {
      return
    }
    set _loaded_src_images 1

    foreach name {run stop step next finish continue edit \
		    stepi nexti up down bottom Movie_on Movie_off \
		    next_line next_check next_hit rewind prev_hit \
		  watch_movie run_expt tdump tp} {
      image create photo ${name}_img -file [file join $gdb_ImageDir ${name}.gif]
    }
  }

 # ------------------------------------------------------------------
  # METHOD:  enable_ui - enable/disable the appropriate buttons and menus
  # Called from the busy, idle, and no_inferior hooks.
  #
  # on must be:
  # value      Control    Other    Trace    State
  #   0          off       off      off     gdb is busy
  #   1          on        on       off     gdb has inferior, and is idle
  #   2          off       on       off     gdb has no inferior, and is idle
  # ------------------------------------------------------------------
  public method enable_ui {on} {
    global tcl_platform
    debug "Toolbar::enable_ui $on - Browsing=$Browsing"

    # Do the enabling so that all the disabling happens first, this way if a
    # button belongs to two groups, enabling takes precedence, which is probably right.

    switch $on {
      0 {
	set enable_list {Control disabled \
			   Other disabled \
			   Trace disabled \
			   Attach disabled \
			   Detach disabled}
      }
      1 {
	if {!$Browsing} {
	  set enable_list {Trace disabled \
			     Control normal \
			     Other normal \
			     Attach disabled \
			     Detach normal }
	  # set the states of stepi and nexti correctly
	  _set_stepi
	} else {
	  set enable_list {Control disabled Other normal Trace normal}
	}

      }
      2 {
	set enable_list {Control disabled \
			   Trace disabled \
			   Other normal \
			   Attach normal \
			   Detach disabled }
      }
      default {
	debug "Unknown type: $on in enable_ui"
	return
      }
    }

    debug "Enable list is: $enable_list"
    foreach {type state} $enable_list {
      if {[info exists ${type}Buttons]} {
	foreach button [set ${type}Buttons] {
	  $button configure -state $state
	}
      }
      if {[info exists menu_classes($type)]} {
	change_menu_state $menu_classes($type) $state
      }
    }

  }

  # ------------------------------------------------------------------
  # METHOD:  change_menu_state - Does the actual job of enabling menus...
  #          Pass normal or disabled for the state.
  # ------------------------------------------------------------------
  method change_menu_state {menuList state} {

    foreach elem $menuList {
      [lindex $elem 0] entryconfigure [lindex $elem 1] -state $state
    }	
  }


  # 
  # The next set of functions are the generic button groups that gdb uses.
  # Then toolbars that derive from this class can just mix and match
  # from the standard set as they please.
  #

  # ------------------------------------------------------------------
  #  METHOD:  create_control_buttons - Creates the step, continue, etc buttons.
  # ------------------------------------------------------------------
  
  method create_control_buttons {} {
    add_button step Control [code $source inferior step] \
      "Step (S)" -image step_img
    
    add_button next Control [code $source inferior next] \
      "Next (N)" -image next_img
    
    add_button finish Control [code $source inferior finish] \
      "Finish (F)" -image finish_img
    
    add_button continue Control [code $source inferior continue] \
      "Continue (C)" -image continue_img
    
    # A spacer before the assembly-level items looks good.  It helps
    # to indicate that these are somehow different.
    add_button_separator
    
    add_button stepi Control [code $source inferior stepi] \
      "Step Asm Inst (S)" -image stepi_img
    
    add_button nexti Control [code $source inferior nexti] \
      "Next Asm Inst (N)" -image nexti_img
    
    _set_stepi

    set Run_control_buttons {step next finish continue -stepi nexti}
    
  }

  # ------------------------------------------------------------------
  #  METHOD:  create_trace_buttons - Creates the next hit, etc.
  # ------------------------------------------------------------------
  
  method create_trace_buttons {{show 0}} {

    if {$show} {
      set command add_button
    } else {
      set command create_button
    }

    $command tfindstart Trace {tfind_cmd "tfind start"} "First Hit <F>" \
      -image rewind_img
    
    $command tfind Trace {tfind_cmd tfind} "Next Hit <N>" -image next_hit_img
    
    $command tfindprev Trace {tfind_cmd "tfind -"} "Previous Hit <P>" \
      -image prev_hit_img
    
    $command tfindline Trace {tfind_cmd "tfind line"} "Next Line Hit <L>" \
      -image next_line_img
    
    $command tfindtp Trace { tfind_cmd "tfind tracepoint"} \
      "Next Hit Here <H>" -image next_check_img

    set Trace_control_buttons {tfindstart tfind tfindprev tfindline tfindtp}

    # This is a bit of a hack, but I need to bind the standard_toolbar bindings
    # and appearances to these externally, since I am not inserting them in 
    # the original toolbar...  Have to add a method to the libgui toolbar to do this.

    if {!$show} {
      foreach name $Trace_control_buttons {
	# Make sure the button acts the way we want, not the default Tk
	# way.
	set button $Buttons($name)
	$button configure -takefocus 0 -highlightthickness 0 \
	  -relief flat -borderwidth 1	
	set index [lsearch -exact [bindtags $button] Button]
	bindtags $button [lreplace [bindtags $button] $index $index \
			    ToolbarButton]
      }
    }    
  }


  # ------------------------------------------------------------------
  #  METHOD:  create_window_buttons - Creates the registers, etc, buttons
  # ------------------------------------------------------------------
  
  method create_window_buttons {} {
    add_button reg Other {ManagedWin::open RegWin} "Registers (Ctrl+R)" -image reg_img

    add_button mem Other {ManagedWin::open MemWin} "Memory (Ctrl+M)" -image memory_img

    add_button stack Other {ManagedWin::open StackWin} "Stack (Ctrl+S)" -image stack_img

    add_button watch Other {ManagedWin::open WatchWin} "Watch Expressions (Ctrl+W)" \
      -image watch_img

    add_button vars Other {ManagedWin::open LocalsWin} "Local Variables (Ctrl+L)" \
      -image vars_img

    if {[pref get gdb/control_target]} {
      add_button bp Other {ManagedWin::open BpWin} "Breakpoints (Ctrl+B)" -image bp_img
    }

    if {[pref get gdb/mode]} {
      add_button tp Other {ManagedWin::open BpWin -tracepoints 1} \
	"Tracepoints (Ctrl+T)" -image tp_img
      
      add_button tdump Trace  {ManagedWin::open TdumpWin} "Tdump (Ctrl+D)" -image tdump_img
    }

    add_button con Other {ManagedWin::open Console} "Console (Ctrl+N)" \
      -image console_img
  }

  #
  # The next set of functions create the common menu groupings that
  # are used in gdb menus.
  #


  # ------------------------------------------------------------------
  #  METHOD:  create_view_menu - Creates the standard view menu
  # ------------------------------------------------------------------
  
  method create_view_menu {} {
    new_menu view "View" 0

    add_menu_command Other "Stack" {ManagedWin::open StackWin} \
      -underline 0 -accelerator "Ctrl+S" 
      
    add_menu_command Other "Registers" {ManagedWin::open RegWin} \
      -underline 0 -accelerator "Ctrl+R" 
      
    add_menu_command Other "Memory" {ManagedWin::open MemWin} \
      -underline 0 -accelerator "Ctrl+M" 
      
    add_menu_command Other "Watch Expressions" {ManagedWin::open WatchWin} \
      -underline 0 -accelerator "Ctrl+W" 
    add_menu_command Other "Local Variables" {ManagedWin::open LocalsWin} \
      -underline 0 -accelerator "Ctrl+L" 

    if {[pref get gdb/control_target]} {
      add_menu_command Other "Breakpoints" \
	{ManagedWin::open BpWin -tracepoints 0} \
	-underline 0 -accelerator "Ctrl+B" 
    }

    if {[pref get gdb/mode]} {
      add_menu_command Other "Tracepoints" \
        {ManagedWin::open BpWin -tracepoints 1} \
	-underline 0 -accelerator "Ctrl+T"
      add_menu_command Other "Tdump" {ManagedWin::open TdumpWin} \
	-underline 2 -accelerator "Ctrl+U"
        
    }

    add_menu_command Other "Console" {ManagedWin::open Console} \
      -underline 2 -accelerator "Ctrl+N" 
      
    add_menu_command Other "Function Browser" {ManagedWin::open BrowserWin} \
      -underline 1 -accelerator "Ctrl+F" 
    add_menu_command Other "Thread List" {ManagedWin::open ProcessWin} \
      -underline 0 -accelerator "Ctrl+H"
    if {[info exists ::env(GDBTK_DEBUG)] && $::env(GDBTK_DEBUG)} {
      add_menu_separator
      add_menu_command Other "Debug Window" {ManagedWin::open DebugWin} \
	-underline 3 -accelerator "Ctrl+U"
    }
  }

  # ------------------------------------------------------------------
  #  METHOD:  create_control_menu - Creates the standard control menu
  # ------------------------------------------------------------------
  
  method create_control_menu {} {
    new_menu cntrl "Control" 0
    
    add_menu_command Control "Step" [code $source inferior step] \
      -underline 0 -accelerator S
    
    add_menu_command Control "Next" [code $source inferior next] \
      -underline 0 -accelerator N
    
    add_menu_command Control "Finish" [code $source inferior finish] \
      -underline 0 -accelerator F
    
    add_menu_command Control "Continue" \
      [code $source inferior continue] \
      -underline 0 -accelerator C
    
    add_menu_separator
    add_menu_command Control "Step Asm Inst" \
      [code $source inferior stepi] \
      -underline 1 -accelerator S
    
    add_menu_command Control "Next Asm Inst" \
      [code $source inferior nexti] \
      -underline 1 -accelerator N
    
    # add_menu_separator
    # add_menu_command Other "Automatic Step" auto_step

  }

  # ------------------------------------------------------------------
  #  METHOD:  create_trace_menu - Creates the standard trace menu
  # ------------------------------------------------------------------
  
  method create_trace_menu {} {
    new_menu trace "Trace" 0
    
    add_menu_command Other "Save Trace Commands..." "save_trace_commands" \
      -underline 0

    add_menu_separator

    add_menu_command Trace "Next Hit" {tfind_cmd tfind} \
      -underline 0 -accelerator N
    
    add_menu_command Trace "Previous Hit" {tfind_cmd "tfind -"} \
      -underline 0 -accelerator P
    
    add_menu_command Trace "First Hit" {tfind_cmd "tfind start"} \
      -underline 0 -accelerator F
    
    add_menu_command Trace "Next Line Hit" {tfind_cmd "tfind line"} \
      -underline 5 -accelerator L
    
    add_menu_command Trace "Next Hit Here" {tfind_cmd "tfind tracepoint"} \
      -underline 9 -accelerator H
    
    add_menu_separator
    add_menu_command Trace "Tfind Line..." \
      "ManagedWin::open TfindArgs -Type LN" \
      -underline 9 -accelerator E
    
    add_menu_command Trace "Tfind PC..." \
      "ManagedWin::open TfindArgs -Type PC" \
      -underline 7 -accelerator C
    
    add_menu_command Trace "Tfind Tracepoint..." \
      "ManagedWin::open TfindArgs -Type TP" \
      -underline 6 -accelerator T

    add_menu_command Trace "Tfind Frame..." \
      "ManagedWin::open TfindArgs -Type FR" \
      -underline 6 -accelerator F
  }
  
  # ------------------------------------------------------------------
  #  METHOD:  create_help_menu - Creates the standard help menu
  # ------------------------------------------------------------------  
  method create_help_menu {} {
    new_menu help "Help" 0
    add_menu_command Other "Help Topics" {HtmlViewer::open_help index.html} \
      -underline 0
    add_menu_separator
    add_menu_command Other "About GDB..." {ManagedWin::open About -transient} \
      -underline 0
  }

  # ------------------------------------------------------------------
  #  METHOD:  set_hook - run when user enters a `set' command.
  # ------------------------------------------------------------------  
  method set_hook {varname value} {
    debug "Got $varname = $value"
    if {$varname == "os"} {
      set save_menu $current_menu
      set current_menu $Menu.view
      set title "Kernel Objects"
      if {[catch {$current_menu index $title} index]} {
	set index none
      }
      if {$value == ""} {
	# No OS, so remove KOD from View menu.
	if {$index != "none"} {
	  $current_menu delete $index
	}
      } else {
	# Add KOD to View menu, but only if it isn't already there.
	if {$index == "none"} {
	  add_menu_command Other $title {ManagedWin::open KodWin} \
	    -underline 0 -accelerator "Ctrl+K"
	}
      }
      set current_menu $save_menu

      global gdb_kod_cmd
      set gdb_kod_cmd $value
    }
  }

  #
  #  PROTECTED DATA
  #

  #
  # FIXME - Need to break the images into the sets needed for
  # each button group, and load them when the button group is
  # created.

  # This is set if we've already loaded the standard images.
  private common _loaded_images 0

  # This is set if we've already loaded the standard images.  Private
  # variable.
  private common _loaded_src_images 0

  #
  #  PUBLIC DATA
  #

  # This is a handle on our parent source window.
  protected variable source {}

  public variable Tracing 0     ;# Is tracing enabled for this gdb?
  public variable Browsing   0  ;# Are we currently browsing a trace experiment?
  public variable Collecting 0  ;# Are we currently collecting a trace experiment?

  # The list of all control buttons (ones which should be disabled when
  # not running anything or when inferior is running)
  protected variable ControlButtons {}

  # The list of all other buttons (which are disabled when inderior is
  # running)
  protected variable OtherButtons {}

  # The list of buttons that are enabled when we are in trace browse
  # mode...
  protected variable TraceButtons {}

  # This is the list of buttons that are being built up
  #
  private variable button_list {}

  #
  # This is an array of buttons names -> Tk Window names
  #

  protected variable Buttons

  # The main window's menu
  private variable Menu

  #The frame to contain the buttons:
  protected variable ButtonFrame

  # This array holds the menu classes.  The key is the class name,
  # and the value is the list of menus belonging to this class.

  protected variable menu_classes

  # These buttons go in the control area when we are browsing
  protected variable Trace_control_buttons 

  # And these go in the control area when we are running
  protected variable Run_control_buttons

  protected variable item_number -1
  protected variable current_menu {}
}
