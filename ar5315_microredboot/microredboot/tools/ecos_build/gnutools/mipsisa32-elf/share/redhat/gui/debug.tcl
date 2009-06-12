# -----------------------------------------------------------------------------
# NAME:	
#		::debug
#
# DESC:	
#		This namespace implements general-purpose debugging functions
#		to display information as a program runs.  In addition, it 
#		includes profiling (derived from Sage 1.1) and tracing.  For 
#		output it can write to files, stdout, or use a debug output 
#		window.
#
# NOTES:	
#		Output of profiler is compatible with sageview.
#
# -----------------------------------------------------------------------------

package provide debug 1.0

namespace eval ::debug {
  namespace export debug dbug
  variable VERSION 1.1
  variable absolute
  variable stack ""
  variable outfile "trace.out"
  variable watch 0
  variable watchstart 0
  variable debugwin ""
  variable tracedVars
  variable logfile ""
  variable initialized 0
  variable stoptrace 0
  variable tracing 0
  variable profiling 0
  variable level 0

  # here's where we'll store our collected profile data
  namespace eval data {
    variable entries
  }

  proc logfile {file} {
    variable logfile
    if {$logfile != "" && $logfile != "stdout" && $logfile != "stderr"} {
      catch {close $logfile}
    }
    
    if {$file == ""} {
      set logfile ""
    } elseif {$file == "stdout" || $file == "stderr"} {
      set logfile $file
    } else {
      set logfile [open $file w+]
      fconfigure $logfile -buffering line -blocking 0
    }
  }

# ----------------------------------------------------------------------------
# NAME:		debug::trace_var
# SYNOPSIS:	debug::trace_var {varName mode}
# DESC:		Sets up variable trace.  When the trace is activated, 
#		debugging messages will be displayed.
# ARGS:		varName - the variable name
#		mode - one of more of the following letters
#			r - read
#			w - write
#			u - unset
# -----------------------------------------------------------------------------
  proc trace_var {varName mode} {
    variable tracedVars
    lappend tracedVars [list $varName $mode] 
    uplevel \#0 trace variable $varName $mode ::debug::touched_by
  }

# ----------------------------------------------------------------------------
# NAME:		debug::remove_trace
# SYNOPSIS:	debug::remove_trace {var mode}
# DESC:		Removes a trace set up with "trace_var".
# ----------------------------------------------------------------------------
  proc remove_trace {var mode} {
    uplevel \#0 trace vdelete $var $mode ::debug::touched_by
  }

# ----------------------------------------------------------------------------
# NAME:		debug::remove_all_traces
# SYNOPSIS:	debug::remove_all_traces
# DESC:		Removes all traces set up with "trace_var".
# ----------------------------------------------------------------------------
  proc remove_all_traces {} {
    variable tracedVars
    if {[info exists tracedVars]} {
      foreach {elem} $tracedVars {
	eval remove_trace $elem
      }
      unset tracedVars
    }
  }

# ----------------------------------------------------------------------------
# NAME:		debug::touched_by
# SYNOPSIS:	debug::touched_by {v a m}
# DESC:		Trace function used by trace_var. Currently writes standard
#		debugging messages or priority "W".
# ARGS:		v - variable
#		a - array element or ""
#		m - mode
# ----------------------------------------------------------------------------
  proc touched_by {v a m} {
    if {$a==""} {
      upvar $v foo
      dbug W "Variable $v touched in mode $m"
    } else {
      dbug W "Variable ${v}($a) touched in mode $m"
      upvar $v($a) foo
    }
    dbug  W "New value: $foo"
    show_call_stack 2
  }
  
# ----------------------------------------------------------------------------
# NAME:		debug::show_call_stack
# SYNOPSIS:	debug::show_call_stack {{start_decr 0}}
# DESC:		Function used by trace_var to print stack trace. Currently 
#		writes standard debugging messages or priority "W".
# ARGS:		start_decr - how many levels to go up to start trace
# ----------------------------------------------------------------------------
  proc show_call_stack {{start_decr 0}} {
    set depth [expr {[info level] - $start_decr}]
    if {$depth == 0} {
      dbug W "Called at global scope"
    } else {
      dbug W "Stack Trace follows:"
      for {set i $depth} {$i > 0} {incr i -1} {
	dbug W "Level $i: [info level $i]"
      }
    }
  }
  
# ----------------------------------------------------------------------------
# NAME:		debug::createData
# SYNOPSIS:	createData { name }
# DESC:		Basically creates a data structure for storing profiling 
#		information about a function.
# ARGS:		name - unique (full) function name
# -----------------------------------------------------------------------------
  proc createData {name} {
    lappend data::entries $name
    
    namespace eval data::$name {
      variable totaltimes 0
      variable activetime 0
      variable proccounts 0
      variable timers 0
      variable timerstart 0
      variable nest 0
    }
  }
  
