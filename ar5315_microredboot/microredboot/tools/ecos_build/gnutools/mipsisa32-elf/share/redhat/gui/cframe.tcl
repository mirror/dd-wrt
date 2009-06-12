# cframe.tcl - Frame controlled by checkbutton.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

itcl_class Checkframe {
  inherit Widgetframe

  # The checkbutton text.
  public text {} {
    _set_option -text $text 0
  }

  # This holds the last value of -variable.  We use it to unset our
  # trace when the variable changes (or is deleted).  Private
  # variable.
  protected _saved_variable {}

  # The checkbutton variable.
  public variable {} {
    _var_changed
  }

  # The checkbutton -onvalue.
  public onvalue 1 {
    _set_option -onvalue $onvalue
  }

  # The checkbutton -offvalue.
  public offvalue 0 {
    _set_option -offvalue $offvalue
  }

  # The checkbutton -command.
  public command {} {
    _set_option -command $command 0
  }

  # This holds balloon help for the checkbutton.
  public help {} {
    if {[winfo exists [namespace tail $this].check]} then {
      balloon register [namespace tail $this].check $help
    }
  }

  # This holds a list of all widgets which should be immune to
  # enabling/disabling.  Private variable.
  protected _avoid {}

  constructor {config} {
    checkbutton [namespace tail $this].check -text $text -variable $variable -padx 2 \
      -command $command -onvalue $onvalue -offvalue $offvalue
    balloon register [namespace tail $this].check $help
    _add [namespace tail $this].check
  }

  # Exempt a child from state changes.  Argument EXEMPT is true if the
  # child should be exempted, false if it should be re-enabled again.
  # Public method.
  method exempt {child {exempt 1}} {
    if {$exempt} then {
      if {[lsearch -exact $_avoid $child] == -1} then {
	lappend _avoid $child
      }
    } else {
      set _avoid [lremove $_avoid $child]
      _set_visibility $child
    }
  }

  # This is run when the state of the frame's children should change.
  # Private method.
  method _set_visibility {{child {}}} {
    if {$variable == ""} then {
      # No variable means everything is ok.  The behavior here is
      # arbitrary; this is a losing case.
      set state normal
    } else {
      upvar \#0 $variable the_var
      if {! [string compare $the_var $onvalue]} then {
	set state normal
      } else {
	set state disabled
      }
    }

    if {$child != ""} then {
      $child configure -state $state
    } else {
      # FIXME: we force our logical children to be actual children of
      # the frame.  Instead we should ask the geometry manager what's
      # going on.
      set avoid(_) {}
      unset avoid(_)
      foreach child $_avoid {
	set avoid($child) {}
      }
      foreach child [winfo children [namespace tail $this].iframe.frame] {
	if {! [info exists avoid($child)]} then {
	  catch {$child configure -state $state}
	}
      }
    }
  }

  # This is run to possibly update some option on the checkbutton.
  # Private method.
  method _set_option {option value {set_vis 1}} {
    if {[winfo exists [namespace tail $this].check]} then {
      [namespace tail $this].check configure $option $value
      if {$set_vis} then {
	_set_visibility
      }
    }
  }

  # This is run when our associated variable changes.  We use the
  # resulting information to set the state of our children.  Private
  # method.
  method _trace {name1 name2 op} {
    if {$op == "u"} then {
      # The variable got deleted.  So we stop looking at it.
      uplevel \#0 [list trace vdelete $_saved_variable uw [list $this _trace]]
      set _saved_variable {}
      set variable {}
    } else {
      # Got a write.
      _set_visibility
    }
  }

  # This is run when the -variable changes.  We remove our old trace
  # (if there was one) and add a new trace (if we need to).  Private
  # method.
  method _var_changed {} {
    if {$_saved_variable != ""} then {
      # Remove the old trace.
      uplevel \#0 [list trace vdelete $_saved_variable uw [list $this _trace]]
    }
    set _saved_variable $variable

    if {$variable != ""} then {
      # Set a new trace.
      uplevel \#0 [list trace variable $variable uw [list $this _trace]]
    }
  }
}
