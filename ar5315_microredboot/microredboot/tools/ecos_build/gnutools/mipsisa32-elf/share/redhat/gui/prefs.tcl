# prefs.tcl - Preference handling.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# KNOWN BUGS:
# * When we move to the next tcl/itcl, rewrite to use namespaces and
#   possibly ensembles.

# Global state.
defarray PREFS_state {
  inhibit-event 0
  initialized 0
}

# This is called when a trace on some option fires.  It makes sure the
# relevant handlers get run.
proc PREFS_run_handlers {name1 name2 op} {
  upvar $name1 state
  set option [lindex $name2 0]

  global PREFS_state
  # Notify everybody else unless we've inhibited event generation.
  if {! $PREFS_state(inhibit-event) && $PREFS_state(ide_running)} then {
    ide_property set preference/$option $state([list $option value]) global
  }

  # Run local handlers.
  run_hooks PREFS_state([list $option handler]) $option \
    $state([list $option value])
}

# This is run when we see a property event.  It updates our internal
# state.
proc PREFS_handle_property_event {exists property value} {
  global PREFS_state

  # If it isn't a preference property, ignore it.
  if {! [string match preference/* $property]} then {
    return
  }
  # [string length preference/] == 11.
  set name [string range $property 11 end]

  if {$exists} then {
    incr PREFS_state(inhibit-event)
    set PREFS_state([list $name value]) $value
    incr PREFS_state(inhibit-event) -1
  } elseif {$PREFS_state(ide_running)} then {
    # It doesn't make sense to remove a property that mirrors some
    # preference.  So disallow by immediately redefining.  Use
    # initialize and not set because several clients are likely to run
    # this at once.
    ide_property initialize preference/$name \
      $PREFS_state([list $name value]) global
  }
}

# pref define NAME DEFAULT
# Define a new option
# NAME is the option name
# DEFAULT is the default value of the option
proc PREFS_cmd_define {name default} {
  global PREFS_state

  # If the option has already been defined, do nothing.
  if {[info exists PREFS_state([list $name value])]} then {
    return
  }

  if {$PREFS_state(ide_running)} then {
    # We only store the value in the database.
    ide_property initialize preference/$name $default global
    set default [ide_property get preference/$name]
  }

  # We set our internal state no matter what.  It is harmless if our
  # definition causes a property-set event.
  set PREFS_state([list $name value]) $default
  set PREFS_state([list $name handler]) {}

  # Set up a variable trace so that the handlers can be run.
  trace variable PREFS_state([list $name value]) w PREFS_run_handlers
}

# pref get NAME
# Return value of option NAME
proc PREFS_cmd_get {name} {
  global PREFS_state
  return $PREFS_state([list $name value])
}

# pref getd NAME
# Return value of option NAME
# or define it if necessary and return ""
proc PREFS_cmd_getd {name} {
  global PREFS_state
  PREFS_cmd_define $name ""
  return [pref get $name]
}

# pref varname NAME
# Return name of global variable that represents option NAME
# This is suitable for (eg) a -variable option on a radiobutton
proc PREFS_cmd_varname {name} {
  return PREFS_state([list $name value])
}

# pref set NAME VALUE
# Set the option NAME to VALUE
proc PREFS_cmd_set {name value} {
  global PREFS_state

  # For debugging purposes, make sure the preference has already been
  # defined.
  if {! [info exists PREFS_state([list $name value])]} then {
    error "attempt to set undefined preference $name"
  }

  set PREFS_state([list $name value]) $value
}

# pref setd NAME VALUE
# Set the option NAME to VALUE
# or define NAME and set the default to VALUE
proc PREFS_cmd_setd {name value} {
  global PREFS_state

  if {[info exists PREFS_state([list $name value])]} then {
    set PREFS_state([list $name value]) $value
  } else {
    PREFS_cmd_define $name $value
  }
}

# pref add_hook NAME HOOK
# Add a command to the hook that is run when the preference name NAME
# changes.  The command is run with the name of the changed option and
# the new value as arguments.
proc PREFS_cmd_add_hook {name hook} {
  add_hook PREFS_state([list $name handler]) $hook
}

# pref remove_hook NAME HOOK
# Remove a command from the per-preference hook.
proc PREFS_cmd_remove_hook {name hook} {
  remove_hook PREFS_state([list $name handler]) $hook
}

# pref init ?IDE_RUNNING?
# Initialize the preference module.  IDE_RUNNING is an optional
# boolean argument.  If 0, then the preference module will assume that
# it is not connected to the IDE backplane.  The default is based on
# the global variable IDE_ENABLED.
proc PREFS_cmd_init {{ide_running "unset"}} {
  global PREFS_state IDE_ENABLED

  if {! $PREFS_state(initialized)} then {

    if {$ide_running == "unset"} then {
      if {[info exists IDE_ENABLED]} then {
	set ide_running $IDE_ENABLED
      } else {
	set ide_running 0
      }
    }

    set PREFS_state(initialized) 1
    set PREFS_state(ide_running) $ide_running
    if {$ide_running} then {
      property add_hook "" PREFS_handle_property_event
    }
  }
}

# pref list
# Return a list of the names of all preferences defined by this
# application.
proc PREFS_cmd_list {} {
  global PREFS_state

  set list {}
  foreach item [array names PREFS_state] {
    if {[lindex $item 1] == "value"} then {
      lappend list [lindex $item 0]
    }
  }

  return $list
}

# The primary interface to all preference subcommands.
proc pref {dispatch args} {
  if {[info commands PREFS_cmd_$dispatch] == ""} then {
    error "unrecognized key \"$dispatch\""
  }

  eval PREFS_cmd_$dispatch $args
}
