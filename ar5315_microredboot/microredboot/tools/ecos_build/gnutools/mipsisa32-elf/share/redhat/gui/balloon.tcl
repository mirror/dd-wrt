# balloon.tcl - Balloon help.
# Copyright (C) 1997, 1998, 2000 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# KNOWN BUGS:
# * On Windows, various delays should be determined from system;
#   presently they are hard-coded.
# * Likewise, balloon positioning on Windows is a hack.

itcl_class Balloon {
  # Name of associated global variable which should be set whenever
  # the help is shown.
  public variable {}

  # Name of associated toplevel.  Private variable.
  protected _top {}

  # This is non-empty if there is an after script pending.  Private
  # method.
  protected _after_id {}

  # This is an array mapping window name to help text.
  protected _help_text

  # This is an array mapping window name to notification proc.
  protected _notifiers

  # This is set to the name of the parent widget whenever the mouse is
  # in a widget with balloon help.
  protected _active {}

  # This is true when we're already calling a notification proc.
  # Private variable.
  protected _in_notifier 0

  # This holds the parent of the most recently entered widget.  It is
  # used to determine when the user is moving through a toolbar.
  # Private variable.
  protected _recent_parent {}

  constructor {top} {
    global tcl_platform

    set _top $top
    set class [$this info class]

    # The standard widget-making trick.
    set hull [namespace tail $this]
    set old_name $this
    ::rename $this $this-tmp-
    ::toplevel $hull -class $class -borderwidth 1 -background black
    ::rename $hull $old_name-win-
    ::rename $this $old_name

    # By default we are invisible.  When we are visible, we are
    # borderless.
    wm withdraw  [namespace tail $this]
    wm overrideredirect  [namespace tail $this] 1

    # Put some bindings on the toplevel.  We don't use
    # bind_for_toplevel_only because *do* want these bindings to be
    # run when the event happens on some child.
    bind $_top <Enter> [list $this _enter %W]
    bind $_top <Leave> [list $this _leave]
    # Only run this one if we aren't already destroyed.
    bind $_top <Destroy> [format {
      if {[info commands %s] != ""} then {
	%s _subdestroy %%W
      }
    } $this $this]
    bind $_top <Unmap> [list $this _unmap %W]
    # Add more here as required.
    bind $_top <1> [format {
      %s _cancel
      %s _unshowballoon
    } $this $this]
    bind $_top <3> [format {
      %s _cancel
      %s _unshowballoon
    } $this $this]

    if {$tcl_platform(platform) == "windows"} then {
      set bg SystemInfoBackground
      set fg SystemInfoText
    } else {
      # This color is called `LemonChiffon' by my X installation.
      set bg \#ffffffffcccc
      set fg black
    }

    # Where we display stuff.
    label [namespace tail $this].label -background $bg -foreground $fg -font global/status \
      -anchor w -justify left
    pack [namespace tail $this].label -expand 1 -fill both

    # Clean up when the label is destroyed.  This has the hidden
    # assumption that the balloon widget is a child of the toplevel to
    # which it is connected.
    bind [namespace tail $this].label <Destroy> [list $this delete]
  }

  destructor {
    catch {_cancel}
    catch {after cancel [list $this _unshowballoon]}
    catch {destroy $this}
  }

  method configure {config} {}

  # Register a notifier for a window.
  method notify {command window {tag {}}} {
    if {$tag == ""} then {
      set item $window
    } else {
      set item $window,$tag
    }

    if {$command == ""} then {
      unset _notifiers($item)
    } else {
      set _notifiers($item) $command
    }
  }

  # Register help for a window.
  method register {window text {tag {}}} {
    if {$tag == ""} then {
      set item $window
    } else {
      # Switching on the window class is bad.  Do something better.
      set class [winfo class $window]

      # Switching on window class is bad.  Do something better.
      switch -- $class {
	Menu {
	  # Menus require bindings that other items do not require.
	  # So here we make sure the menu has the binding.  We could
	  # speed this up by keeping a special entry in the _help_text
	  # array if we wanted.  Note that we pass in the name of the
	  # window as we know it.  That lets us work even when we're
	  # actually getting events for a clone window.  This is less
	  # than ideal, because it means we have to hijack the
	  # MenuSelect binding, but we live with it.  (The other
	  # choice is to make a new bindtag per menu -- yuck.)
	  # This is relatively nasty: we have to encode the window
	  # name as passed to the _motion method; otherwise the
	  # cloning munges it.  Sigh.
	  regsub -all -- \\. $window ! munge
	  bind $window <<MenuSelect>> [list $this _motion %W $munge]
	}

	Canvas {
	  # If we need to add a binding for this tag, do so.
	  if {! [info exists _help_text($window,$tag)]} then {
	    $window bind $tag <Enter> +[list $this _enter $window $tag]
	    $window bind $tag <Leave> +[list $this _leave]
	    $window bind $tag <1> +[format {
	      %s _cancel
	      %s _unshowballoon
	    } $this $this]
	  }
	}

	Text {
	  # If we need to add a binding for this tag, do so.
	  if {! [info exists _help_text($window,$tag)]} then {
	    $window tag bind $tag <Enter> +[list $this _enter $window $tag]
	    $window tag bind $tag <Leave> +[list $this _leave]
	    $window tag bind $tag <1> +[format {
	      %s _cancel
	      %s _unshowballoon
	    } $this $this]
	  }
	}
      }

      set item $window,$tag
    }

    set _help_text($item) $text
    if {$_active == $item} then {
      _set_variable $item
      # If the label is already showing, then we re-show it.  Why not
      # just set the -text on the label?  Because if the label changes
      # size it might be offscreen, and we need to handle that.
      if {[wm state [namespace tail $this]] == "normal"} then {
	showballoon $window $tag
      }
    }
  }

  # Cancel any pending after handler.  Private method.
  method _cancel {} {
    if {$_after_id != ""} then {
      after cancel $_after_id
      set _after_id {}
    }
  }

  # This is run when the toplevel, or any child, is entered.  Private
  # method.
  method _enter {W {tag {}}} {
    _cancel

    # Don't bother for menus, since we know we use a different
    # mechanism for them.
    if {[winfo class $W] == "Menu"} then {
      return
    }

    # If we just moved into the parent of the last child, then do
    # nothing.  We want to keep the parent the same so the right thing
    # can happen if we move into a child of this same parent.
    set delay 1000
    if {$W != $_recent_parent} then {
      if {[winfo parent $W] == $_recent_parent} then {
	# As soon as possible.
	set delay idle
      } else {
	set _recent_parent ""
      }
    }

    if {$tag == ""} then {
      set index $W
    } else {
      set index $W,$tag
    }
    set _active $index
    if {[info exists _help_text($index)]} then {
      # There is some help text.  So arrange to display it when the
      # time is up.  We arbitrarily set this to 1 second.
      set _after_id [after $delay [list $this showballoon $W $tag]]

      # Set variable here; that way simply entering a window will
      # cause the text to appear.
      _set_variable $index
    }
  }

  # This is run when the toplevel, or any child, is left.  Private
  # method.
  method _leave {} {
    _cancel
    _unshowballoon
    _set_variable {}
    set _active {}
  }

  # This is run to undisplay the balloon.  Note that it does not
  # change the text stored in the variable.  That is handled
  # elsewhere.  Private method.
  method _unshowballoon {} {
    wm withdraw  [namespace tail $this]
  }

  # Set the variable, if it exists.  Private method.
  method _set_variable {index} {
    # Run the notifier.
    if {$index == ""} then {
      set value ""
    } elseif {[info exists _notifiers($index)] && ! $_in_notifier} then {
      if {$variable != ""} {
	upvar $variable var
	set var $_help_text($index)
      }
      set _in_notifier 1
      uplevel \#0 $_notifiers($index)
      set _in_notifier 0
      # Get value afterwards to give notifier a chance to change it.
      if {$variable != ""} {
	upvar $variable var
	set _help_text($index) $var
      } 
      set value $_help_text($index)
    } else {
      set value $_help_text($index)
    }

    if {$variable != ""} then {
      upvar $variable var
      set var $value
    }
  }

  # This is run to show the balloon.  Private method.
  method showballoon {W tag {keep 0}} {
    global tcl_platform

    if {$tag == ""} then {
      # An ordinary window.  Position below the window, and right of
      # center.
      set _active $W
      set left [expr {[winfo rootx $W] + round ([winfo width $W] * .75)}]
      set ypos [expr {[winfo rooty $W] + [winfo height $W]}]
      set alt_ypos [winfo rooty $W]

      # Balloon shown, so set parent info.
      set _recent_parent [winfo parent $W]
    } else {
      set _active $W,$tag
      # Switching on class name is bad.  Do something better.  Can't
      # just use the widget's bbox method, because the results differ
      # for Text and Canvas widgets.  Bummer.
      switch -- [winfo class $W] {
	Menu {
	  # Recognize but do nothing.
	}

	Text {
	  lassign [$W bbox $tag.first] x y width height
	  set left [expr {[winfo rootx $W] + $x + round ($width * .75)}]
	  set ypos [expr {[winfo rooty $W] + $y + $height}]
	  set alt_ypos [expr {[winfo rooty $W] - $y}]
	}

	Canvas {
	  lassign [$W bbox $tag] x1 y1 x2 y2
	  # Must subtract out coordinates of top-left corner of canvas
	  # window; otherwise this will get the wrong position when
	  # the canvas has been scrolled.
	  set tlx [$W canvasx 0]
	  set tly [$W canvasy 0]
	  # Must round results because canvas coordinates are floats.
	  set left [expr {round ([winfo rootx $W] + $x1 - $tlx
				 + ($x2 - $x1) * .75)}]
	  set ypos [expr {round ([winfo rooty $W] + $y2 - $tly)}]
	  set alt_ypos [expr {round ([winfo rooty $W] + $y1 - $tly)}]
	}

	default {
	  error "unrecognized window class for window \"$W\""
	}
      }
    }

    set help $_help_text($_active)

    # On Windows, the popup location is always determined by the
    # cursor.  Actually, the rule seems to be somewhat more complex.
    # Unfortunately it doesn't seem to be written down anywhere.
    # Experiments show that the location is determined by the cursor
    # if the text is wider than the widget; and otherwise it is
    # centered under the widget.  FIXME: we don't deal with those
    # cases.
    if {$tcl_platform(platform) == "windows"} then {
      # FIXME: for now this is turned off.  It isn't enough to get the
      # cursor size; we actually have to find the bottommost "on"
      # pixel in the cursor and use that for the height.  I don't know
      # how to do that.
      # lassign [ide_cursor size] dummy height
      # lassign [ide_cursor position] left ypos
      # incr ypos $height
    }

    if {[info exists left] && $help != ""} then {
      [namespace tail $this].label configure -text $help
      set lw [winfo reqwidth [namespace tail $this].label]
      set sw [winfo screenwidth [namespace tail $this]]
      set bw [$this-win- cget -borderwidth]
      if {$left + $lw + 2 * $bw >= $sw} then {
	set left [expr {$sw - 2 * $bw - $lw}]
      }

      set lh [winfo reqheight [namespace tail $this].label]
      if {$ypos + $lh >= [winfo screenheight [namespace tail $this]]} then {
	set ypos [expr {$alt_ypos - $lh}]
      }

      wm positionfrom  [namespace tail $this] user
      wm geometry  [namespace tail $this] +${left}+${ypos}
      update
      wm deiconify  [namespace tail $this]
      raise  [namespace tail $this]

      if {!$keep} {
	# After 6 seconds, close the window.  The timer is reset every
	# time the window is shown.
	after cancel [list $this _unshowballoon]
	after 6000 [list $this _unshowballoon]
      }
    }
  }

  # This is run when a window or tag is destroyed.  Private method.
  method _subdestroy {W {tag {}}} {
    if {$tag == ""} then {
      # A window.  Remove the window and any associated tags.  Note
      # that this is called for all Destroy events on descendents,
      # even for windows which were never registered.  Hence the use
      # of catch.
      catch {unset _help_text($W)}
      foreach thing [array names _help_text($W,*)] {
	unset _help_text($thing)
      }
    } else {
      # Just a tag.  This one can't be called by mistake, so this
      # shouldn't need to be caught.
      unset _help_text($W,$tag)
    }
  }

  # This is run in response to a MenuSelect event on a menu.
  method _motion {window name} {
    # Decode window name.
    regsub -all -- ! $name . name

    if {$variable == ""} then {
      # There's no point to doing anything.
      return
    }

    set n [$window index active]
    if {$n == "none"} then {
      set index ""
      set _active {}
    } elseif {[info exists _help_text($name,$n)]} then {
      # Tag specified by index number.
      set index $name,$n
      set _active $name,$n
    } elseif {! [catch {$window entrycget $n -label} label]
	      && [info exists _help_text($name,$label)]} then {
      # Tag specified by index name.
      set index $name,$label
      set _active $name,$label
    } else {
      # No help for this item.
      set index ""
      set _active {}
    }

    _set_variable $index
  }

  # This is run when some widget unmaps.  If the widget is the current
  # widget, then unmap the balloon help.  Private method.
  method _unmap w {
    if {$w == $_active} then {
      _cancel
      _unshowballoon
      _set_variable {}
      set _active {}
    }
  }
}