  proc debugwin {obj} {
    variable debugwin
    set debugwin $obj
  }

# -----------------------------------------------------------------------------
# NAME:		debug::debug
#
# SYNOPSIS:	debug { {msg ""} }
#
# DESC:		Writes a message to the proper output. The priority of the 
#		message is assumed to be "I" (informational). This function
#		is provided for compatibility with the previous debug function.
#		For higher priority messages, use dbug.
#
# ARGS:		msg - Message to be displayed. 
# -----------------------------------------------------------------------------

  proc debug {{msg ""}} {
    set cls [string trimleft [uplevel namespace current] :]
    if {$cls == ""} {
      set cls "global"
    }
    
    set i [expr {[info level] - 1}]
    if {$i > 0} {
      set func [lindex [info level $i] 0]
      set i [string first "::" $func]
      if {$i != -1} {
	# itcl proc has class prepended to func
	# strip it off because we already have class in $cls
	set func [string range $func [expr {$i+2}] end]
      }
    } else {
      set func ""
    }

    ::debug::_putdebug I $cls $func $msg
  }

# -----------------------------------------------------------------------------
# NAME:		debug::dbug
#
# SYNOPSIS:	dbug { level msg }
#
# DESC:		Writes a message to the proper output. Unlike debug, this
#		function take a priority level.
#
# ARGS:		msg   - Message to be displayed.
#		level - One of the following:
#				"I" - Informational only 
#				"W" - Warning
#				"E" - Error
#				"X" - Fatal Error
# -----------------------------------------------------------------------------
  proc dbug {level msg} {
    set cls [string trimleft [uplevel namespace current] :]
    if {$cls == ""} {
      set cls "global"
    }
    
    set i [expr {[info level] - 1}]
    if {$i > 0} {
      set func [lindex [info level $i] 0]
    } else {
      set func ""
    }
    
    ::debug::_putdebug $level $cls $func $msg
  }

# -----------------------------------------------------------------------------
# NAME:		debug::_putdebug
#
# SYNOPSIS:	_putdebug { level cls func msg }
#
# DESC:	Writes a message to the proper output. Will write to a debug
#	window if one is defined. Otherwise will write to stdout.
#
# ARGS:		msg   - Message to be displayed.
#		cls   - name of calling itcl class or "global"
#		func  - name of calling function
#		level - One of the following:
#			"I" - Informational only 
#			"W" - Warning
#			"E" - Error
#			"X" - Fatal Error
# -----------------------------------------------------------------------------
  proc _putdebug {lev cls func msg} {
    variable debugwin
    variable logfile
    if {$debugwin != ""} {
      $debugwin puts $lev $cls $func $msg
    }
    if {$logfile == "stdout"} {
      if {$func != ""} { append cls ::$func }
      puts $logfile "$lev: ($cls) $msg"
    } elseif {$logfile != ""} {
      puts $logfile [concat [list $lev] [list $cls] [list $func] [list $msg]]
    }
  }

