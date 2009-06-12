# toolbar.tcl - Handle layout for a toolbar.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# This holds global state for this module.
defarray TOOLBAR_state {
  initialized 0
  button ""
  window ""
  relief flat
  last   ""
}

proc TOOLBAR_button_enter {w} {
  global TOOLBAR_state
  
  #save older relief (it covers buttons that
  #interacte like checkbuttons)
  set TOOLBAR_state(relief) [$w cget -relief]
    
  if {[$w cget -state] != "disabled"} then {
  
    if {$TOOLBAR_state(button) == $w} then {
      set relief sunken
    } else {
      set relief raised
    }
    
    $w configure \
	-state active \
	-relief $relief
  }

  #store last action to synchronize operations
  set TOOLBAR_state(last) enter
  set TOOLBAR_state(window) $w
}

proc TOOLBAR_button_leave {w} {
    global TOOLBAR_state
    if {[$w cget -state] != "disabled"} then {
	$w configure -state normal
    }

    #restore original relief
    if {
	$TOOLBAR_state(window) == $w
        && $TOOLBAR_state(last) == "enter"
    } then {
	$w configure -relief $TOOLBAR_state(relief)
    } else {
	$w configure -relief flat
    }

    set TOOLBAR_state(window) ""
    #store last action to synch operations (enter->leave)
    set TOOLBAR_state(last) leave
}

proc TOOLBAR_button_down {w} {
  global TOOLBAR_state
  if {[$w cget -state] != "disabled"} then {
    set TOOLBAR_state(button) $w
    $w configure -relief sunken
  }
}

proc TOOLBAR_button_up {w} {
  global TOOLBAR_state
  if {$w == $TOOLBAR_state(button)} then {
    set TOOLBAR_state(button) ""
    
    #restore original relief
      $w configure -relief $TOOLBAR_state(relief)      
    
    if {$TOOLBAR_state(window) == $w
	&& [$w cget -state] != "disabled"} then {

      #SN does the toolbar bindings using "+" so that older
      #bindings don't disapear. So no need to invoke the command.
      #other applications should do the same so that we can delete
      #this hack
      global sn_options
      if {! [array exists sn_options]} {
	#invoke the binding
	uplevel \#0 [list $w invoke]
      }
      if {[winfo exists $w]} then {
	if {[$w cget -state] != "disabled"} then {
	  $w configure -state normal
	}
      }
      # HOWEVER, if the pointer is still over the button, and it
      # is enabled, then raise it again.

      if {[string compare [winfo containing \
			     [winfo pointerx $w] \
			     [winfo pointery $w]] $w] == 0} { 
	$w configure -relief raised
      }	
    }
  }
}

# Set up toolbar bindings.
proc TOOLBAR_maybe_init {} {
  global TOOLBAR_state
  if {! $TOOLBAR_state(initialized)} then {
    set TOOLBAR_state(initialized) 1

    # We can't put our bindings onto the widget (and then use "break"
    # to avoid the class bindings) because that interacts poorly with
    # balloon help.
    bind ToolbarButton <Enter> [list TOOLBAR_button_enter %W]
    bind ToolbarButton <Leave> [list TOOLBAR_button_leave %W]
    bind ToolbarButton <1> [list TOOLBAR_button_down %W]
    bind ToolbarButton <ButtonRelease-1> [list TOOLBAR_button_up %W]
  }
}

#Allows changing options of a toolbar button from the application
#especially the relief value
proc TOOLBAR_command {w args} {
    global TOOLBAR_state
    
    set len [llength $args]
    for {set i 0} {$i < $len} {incr i} {
	set cmd [lindex $args $i]
	switch -- $cmd {
	  "relief" -
	  "-relief" {
	  	incr i
	        set TOOLBAR_state(relief) [lindex $args $i]
		$w configure $cmd [lindex $args $i]
	    }
	  "window" -
	  "-window" {
	  	incr i
		set TOOLBAR_state(window) [lindex $args $i]
	  }
	  default {
	  	#normal widget options
		incr i
		$w configure $cmd [lindex $args $i]
	  }
	}
    }
}

# Pass this proc a frame and some children of the frame.  It will put
# the children into the frame so that they look like a toolbar.
# Children are added in the order they are listed.  If a child's name
# is "-", then the appropriate type of separator is entered instead.
# If a child's name is "--" then all remaining children will be placed
# on the right side of the window.
#
# For non-flat mode, each button must display an image, and this image
# must have a twin.  The primary (raised) image's name must end in
# "u", and the depressed image's name must end in "d".  Eg the edit
# images should be called "editu" and "editd".  There's no doubt that
# this is a hack.
#
# If you want to add a button that doesn't have an image (or whose
# image doesn't have a twin), you must wrap it in a frame.
#
# FIXME: someday, write a `toolbar button' widget that handles the
# image mess invisibly.
proc standard_toolbar {frame args} {
  global tcl_platform

  # For now, there are two different layouts, depending on which kind
  # of icons we're using.  This is just a test feature and will be
  # eliminated once we decide on an icon style.  

  TOOLBAR_maybe_init

  # We reserve column 0 for some padding.
  set column 1
  if {$tcl_platform(platform) == "windows"} then {
    # See below to understand this.
    set row 1
  } else {
    set row 0
  }
  # This is set if we see "--" and thus the filling happens in the
  # center.
  set center_fill 0
  set sticky w
  foreach button $args {
    grid columnconfigure $frame $column -weight 0

    if {$button == "-"} then {
      # A separator.
      set f [frame $frame.[gensym] -borderwidth 1 -width 2 -relief sunken]
      grid $f -row $row -column $column -sticky ns${sticky} -padx 4
    } elseif {$button == "--"} then {
      # Everything after this is put on the right.  We do this by
      # adding a column that sucks up all the space.
      set center_fill 1
      set sticky e
      grid columnconfigure $frame $column -weight 1 -minsize 7
    } elseif {[winfo class $button] != "Button"} then {
      # Something other than a button.  Just put it into the frame.
      grid $button -row $row -column $column -sticky $sticky -pady 2
    } else {
      # A button.
      # FIXME: does Windows allow focus traversal?  For now we're
      # just turning it off.
      $button configure -takefocus 0 -highlightthickness 0 \
	-relief flat -borderwidth 1
      grid $button -row $row -column $column -sticky $sticky -pady 2

      # Make sure the button acts the way we want, not the default Tk
      # way.
      set index [lsearch -exact [bindtags $button] Button]
      bindtags $button [lreplace [bindtags $button] $index $index \
			  ToolbarButton]
    }

    incr column
  }

  # On Unix, it looks a little more natural to have a raised toolbar.
  # On Windows the toolbar is flat, but there is a horizontal
  # separator between the toolbar and the menubar.  On both platforms
  # we provide some space to the left of the leftmost widget.
  grid columnconfigure $frame 0 -minsize 7 -weight 0

  if {$tcl_platform(platform) == "windows"} then {
    $frame configure -borderwidth 0 -relief flat
    set name $frame.[gensym]
    frame $name -height 2 -borderwidth 1 -relief sunken
    grid $name -row 0 -column 0 -columnspan $column -pady 1 -sticky ew
  } else {
    $frame configure -borderwidth 2 -relief raised
  }

  if {! $center_fill} then {
    # The rightmost column sucks up the extra space.
    incr column -1
    grid columnconfigure $frame $column -weight 1
  }
}