################################################################

# Find (and possibly create) balloon widget associated with window.
proc BALLOON_find_balloon {window} {
  # Find our associated toplevel.  If it is a menu, then keep going.
  set top [winfo toplevel $window]
  while {[winfo class $top] == "Menu"} {
    set top [winfo toplevel [winfo parent $top]]
  }

  if {$top == "."} {
    set bname .__balloon
  } else {
    set bname $top.__balloon
  }
  
  # If the balloon help for this toplevel doesn't exist, then create
  # it.  Yes, this relies on a magic name for the balloon help widget.
  if {! [winfo exists $bname]} then {
    Balloon $bname $top
  }  
  return $bname
}

# This implements "balloon register".
proc BALLOON_command_register {window text {tag {}}} {
  set b [BALLOON_find_balloon $window]
  $b register $window $text $tag
}

# This implements "balloon notify".
proc BALLOON_command_notify {command window {tag {}}} {
  set b [BALLOON_find_balloon $window]
  $b notify $command $window $tag
}

# This implements "balloon show".
proc BALLOON_command_show {window {tag {}} {keep 0}} {
  set b [BALLOON_find_balloon $window]
  $b showballoon $window $tag $keep
}

proc BALLOON_command_withdraw {window} {
  set b [BALLOON_find_balloon $window]
  $b _unmap $window
}
    