  proc _puttrace {enter lev func {ar ""}} {
    variable debugwin
    variable logfile
    variable stoptrace
    variable tracing

    if {!$tracing} { return }

    set func [string trimleft $func :]
    if {$func == "DebugWin::put_trace" || $func == "DebugWin::_buildwin"} {
      if {$enter} {
	incr stoptrace
      } else {
	incr stoptrace -1
      }
    }
    
    if {$stoptrace == 0} {
      incr stoptrace
      # strip off leading function name
      set ar [lrange $ar 1 end]
      if {$debugwin != ""} {
	$debugwin put_trace $enter $lev $func $ar
      }
      
      if {$logfile != ""} {
	puts $logfile [concat {T} [list $enter] [list $lev] [list $func] \
			 [list $ar]]
      }
      incr stoptrace -1
    }
  }

# -----------------------------------------------------------------------------
# NAME:		debug::init
# SYNOPSIS:	init
# DESC:		Installs hooks in all procs and methods to enable profiling
#		and tracing.
# NOTES:	Installing these hooks slows loading of the program. Running
#		with the hooks installed will cause significant slowdown of
#		program execution. 
# -----------------------------------------------------------------------------
  proc init {} {
    variable VERSION
    variable absolute
    variable initialized

    # create the arrays for the .global. level
    createData .global.
    
    # start the absolute timer
    set absolute [clock clicks]

    # rename waits, exit, and all the ways of declaring functions
    rename ::vwait ::original_vwait
    interp alias {} ::vwait {} [namespace current]::sagevwait
    createData .wait.

    rename ::tkwait ::original_tkwait
    interp alias {} ::tkwait {} [namespace current]::sagetkwait
    
    rename ::exit ::original_exit
    interp alias {} ::exit {} [namespace current]::sageexit

    rename ::proc ::original_proc
    interp alias {} ::proc {} [namespace current]::sageproc

    rename ::itcl::parser::method ::original_method
    interp alias {} ::itcl::parser::method {} [namespace current]::sagemethod

    rename ::itcl::parser::proc ::original_itclproc
    interp alias {} ::itcl::parser::proc {} [namespace current]::sageitclproc

    rename ::body ::original_itclbody
    interp alias {} ::body {} [namespace current]::sageitclbody

    # redefine core procs
    #    foreach p [uplevel \#0 info procs] {
    #      set args ""
    #      set default ""
    #      # get the list of args (some could be defaulted)
    #      foreach arg [info args $p] {
    #	if { [info default $p $arg default] } {
    #	  lappend args [list $arg $default]
    #	} else {
    #	  lappend args $arg
    #	}
    #      }
    #      uplevel \#0 proc [list $p] [list $args] [list [info body $p]]
    #}
  
    set initialized 1
    resetWatch 0
    procEntry .global.
    startWatch
  }

# -----------------------------------------------------------------------------
# NAME:		::debug::trace_start
# SYNOPSIS:	::debug::trace_start
# DESC:		Starts logging of function trace information.
# -----------------------------------------------------------------------------
  proc trace_start {} {
    variable tracing
    set tracing 1
  }
  
# -----------------------------------------------------------------------------
# NAME:		::debug::trace_stop
# SYNOPSIS:	::debug::trace_stop
# DESC:		Stops logging of function trace information.
# -----------------------------------------------------------------------------
  proc trace_stop {} {
    variable tracing
    set tracing 0
  }

# -----------------------------------------------------------------------------
# NAME:		debug::sagetkwait
# SYNOPSIS:	sagetkwait {args}
# DESC:		A wrapper function around tkwait so we know how much time the
#		program is spending in the wait state.
# ARGS:		args - args to pass to tkwait
# ----------------------------------------------------------------------------
  proc sagetkwait {args} {
    # simulate going into the .wait. proc
    stopWatch
    procEntry .wait.
    startWatch
    uplevel ::original_tkwait $args
    # simulate the exiting of this proc
    stopWatch
    procExit .wait.
    startWatch
  }
  
# ----------------------------------------------------------------------------
# NAME:		debug::sagevwait
# SYNOPSIS:	sagevwait {args}
# DESC:		A wrapper function around vwait so we know how much time the
#		program is spending in the wait state.
# ARGS:		args - args to pass to vwait
# ----------------------------------------------------------------------------
  proc sagevwait {args} {
    # simulate going into the .wait. proc
    stopWatch
    procEntry .wait.
    startWatch
    uplevel ::original_vwait $args    
    # simulate the exiting of this proc
    stopWatch
    procExit .wait.
    startWatch
  }
  
# -----------------------------------------------------------------------------
# NAME:		debug::sageexit
# SYNOPSIS:	sageexit {{value 0}}
# DESC:		A wrapper function around exit so we can turn off profiling
#		and tracing before exiting.
# ARGS:		value - value to pass to exit
# -----------------------------------------------------------------------------
  proc sageexit {{value 0}} {
    variable program_name GDBtk
    variable program_args ""
    variable absolute
    
    # stop the stopwatch
    stopWatch

    set totaltime [getWatch]

    # stop the absolute timer
    set stop [clock clicks]
    
    # unwind the stack and turn off everyone's timers
    stackUnwind
        
    # disengage the proc callbacks
    ::original_proc procEntry {name} {}
    ::original_proc procExit {name args} {}
    ::original_proc methodEntry {name} {}
    ::original_proc methodExit {name args} {}
    
    set absolute [expr {$stop - $absolute}]

    # get the sage overhead time
    set sagetime [expr {$absolute - $totaltime}]
    
    # save the data
    variable outfile
    variable VERSION
    set f [open $outfile w]
    puts $f "set VERSION {$VERSION}"
    puts $f "set program_name {$program_name}"
    puts $f "set program_args {$program_args}"
    puts $f "set absolute $absolute"
    puts $f "set sagetime $sagetime"
    puts $f "set totaltime $totaltime"
    
    foreach procname $data::entries {
      set totaltimes($procname) [set data::${procname}::totaltimes]
      set proccounts($procname) [set data::${procname}::proccounts]
      set timers($procname) [set data::${procname}::timers]
    }

    puts $f "array set totaltimes {[array get totaltimes]}"
    puts $f "array set proccounts {[array get proccounts]}"
    puts $f "array set timers {[array get timers]}"
    close $f
    original_exit $value
  }
  
  
  proc sageproc {name args body} {
    # stop the watch
    stopWatch

    # update the name to include the namespace if it doesn't have one already
    if {[string range $name 0 1] != "::"} {
      # get the namespace this proc is being defined in
      set ns [uplevel namespace current]
      if { $ns == "::" } {
        set ns ""
      }
      set name ${ns}::$name
    }

    createData $name          
    # create the callbacks for proc entry and exit
    set ns [namespace current]
    set extra "${ns}::stopWatch;"
    append extra "set __.__ {};trace variable __.__ u {${ns}::stopWatch;${ns}::procExit $name;${ns}::startWatch};"
    append extra "[namespace current]::procEntry $name;"
    append extra "[namespace current]::startWatch;"

    set args [list $args]
    set body [list [concat $extra $body]]
    
    startWatch

    # define the proc with our extra stuff snuck in
    uplevel ::original_proc $name $args $body
  }

