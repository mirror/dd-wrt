# Interface between GDB and Insight.
# Copyright 1997, 1998, 1999, 2001, 2002 Red Hat, Inc.
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


# This variable is reserved for this module.  Ensure it is an array.
global gdbtk_state
set gdbtk_state(busyCount) 0

# *** DEPRECATED: Use GDBEventHandler::breakpoint instead.
# This is run when a breakpoint changes.  The arguments are the
# action, the breakpoint number, and the breakpoint info.
#define_hook gdb_breakpoint_change_hook

# *** DEPRECATED: Use GDBEventHandler::set_variable instead.
# This is run when a `set' command successfully completes in gdb.  The
# first argument is the gdb variable name (as a Tcl list).  The second
# argument is the new value.
#define_hook gdb_set_hook

# ------------------------------------------------------------
#  PROC:  gdbtk_tcl_set_variable - A "set" command was issued
#          in gdb to change an internal variable. Notify
#          gui.
# ------------------------------------------------------------
proc gdbtk_tcl_set_variable {var val} {
  set e [SetVariableEvent \#auto -variable $var -value $val]
  GDBEventHandler::dispatch $e
  delete object $e
}

####################################################################
#                                                                  #
#                        GUI STATE HOOKS                           #
#                                                                  #
####################################################################
# !!!!!   NOTE   !!!!!
# For debugging purposes, please put debug statements at the very
# beginning and ends of all GUI state hooks.

# *** DEPRECATED: Use GDBEventHandler::busy instead.
# GDB_BUSY_HOOK
#   This hook is used to register a callback when the UI should
#   be disabled because the debugger is either busy or talking
#   to the target.
#
#   All callbacks should disable ALL user input which could cause
#   any state changes in either the target or the debugger.
#define_hook gdb_busy_hook

# *** DEPRECATED: Use GDBEventHandler::idle instead.
# GDB_IDLE_HOOK
#   This hook is used to register a callback when the UI should
#   be enabled because the debugger is no longer busy.
#
#   All callbacks should enable user input. These callbacks
#   should also be as fast as possible to avoid any significant
#   time delays when enabling the UI.
define_hook gdb_idle_hook

# *** DEPRECATED: Use GDBEventHandler::update instead.
# GDB_UPDATE_HOOK
#   This hook is used to register a callback to update the widget
#   when debugger state has changed.
#define_hook gdb_update_hook

# GDB_NO_INFERIOR_HOOK
#   This hook is used to register a callback which should be invoked
#   whenever there is no inferior (either at startup time or when
#   an inferior is killed).
#
#   All callbacks should reset their windows to a known, "startup"
#   state.
define_hook gdb_no_inferior_hook

# GDB_DISPLAY_CHANGE_HOOK
# This is run when a display changes.  The arguments are the action,
# the breakpoint number, and (optionally) the value.
define_hook gdb_display_change_hook

# GDB_TRACE_FIND_HOOK
#    This hook is run by the trace find command.  It is used to switch
#    from control to browse mode when the user runs tfind commands...
#
define_hook gdb_trace_find_hook

# ------------------------------------------------------------------
#  gdbtk_tcl_preloop - This function is called after gdb is initialized
#  but before the mainloop is started.  It sets the app name, and
#  opens the first source window.
# ------------------------------------------------------------------

proc gdbtk_tcl_preloop { } {
  global gdb_exe_name gdb_current_directory

  set_baud

  tk appname gdbtk
  # If there was an error loading an executible specified on the command line
  # then we will have called pre_add_symbol, which would set us to busy,
  # but not the corresponding post_add_symbol.  Do this here just in case...
  after idle gdbtk_idle 
  ManagedWin::startup

  if {$gdb_exe_name != ""} {
    # At startup, file_changed_hook is called too late for us, so we
    # must notice the initial session by hand.  If the arguments exist
    # -- if the user used `gdb --args' -- then we want the new
    # arguments and pwd to override what is set in the session.
    set current_args [gdb_get_inferior_args]
    set current_dir $gdb_current_directory
    Session::notice_file_change
    if {[string length $current_args] > 0} {
      gdb_set_inferior_args $current_args
      gdb_cmd "cd $current_dir"
    }
  }
  
  gdbtk_update
}


# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_busy - Dispatch a busy event
#
#         Use this procedure from within GUI code to indicate that
#         the debugger is busy, either running the inferior or
#         talking to the target.
# ------------------------------------------------------------------
proc gdbtk_busy {} {

  set e [BusyEvent \#auto]
  GDBEventHandler::dispatch $e
  delete object $e

  # Force the screen to update
  update
}

# ------------------------------------------------------------------
#   PROCEDURE:  gdbtk_update - run all update hooks
#
#          Use this procedure to force all widgets to update
#          themselves. This hook is usually run after command
#          that could change target state.
# ------------------------------------------------------------------
proc gdbtk_update {} {

  set e [UpdateEvent \#auto]
  GDBEventHandler::dispatch $e
  delete object $e
  
  # Force the screen to update
  update
}

# ------------------------------------------------------------------
#   PROCEDURE:  gdbtk_update_safe - run all update hooks in a safe way
#
#          Use this procedure to force all widgets to update
#          themselves. This hook is usually run after command
#          that could change target state.
#          Like gdbtk_update but safe to be used in "after idle"
#          which is used in update hooks.
# ------------------------------------------------------------------
proc gdbtk_update_safe {} {
  global gdb_running

  # Fencepost: Do not update if we are running the target
  # We get here because script commands may have changed memory or
  # registers and "after idle" events registered as a consequence
  # If we try to update while the target is running we are doomed.
  if {!$gdb_running} {
    gdbtk_update
  }
}

# ------------------------------------------------------------------
#   PROCEDURE: gdbtk_idle - dispatch IdleEvent
#
#          Use this procedure to free the UI for more user input.
#          This should only be run AFTER all communication with
#          the target has halted, otherwise the risk of two (or
#          more) widgets talking to the target arises.
# ------------------------------------------------------------------
proc gdbtk_idle {} {
  global gdb_running

  # Put the unfiltered hook back in place, just in case
  # somebody swapped it out, and then died before they
  # could replace it.
  gdb_restore_fputs

  set err [catch {run_hooks gdb_idle_hook}]
  if {$err} {
    dbug E "Error running gdb_idle_hook: $::errorInfo"
  }

  set e [IdleEvent \#auto]
  GDBEventHandler::dispatch $e
  delete object $e

  if {!$gdb_running} {
    set err [catch {run_hooks gdb_no_inferior_hook} txt]
    if {$err} { 
      dbug E "no_inferior_hook error: $txt" 
    }
  }

  # Force the screen to update
  update
}

define_hook download_progress_hook

# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_quit_check - Ask if the user really wants to quit.
# ------------------------------------------------------------------
proc gdbtk_quit_check {} {
  global gdb_downloading gdb_running gdb_exe_name
  
  if {$gdb_downloading} {
    set msg "Downloading to target,\n really close the debugger?"
    if {![gdbtk_tcl_query $msg no]} {
      return 0
    }
  } elseif {$gdb_running} {
    # While we are running the inferior, gdb_cmd is fenceposted and
    # returns immediately. Therefore, we need to ask here. Do we need
    # to stop the target, too?
    set msg "A debugging session is active.\n"
    append msg "Do you still want to close the debugger?"
    if {![gdbtk_tcl_query $msg no]} {
      return 0
    }
  }
  
  return 1
}

# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_quit - Quit the debugger
#         Call this procedure anywhere the user can request to quit.
#         This procedure will ask all the right questions before
#         exiting.
# ------------------------------------------------------------------
proc gdbtk_quit {} {
  if {[gdbtk_quit_check]} {
    gdbtk_force_quit
  }
}

# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_force_quit - Quit the debugger immediately
# ------------------------------------------------------------------
proc gdbtk_force_quit {} {
  # If we don't delete source windows, GDB hooks will
  # try to update them as we exit
  foreach win [ManagedWin::find SrcWin] {
    delete object $win
  }
  # Calling gdb_force_quit is probably not necessary here
  # because it should have been called when the source window(s)
  # were deleted, but just in case...
  gdb_force_quit
}

# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_cleanup - called by GDB immediately
#         before exiting.  Last chance to cleanup!
# ------------------------------------------------------------------
proc gdbtk_cleanup {} {
  global gdb_exe_name

  # Save the session
  if {$gdb_exe_name != ""} {
    Session::save
  }

  # This is a sign that it is too late to be doing updates, etc...
  set ::gdb_shutting_down 1

  # Shutdown the window manager and save all preferences
  # This way a "quit" in the console window will cause
  # preferences to be saved.
  ManagedWin::shutdown
  pref_save
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_query -
# ------------------------------------------------------------------
proc gdbtk_tcl_query {message {default yes}} {
  global gdb_checking_for_exit gdbtk_state tcl_platform

  # FIXME We really want a Help button here.  But Tk's brain-damaged
  # modal dialogs won't really allow it.  Should have async dialog
  # here.

  set title "GDB"
  set modal "task"

  # If we are checking whether to exit gdb, we want a system modal
  # box.  Otherwise it may be hidden by some other program, and the
  # user will have no idea what is going on.
  if {[info exists gdb_checking_for_exit] && $gdb_checking_for_exit} {
    set modal "system"
  }
  
  if {$tcl_platform(platform) == "windows"} {
    # On Windows, we want to only ask each question once.
    # If we're already asking the question, just wait for the answer
    # to come back.
    set ans [list answer $message]
    set pending [list pending $message]

    if {[info exists gdbtk_state($pending)]} {
      incr gdbtk_state($pending)
    } else {
      set gdbtk_state($pending) 1
      set gdbtk_state($ans) {}

      ide_messageBox [list set gdbtk_state($ans)] -icon warning \
	-default $default -message $message -title $title \
	-type yesno -modal $modal -parent .
    }

    vwait gdbtk_state($ans)
    set r $gdbtk_state($ans)
    if {[incr gdbtk_state($pending) -1] == 0} {
      # Last call waiting for this answer, so clear it.
      unset gdbtk_state($pending)
      unset gdbtk_state($ans)
    }
  } else {
    # On Unix, apparently it doesn't matter how many times we ask a
    # question.
    set r [tk_messageBox -icon warning -default $default \
	     -message $message -title $title \
	     -type yesno -parent .]
  }

  update idletasks
  return [expr {$r == "yes"}]
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_warning -
# ------------------------------------------------------------------
proc gdbtk_tcl_warning {message} {
  debug "$message"

# ADD a warning message here if the gui must NOT display it
# add the message at the beginning of the switch followed by - 

  switch -regexp $message {
        "Unable to find dynamic linker breakpoint function.*" {return}
        default {show_warning $message}
       }
}

# ------------------------------------------------------------------
# PROC: show_warning -
# ------------------------------------------------------------------
proc show_warning {message} {
  global tcl_platform

  # FIXME We really want a Help button here.  But Tk's brain-damaged
  # modal dialogs won't really allow it.  Should have async dialog
  # here.
  set title "GDB"
  set modal "task"

# On Windows, we use ide_messageBox which runs the Win32 MessageBox function
# in another thread.  This permits a program which handles IDE requests from
# other programs to not return from the request until the MessageBox completes.
# This is not possible without using another thread, since the MessageBox
# function call will be running its own event loop, and will be higher on the
# stack than the IDE request.
#
# On Unix tk_messageBox runs in the regular Tk event loop, so
# another thread is not required.

 
  if {$tcl_platform(platform) == "windows"} {
      ide_messageBox [list set r] -icon warning \
        -default ok -message $message -title $title \
        -type ok -modal $modal -parent .
  } else {
    set r [tk_messageBox -icon warning -default ok \
             -message $message -title $title \
             -type ok -parent .]
  }
} 

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_ignorable_warning -
# ------------------------------------------------------------------
proc gdbtk_tcl_ignorable_warning {class message} {
  catch {ManagedWin::open WarningDlg -center -transient \
	   -message [list $message] -ignorable $class}
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_fputs -
# ------------------------------------------------------------------
proc gdbtk_tcl_fputs {message} {
  global gdbtk_state
  # Restore the fputs hook, in case anyone forgot to put it back...
  gdb_restore_fputs

  if {$gdbtk_state(console) != ""} {
    $gdbtk_state(console) insert $message
  }
}

# ------------------------------------------------------------------
# PROC: echo -
# ------------------------------------------------------------------
proc echo {args} {
  gdbtk_tcl_fputs [concat $args]\n
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_fputs_error - write an error message
# ------------------------------------------------------------------
proc gdbtk_tcl_fputs_error {message} {
  if {$::gdbtk_state(console) != ""} {
    $::gdbtk_state(console) insert $message err_tag
    update
  }
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_fputs_log - write a log message
# ------------------------------------------------------------------
proc gdbtk_tcl_fputs_log {message} {
  if {$::gdbtk_state(console) != ""} {
    $::gdbtk_state(console) insert $message log_tag
    update
  }
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_fputs_target - write target output
# ------------------------------------------------------------------
proc gdbtk_tcl_fputs_target {message} {
  if {$::gdbtk_state(console) != ""} {
    $::gdbtk_state(console) insert $message target_tag
    update
  }
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_flush -
# ------------------------------------------------------------------
proc gdbtk_tcl_flush {} {
  debug [info level 0]
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_start_variable_annotation -
# ------------------------------------------------------------------
proc gdbtk_tcl_start_variable_annotation {valaddr ref_type stor_cl
					  cum_expr field type_cast} {
  debug [info level 0]
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_end_variable_annotation -
# ------------------------------------------------------------------
proc gdbtk_tcl_end_variable_annotation {} {
  debug [info level 0]
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_breakpoint - A breakpoint was changed -- notify
#                               gui.
# ------------------------------------------------------------------
proc gdbtk_tcl_breakpoint {action bpnum} {
#  debug "BREAKPOINT: $action $bpnum"
  set e [BreakpointEvent \#auto -action $action -number $bpnum]
  GDBEventHandler::dispatch $e
  delete object $e
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_tracepoint - A tracepoint was changed -- notify
#                               gui.
# ------------------------------------------------------------------
proc gdbtk_tcl_tracepoint {action tpnum} {
#  debug "TRACEPOINT: $action $tpnum"
  set e [TracepointEvent \#auto -action $action -number $tpnum]
  GDBEventHandler::dispatch $e
  delete object $e
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_trace_find_hook -
# ------------------------------------------------------------------
proc gdbtk_tcl_trace_find_hook {arg from_tty} {
#  debug "$arg $from_tty"
  run_hooks gdb_trace_find_hook $arg $from_tty
}

################################################################
#
# Handle `readline' interface.
#

# Run a command that is known to use the "readline" interface.  We set
# up the appropriate buffer, and then run the actual command via
# gdb_cmd.  Calls into the "readline" callbacks just return values
# from our list.

# ------------------------------------------------------------------
# PROC: gdb_run_readline_command -
# ------------------------------------------------------------------
proc gdb_run_readline_command {command args} {
  global gdbtk_state
  debug "$command $args"
  set gdbtk_state(readlineArgs) $args
  set gdbtk_state(readlineShowUser) 1
  gdb_cmd $command
}

# ------------------------------------------------------------------
# PROC: gdb_run_readline_command_no_output
# Run a readline command, but don't show the commands to the user.
# ------------------------------------------------------------------
proc gdb_run_readline_command_no_output {command args} {
  global gdbtk_state
  debug "$command $args"
  set gdbtk_state(readlineArgs) $args
  set gdbtk_state(readlineShowUser) 0
  gdb_cmd $command
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_readline_begin -
# ------------------------------------------------------------------
proc gdbtk_tcl_readline_begin {message} {
  global gdbtk_state
#  debug
  set gdbtk_state(readline) 0
  if {$gdbtk_state(console) != "" && $gdbtk_state(readlineShowUser)} {
    $gdbtk_state(console) insert $message
  }
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_readline -
# ------------------------------------------------------------------
proc gdbtk_tcl_readline {prompt} {
  global gdbtk_state
#  debug "prompt=$prompt"
  if {[info exists gdbtk_state(readlineArgs)]} {
    # Not interactive, so pop the list, and print element.
    set cmd [lvarpop gdbtk_state(readlineArgs)]
    if {$gdbtk_state(console) != "" && $gdbtk_state(readlineShowUser)} {
      $gdbtk_state(console) insert $cmd
    }
  } else {
    # Interactive.
#    debug "interactive"
    set gdbtk_state(readline) 1
    $gdbtk_state(console) activate $prompt
    vwait gdbtk_state(readline_response)
    set cmd $gdbtk_state(readline_response)
#    debug "got response: $cmd"
    unset gdbtk_state(readline_response)
    set gdbtk_state(readline) 0
  }
  return $cmd
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_readline_end -
# ------------------------------------------------------------------
proc gdbtk_tcl_readline_end {} {
  global gdbtk_state
#  debug
  catch {unset gdbtk_state(readlineArgs)}
  catch {unset gdbtk_state(readlineActive)}
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_busy - this is called immediately before gdb 
#    executes a command.
#
# ------------------------------------------------------------------
proc gdbtk_tcl_busy {} {
  global gdbtk_state
  if {[incr gdbtk_state(busyCount)] == 1} {
    gdbtk_busy
  }
}

################################################################
#
# 
#

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_idle - this is called immediately after gdb 
#    executes a command.
# ------------------------------------------------------------------
proc gdbtk_tcl_idle {} {
  global gdbtk_state
  if {$gdbtk_state(busyCount) > 0
      && [incr gdbtk_state(busyCount) -1] == 0} {
    gdbtk_update
    gdbtk_idle
  }
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_tstart -
# ------------------------------------------------------------------
proc gdbtk_tcl_tstart {} {
  set srcwin [lindex [manage find src] 0]
  $srcwin.toolbar do_tstop 0
  
}

# ------------------------------------------------------------------
# PROC: gdbtk_tcl_tstop -
# ------------------------------------------------------------------
proc gdbtk_tcl_tstop {} {
  set srcwin [lindex [manage find src] 0]
  $srcwin.toolbar do_tstop 0
  
}


# ------------------------------------------------------------------
# PROC: gdbtk_tcl_display -
#
# A display changed.  ACTION is `enable', `disable', `delete',
# `create', or `update'.  VALUE is only meaningful in the `update'
# case.
# ------------------------------------------------------------------
proc gdbtk_tcl_display {action number {value {}}} {
  # Handle create explicitly.
  if {$action == "create"} {
    manage create_if_never data
  }
  run_hooks gdb_display_change_hook $action $number $value
}

# ------------------------------------------------------------------
#  PROCEDURE: gdbtk_register_changed
#         This hook is called from value_assign to inform us that
#         the user has changed the contents of a register.
# ------------------------------------------------------------------
proc gdbtk_register_changed {} {
  after idle gdbtk_update_safe
}

# ------------------------------------------------------------------
#  PROCEDURE: gdbtk_memory_changed
#         This hook is called from value_assign to inform us that
#         the user has changed the contents of memory (including
#         the program's variables).
# ------------------------------------------------------------------
proc gdbtk_memory_changed {} {
  after idle gdbtk_update_safe
}

####################################################################
#                                                                  #
#                           FILE HOOKS                             #
#                                                                  #
#    There are a number of hooks that are installed in gdb to      #
#    aid with file-like commands (selecting an exectuable and      #
#    loading symbols):                                             #
#         - exec_file_display_hook                                 #
#            Called in exec_file_command. The tcl hook is          #
#            "gdbtk_tcl_exec_file_display"                         #
#         - file_changed_hook                                      #
#            Called in file_command. The tcl hook is               #
#            "gdbtk_tcl_file_changed"                              #
#         - pre_add_symbol_hook                                    #
#            Called in symbol_file_add before loading. The tcl     #
#            hook is "gdbtk_tcl_pre_add_symbol"                    #
#         - post_add_symbol_hook                                   #
#            Called in symbol_file_add when finished loading       #
#            a symbol file. The tcl hook is                        #
#            "gdbtk_tcl_post_add_symbol"                           #
#                                                                  #
#  Together, these hooks should give the gui enough information    #
#  to cover the two most common uses of file commands:             #
#  1. executable with symbols                                      #
#  2. separate executable and symbol file(s)                       #
#                                                                  #
####################################################################
define_hook file_changed_hook

# ------------------------------------------------------------------
#  PROCEDURE:  gdbtk_tcl_pre_add_symbol
#         This hook is called before any symbol files
#         are loaded so that we can inform the user.
# ------------------------------------------------------------------
proc gdbtk_tcl_pre_add_symbol {file} {

  gdbtk_busy

  # Display some feedback to the user
  set srcs [ManagedWin::find SrcWin]
  foreach w $srcs {
    $w set_status "Reading symbols from $file..."
  }
  update idletasks
}

# ------------------------------------------------------------------
#   PROCEDURE: gdbtk_tcl_post_add_symbol
#          This hook is called after we finish reading a symbol
#          file, so the source windows' combo boxes need filling.
# ------------------------------------------------------------------
proc gdbtk_tcl_post_add_symbol {} {

  set srcs [ManagedWin::find SrcWin]
  foreach w $srcs {
    $w fillNameCB
  }
  gdbtk_idle
}

# ------------------------------------------------------------------
#  PROCEDURE: gdbtk_tcl_file_changed
#         This hook is called whenever the exec file changes.
#         This is called AFTER symbol reading, so it is
#         ok to point to main when we get called.
# ------------------------------------------------------------------
proc gdbtk_tcl_file_changed {filename} {

  if {$filename == ""} {
    gdb_clear_file
    catch {run_hooks gdb_clear_file_hook}
    set ::gdb_exe_name ""
    set ::gdb_loaded 0
    set ::gdb_running 0
    gdbtk_update
  } else {
    SrcWin::point_to_main
    run_hooks file_changed_hook
  }
}

# ------------------------------------------------------------------
#  PROCEDURE: gdbtk_tcl_exec_file_display 
#         This hook is called from exec_file_command. It's purpose
#         is to setup the gui for a new file. Note that we cannot
#         look for main, since this hook is called BEFORE we
#         read symbols. If the user used the "file" command,
#         gdbtk_tcl_file_changed will set the source window to
#         look at main. If the user used "exec-file" and "add-symbol"
#         commands, then we cannot look for main.
# ------------------------------------------------------------------
proc gdbtk_tcl_exec_file_display {filename} {
  global gdb_exe_changed

  # DO NOT CALL set_exe here! 

  # Clear out the GUI, don't do it if filename is "" so that
  # you avoid distracting flashes in the source window.

  if {$filename != ""} {
    gdbtk_clear_file
  }

  # set_exe calls file command with the filename in
  # quotes, so we need to strip them here.
  # We need to make sure that we turn filename into
  # an absolute path or sessions won't work.
  if {[file tail $filename] == $filename} {
    # want full pathname
    set filename [file join $::gdb_current_directory $filename]
  }
  set_exe_name $filename
  set gdb_exe_changed 0

  SrcWin::point_to_main
}

# ------------------------------------------------------------------
#  PROCEDURE: gdbtk_locate_main 
#         This proc tries to locate a suitable main function from
#         a list of names defined in the gdb/main_names preference; 
#         returns the linespec (see below) if found, or a null string
#         if not.
#
#  The return linespec looks like this:
#  0: basename of the file
#  1: function name
#  2: full filename
#  3: source line number
#  4: address
#  5: current PC - which will often be the same as address, but not when
#  we are browsing, or walking the stack.
#  6: shared library name if the pc is in a shared lib
#
# ------------------------------------------------------------------
proc gdbtk_locate_main {} {
  set result {}
  set main_names [pref get gdb/main_names]
  debug "Searching $main_names"

  foreach main $main_names {
    if {![catch {gdb_search functions $main -static 1}] \
        && ![catch {gdb_loc $main} linespec]} {
      set result $linespec
      break
    }
  }
  if {$result == {} 
      && ![catch gdb_entry_point entry_point]
      && ![catch {gdb_loc "*$entry_point"} linespec]} {
    set result $linespec
  }
  
  # need to see if result is valid
  lassign $result file func ffile line addr rest
  if {$addr == 0x0 && $func == {}} { set result {} }

  return $result
}

##############################################
#  The rest of this file is an assortment of Tcl wrappers
#  for various bits of gdb functionality.
#
#############################################

# ------------------------------------------------------------------
# PROC: set_exe_name - Update the executable name
# ------------------------------------------------------------------
proc set_exe_name {exe} {
  global gdb_exe_name gdb_exe_changed
  #debug "exe=$exe  gdb_exe_name=$gdb_exe_name"

  set gdb_exe_name $exe
  set gdb_exe_changed 1    
}


# ------------------------------------------------------------------
# PROC: set_exe -
# ------------------------------------------------------------------ 
proc set_exe {} {
  global gdb_exe_name gdb_exe_changed gdb_target_changed gdb_loaded file_done
#  debug "gdb_exe_changed=$gdb_exe_changed gdb_exe_name=$gdb_exe_name"
  if {$gdb_exe_changed} {
    set gdb_exe_changed 0
    if {$gdb_exe_name == ""} { return }
    set err [catch {gdb_cmd "file '$gdb_exe_name'" 1} msg]
    if {$err} {
      dbug E "$msg"
      set l [split $msg :]
      set errtxt [join [lrange $l 1 end] :]
      set msg "Error loading \"$gdb_exe_name\":\n"
      append msg $errtxt
      tk_messageBox -title "Error" -message $msg -icon error \
	-type ok
      set gdb_exe_name {}
      set file_done 0
      return
    } elseif {[string match {*no debugging symbols found*} $msg]} {
      tk_messageBox -icon error -default ok \
	-title "GDB" -type ok \
	-message "This executable has no debugging information."
    }

    # force new target command
    set gdb_target_changed 1
    set gdb_loaded 0
    set file_done 1
  }
}

# ------------------------------------------------------------------
#  _open_file - open a file dialog to select a file for debugging.
#  If filename is not "", then open this file.
# ------------------------------------------------------------------

proc _open_file {{file ""}} {
  global gdb_running gdb_downloading tcl_platform
  
  if {$gdb_running || $gdb_downloading} {
    # We are already running/downloading something..
    if {$gdb_running} {
      set msg "A debugging session is active.\nAbort session and load new file?"
    } else {
      set msg "A download is in progress.\nAbort download and load new file?"
    }
    if {![gdbtk_tcl_query $msg no]} {
      return 0
    }
  }

  if {[string compare $file ""] == 0} {
    set curFocus [focus]
    
    # Make sure that this is really a modal dialog...
    # FIXME: Add a disable_all to ide_grab_support.
    
    ide_grab_support disable_except {}
    
    set file [tk_getOpenFile -parent . -title "Load New Executable"]
  
    ide_grab_support enable_all
    
    # If no one had the focus before, leave it that way (since I
    # am not sure how this could happen...  Also, the vwait in 
    # tk_getOpenFile could have allowed the curFocus window to actually
    # be destroyed, so make sure it is still around.
    
    if {$curFocus != "" && [winfo exists $curFocus]} {
      raise [winfo toplevel $curFocus]
      focus $curFocus
    }
  } elseif {![file exists $file]} {
    tk_messageBox -message "File \"$file\" does not exist"
    return 0
  }
    

  if {$file == ""} {
    return 0
  }
  # Add the base dir for this file to the source search path.
  set root [file dirname $file]
  if {$tcl_platform(platform) == "windows"} {
    set root [ide_cygwin_path to_posix $root]
    set file [ide_cygwin_path to_posix $file]
  }
  
  catch {gdb_cmd "cd $root"}

  # Clear out gdb's internal state, so that it will allow us
  # (the gui) to ask the user questions.
  gdb_clear_file

  # The gui needs to set this...
  set_exe_name $file
  
  # set_exe needs to be called anywhere the gui does a file_command...
  if {[set_exe] == "cancel"} {
    gdbtk_update
    gdbtk_idle
    return 0
  }

  return 1
}

# ------------------------------------------------------------------
#  _close_file - close the current executable and prepare for
#    another executable.
# ------------------------------------------------------------------
proc _close_file {} {

  # If there is already an inferior, ask him if he wants to close
  # the file. If there is already an exec file loaded (and not run)
  # also ask, but don't ask twice.
  set okay 1
  if {[gdb_target_has_execution]} {
    set okay [gdbtk_tcl_query "Program is already running.\nClose file anyway?"]
  } elseif {$::gdb_exe_name != ""} {
    set okay [gdbtk_tcl_query "Program already loaded.\nClose file anyway?"]
  } else {
    # No exec file yet
    return
  }

  if {$okay} {
    Session::save
    gdb_clear_file
    gdbtk_tcl_file_changed ""

    # Print out a little message to all console windows
    foreach cw [ManagedWin::find Console] {
      $cw insert "No executable file now.\n"
    }
  }
}

# ------------------------------------------------------------------
# PROC: set_target_name - Update the target name.  
#
# This function will prompt for a new target and update
# all variables.
#
# If $prompt is 0 it will just update gdb_target_cmd from gdb_target.
#
# RETURN:
#     1 if successful, 
#     0 if the not (the user canceled the target selection dialog)
# ------------------------------------------------------------------
proc set_target_name {{prompt 1}} {
  global gdb_target_name gdb_target_changed gdb_exe_changed
  global gdb_target_cmd gdb_pretty_name
#  debug
  set cmd_tmp $gdb_target_cmd
  set name_tmp $gdb_target_name

#  debug "gdb_target_name=$gdb_target_name; name_tmp=$name_tmp"
  if {$prompt} {
    set win [ManagedWin::open TargetSelection -exportcancel 1 -center \
	       -transient]
    # need to call update here so the target selection dialog can take over
    update idletasks
  }

#  debug "gdb_target_name=$gdb_target_name"
  if {$gdb_target_name == "CANCEL"} {
    set gdb_target_cmd $cmd_tmp
    set gdb_target_name $name_tmp
    return 0
  }
  set target $gdb_target_name
  set targ [TargetSelection::getname $target cmd]
  set gdb_target_cmd $cmd_tmp
  set gdb_pretty_name [TargetSelection::getname $target pretty-name]

#  debug "target=$target pretty_name=$gdb_pretty_name"
  set targ_opts ""
  switch -regexp -- $gdb_target_name {
    sim|ice {
      set targ $gdb_target_name
      set targ_opts [pref getd gdb/load/${gdb_target_name}-opts]
    }
    default {
      set port [pref getd gdb/load/$target-port]
      if {$port == ""} {
	set port [pref get gdb/load/default-port]
      }
      set portnum [pref getd gdb/load/$target-portname]
      if {$portnum == ""} {
	set portnum [pref get gdb/load/default-portname]
      }
      set hostname [pref getd gdb/load/$target-hostname]
      if {$hostname == ""} {
	set hostname [pref getd gdb/load/default-hostname]
      }
      # replace "com1" with the real port name
      set targ [lrep $targ "com1" $port]
      # replace "tcpX" with hostname:portnum
      set targ [lrep $targ "tcpX" ${hostname}:${portnum}]
      # replace "ethX" with hostname
      set targ [lrep $targ "ethX" e=${hostname}]
    }
  }
  
#  debug "targ=$targ gdb_target_cmd=$gdb_target_cmd"
  if {$gdb_target_cmd != $targ || $gdb_target_changed} {
    set gdb_target_changed 1
    set gdb_target_cmd "$targ $targ_opts"
  }
  return 1
}

# ------------------------------------------------------------------
# PROC: set_target - Change the target
# ------------------------------------------------------------------
proc set_target {} {
  global gdb_target_cmd gdb_target_changed gdb_pretty_name gdb_target_name
#  debug "gdb_target_changed=$gdb_target_changed gdb_target_cmd=\"$gdb_target_cmd\""
#  debug "gdb_target_name=$gdb_target_name"
  if {$gdb_target_cmd == "" && ![TargetSelection::native_debugging]} {
    if {$gdb_target_name == ""} {
      set prompt 1

      # get the default
      #set gdb_target_name [pref getd gdb/load/target]
    } else {
      set prompt 0
    }
    if {![set_target_name $prompt]} {
      set gdb_target_name ""
      return CANCELED
    }
  }
  
  if {$gdb_target_changed} {
    set srcWin [lindex [ManagedWin::find SrcWin] 0]

    $srcWin set_status "Trying to communicate with target $gdb_pretty_name" 1
    update
    catch {gdb_cmd "detach"}
    debug "CONNECTING TO TARGET: $gdb_target_cmd"
    set err [catch {gdb_immediate "target $gdb_target_cmd"} msg ]
    $srcWin set_status

    if {$err} {
      if {[string first "Program not killed" $msg] != -1} {
	return CANCELED
      }
      update
      set dialog_title "GDB"
      set debugger_name "GDB"
      tk_messageBox -icon error -title $dialog_title -type ok \
	-message "$msg\n\n$debugger_name cannot connect to the target board\
using [lindex $gdb_target_cmd 1].\nVerify that the board is securely connected and, if\
necessary,\nmodify the port setting with the debugger preferences."
      return ERROR
    }
    
    if {![catch {pref get gdb/load/$gdb_target_name-after_attaching} aa] && $aa != ""} {
      if {[catch {gdb_cmd $aa} err]} {
	catch {[ManagedWin::find Console] insert $err err_tag}
      }
    }
    set gdb_target_changed 0
    return TARGET_CHANGED
  }
  return TARGET_UNCHANGED
}

# ------------------------------------------------------------------
# PROC: run_executable -
#
# This procedure is used to run an executable.  It is called when the 
# run button is used.
# ------------------------------------------------------------------
proc run_executable { {auto_start 1} } {
  global gdb_loaded gdb_downloading gdb_target_name
  global gdb_exe_changed gdb_target_changed gdb_program_has_run
  global gdb_running gdb_exe_name tcl_platform

#  debug "auto_start=$auto_start gdb_target_name=$gdb_target_name"

  set gdb_running_saved $gdb_running
  set gdb_running 0

  # No executable was specified.  Prompt the user for one.
  if {$gdb_exe_name == ""} {
    if {[_open_file]} {
      run_executable $auto_start
      return
    } else {
      # The user canceled the load of a new executable.
      return
    }
  }

  if {$gdb_downloading} { return }
  if {[pref get gdb/control_target]} {
    # Breakpoint mode
    set_exe

    # Attach
    if {$gdb_target_name == "" || [pref get gdb/src/run_attach]} {
      if {[gdbtk_attach_remote] == "ATTACH_CANCELED"} {
	return
      }
    }

    # Download
    if {[pref get gdb/src/run_load] && $gdb_target_name != "exec"} {
      debug "Downloading..."
      set gdb_loaded 0
      
      # if the app has not been downloaded or the app has already
      # started, we need to redownload before running
      if {!$gdb_loaded} {
	if {[Download::download_it]} {
	  # user cancelled the command
#	  debug "user cancelled the command $gdb_running"
	  set gdb_loaded 0
	  gdbtk_update
	  gdbtk_idle
	}
	if {!$gdb_loaded} {
	  # The user cancelled the download after it started
#	  debug "User cancelled the download after it started $gdb_running"
	  gdbtk_update
	  gdbtk_idle
	  return
	}
      }
    }

    # _Now_ set/clear breakpoints
    if {[pref get gdb/load/exit] && ![TargetSelection::native_debugging]} {
      debug "Setting new BP at exit"
      catch {gdb_cmd "clear exit"}
      catch {gdb_cmd "break exit"}
    }
      
    if {[pref get gdb/load/main]} {
      set main "main"
      if {[set linespec [gdbtk_locate_main]] != ""} {
        set main [lindex $linespec 1]
      }
      debug "Setting new BP at $main"
      catch {gdb_cmd "clear $main"}
      catch {gdb_cmd "break $main"}
    }

    # set BP at user-specified function
    if {[pref get gdb/load/bp_at_func]} {
      foreach bp [pref get gdb/load/bp_func] {
	debug "Setting BP at $bp"
	catch {gdb_cmd "clear $bp"}
	catch {gdb_cmd "break $bp"}
      }
    }

    # This is a hack.  If the target is "sim" the opts are appended
    # to the target command. Otherwise they are assumed to be command line
    # args.  What about simulators that accept command line args?
    if {$gdb_target_name != "sim"} {
      # set args
      set gdb_args [pref getd gdb/load/$gdb_target_name-opts]
      if { $gdb_args != ""} {
	debug "set args $gdb_args"
	gdb_set_inferior_args $gdb_args
      }
    }

    # If the user requested it, start an xterm for use as the
    # inferior's tty.
    if {$tcl_platform(platform) != "windows"
	&& [pref getd gdb/process/xtermtty] == "yes"} {
      tty::create
    }

    # 
    # Run

    if {$auto_start} {
      if {[pref get gdb/src/run_run]} {
	debug "Runnning target..."
	set run run
      } else {
	debug "Continuing target..."
	set run cont
      }
      if {$gdb_target_name == "exec"} {
	set run run
      }
      if {[catch {gdb_immediate $run} msg]} {
	dbug W "msg=$msg"
	gdbtk_idle
	if {[string match "*help target*" $msg]} {
	  set_target_name
	  run_executable $auto_start
	  return
	}
	if {[string match "No executable*" $msg]} {
	  # No executable was specified.  Prompt the user for one.
	  if {[_open_file]} {
	    run_executable $auto_start
	  } else {
	    debug "CANCELLED"
	  }
	  return
	}
	set gdb_running $gdb_running_saved
      } else {
	debug RUNNING
	set gdb_running 1
      }
    } else {
      SrcWin::point_to_main
    }

    gdbtk_update
    gdbtk_idle
  } elseif {[pref get gdb/mode]} {
    # tracepoint -- need to tstart
    set gdb_running 1
    tstart
  }
  return
}

# ------------------------------------------------------------------
#  PROC: gdbtk_attach_remote - attach to the target
#        This proc returns the following status messages:
#
#        ATTACH_ERROR: An error occurred connecting to target.
#        ATTACH_CANCELED: The attach was canceled.
#        ATTACH_TARGET_CHANGED: Successfully attached, target changed.
#        ATTACH_TARGET_UNCHANGED: Successfully attached, target unchanged.
#        UNKNOWN: An unknown error occurred.
# ------------------------------------------------------------------
proc gdbtk_attach_remote {} {
  global gdb_loaded

  debug "Attaching...."
  set r UNKNOWN
  while {1} {

    switch [set_target] {

      ERROR {
	# target command failed, ask for a new target name
	if {![set_target_name]} {
	  # canceled again
	  set r ATTACH_ERROR
	  break
	}
      }

      TARGET_CHANGED {
	# success -- target changed
	set gdb_loaded 0
	set r ATTACH_TARGET_CHANGED
	break
      }

      CANCELED {
	# command cancelled by user
	set r ATTACH_CANCELED
	break
      }

      TARGET_UNCHANGED {
	# success -- target NOT changed (i.e., rerun)
	set r ATTACH_TARGET_UNCHANGED
	break
      }
    }
  }

#  debug "Attach returning: \"$r\""
  return $r
}

# ------------------------------------------------------------------
# PROC:  gdbtk_connect: connect to a remote target 
#                      in asynch mode if async is 1
# ------------------------------------------------------------------
proc gdbtk_connect {{async 0}} {
  global file_done

  debug "async=$async"

  gdbtk_busy

  set result [gdbtk_attach_remote]
  switch $result {
    ATTACH_ERROR {
      set successful 0
    }

    ATTACH_TARGET_CHANGED {
	if {[pref get gdb/load/check] && $file_done} {
	  set err [catch {gdb_cmd "compare-sections"} errTxt]
	  if {$err} {
	    set successful 0
	    tk_messageBox -title "Error" -message $errTxt \
	      -icon error -type ok
	    break
	  }
	}

	tk_messageBox -title "GDB" -message "Successfully connected" \
	  -icon info -type ok
	set successful 1
    }

    ATTACH_CANCELED {
	tk_messageBox -title "GDB" -message "Connection Canceled" -icon info \
	  -type ok
	set successful 0
    }

    ATTACH_TARGET_UNCHANGED {
	tk_messageBox -title "GDB" -message "Successfully connected" \
	  -icon info -type ok
	set successful 1
    }

    default {
	dbug E "Unhandled response from gdbtk_attach_remote: \"$result\""
	set successful 0
    }
  }

  gdbtk_idle

  # Whenever we attach, we need to do an update
  if {$successful} {
    gdbtk_attached
  }
  return $successful
}

# ------------------------------------------------------------------
#  PROC: gdbtk_step - step the target
# ------------------------------------------------------------------
proc gdbtk_step {} {
  catch {gdb_immediate step}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_next
# ------------------------------------------------------------------
proc gdbtk_next {} {
  catch {gdb_immediate next}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_finish
# ------------------------------------------------------------------
proc gdbtk_finish {} {
  catch {gdb_immediate finish}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_continue
# ------------------------------------------------------------------
proc gdbtk_continue {} {
  catch {gdb_immediate continue}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_stepi
# ------------------------------------------------------------------
proc gdbtk_stepi {} {
  catch {gdb_immediate stepi}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_nexti
# ------------------------------------------------------------------
proc gdbtk_nexti {} {
  catch {gdb_immediate nexti}
}

# ------------------------------------------------------------------
#  PROC: gdbtk_attached
# ------------------------------------------------------------------
#
# This is called AFTER gdb has successfully done an attach.  Use it to 
# bring the GUI up to a current state...
proc gdbtk_attached {} {
  gdbtk_update
}

# ------------------------------------------------------------------
#  PROC: gdbtk_detached
# ------------------------------------------------------------------
#
# This is called AFTER gdb has successfully done an detach.  Use it to 
# bring the GUI up to a current state...
proc gdbtk_detached {} {
  if {!$::gdb_shutting_down} {
    run_hooks gdb_no_inferior_hook
  }
}

# ------------------------------------------------------------------
#  PROC: gdbtk_stop
# ------------------------------------------------------------------
#
# The stop button is tricky. In order to use the stop button,
# the debugger must be able to keep gui alive while target_wait is
# blocking (so that the user can interrupt or detach from it).
# 
# The best solution for this is to capture gdb deep down where it
# can block. For _any_ target board, this will be in either
# serial or socket code. These places call ui_loop_hook to 
# keep us alive. For native unix, we use an interval timer.
# Simulators either call ui_loop_hook directly (older sims, at least)
# or they call gdb's os_poll_quit callback, where we insert a call
# to ui_loop_hook. Some targets (like v850ice and windows native)
# require a call to ui_loop_hook directly in target_wait. See comments
# before gdb_stop and x_event to find out more about how this is accomplished.
#
# The stop button's behavior:
# Pressing the stop button should attempt to stop the target. If, after
# some time (like 3 seconds), gdb fails to fall out of target_wait (i.e.,
# the gui's idle hooks are run), then open a dialog asking the user if
# he'd like to detach.
proc gdbtk_stop {} {
  global _gdbtk_stop

  if {$_gdbtk_stop(timer) == ""} {
    add_hook gdb_idle_hook gdbtk_stop_idle_callback
    set _gdbtk_stop(timer) [after 3000 gdbtk_detach]
    catch {gdb_stop}
  }
}

# ------------------------------------------------------------------
#  PROC: gdbtk_stop_idle_callback
# ------------------------------------------------------------------
# This callback normally does nothing. When the stop button has
# been pressed, though, and gdb has successfully stopped the target,
# this callback will clean up after gdbtk_stop, removing the "Detach"
# dialog (if it's open) and gettingg rid of any outstanding timers
# and hooks.
proc gdbtk_stop_idle_callback {} {
  global _gdbtk_stop gdbtk_state

  # Check if the dialog asking if user wants to detach is open
  # and unpost it if it exists.
  if {$_gdbtk_stop(msg) != ""} {
    set ans [list answer $_gdbtk_stop(msg)]
    set gdbtk_state($ans) no
  }

  if {$_gdbtk_stop(timer) != ""} {
    # Cancel the timer callback
    after cancel $_gdbtk_stop(timer)
    set _gdbtk_stop(timer) ""
    catch {remove_hook gdb_idle_hook gdbtk_stop_idle_callback}
  }
}

# ------------------------------------------------------------------
#  PROC: gdbtk_detach
# ------------------------------------------------------------------
# This proc is installed as a timer event when the stop button
# is pressed. If target_wait doesn't return (we were unable to stop
# the target), then this proc is called.
#
# Open a dialog box asking if the user would like to detach. If so,
# try to detach. If not, do nothing and go away.
proc gdbtk_detach {} {
  global _gdbtk_stop

  set _gdbtk_stop(msg) "No response from target. Detach from target\n(and stop debugging it)?"
  if {[gdbtk_tcl_query  $_gdbtk_stop(msg) no]} {
    catch {gdb_stop detach}
  }

  set _gdbtk_stop(timer) ""
  set _gdbtk_stop(msg) ""
  remove_hook gdb_idle_hook gdbtk_stop_idle_callback
}

# ------------------------------------------------------------------
#  PROC: gdbtk_run
# ------------------------------------------------------------------
proc gdbtk_run {} {
  run_executable
}

# ------------------------------------------------------------------
# PROC:  gdbtk_attach_native: attach to a running target
# ------------------------------------------------------------------
proc gdbtk_attach_native {} {
    ManagedWin::open_dlg AttachDlg ;#-transient

    debug "ManagedWin got [AttachDlg::last_button] [AttachDlg::pid]"

    if {[AttachDlg::last_button]} {
	set pid [AttachDlg::pid]
	set symbol_file [AttachDlg::symbol_file]
	if {![_open_file $symbol_file]} {
	    ManagedWin::open WarningDlg -transient \
		    -message "Could not load symbols from $symbol_file."
	    return
	}
	
	if {[catch {gdb_cmd "attach $pid"} result]} {
	    ManagedWin::open WarningDlg -transient \
		    -message [list "Could not attach to $pid:\n$result"]
	    return
	}
    }
}

# ------------------------------------------------------------------
# PROC: set_baud -  Tell GDB the baud rate.
# ------------------------------------------------------------------
proc set_baud {} {
  global gdb_target_name
  #set target [ide_property get target-internal-name]
  set baud [pref getd gdb/load/${gdb_target_name}-baud]
  if {$baud == ""} {
    set baud [pref get gdb/load/baud]
  }
#  debug "setting baud to $baud"
  catch {gdb_cmd "set remotebaud $baud"}
}

# ------------------------------------------------------------------
# PROC: do_state_hook -
# ------------------------------------------------------------------
proc do_state_hook {varname ind op} {
  run_hooks state_hook $varname
}

# ------------------------------------------------------------------
# PROC: gdbtk_disconnect -
# ------------------------------------------------------------------
proc gdbtk_disconnect {{async 0}} {
   global gdb_loaded gdb_target_changed
   catch {gdb_cmd "detach"}
   # force a new target command to do something
   set gdb_loaded 0
   set gdb_target_changed 1
   set gdb_running 0
   gdbtk_idle
   gdbtk_update
 }

# ------------------------------------------------------------------
# PROC: tstart -
# ------------------------------------------------------------------
proc tstart {} {
   if {[catch {gdb_cmd "tstart"} errTxt]} {
     tk_messageBox -title "Error" -message $errTxt -icon error \
       -type ok
    gdbtk_idle
     return 0
   }
  return 1
}

# ------------------------------------------------------------------
# PROC: tstop -
# ------------------------------------------------------------------
proc tstop {} {

   if {[catch {gdb_cmd "tstop"} errTxt]} {
     tk_messageBox -title "Error" -message $errTxt -icon error \
       -type ok
     gdbtk_idle
     return 0
   }
   return 1
 }

# ------------------------------------------------------------------
# PROC: source_file -
# ------------------------------------------------------------------
proc source_file {} {
  set file_name [tk_getOpenFile -title "Choose GDB Command file"]
  if {$file_name != ""} {
    gdb_cmd "source $file_name"
  }
}


# -----------------------------------------------------------------------------
# NAME:		gdbtk_signal
#
# SYNOPSIS:	gdbtk_signal {name longname}
#
# DESC:		This procedure is called from GDB when a signal	
#		is generated, for example, a SIGSEGV.
#
# ARGS:		name - The name of the signal, as returned by
#			target_signal_to_name().
#		longname - A description of the signal.
# -----------------------------------------------------------------------------
proc gdbtk_signal {name {longname ""}} {
  dbug W "caught signal $name $longname"
  set longname
  set message "Program received signal $name, $longname"
  set srcs [ManagedWin::find SrcWin]
  foreach w $srcs {
    $w set_status $message
  }
  gdbtk_tcl_ignorable_warning signal $message
  update idletasks
}

# Hook for clearing out executable state. Widgets should register a callback
# for this hook if they have anything that may need cleaning if the user
# requests to re-load an executable.
define_hook gdb_clear_file_hook

# -----------------------------------------------------------------------------
# NAME:       gdbtk_clear_file
#
# SYNOPSIS:   gdbtk_clear_file
#
# DESC:       This procedure is called when the user requests a new exec
#             file load. It runs the gdb_clear_file_hook, which tells
#             all widgets to clear state. It CANNOT call gdb_clear_file,
#             since this hook runs AFTER we load a new exec file (i.e.,
#             gdb_clear_file would clear the file name).
#
# ARGS:       none
# -----------------------------------------------------------------------------
proc gdbtk_clear_file {} {
  global gdb_target_name

  debug
  # Give widgets a chance to clean up
  catch {run_hooks gdb_clear_file_hook}

  # Save the target name in case the user has already selected a
  # target. No need to force the user to select it again.
  set old_target $gdb_target_name

  # Finally, reset our state
  initialize_gdbtk

  set gdb_target_name $old_target
}

# ------------------------------------------------------------------
#  PROC: intialize_gdbtk - (re)initialize gdbtk's state
# ------------------------------------------------------------------
proc initialize_gdbtk {} {
  global gdb_exe_changed gdb_target_changed gdb_running gdb_downloading \
    gdb_loaded gdb_program_has_run file_done gdb_pretty_name gdb_exec \
    gdb_target_cmd download_dialog gdb_pretty_name gdb_exe_name _gdbtk_stop \
    gdb_target_name gdb_target_changed gdbtk_state gdb_kod_cmd gdb_shutting_down

  # initialize state variables
  set gdb_exe_changed 0
  set gdb_target_changed 0
  set gdb_running 0
  set gdb_downloading 0
  set gdb_loaded 0
  set gdb_program_has_run 0
  set file_done 0
  set gdb_pretty_name {}
  set gdb_exec {}
  set gdb_target_cmd ""
  set gdb_running 0
  set gdb_shutting_down 0

  set download_dialog ""

  # gdb_pretty_name is the name of the GDB target as it should be
  # displayed to the user.
  set gdb_pretty_name ""

  # gdb_exe_name is the name of the executable we are debugging.  
  set gdb_exe_name ""

  # Initialize readline
  if {![info exists gdbtk_state(readline)]} {
    # Only do this once...
    set gdbtk_state(readline) 0
    set gdbtk_state(console) ""
    set gdbtk_state(readlineShowUser) 1
  }

  # check for existence of a kod command and get it's name and
  # text for menu entry
  set gdb_kod_cmd ""
  set msg ""
  if {![catch {gdb_cmd "show os"} msg] && ($msg != "")} {
    set line1 [string range $msg 0 [expr [string first \n $msg] -1]]
    if {[regexp -- \"(.*)\" $line1 dummy cmd]} {
      set gdb_kod_cmd $cmd
    }
  }
#  debug "kod_cmd=$gdb_kod_cmd"

  # setup stop button
  set _gdbtk_stop(timer) ""
  set _gdbtk_stop(msg) ""

  # gdb_target_name is the name of the GDB target; that is, the argument
  # to the GDB target command.
  set gdb_target_name ""

  # By setting gdb_target_changed, we force a target dialog
  # to be displayed on the first "run"
  set gdb_target_changed 1
}

# The architecture changed. Inform the UI.
proc gdbtk_tcl_architecture_changed {} {
  set e [ArchChangedEvent \#auto]
  GDBEventHandler::dispatch $e
  delete object $e
}