# This implements "balloon variable".
proc BALLOON_command_variable {window args} {
  if {[llength $args] == 0} then {
    # Fetch.
    set b [BALLOON_find_balloon $window]
    return [$b cget -variable]
  } else {
    # FIXME: no arg checking here.
    # Set.
    set b [BALLOON_find_balloon $window]
    $b configure -variable [lindex $args 0]
  }
}

# The primary interface to balloon help.
# Usage:
#  balloon notify COMMAND WINDOW ?TAG?
#    Run COMMAND just before the help text for WINDOW (and TAG, if
#    given) is displayed.  If COMMAND is the empty string, then
#    notification is disabled for this window.
#  balloon register WINDOW TEXT ?TAG?
#    Associate TEXT as the balloon help for WINDOW.
#    If TAG is given, the use the appropriate tag for association.
#    For menu widgets, TAG is a menu index.
#    For canvas widgets, TAG is a tagOrId.
#    For text widgets, TAG is a text index.  If you want to use
#      the text tag FOO, use `FOO.last'.
#  balloon show WINDOW ?TAG?
#    Immediately pop up the balloon for the given window and tag.
#    This should be used sparingly.  For instance, you might need to
#    use it if the tag you're interested in does not track the mouse,
#    but instead is added just before show-time.
#  balloon variable WINDOW ?NAME?
#    If NAME specified, set balloon help variable associated
#    with window.  This variable is set to the text whenever the
#    balloon help is on.  If NAME is specified but empty,
#    no variable is set.  If NAME not specified, then the
#    current variable name is returned.
#  balloon withdraw WINDOW
#    Withdraw the balloon window associated with WINDOW.  This should
#    be used sparingly.
proc balloon {key args} {
  if {[info commands BALLOON_command_$key] == "" } then {
    error "unrecognized key \"$key\""
  }

  eval BALLOON_command_$key $args
}