  proc sageitclbody {name args body} {
    # stop the watch
    stopWatch

    if {$name == "iwidgets::Scrolledwidget::_scrollWidget"} {
      # Hack.  This causes too many problems for the scrolled debug window
      # so just don't include it in the profile functions.
      uplevel ::original_itclbody $name [list $args] [list $body]
      return
    }

    set fullname $name
    # update the name to include the namespace if it doesn't have one already
    if {[string range $name 0 1] != "::"} {
      # get the namespace this proc is being defined in
      set ns [uplevel namespace current]
      if { $ns == "::" } {
        set ns ""
      }
      set fullname ${ns}::$name
    }
    
    createData $fullname          
    # create the callbacks for proc entry and exit
    set ns [namespace current]
    set extra "${ns}::stopWatch;"
    append extra "set __.__ {};trace variable __.__ u {${ns}::stopWatch;${ns}::procExit $fullname;${ns}::startWatch};"
    append extra "[namespace current]::procEntry $fullname;"
    append extra "[namespace current]::startWatch;"

    set args [list $args]
    set body [list [concat $extra $body]]
    
    startWatch

    # define the proc with our extra stuff snuck in
    uplevel ::original_itclbody $name $args $body
  }

  proc sageitclproc {name args} {
    # stop the watch
    stopWatch

    set body [lindex $args 1]
    set args [lindex $args 0]

    if {$body == ""} {
      set args [list $args]
      set args [concat $args $body]
    } else {
      # create the callbacks for proc entry and exit
      set ns [namespace current]
      set extra "${ns}::stopWatch;"
      append extra "set __.__ {};trace variable __.__ u {${ns}::stopWatch;${ns}::methodExit $name;${ns}::startWatch};"
      append extra "[namespace current]::methodEntry $name;"
      append extra "[namespace current]::startWatch;"

      set args [list $args [concat $extra $body]]
    }

    startWatch
    uplevel ::original_itclproc $name $args
  }

  proc sagemethod {name args} {
    # stop the watch
    stopWatch

    set body [lindex $args 1]
    set args [lindex $args 0]

    if {[string index $body 0] == "@" || $body == ""} {
      set args [list $args]
      set args [concat $args $body]
    } else {
      # create the callbacks for proc entry and exit
      set ns [namespace current]
      set extra "${ns}::stopWatch;"
      append extra "set __.__ {};trace variable __.__ u {${ns}::stopWatch;${ns}::methodExit $name;${ns}::startWatch};"
      append extra "[namespace current]::methodEntry $name;"
      append extra "[namespace current]::startWatch;"

      set args [list $args [concat $extra $body]]
    }

    startWatch
    uplevel ::original_method $name $args
  }
  
  proc push {v} {
    variable stack 
    variable level
    lappend stack $v
    incr level
  }
  
  proc pop {} {
    variable stack
    variable level
    set v [lindex $stack end]
    set stack [lreplace $stack end end]
    incr level -1
    return $v
  }
  
  proc look {} {
    variable stack
    return [lindex $stack end]   
  }
  
  proc stackUnwind {} {
    # Now unwind all the stacked procs by calling procExit on each.
    # It is OK to use procExit on methods because the full name
    # was pushed on the stack
    while { [set procname [look]] != "" } {
      procExit $procname
    }
  }
  
  # we need args because this is part of a trace callback
  proc startWatch {args} {
    variable watchstart
    set watchstart [clock clicks]
  }
  
  proc resetWatch {value} {
    variable watch
    set watch $value
  }
  
  proc stopWatch {} {
    variable watch
    variable watchstart
    set watch [expr {$watch + ([clock clicks] - $watchstart)}]    
    return $watch
  }

  proc getWatch {} {
    variable watch
    return $watch
  }
  
  proc startTimer {v} {
    if { $v != "" } {
      set data::${v}::timerstart [getWatch]
    }
  }
  
  proc stopTimer {v} {
    if { $v == "" } return
    set stop [getWatch]
    set data::${v}::timers [expr {[set data::${v}::timers] + ($stop - [set data::${v}::timerstart])}]
  }
  
  proc procEntry {procname} {
    variable level
    _puttrace 1 $level $procname [uplevel info level [uplevel info level]]

    set time [getWatch]
    
    # stop the timer of the caller
    set caller [look]
    stopTimer $caller 
    
    incr data::${procname}::proccounts
    
    if { [set data::${procname}::nest] == 0 } {
      set data::${procname}::activetime $time
    }
    incr data::${procname}::nest

    # push this proc on the stack
    push $procname
    
    # start the timer for this
    startTimer $procname
  }

  proc methodEntry {procname} {
    variable level

    set time [getWatch]
    
    # stop the timer of the caller
    set caller [look]
    stopTimer $caller 
    
    # get the namespace this method is in
    set ns [uplevel namespace current]
    if { $ns == "::" } {
      set ns ""
    }
    set name ${ns}::$procname
    _puttrace 1 $level $name [uplevel info level [uplevel info level]]

    if {![info exists data::${name}::proccounts]} {
      createData $name
    }

    incr data::${name}::proccounts
    
    if { [set data::${name}::nest] == 0 } {
      set data::${name}::activetime $time
    }
    incr data::${name}::nest

    # push this proc on the stack
    push $name
    
    # start the timer for this
    startTimer $name
  }

  # we need the args because this is called from a vartrace handler
  proc procExit {procname args} {
    variable level

    set time [getWatch]
    # stop the timer of the proc
    stopTimer [pop]

    _puttrace 0 $level $procname
    
    set r [incr data::${procname}::nest -1]
    if { $r == 0 } {
      set data::${procname}::totaltimes \
	[expr {[set data::${procname}::totaltimes] \
		 + ($time - [set data::${procname}::activetime])}]
    }
    
    # now restart the timer of the caller
    startTimer [look]
  }

  proc methodExit {procname args} {
    variable level

    set time [getWatch]
    # stop the timer of the proc
    stopTimer [pop]
    
    # get the namespace this method is in
    set ns [uplevel namespace current]
    if { $ns == "::" } {
      set ns ""
    }
    set procname ${ns}::$procname

    _puttrace 0 $level $procname

    set r [incr data::${procname}::nest -1]
    if { $r == 0 } {
      set data::${procname}::totaltimes \
	[expr {[set data::${procname}::totaltimes] \
		 + ($time - [set data::${procname}::activetime])}]
    }
    
    # now restart the timer of the caller
    startTimer [look]
  }
}
