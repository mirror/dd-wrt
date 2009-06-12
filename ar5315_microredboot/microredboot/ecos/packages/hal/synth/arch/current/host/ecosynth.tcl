# {{{  Banner                                                   

# ============================================================================
# 
#      ecosynth.tcl
# 
#      The eCos synthetic target I/O auxiliary
# 
# ============================================================================
# ####COPYRIGHTBEGIN####
#                                                                           
#  ----------------------------------------------------------------------------
#  Copyright (C) 2002 Bart Veer
# 
#  This file is part of the eCos host tools.
# 
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the Free 
#  Software Foundation; either version 2 of the License, or (at your option) 
#  any later version.
#  
#  This program is distributed in the hope that it will be useful, but WITHOUT 
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
#  more details.
#  
#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
#  ----------------------------------------------------------------------------
#                                                                           
# ####COPYRIGHTEND####
# ============================================================================
# #####DESCRIPTIONBEGIN####
# 
#  Author(s):   bartv
#  Contact(s):  bartv
#  Date:        2002/08/05
#  Version:     0.01
#  Description:
#      The main script for the eCos synthetic target auxiliary. This should
#      only ever be invoked from inside ecosynth.
# 
# ####DESCRIPTIONEND####
# ============================================================================

# }}}

# {{{  Overview                                                 

# When an eCos synthetic target application runs it will usually
# fork/execve an auxiliary program, ecosynth, to provide certain
# I/O functionality. This happens as part of hardware initialization.
#
# The ecosynth executable in turn runs this ecosynth.tcl script which
# either does most of the real work or delegates it to other scripts.
# Those other scripts may in turn exec other programs to perform any
# I/O operations that cannot easily be done at the Tcl level. For
# example performing low-level ethernet operations generally requires
# working at the C level or equivalent, so a separate executable would
# be appropriate. The ecosynth executable will transfer control to
# this script after the appinit() call, which should have performed
# certain initialization steps.
#
#  1) the Tcl interpreter will be fully initialized.
#
#  2) usually Tk will have been loaded and initialized as well. This
#     can be suppressed using a command-line option -nw.
#
#  3) there will be a namespace synth:: for use by ecosynth.tcl
#     Auxiliary scripts are expected to use their own namespace
#     where possible.
#
#  4) there will be two channels synth::_channel_from_app and
#     synth::_channel_to_app. These correspond to a pipe between
#     the eCos application and the auxiliary. The application will
#     send requests using this pipe and expect replies. I/O
#     operations are always initiated by a request from the
#     application, but the auxiliary can raise an interrupt via
#     the SIGIO signal.
#
#     Other standard channels stdin, stdout and stderr will have
#     been inherited from the eCos application.
#
#  5) some Tcl commands implemented in C will have been added to the
#     interpreter. The most notable is synth::_send_SIGIO, used to
#     raise an interrupt within the application. 
#
#  6) similarly some variables will have been added to the interpreter.
#
#
# Configuring everything can get somewhat complicated. It is the eCos
# application that knows what I/O facilities within the auxiliary it
# wants to access, but it may not know all the details. The eCos
# component architecture makes things a bit more complicated still,
# generic code such as this ecosynth.tcl script has no way of knowing
# what I/O facilities might be provided by some package or other.
#
# For example, a target-side ethernet driver may want to send outgoing
# ethernet packets to some script or program on the host and receive
# incoming ethernet packets. However it does not necessarily know what
# the host-side should do with those ethernet packets, e.g. use a
# spare Linux ethernet device, use the Linux kernel's ethertap
# facility, ... Although that kind of information could be handled by
# target-side configury, host-side configuration files will often be
# more appropriate. Specifically it would allow a single eCos
# synthetic target application to run in a variety of environments,
# using different ways to provide the I/O, with no need to do any
# reconfiguring or rebuilding of the target side.
#
#
# The basic approach that is taken is:
#
#   1) the eCos application tells the auxiliary what I/O facilities
#      it is interested in. This should happen as a result
#      of static constructors or of device driver initialization
#      routines. The application has no control over the implementation
#      of the I/O facilities, it just expects something on the other
#      end to respond sensibly to requests.
#
#      For example, a synthetic target ethernet driver will supply
#      an initialization routine via its NETDEVTAB_ENTRY. This
#      routine will send a request to the auxiliary asking for a
#      device of type ethernet, id "eth0", provided by
#      CYGPKG_DEVS_ETH_ECOSYNTH, version current. The auxiliary will
#      now attempt to load a suitable Tcl script ethernet.tcl from a
#      location determined using the package name and version.
#      
#   2) there is a primary target definition file which can be
#      specified as the final argument on the command line, with
#      "default" being the default value. The code will look for
#      this file with or without a .tdf extension, first in the
#      current directory, then in ~/.ecos/synth/. This file is
#      actually a Tcl script that gets executed in the current
#      interpreter. However typically it will only contain
#      entries such as:
#
#          synth_device eth0 {
#              ...
#          }
#
#   3) There are additional optional configuration files
#      ~/.ecos/synth/initrc.tcl and ~/.ecos/synth/mainrc.tcl which can
#      be used for further customization. initrc.tcl will get run
#      early on, mainrc.tcl will get run once initialization is
#      complete. Specifically the target-side code will send an
#      appropriate request after invoking all the static constructors.
#      At this time the auxiliary will run mainrc.tcl, and in addition
#      it may issue warnings about unused arguments etc.
#
#   4) there may also be separate configuration files for GUI
#      preferences etc. These are distinct from the initrc and
#      mainrc files in that they are generated rather than
#      hand-written.

# }}}
# {{{  Basic initialization                                     

# ----------------------------------------------------------------------------
# There should be two channels corresponding to the pipe between the
# eCos application and the auxiliary. These should be configured
# appropriately. If either channel does not exist then that is a
# very good indication that the system is not properly initialized,
# i.e. that this script is not being run by the ecosynth executable
# and by implication that no eCos application is involved.

if {([info exists synth::_channel_to_app] == 0) ||
    ([info exists synth::_channel_from_app] == 0) ||
    ([info exists synth::_ecosynth_version] == 0) ||
    ([info exists synth::_ppid] == 0) ||
    ([info exists synth::_ecosynth_repository] == 0) ||
    ([info exists synth::_ecosynth_libexecdir] == 0) ||
    ([info exists synth::_ecosynth_package_dir] == 0) ||
    ([info exists synth::_ecosynth_package_version] == 0) ||
    ([info exists synth::_ecosynth_package_install] == 0) ||
    ([info commands synth::_send_SIGIO] == "") ||
    ([info commands synth::_send_SIGKILL] == "") } {

    puts stderr "ecosynth.tcl: the current interpreter has not been properly initialized."
    puts stderr "    This script should only be invoked by the ecosynth executable when"
    puts stderr "    an eCos synthetic target application is run."
    exit 1
}

# Is the script currently being executed the most recent version?
# This check should only happen if an environment variable
# ECOSYNTH_DEVEL is set, because the installed tools may have come
# from somewhere other than the current repository.
if { [info exists ::env(ECOSYNTH_DEVEL)] } {
    set _orig_name [file join $synth::_ecosynth_repository $synth::_ecosynth_package_dir $synth::_ecosynth_package_version \
	    "host" [file tail [info script]]]
    if { [file exists $_orig_name] && [file readable $_orig_name] && ($_orig_name != [info script]) } {
	if { [file mtime $_orig_name] >= [file mtime [info script]] } {
	    puts "$_orig_name is more recent than install: executing that."
	    source $_orig_name
	    return
	}
    }
    unset _orig_name
}

fconfigure $synth::_channel_to_app   -buffering none
fconfigure $synth::_channel_from_app -encoding binary
fconfigure $synth::_channel_to_app   -encoding binary
fconfigure $synth::_channel_from_app -translation binary
fconfigure $synth::_channel_to_app   -translation binary

# Define additional globals and procedures inside the synth:: namespace.
# Variables and functions that begin with an _ are considered internal
# and should not be used directly.
namespace eval synth {
    
    # Unfortunately the name of the eCos executable is lost at this stage.
    # Within the eCos application it was held in argv[0], but that has been
    # overridden with the name of the auxiliary. However we have access to the
    # parent process id so we can use /proc to get the required information.
    variable ecos_appname ""
    catch {
	set synth::ecos_appname [file readlink "/proc/[set synth::_ppid]/exe"]
	set synth::ecos_appname [file tail $synth::ecos_appname]
    }

    # The install location can be determined from the script name.
    # This is used for e.g. loading bitmaps, even if ECOSYNTH_DEVEL
    # is set, because some of the files may be generated.
    # ECOSYNTH_DEVEL only affects Tcl scripts.
    variable install_dir [file join $synth::_ecosynth_libexecdir "ecos" $synth::_ecosynth_package_install]
    
    # Is the eCos application still running? This is worth keeping
    # track of so that send_reply and raise_interrupt do not try to
    # interact with a program that is no longer running.
    variable ecos_running 1

    # This variable is used to enter the event loop
    variable _ecosynth_exit 0

    # Is GUI mode enabled?
    variable flag_gui [expr { "" != [info commands "tk"] } ]

    # When running in GUI mode the GUI should stay up even after the application
    # has exited, so that the user can take a good look around. When running in
    # non-GUI mode this program should exit as soon it has finished cleaning up.
    variable flag_immediate_exit    [expr { 0 == $synth::flag_gui} ]
    
    # Is the GUI ready to accept output?
    variable flag_gui_ready 0
    
    # Flags and variables related to command-line arguments
    variable flag_help          0
    variable flag_keep_going    0
    variable flag_no_rc         0
    variable flag_verbose       0
    variable flag_debug         0
    variable logfile            ""
    variable target_definition  ""
    variable geometry           "<none>"
}

# }}}
# {{{  Hooks & atexit support                                   

# ----------------------------------------------------------------------------
# A lot of the flexibility of ecosynth is provided by hooks. Device scripts
# and, more importantly, the per-user initrc and mainrc scripts can install
# hooks that get called when an event occurs, for example when the eCos
# applications attempts to transmit an ethernet packet.
#
# A special hook is used to implement atexit handling. This involves redefining
# the "exit" command so that it will invoke the appropriate hooks first.

namespace eval synth {
    # All hooks are held in an array, indexed by the hook name, with each
    # array entry being a list of functions to be invoked.
    array set _hooks [list]

    proc hook_define { name } {
	if { [info exists synth::_hooks($name)] } {
	    synth::report_error "Attempt to create hook $name which already exists.\n"
	} else {
	    set synth::_hooks($name) [list]
	}
    }

    proc hook_defined { name } {
	return [info exists synth::_hooks($name)]
    }
    
    proc hook_add { name function } {
	if { ! [info exists synth::_hooks($name)] } {
	    synth::report_error "Attempt to attach a function to an unknown hook $name\n"
	    set synth::_hooks($name) [list]
	}
	lappend synth::_hooks($name) $function
    }

    proc hook_call { name args } {
	if { ! [info exists synth::_hooks($name) ] } {
	    synth::report_error "Attempt to invoke unknown hook $name\n"
	} else {
	    foreach function $synth::_hooks($name) {
		$function $args
	    }
	}
    }

    # Define an initial set of hooks
    synth::hook_define "exit"                  ;# The auxiliary is exiting
    synth::hook_define "ecos_exit"             ;# The eCos application has exited
    synth::hook_define "ecos_initialized"      ;# eCos static constructors have run
    synth::hook_define "help"                  ;# --help
}

# Rename the builtin exit command so that it can still be accessed.
rename exit _hook_real_exit

# And define a replacement for exit which will invoke the appropriate
# hook. Care has to be taken in case of recursive exit calls, each
# hook function is only called once.

proc exit { { code 0 } } {
    while { [llength $synth::_hooks(exit)] > 0 } {
	set handler [lindex $synth::_hooks(exit) end]
	set synth::_hooks(exit) [lrange $synth::_hooks(exit) 0 end-1]

	# For now assume no errors - it is not clear what could be done
	# about them anyway.
	catch { eval $handler [list]}
    }
    _hook_real_exit $code
}

# }}}
# {{{  Output                                                   

# ----------------------------------------------------------------------------
# The usual set of utilities for issuing warnings, errors, ...
#
# There are three possibilities to consider:
#
#   1) running in text-only mode. The output should just go to stdout
#
#   2) running in GUI mode and the text window exists. Just update the
#      window
#
#   3) running in GUI mode but the text window is not yet ready. The
#      output needs to be buffered for now, and will be flushed
#      later.
#
# Also, if for some reason this program exits while there is output still
# buffered that output should also go to stdout.
#
# If any errors occur during initialization, e.g. an invalid device script
# or user initialization scripts, those get reported and an error count
# is maintained. When the eCos application reports that initialization is
# complete it will be sent back a status for the auxiliary, and will
# exit if things have not started up correctly. This tries to ensure that
# if there are multiple errors the user sees all of them.

namespace eval synth {

    variable _pending_output [list]
    variable _logfd ""
    variable _error_count 0

    proc logfile_open { } {
	synth::report "Opening logfile $synth::logfile"
	set msg ""
	if { [catch { set synth::_logfd [open $synth::logfile "w"] } msg ] } {
	    synth::report_error "Unable to open logfile \"$synth::logfile\"\n    $msg\n"
	}
    }

    # A default implementation of output. This gets overwritten later when running
    # in GUI mode, so if GUI mode is enabled then this proc must be called before
    # the GUI is ready and the data must be queued.
    proc output { msg filter } {
	if { ! $synth::flag_gui } {
	    # If a logfile exists, output normally goes there rather than
	    # to standard output. The exception is for errors which
            # always go to stderr, in addition to the logfile.
	    if { "" != $synth::_logfd } {
		puts -nonewline $synth::_logfd $msg
		if { "error" == $filter } {
		    puts -nonewline stderr $msg
		}
	    } else {
		if { "error" == $filter } {
		    puts -nonewline stderr $msg
		} else {
		    puts -nonewline $msg
		}
	    }
	} else {
	    lappend synth::_pending_output [list $msg $filter]
	}
    }
    
    # Invoked by the text window code once everything is ready
    # and synth::output has been redefined.
    proc _flush_output { } {
	foreach msg $synth::_pending_output {
	    synth::output [lindex $msg 0] [lindex $msg 1]
	}
	set synth::_pending_output [list]
    }

    # Cope with early exits. This will only have an effect if
    # _flush_output has not been called yet, and by implication
    # if synth::output has not yet been redefined.
    proc _exit_flush_output { arg_list } {
	if { 0 != [llength $synth::_pending_output] } {
	    set synth::flag_gui 0
	    synth::_flush_output
	}
    }
    synth::hook_add "exit" synth::_exit_flush_output
    
    proc report { msg } {
	synth::output $msg "report"
    }

    proc report_warning { msg } {
	synth::output "Warning: $msg" "warning"
    }

    proc report_error { msg } {
	incr synth::_error_count
	synth::output "Error: $msg" "error"
    }

    # Internal errors indicate a serious problem within ecosynth or
    # a device-specific script. For now this results in output to
    # stderr, a backtrace, and termination of the auxiliary, which
    # should also cause the eCos application to shut down.
    #
    # An alternative approach would involve calling ::error and
    # benefitting from its backtrace generation, but there are various
    # places where it makes to sense to catch problems and call
    # synth::error rather than internal_error
    proc internal_error { msg } {
	puts stderr "ecosynth, an internal error has occurred:"
	puts stderr "    $msg"
	puts stderr "---------- backtrace -------------------------------------------------"
	for { set level [info level] } { $level > 0 } { incr level -1 } {
	    puts stderr [info level $level]
	}
	puts stderr "----------------------------------------------------------------------"
	puts stderr "ecosynth, exiting."
	exit 1
    }

    # Dummy implementations of the exported filter routines, in case a script
    # tries to create a filter even when not running in graphical mode
    variable _dummy_filters [list]
    
    proc filter_exists { name } {
	set result 0
	if { -1 != [lsearch -exact $synth::_dummy_filters $name] } {
	    set result 1
	}
	return $result
    }

    proc filter_get_list { } {
	return $synth::_dummy_filters
    }

    proc filter_add { name args } {
	if { [synth::filter_exists $name] } {
	    synth::internal_error "attempt to install filter $name twice.\n"
	}
	lappend synth::_dummy_filters $name
    }
}

# }}}
# {{{  Argument processing and global options                   

# ----------------------------------------------------------------------------
# Argument processing. The eCos application will usually just pass its
# command line arguments to the auxiliary. Four special arguments will
# have been examined already:
#
#    -io, --io
#        I/O facilities, i.e. the auxiliary should run
#    -ni, -nio, --ni, --nio
#        No I/O facilities, i.e. the auxiliary should not be run.
#    -nw, --nw, --no-windows
#        No windows, i.e. disable the GUI
#    -w, --w, --windows
#        Enable the GUI
#
# There are a number of additional flags available as standard:
#
#    -v, --version
#        The usual
#    -h, --help
#        Ditto
#    -k, --k, --keep-going
#        Ignore errors as much as possible
#    -nr, --no-rc
#        Skip the initrc and mainrc files
#    -x, --exit
#        The auxiliary should exit at the same time as the eCos application.
#    -nx, --no-exit
#        Inverse of the above
#    -V, --verbose
#        The usual
#    --debug
#        Not intended for end users
#    -l <file>, -l=<file>, --logfile <file>, --logfile=<file>
#        Send all output to the specified file. In GUI mode this is in addition
#        to the main text window. In non-GUI mode this is instead of stdout.
#    -t <file>, -t=<file>, --target <file>, --target=<file>
#        Specify the target definition file.
#
# Many X applications accept a common set of options, e.g. -display,
# -geometry, etc. Although usually Tk will process these, there are
# some problems - see ecosynth.c, ecosynth_appinit() for details.
# Hence -geometry has to be processed here.
#
#    -geometry <geom>
#
#
# Some device-specific scripts may want to support additional
# command line arguments. This is somewhat messy, since the core
# code has no way of knowing what devices might be available and
# hence what the actual valid arguments are. It would be possible
# to just ignore any arguments that are not used by any device,
# but that could really confuse a user who has made a typo. Instead
# the code below keeps track of which arguments have been "consumed",
# allowing it to issue a warning about unconsumed arguments after
# initialization.
#
# Arguments can take the following forms:
#
#    1) -flag or --flag.
#    2) -name=value or --name=value
#    3) -name value or --name value
#
# There is a possibility of confusion if any of the values begin with a hyphen.
# It is hard to do anything about this without advance knowledge of what all
# the valid arguments are. Instead the user can avoid problems by using
# the -name=value variant on the command line.
#
# There is also possible confusion if a single argument can occur multiple
# times. If that is permitted then things can get rather messy, and
# the current API does not really handle it.

namespace eval synth {

    # Keep track of all arguments which have not yet been consumed.
    array set _argv_unconsumed [list]
    for { set i 0 } { $i < $::argc } { incr i } {
	set synth::_argv_unconsumed($i) [lindex $::argv $i]
    }

    # Provide a list of just those arguments that have not yet
    # been consumed.
    proc argv_get_unconsumed { } {
	set result [list]
	for { set i 0 } { $i < $::argc } {incr i } {
	    if { [info exists synth::_argv_unconsumed($i)] } {
		lappend result $synth::_argv_unconsumed($i)
	    }
	}
	return $result
    }

    proc _argv_consume { index } {
	if { [info exists synth::_argv_unconsumed($index)] } {
	    unset synth::_argv_unconsumed($index)
	}
    }
    
    # Internal routine. Given a string of the form "-flag" or "-name=",
    # return an index within argv or -1 if not found. As a side effect
    # this "consumes" the argument.
    proc _argv_lookup { name } {
	set result -1
	if { "=" != [string index $name end] } {
	    for { set i 0 } { $i < $::argc } { incr i } {
		set arg [lindex $::argv $i]
		if { [string equal $arg $name] || [string equal $arg "-[set name]"] } {
		    set result $i
		    synth::_argv_consume $i
		    break
		}
	    }
	} else {
	    set name [string range $name 0 end-1]
	    set len  [string length $name]
	    for { set i 0 } { $i < $::argc } { incr i } {
		set arg [lindex $::argv $i]
		if { [string equal -length $len $arg $name] } {
		    if { "=" == [string index $arg $len] } {
			set result $i
			synth::_argv_consume $i
			break;
		    } elseif { ([string length $arg] == $len) && ($i < ($::argc - 1)) } {
			set result $i
			synth::_argv_consume $i
			synth::_argv_consume [expr $i + 1]
			break
		    }
		} elseif { [string equal -length [expr $len + 1] $arg "-[set name]"] } {
		    if { "=" == [string index $arg [expr $len + 1]] } {
			set result $i
			synth::_argv_consume $i
			break
		    } elseif { ([string length $arg] == [expr $len + 1]) && ($i < ($::argc - 1)) } {
			set result $i
			synth::_argv_consume $i
			synth::_argv_consume [expr $i + 1]
			break
		    }
		}
	    }
	}
	return $result
    }

    # Look for a given argument on the command line.
    proc argv_defined { name } {
	set result 0
	set index [synth::_argv_lookup $name]
	if { -1 != $index } {
	    set result 1
	}
	return $result
    }
    
    # Return the value associated with a given argument, which must be present.
    proc argv_get_value { name } {
	if { "=" != [string index $name end] } {
	    synth::internal_error "attempt to get a value for a simple flag argument \"$name\".\n"
	}
	set result ""
	set index [synth::_argv_lookup $name]
	if { -1 == $index } {
	    synth::internal_error "attempt to get the value associated with a non-existent argument \"$name\".\n"
	}
	set arg [lindex $::argv $index]
	set len [string length $name]
	if { [string equal -length $len $arg $name] } {
	    set result [string range $arg $len end]
	} elseif { [string equal -length [expr $len + 1] $arg "-[set name]"] } {
	    set result [string range $arg [expr $len + 1] end]
	} else {
	    set result [lindex $::argv [expr $index + 1]]
	}
	return $result
    }

    # -ni/-nio are not relevant. If present then they would have been handled
    # within the eCos application, the auxiliary would not have been spawned,
    # and this script would not be running.

    # -io will have been processed by the eCos application.
    # -nw, -w, and related options have been processed by the C code.
    # Look them up anyway to consume them.
    synth::_argv_lookup "-io"
    synth::_argv_lookup "-nw"
    synth::_argv_lookup "-no-windows"
    synth::_argv_lookup "-w"
    synth::_argv_lookup "-windows"

    # Now cope with the other standard flags
    if { [synth::argv_defined "-v"] || [synth::argv_defined "--version"] } {
	# Just output the version message and exit. The eCos application
	# will do the same. The version is obtained from configure.in,
	# and also from the install directory. The synthetic target
	# startup code will exit quietly if the auxiliary exits at this
	# stage. This output should go via puts rather than the
	# synth:: output routines, since the GUI will never appear if
	# --version is specified.
	puts "ecosynth: version $synth::_ecosynth_version"
	puts "        : install location [file dirname [info script]]"
	exit 0
    }

    if { [synth::argv_defined "-l="] } {
	set synth::logfile [synth::argv_get_value "-l="]
    } elseif { [synth::argv_defined "-logfile="] } {
	set synth::logfile [synth::argv_get_value "-logfile="]
    }
    if { "" != $synth::logfile } {
	synth::logfile_open
    }

    # -h/--help would normally also result in an immediate exit. However,
    # the device-specific scripts have not yet been loaded so there
    # is no way of reporting their options. Hence the usage information
    # is delayed until later. Suppressing GUI mode as a side effect is
    # probably a good idea as well, that way the output appears in the
    # current console window.
    if { [synth::argv_defined "-h"] || [synth::argv_defined "-help"] } {
	set synth::flag_help 1
	set synth::flag_gui 0
    }
    
    if { [synth::argv_defined "-debug"] } {
	set synth::flag_debug 1
    }
    
    if { [synth::argv_defined "-k"] || [synth::argv_defined "-keep-going"] } {
	set synth::flag_keep_going 1
    }

    if { [synth::argv_defined "-nr"] || [synth::argv_defined "-no-rc"]} {
	set synth::flag_no_rc 1
    }

    if { [synth::argv_defined "-x"] || [synth::argv_defined "-exit"] } {
	set synth::flag_immediate_exit 1
    } elseif { [synth::argv_defined "-nx"] || [synth::argv_defined "-no-exit"] } {
	set synth::flag_immediate_exit 0
    }

    if { [synth::argv_defined "-V"] || [synth::argv_defined "-verbose"] } {
	set synth::flag_verbose 1
    }

    if { [synth::argv_defined "-t="] } {
	set synth::target_definition [synth::argv_get_value "-t="]
    } elseif { [synth::argv_defined "-target="] } {
	set synth::target_definition [synth::argv_get_value "-target="]
    }

    # Arguably -geometry should only be checked when the GUI is enabled,
    # but doing so at all times is harmless.
    # Note that '-geometry ""' means that any value held in the
    # preferences file should be ignored. Hence the regexp below 
    # accepts the empty string, and treats it separately from
    # uninitialized.
    if { [synth::argv_defined "-geometry="] } {
	set synth::geometry [synth::argv_get_value "-geometry="]

	if { ![regexp -- {^([0-9]+x[0-9]+)?([+-][0-9]+[+-][0-9]+)?$} $synth::geometry] } {
	    synth::report_warning "Invalid geometry string $synth::geometry\n"
	    set synth::geometry "<none>"
	}
    }

    if { $synth::flag_debug } {
	synth::report \
	    "Results of initial command-line parsing:\n   \
	          --help            $synth::flag_help\n   \
	          --keep-going      $synth::flag_keep_going\n   \
	          --no-rc           $synth::flag_no_rc\n   \
	          --exit            $synth::flag_immediate_exit\n   \
	          --verbose         $synth::flag_verbose\n   \
	          logfile           $synth::logfile\n   \
	          target definition $synth::target_definition\n   \
	          geometry          $synth::geometry\n   \
	          unconsumed        [synth::get_unconsumed_args]\n"
    }
}

# }}}
# {{{  Create and populate ~/.ecos/synth                        

# ----------------------------------------------------------------------------
# If the per-user configuration directories do not exist yet, create them.
# Also install default initrc.tcl and mainrc.tcl files which do nothing, but
# which can be edited. If problems occur then the user gets a warning
# but execution proceeds.
#
# Some people may object to this automatic creation of directories and
# configuration files. However there is plenty of precedent, and the
# files involved are small. Messages are generated so that the user
# knows what has happened.
#
# Currently the default target definition file is not copied from
# the install tree into the per-user tree. Although some users will
# be happy having this file in ~/.ecos/synth, others may prefer it
# to be more visible in the current directory.

if { ![file exists "~/.ecos"] } {
    synth::report "Creating new directory ~/.ecos for eCos configuration files.\n"
    if { 0 != [catch { file mkdir "~/.ecos" }] } {
	synth::report_warning "failed to create directory ~/.ecos\n"
    }
}
if { [file exists "~/.ecos"] && [file isdirectory "~/.ecos"] && ![file exists "~/.ecos/synth"] } {
    synth::report "Creating new directory ~/.ecos/synth for synthetic target configuration files.\n"
    if { 0 != [catch { file mkdir "~/.ecos/synth" } ] } {
	synth::report_warning "failed to create directory ~/.ecos/synth\n"
    } else {
	# initrc and mainrc are only copied when the directory is first created,
	# so that users can delete them if unwanted - even though the
	# default versions do nothing.
	synth::report "Installing default configuration files ~/.ecos/synth/initrc.tcl and ~/.ecos/synth/mainrc.tcl\n"
	catch { file copy -- [file join $synth::install_dir "user_initrc.tcl"] "~/.ecos/synth/initrc.tcl"}
	catch { file copy -- [file join $synth::install_dir "user_mainrc.tcl"] "~/.ecos/synth/mainrc.tcl"}
    }
}

# }}}
# {{{  Read target definition file                              

# ----------------------------------------------------------------------------
# Once the GUI is up and running it is possible to start reading in some
# configuration files. The first of these is the target definition file.
# Typically this would be ~/.ecos/synth/default.tdf. An alternative
# definition file can be specified on the command line with the
# -t argument, and the code will look in the current directory,
# in ~/.ecos/synth, and in the install tree.
#
# The purpose of the target definition file is to specify exactly
# how I/O gets implemented. For example the eCos application may
# want to access a network device eth0, but that device could be
# implemented in a variety of ways (e.g. a real ethernet device
# on the Linux host, or a fake device provided by the ethertap
# facility). It is the target definition file that provides
# this information.
#
# The file is of course just another Tcl script, running in the
# current interpreter. There seems little point in using a safe
# interpreter given the considerable number of other Tcl scripts
# that are being used, some of which need the ability to e.g.
# run other programs. The main command is synth_device which
# takes two arguments, a device name and some options for that
# device, e.g.:
#
#    synth_device eth0 {
#        use eth1    
#    }
#
#    synth_device eth1 {
#        use tap0
#    }    
#
# When the eCos device driver looks up eth0 this will cause a
# device-specific Tcl script to be loaded, which can examine
# this data.
#
# This code has no way of knowing what constitutes valid or invalid
# contents for an eth0 device, especially since the Tcl script that
# could provide such knowledge has not been loaded. Instead it is
# assumed that the contents is another set of Tcl commands such as
# "physical", which will of course not be defined so the Tcl interpreter
# will invoke "unknown" which is temporarily redefined here. This makes
# it possible for the device-specific commands to have arbitrary number
# of arguments, or to define Tcl fragments for hooks, or whatever.
#
# As with argument processing, the code attempts to keep track of
# which devices and options have been "consumed" and can issue
# warnings about any unused devices or options. This helps to track
# down typos and similar problems. These warnings are only output
# when running at verbose mode, since it is fairly normal practice
# to have a single target definition file which supports 
# a number of different eCos applications with different I/O
# requirements.

namespace eval synth {
    # A list of all devices specified in the target definition file.
    # For now assume that a given device will only be specified once.
    variable _tdf_devices [list]
    
    # An array with additional details of each device. This will have
    # entries such as _tdf_device_options(eth0,4), where the second
    # argument is a per-device index. The value of each entry is
    # a list of the actual command and its arguments. This use of
    # an index allows for multiple instances of a per-device
    # option.
    array set _tdf_device_options [list]

    # While reading in the device details it is necessary to keep track
    # of the current device, if any. Otherwise the implementation of
    # "unknown" will not be able to update _tdf_device_options. An index
    # is needed as well.
    variable _tdf_current_device ""
    variable _tdf_current_index  0

    # Keep track of which devices and options have been consumed
    variable _tdf_consumed_devices [list]
    variable _tdf_consumed_options [list]
    
    proc tdf_has_device { name } {
	return [expr -1 != [lsearch -exact $synth::_tdf_devices $name]]
    }

    proc tdf_get_devices { } {
	return $synth::_tdf_devices
    }
    
    proc _tdf_get_option_index { devname option } {
	synth::_tdf_consume_device $devname
	for { set i 0 } { [info exists synth::_tdf_device_options($devname,$i)] } { incr i } {
	    if { $option == [lindex $synth::_tdf_device_options($devname,$i) 0] } {
		synth::_tdf_consume_option $devname $i
		return $i
	    }
	}
	return -1
    }
    
    proc _tdf_get_option_indices { devname option } {
	synth::_tdf_consume_device $devname
	set result [list]
	for { set i 0 } { [info exists synth::_tdf_device_options($devname,$i)] } { incr i } {
	    if { $option == [lindex $synth::_tdf_device_options($devname,$i) 0] } {
		synth::_tdf_consume_option $devname $i
		lappend result $i
	    }
	}
	return $result
    }
    
    proc tdf_has_option { devname option } {
	return [expr -1 != [synth::_tdf_get_option_index $devname $option]]
    }

    proc tdf_get_option { devname option } {
	set index [synth::_tdf_get_option_index $devname $option]
	if { -1 != $index } {
	    lappend synth::_tdf_consumed_options "$devname,$index"
	    return [lrange $synth::_tdf_device_options($devname,$index) 1 end]
	} else {
	    return [list]
	}
    }

    proc tdf_get_options { devname option } {
	set result [list]
	set indices [synth::_tdf_get_option_indices $devname $option]
	foreach index $indices {
	    lappend result [lrange $synth::_tdf_device_options($devname,$index) 1 end]
	}
	return $result
    }
    
    proc tdf_get_all_options { devname } {
	set result [list]
	for { set i 0 } { [info exists synth::_tdf_device_options($devname,$i)] } { incr i } {
	    lappend synth::_tdf_consumed_options "$devname,$index"
	    lappend result $synth::_tdf_device_options($devname,$i)
	}
	return $result
    }

    proc _tdf_consume_device { name } {
	if { -1 == [lsearch -exact $synth::_tdf_consumed_devices $name] } {
	    lappend synth::_tdf_consumed_devices $name
	}
    }

    proc _tdf_consume_option { devname index } {
	if { -1 == [lsearch -exact $synth::_tdf_consumed_options "$devname,$index"] } {
	    lappend synth::_tdf_consumed_options "$devname,$index"
	}
    }
    
    proc tdf_get_unconsumed_devices { } {
	set result [list]
	foreach devname $synth::_tdf_devices {
	    if { -1 == [lsearch -exact $synth::_tdf_consumed_devices $devname] } {
		lappend result $devname
	    }
	}
	return $result
    }

    proc tdf_get_unconsumed_options { } {
	set result [list]
	foreach devname $synth::_tdf_devices {
	    if { -1 == [lsearch -exact $synth::_tdf_consumed_devices $devname] } {
		# Do not report all the options for a device that has not been consumed at all
		continue
	    }
	    for { set i 0 } { [info exists synth::_tdf_device_options($devname,$i)] } { incr i } {
		if { -1 == [lsearch -exact $synth::_tdf_consumed_options "$devname,$i"] } {
		    lappend result [list $devname $synth::_tdf_device_options($devname,$i)]
		}
	    }
	}
	return $result
    }
}

# Look for the target definition file.
set _tdf $synth::target_definition
if { "" == $_tdf } {
    set _tdf "default"
}
set _config_file ""

set _dirlist [list [pwd] "~/.ecos/synth" $synth::install_dir]
foreach _dir $_dirlist {
    set _candidate "[file join $_dir $_tdf].tdf"  ; # file join does the right thing for absolute paths
    if { [file isfile $_candidate] } {
	set _config_file $_candidate
	break
    } else {
	set _candidate [file join $_dir $_tdf]
	if { [file isfile $_candidate] } {
	    set _config_file $_candidate
	    break
	}
    }
}
if { "" == $_config_file } {
    if { "" != $synth::target_definition } {
	# The user explicitly specified a file, so it must be read in.
	# If it cannot be found then that should be treated as an error.
	set msg "Unable to find target definition file $synth::target_definition\n"
	if { "absolute" !=  [file pathtype $synth::target_definition] } {
	    append msg "    Searched $_dirlist\n"
	}
	synth::report_error $msg
	exit 1
    } else {
	# This is a mild error, because default.tdf should be installed
	# below libexec. However the default file does not actually
        # define anything, it is just a set of comments, so there is
	# nothing to be gained by issuing a warning.
    }
} else {

    set synth::target_definition $_config_file
    
    proc synth_device { name data } {
	if { "" != $synth::_tdf_current_device } {
	    error "synth_device $name is nested inside $synth::_tdf_current_device\nNesting of synth_device entries is not allowed."
	}
	if { -1 != [lsearch -exact $synth::_tdf_devices $name] } {
	    error "Duplicate entry for synth_device $name"
	}
	set synth::_tdf_current_device $name
	set synth::_tdf_current_index 0
	lappend synth::_tdf_devices $name
	eval $data
	# If the eval resulted in an error, propagate it immediately rather than attempt
	# any form of recovery. The downside is that only error per run will be
	# reported.
	set synth::_tdf_current_device ""
    }
    rename unknown _synth_unknown
    proc unknown { args } {
	if { "" == $synth::_tdf_current_device } {
	    # An unknown command at the toplevel. Pass this to the
	    # original "unknown" command, in the unlikely event that
	    # the user really did want to autoload a library or do
	    # something similar. 
	    eval _synth_unknown $arg
	    return
	}

	# Anything else is treated as an option within the synth_device
	set synth::_tdf_device_options($synth::_tdf_current_device,$synth::_tdf_current_index) $args
	incr synth::_tdf_current_index
    }

    set _config_file_msg ""
    set _result [catch { source $_config_file } _config_file_msg ]
    
    rename unknown ""
    rename synth_device ""
    rename _synth_unknown unknown

    if { $_result } {
	# Any problems reading in the target definition file should be
	# treated as an error: I/O is unlikely to behave in the way
	# that the user expects.
	set msg "An error occurred while reading in the target definition file\n    $_config_file\n    $_config_file_msg\n"
	synth::report_error $msg
	exit 1
    }
    unset _result _config_file_msg
}

unset _dirlist _tdf _config_file _candidate

# }}}

if { $synth::flag_gui } {
# {{{  Main GUI code                                            

# {{{  Session file                                             

# ----------------------------------------------------------------------------
# The tool manages a file ~/.ecos/synth/guisession, holding information
# such as the size and position of the main window. The aim is to give
# the user a fairly consistent interface between sessions. The information
# is saved during exit handling, and also in response to the window
# manager WM_SAVE_YOURSELF request. However note that the latter does
# not extend to user session information - restarting the eCos application
# the next time a user logs in is inappropriate for eCos, plus if
# the application is being run inside gdb (a likely scenario) it is gdb
# that should handle restarting the application.
#
# Using a single file has limitations. Specifically the user may be
# switching between a number of different target definition files,
# each resulting in a subtly different layout, and arguably there
# should be separate session information for each one. However
# distinguishing between per-target and global settings would be
# very complicated.
#
# The most obvious implementation involves the options database.
#
# FIXME: implement properly

namespace eval synth {
    # Make sure we are using the right options from .Xdefaults etc.
    tk appname "ecosynth"

    if { $synth::flag_debug } {
	# synth::report "Reading in session file ~/.ecos/synth/guisession\n"
    }
    
    # synth::report_warning "Support for reading session file ~/.ecos/synth/guisession not yet implemented.\n"
    
    if { [file exists "~/.ecos/synth/guisession"] } {
	if {0 != [catch { option readfile "~/.ecos/synth/guisession" userDefault} msg]} {
	    # synth::report_error "Failed to read GUI session information from file ~/.ecos/synth/guisession\n    $msg\n"
	}
    }

    proc _update_session_file { arg_list } {
	# synth::report_warning "Support for updating session file ~/.ecos/synth/guisession not yet implemented.\n"
    }
    proc _handle_wm_save_yourself { } {
	# synth::report_warning "Support for WM_SAVE_YOURSELF not yet implemented\n"
    }

    synth::hook_add "exit" synth::_update_session_file
}

# }}}
# {{{  Load images                                              

# ----------------------------------------------------------------------------
# Load various useful bitmaps etc. into memory, so that they can be accessed
# by any code that needs them.
#
# Running1 is a coloured version of the eCos logo. running2 and running3 are
# used by alternative implementations of the heartbeat: running2 has the
# red and black reversed, and running3 is mostly a mirror image of the normal
# logo.
namespace eval synth {

    proc load_image { image_name filename } {
	set result 0
	set type [file extension $filename]
	if { ! [file exists $filename] } {
	    synth::report_error "Image $filename has not been installed.\n"
	} elseif { ! [file readable $filename] } {
	    synth::report_error "Image $filename is not readable.\n"
	} elseif { (".xbm" == $type) } {
	    if { 0 == [catch { set $image_name [image create bitmap -file $filename] }] } {
		set result 1
	    } else {
		synth::report_error "Bitmap image $filename is invalid.\n"
	    }
	} else {
	    if { 0 == [catch { set $image_name [image create photo -file $filename] }] } {
		set result 1
	    } else {
		synth::report_error "Image $filename is invalid.\n"
	    }
	}
	return $result
    }
    
    set _images [list "tick_yes.xbm" "tick_no.xbm" "save.xbm" "cut.xbm" "copy.xbm" "paste.xbm" \
	    "help.xbm" "running1.ppm" "saveall.xbm" ]
    foreach _image $_images {
	variable image_[file rootname $_image]
	synth::load_image "synth::image_[file rootname $_image]" [file join $synth::install_dir $_image]
    }
    unset _images _image
}

# }}}
# {{{  Balloon help                                             

namespace eval synth {

    variable _balloon_messages
    variable _balloon_pending ""
    
    toplevel .balloon
    label .balloon.info -borderwidth 2 -relief groove -background "light yellow"
    pack .balloon.info -side left -fill both -expand 1
    wm overrideredirect .balloon 1
    wm withdraw .balloon
    
    proc register_balloon_help { widget message } {
	set synth::_balloon_messages($widget) $message
	bind $widget <Enter> { synth::_balloonhelp_pending %W }
	bind $widget <Leave> { synth::_balloonhelp_cancel }
    }
    
    proc _balloonhelp_pending { widget } {
	synth::_balloonhelp_cancel
	set synth::_balloon_pending [after 1200 [list synth::_balloonhelp_show $widget]]
    }
    
    proc _balloonhelp_cancel { } {
	if { "" != $synth::_balloon_pending } {
	    after cancel $synth::_balloon_pending
	    set synth::_balloon_pending ""
	} else {
	    wm withdraw .balloon
	}
    }

    proc _balloonhelp_show { widget } {
	.balloon.info configure -text $synth::_balloon_messages($widget)
	set x [expr [winfo rootx $widget] + 2]
	set y [expr [winfo rooty $widget] + [winfo height $widget] + 2]
	wm geometry .balloon +$x+$y
	wm deiconify .balloon
	raise .balloon
	set synth::_balloon_pending ""
    }
}

# }}}
# {{{  Window manager settings                                  

# ----------------------------------------------------------------------------
# Set up the current program name in the title bar etc.

namespace eval synth {

    if { $synth::flag_debug } {
	synth::report "Performing required interactions with window manager\n"
    }

    # The toplevel is withdrawn during startup. It is possible that
    # some of the windows and other objects created initially will end
    # up being deleted again before the system is fully up and running,
    # and the event loop is entered before then to accept requests from
    # the eCos application. This could cause confusing changes. The
    # toplevel is displayed in response to the constructors-done request.
    wm withdraw .

    # For now disable all attempts to use the "send" command. Leaving it
    # enabled would introduce security problems.
    rename "::send" {}
    
    variable title "eCos synthetic target"
    if { "" != $synth::ecos_appname} {
	append synth::title ": $synth::ecos_appname ($synth::_ppid)"
    }
    wm title . $synth::title

    # Use the specified geometry, or that from the last session.
    # Obviously how well this works depends very much on the
    # window manager being used.
    set _geometry ""
    if { "" ==  $synth::geometry} {
	# Command line request to suppress the preferences. Revert
	# to a default size.
	set _geometry "640x480"
    } elseif { "<none>" == $synth::geometry } {
	# No command line option, use the value from the preferences file
	# FIXME: implement
	set _geometry "640x480"
    } else {
	# There was an explicit -geometry option on the command line. Use it.
	set synth::_geometry $synth::geometry
	if { [regexp -- {^([0-9]+x[0-9]+).*$} $synth::_geometry] } {
	    wm sizefrom . "user"
	}
	if { [regexp -- {^.*([+-][0-9]+[+-][0-9]+)$} $synth::_geometry] } {
	    wm positionfrom . "user"
	}
    }
    wm geometry . $synth::_geometry
    unset synth::_geometry

    set _file [file join $synth::install_dir "ecosicon.xbm"]
    if { [file readable $synth::_file] } {
	wm iconbitmap . "@$synth::_file"
    }
    set _file [file join $synth::install_dir "ecosiconmask.xbm"]
    if { [file readable $synth::_file] } {
	wm iconmask . "@$synth::_file"
    }
    unset synth::_file
    
    if { "" != $synth::ecos_appname } {
	wm iconname . "ecosynth: $synth::ecos_appname"
    } else {
	wm iconname . "ecosynth"
    }

    wm protocol . "WM_DELETE_WINDOW" synth::_handle_exit_request
    wm protocol . "WM_SAVE_YOURSELF" synth::_handle_wm_save_yourself
}

# }}}
# {{{  Exit and kill handling                                   

# ----------------------------------------------------------------------------
# Exit handling. The user may request program termination using various
# different ways:
#   1) File->Exit
#   2) ctrl-Q, the shortcut for the above
#   3) the Window Manager's delete-window request
#
# If eCos has already exited then the request can be handled straightaway.
# The invocation of exit will go via the exit hooks so appropriate
# clean-ups will take place.
#
# If eCos has not already exited then it is assumed that the user wants
# the eCos application to terminate as well as the GUI. This can be achieved
# via the interrupt subsystem. However, there is a risk that the application
# has crashed, or is blocked in gdb, or has interrupts permanently disabled,
# in which case it is not going to respond to the SIGIO. To allow for this
# a number of retries are attempted, and after five seconds of this the
# application is killed off forcibly.

namespace eval synth {
    
    variable _handle_exit_retries 0
    variable _handle_exit_after   ""
    
    proc _handle_exit_request { } {

	if { !$synth::ecos_running } {
	    exit 0
	}
	# Setting this flag causes ecosynth to exit immediately once
	# the application terminates.
	set synth::flag_immediate_exit 1

	# Now ask the application to exit
	synth::request_application_exit

	# Set up a timer to retry this
	if { "" == $synth::_handle_exit_after } {
	    set synth::_handle_exit_after [after 1000 synth::_handle_exit_timer]

	    # And output something so the user knows the request has been received
	    synth::report "Waiting for the eCos application to exit.\n"
	}
    }

    # This routine requests termination of eCos, but not of 
    # ecosynth
    proc _handle_kill_request { } {
	if { $synth::ecos_running } {
	    synth::request_application_exit
	    if { "" == $synth::_handle_exit_after } {
		set synth::_handle_exit_after [after 1000 synth::_handle_exit_timer]
	    }
	}
    }

    proc _handle_exit_timer { } {
	if { $synth::ecos_running } {
	    incr synth::_handle_exit_retries
	    if { $synth::_handle_exit_retries < 5 } {
		synth::request_application_exit
		synth::report "Still waiting for the eCos application to exit.\n"
	    } else {
		synth::_send_SIGKILL
	    }
	    set synth::_handle_exit_after [after 1000 synth::_handle_exit_timer]
	}
    }
}

# }}}
# {{{  Main window layout                                       

# ----------------------------------------------------------------------------
# The window layout is as follows:
#  1) a menu bar at the top (surprise surprise). Many of the menus will be
#     empty or nearly so, but device-specific scripts may want to extend
#     the menus.
#  2) a toolbar. This is primarily for use by device-specific scripts
#  3) a central grid.
#  4) a status line at the bottom.
#
# The central grid is organised as a 3x3 set of frames. The centre frame holds
# the main text display, plus associated scrollbars, and is the frame that
# will expand or shrink as the toplevel is resized. The remaining eight frames
# (nw, n, ne, e, se, s, sw, w) are available for use by device-specific
# scripts, typically under control of settings in the target definition file.
# It is very possible that some or all of these eight frames will be empty,
# and if an entire row or column is empty then Tk will make them invisible.
#
# Possible enhancements:
# 1) implement some sort of paning/resizing around the central text window.
#    That would provide some way of letting the user control the space
#    taken by device-specific subwindows. This would be implemented
#    by modifying the weights assigned to different rows/columns.
# 2) it would be very useful if the main text window could be split,
#    like emacs. This would require multiple text widgets, with output
#    being pasted in each one.
# 3) possibly the text window should not be hard-wired to the centre frame,
#    instead its frame could be selected by preferences somehow.

if { $synth::flag_debug } {
    synth::report "Creating main window layout\n"
}

# The various frames are generally accessed via variables

menu .menubar -borderwidth 1
menu .menubar.file
menu .menubar.edit
menu .menubar.view
menu .menubar.windows
menu .menubar.help

. configure -menu .menubar
.menubar add cascade -label "File"    -underline 0 -menu .menubar.file
.menubar add cascade -label "Edit"    -underline 0 -menu .menubar.edit
.menubar add cascade -label "View"    -underline 0 -menu .menubar.view
.menubar add cascade -label "Windows" -underline 0 -menu .menubar.windows
.menubar add cascade -label "Help"    -underline 0 -menu .menubar.help

.menubar.file add command -label "Save"        -underline 0 -accelerator "Ctrl-S" -command [list synth::_handle_file_save]
.menubar.file add command -label "Save As..."  -underline 5                       -command [list synth::_handle_file_save_as]
.menubar.file add command -label "Save All..." -underline 6                       -command [list synth::_handle_file_save_all]
.menubar.file add command -label "Kill eCos"   -underline 0                       -command [list synth::_handle_kill_request]
.menubar.file add command -label "Exit"        -underline 1 -accelerator "Ctrl-Q" -command [list synth::_handle_exit_request]
bind . <Control-KeyPress-q> [list synth::_handle_exit_request]
bind . <Control-KeyPress-s> [list synth::_handle_file_save]

# Once eCos has exited, the kill option should be disabled
namespace eval synth {
    proc _menubar_ecos_exit_clean { arg_list } {
	.menubar.file entryconfigure "Kill eCos" -state disabled
    }
    synth::hook_add "ecos_exit" synth::_menubar_ecos_exit_clean
}

frame .toolbar -borderwidth 1 -relief groove
if { "" != $synth::image_save } {
    button .toolbar.save    -image $synth::image_save -borderwidth 0 -command [list synth::_handle_file_save]
    pack .toolbar.save -side left -padx 2
    synth::register_balloon_help .toolbar.save "Save visible output"
}
if { "" != $synth::image_saveall } {
    button .toolbar.saveall -image $synth::image_saveall -borderwidth 0 -command [list synth::_handle_file_save_all]
    pack .toolbar.saveall -side left -padx 2
    synth::register_balloon_help .toolbar.saveall "Save all output"
}
if { "" != $synth::image_cut } {
    button .toolbar.cut    -image $synth::image_cut -borderwidth 0 -state disabled -command [list synth::_handle_edit_cut]
    pack .toolbar.cut -side left -padx 2
    synth::register_balloon_help .toolbar.cut "Cut"
}
if { "" != $synth::image_copy } {

    button .toolbar.copy    -image $synth::image_copy -borderwidth 0 -command [list synth::_handle_edit_copy]
    pack .toolbar.copy -side left -padx 2
    synth::register_balloon_help .toolbar.copy "Copy"
}
if { "" != $synth::image_paste } {
    button .toolbar.paste    -image $synth::image_paste -borderwidth 0 -state disabled -command [list synth::_handle_edit_paste]
    pack .toolbar.paste -side left -padx 2
    synth::register_balloon_help .toolbar.paste "Paste"
}
pack .toolbar -side top -fill x

frame .main
frame .main.nw -borderwidth 0
frame .main.n  -borderwidth 0
frame .main.ne -borderwidth 0
frame .main.e  -borderwidth 0
frame .main.se -borderwidth 0
frame .main.s  -borderwidth 0
frame .main.sw -borderwidth 0
frame .main.w  -borderwidth 0

frame .main.centre

frame .main.border_nw_n      -width 2 -background black -borderwidth 0
frame .main.border_n_ne      -width 2 -background black -borderwidth 0
frame .main.border_w_centre  -width 2 -background black -borderwidth 0
frame .main.border_centre_e  -width 2 -background black -borderwidth 0
frame .main.border_sw_s      -width 2 -background black -borderwidth 0
frame .main.border_s_se      -width 2 -background black -borderwidth 0
frame .main.border_nw_w      -height 2 -background black -borderwidth 0
frame .main.border_n_centre  -height 2 -background black -borderwidth 0
frame .main.border_ne_e      -height 2 -background black -borderwidth 0
frame .main.border_w_sw      -height 2 -background black -borderwidth 0
frame .main.border_centre_s  -height 2 -background black -borderwidth 0
frame .main.border_e_se      -height 2 -background black -borderwidth 0

text .main.centre.text -xscrollcommand [list .main.centre.horizontal set] -yscrollcommand [list .main.centre.vertical set]
scrollbar .main.centre.horizontal -orient horizontal -command [list .main.centre.text xview]
scrollbar .main.centre.vertical   -orient vertical -command [list .main.centre.text yview]
grid configure .main.centre.text -row 0 -column 0 -sticky news
grid configure .main.centre.vertical -row 0 -column 1 -sticky ns
grid configure .main.centre.horizontal -row 1 -column 0 -sticky ew
# Is there anything useful to be done in 1,1? e.g. a >> button to
# go directly to perform ".main.centre.text see end"

# Make sure that row 0 column 0, i.e. the text widget rather than the
# scrollbars, grows to fit all available space.
grid rowconfigure .main.centre 0 -weight 1
grid rowconfigure .main.centre 1 -weight 0
grid columnconfigure .main.centre 0 -weight 1
grid columnconfigure .main.centre 1 -weight 0

grid configure .main.nw              -row 0 -column 0 -sticky news
grid configure .main.border_nw_n     -row 0 -column 1 -sticky ns
grid configure .main.n               -row 0 -column 2 -sticky news
grid configure .main.border_n_ne     -row 0 -column 3 -sticky ns
grid configure .main.ne              -row 0 -column 4 -sticky news
grid configure .main.border_nw_w     -row 1 -column 0 -sticky ew
grid configure .main.border_n_centre -row 1 -column 1 -columnspan 3  -sticky ew
grid configure .main.border_ne_e     -row 1 -column 4 -sticky ew
grid configure .main.w               -row 2 -column 0 -sticky news
grid configure .main.border_w_centre -row 2 -column 1 -sticky ns
grid configure .main.centre          -row 2 -column 2 -sticky news
grid configure .main.border_centre_e -row 2 -column 3 -sticky ns
grid configure .main.e               -row 2 -column 4 -sticky news
grid configure .main.border_w_sw     -row 3 -column 0 -sticky ew
grid configure .main.border_centre_s -row 3 -column 1 -columnspan 3 -sticky ew
grid configure .main.border_e_se     -row 3 -column 4 -sticky ew
grid configure .main.sw              -row 4 -column 0 -sticky news
grid configure .main.border_sw_s     -row 4 -column 1 -sticky ns
grid configure .main.s               -row 4 -column 2 -sticky news
grid configure .main.border_s_se     -row 4 -column 3 -sticky ns
grid configure .main.se              -row 4 -column 4 -sticky news
grid columnconfigure .main 0 -weight 0
grid columnconfigure .main 1 -weight 0
grid columnconfigure .main 2 -weight 1
grid columnconfigure .main 3 -weight 0
grid columnconfigure .main 4 -weight 0
grid rowconfigure .main 0 -weight 0
grid rowconfigure .main 1 -weight 0
grid rowconfigure .main 2 -weight 1
grid rowconfigure .main 3 -weight 0
grid rowconfigure .main 4 -weight 0

# The .main frame should not be packed into the main window yet.
# Until all devices have been instantiated the various subwindows
# are not yet known, so the desired size of .main is not known
# either. Packing it too early and then adding more windows
# causes confusion.

# }}}
# {{{  Help                                                     

# ----------------------------------------------------------------------------
# Two main sources of documentation are of interest to the synthetic
# target. The first is the toplevel eCos documentation. The second
# is the documentation specific to the generic target. Device-specific
# scripts may want to add menu entries for their own documentation.
#
# The problems are:
#   1) where to find the documentation
#   2) how to view it?
#
# The documentation should be in the component repository. If there is
# a variable ECOS_REPOSITORY then that gives the appropriate information.
# Otherwise things get messy because the repository being used for
# eCos may not match the repository used when building the host-side
# support - the versions should match but the repository may have
# moved. Never the less that is the best we can do.
# NOTE: sources.redhat.com might provide another alternative, but the
# documentation is not organized in the same way as the repository.
#
# As for how to view the documentation, this is up to user preferences
# but ecosynth has built-in knowledge of three different viewers. 

namespace eval synth {

    if { $synth::flag_debug } {
	synth::report "Setting up help menu\n"
    }
    
    variable _browser1   "mozilla -remote openURL(%s)"
    variable _browser2   "mozilla %s"
    variable _browser3   "gnome-help-browser %s"
    variable _main_help  ""
    variable _synth_help ""
    set _repo ""
    
    if { [info exists env(ECOS_REPOSITORY)] } {
	set _repo $env(ECOS_REPOSITORY)
    } else {
	set _repo $synth::_ecos_repository
    }
    if { ![file exists [file join $_repo "ecos.db"]] } {
	synth::report_warning "Failed to locate eCos component repository.\n   \
		Please define an environment variable ECOS_REPOSITORY.\n"
    } else {
	# FIXME: this needs attention once the documentation is more sorted
	set synth::_main_help [file join $_repo "index.html"]
	if { ![file exists $synth::_main_help] } {
	    if { 0 } {
		synth::report_warning "Failed to locate toplevel documentation file $synth::_main_help\n   \
	                               Help->eCos menu option disabled.\n"
	    }
	    set synth::_main_help ""
	} else {
	    set synth::_main_help "file://$_main_help"
	}
	
	set synth::_synth_help [file join $_repo $synth::_ecosynth_package_dir $synth::_ecosynth_package_version "doc/hal-synth-arch.html"]
	if { ![file exists $synth::_synth_help] } {
	    synth::report_warning "Failed to locate synthetic target documentation $synth::_synth_help\n   \
		    Help->Synthetic target menu option disabled.\n"
	    set synth::_synth_help ""
	} else {
	    set synth::_synth_help "file://$_synth_help"
	}
    }

    if { "" != $_main_help } {
	.menubar.help add command -label "eCos" -command [list synth::_menu_help $synth::handle_help]
    } else {
	.menubar.help add command -label "eCos" -state disabled
    }
    if { "" != $_synth_help } {
	.menubar.help add command -label "Synthetic target" -command [list synth::handle_help "$synth::_synth_help"]
    } else {
	.menubar.help add command -label "Synthetic target" -state disabled
    }
    
    unset _repo
    
    proc handle_help { which } {
	set command [format $synth::_browser1 $which]
	if { 0 != [catch { eval exec -- "$command > /dev/null" } & ] } {
	    set command [format $synth::_browser2 $which]
	    if { 0 != [catch { eval exec -- "$command > /dev/null &" } ] } {
		set command [format $synth::_browser3 $which]
		if { 0 != [catch { eval exec -- "$command > /dev/null &"} ] } {
		    synth::report_warning "Unable to start a help browser.\n   Please check the settings in Edit->Preferences.\n"
		}
	    }
	}
    }

    # FIXME: add an about box as well.
}

# }}}
# {{{  Filters                                                  

# ----------------------------------------------------------------------------
# The central text window is expected to provide the bulk of the information
# to the user. This information can be voluminous, so filtering is desirable.
#
# There are two types of filters. System filters are provided by ecosynth
# itself and by device-specific scripts. For example ecosynth has a filter
# for warnings, and the console device has a filter for eCos trace messages.
# In addition users can specify their own filters using regular expressions,
# and those filters take priority. Note that there is an assumption that
# output is predominantly line-based: if partial lines get output then
# some confusion is possible.
#
# With tk the implementation is relatively straightforward: the text widget's
# tag facility does all the hard work of controlling how text gets displayed.
# It is possible to show or hide text using -elide, colours can be controlled
# using -background and -foreground, ... Not all of this functionality
# is made available to the user just yet.

namespace eval synth {
    # The bulk of the information is held in arrays, indexed by the name of
    # the filter. Lists are used to keep track of all valid names.
    variable _system_filter_list      [list]
    variable _system_filter_settings
    variable _user_filter_list        [list]
    variable _user_filter_settings

    # Does a given system filter already exist?
    proc filter_exists { name } {
	set result 0
	if { -1 != [lsearch -exact $synth::_system_filter_list $name] } {
	    set result 1
	}
	return $result
    }

    proc filter_get_list { } {
	return $synth::_system_filter_list
    }
    
    # Parsing support. All filters take a number of standard options:
    #
    #   -text "message"      - how to identify this filter to the user
    #   -hide [0|1]          - whether or not this text should be hidden by default
    #   -foreground <colour>
    #   -background <colour>
    #
    # The details of the currently supported options are known only to
    # filter_parse_options and filter_add, allowing new options such
    # as font manipulation to be added in future.
    #
    # There are two ways of adding a filter. filter_add is typically used
    # inside ecosynth.tcl with known good data. filter_add_parsed is
    # used with user-provided data, e.g. from the target definition file,
    # after a call to filter_validate.
    proc filter_parse_options { arg_list result_ref message_ref } {
	upvar 1 $result_ref result
	upvar 1 $message_ref message
	set message ""

	set text_set        0
	set hide_set        0
	set foreground_set  0
	set background_set  0
	
	set len [llength $arg_list]
	for { set i 0 } { $i < $len } { incr i } {
	    set arg [lindex $arg_list $i]
	    if { ( "-text" == $arg) ||
	         ( "-hide" == $arg) ||
	         ( "-foreground" == $arg) || ( "-fg" == $arg) ||
	         ( "-background" == $arg) || ( "-bg" == $arg) } {

		 incr i
                 if { $i >= $len } {
		     append message "    Missing data after argument $arg\n"
		 } else {
		     set data [lindex $arg_list $i]
		     if { "-text" == $arg } {
			 if { $text_set } {
			     append message "    Attempt to set -text option twice.\n"
			 } else {
			     set text_set 1
			     set result("-text") $data
			 }
		     } elseif { "-hide" == $arg } {
			 if { $hide_set } {
			     append message "    Attempt to set -hide option twice.\n"
			 } else {
			     set hide_set 1
			     if { ! [string is boolean -strict $data] } {
				 append message "    -hide should be given a boolean value, not \"$data\"\n"
			     } else {
				 set result("-hide") [expr $data ? 1 : 0]
			     }
			 }
		     } elseif { ( "-foreground" == $arg) || ( "-fg" == $arg ) } {
			 if { $foreground_set } {
			     append message "    Attempt to set -foreground twice.\n"
			 } else {
			     set foreground_set 1
			     # FIXME: is there some way of validating this color?
			     set result("-foreground") $data
			 }
		     } elseif { ( "-background" == $arg) || ( "-bg" == $arg ) } {
			 if { $background_set } {
			     append message "    Attempt to set -background twice.\n"
			 } else {
			     set background_set 1
			     # FIXME: is there some way of validating this color?
			     set result("-background") $data
			 }
		     }
		 }
	     } else {
		 append message "    Unknown option \"$arg\".\n"
	     }
	}

	if { "" == $message } {
	    return 1
	} else {
	    return 0
	}
    }

    # Add a new system filter, after the options have been parsed 
    proc filter_add_parsed { name data_ref } {
	upvar 1 $data_ref data

	set text       $name
	set hide       0
	set foreground "<default>"
	set background "<default>"
	if { [info exists data("-text")] } {
	    set text $data("-text")
	}
	if { [info exists data("-hide")] } {
	    set hide $data("-hide")
	}
	if { [info exists data("-foreground")] } {
	    set foreground $data("-foreground")
	} 
	if { [info exists data("-background")] } {
	    set background $data("-background")
	}
	
	if { $hide } {
	    .main.centre.text tag configure $name -elide 1
	} else {
	    .main.centre.text tag configure $name -elide 0
	}
	if { "<default>" == $foreground } {
	    .main.centre.text tag configure $name -foreground [.main.centre.text cget -foreground]
	} else {
	    set msg ""
	    if [catch {	.main.centre.text tag configure $name -foreground $foreground } msg ] {
		synth::report_warning "Unable to configure color \"$foreground\"\n    $msg\n"
		set foreground "<default>"
		.main.centre.text tag configure $name -foreground [.main.centre.text cget -foreground]
	    }
	}
	if { "<default>" == $background } {
	    .main.centre.text tag configure $name -background [.main.centre.text cget -background]
	} else {
	    set msg ""
	    if [catch {	.main.centre.text tag configure $name -background $background } msg ] {
		synth::report_warning "Unable to configure color \"$background\"\n    $msg\n"
		set background "<default>"
		.main.centre.text tag configure $name -background [.main.centre.text cget -background]
	    }
	}
	
	lappend synth::_system_filter_list $name
	set synth::_system_filter_settings($name,text)       $text
	set synth::_system_filter_settings($name,hide)       $hide
	set synth::_system_filter_settings($name,foreground) $foreground
	set synth::_system_filter_settings($name,background) $background

	# System tags should only get added during initialization. Hence the
	# first time the system filters window is brought up all filters
	# should be defined. However, just in case a new filter is added
	# in the middle of a run...
	if { [winfo exists .system_filters] } {
	    destroy .system_filters
	}
    }
    
    # Add a new system filter, performing the appropriate parsing.
    proc filter_add { name args } {

	if { [synth::filter_exists $name] } {
	    synth::internal_error "attempt to install filter $name twice.\n"
	}
	array set data [list]
	set   msg ""

	if { ![synth::filter_parse_options $args data msg] } {
	    # Any dubious arguments to the internal filter_add are treated as fatal.
	    synth::internal_error "unable to create new filter $name.\n$msg"
	} else {
	    filter_add_parsed $name data
	}
    }

    filter_add "report"  -text "ecosynth messages"
    filter_add "error"   -text "ecosynth errors"   -foreground red
    # amber is not a standard colour. Amber leds emit light in the range
    # 595-605 nm, corresponding to rgb values of approx. FF4200.
    # OrangeRed is close enough at FF4500
    filter_add "warning" -text "ecosynth warnings" -foreground OrangeRed

    # Bring up the system filters window, creating it if necessary.
    # Keeping the toplevel around but iconified/withdrawn when
    # unwanted means that properties such as size and position will
    # tend to be preserved.
    variable _system_filter_new_settings
    variable _system_filter_widgets
    
    proc _menu_view_system_filters { } {
	if { [winfo exists .system_filters] } {
	    if { "normal" == [wm state .system_filters] } {
		raise .system_filters
	    } else {
		wm deiconify .system_filters
	    }
	    return
	}
	toplevel .system_filters
	wm title .system_filters "ecosynth system filters"
	wm protocol .system_filters "WM_DELETE_WINDOW" [list synth::_menu_view_system_filters_cancel]
	wm group .system_filters .
	
	frame .system_filters.main
	label .system_filters.main.header1 -text "Filter"
	label .system_filters.main.header2 -text "Hide"
	label .system_filters.main.header3 -text "Foreground"
	label .system_filters.main.header4 -text "Background"
	set text_fg [.system_filters.main.header1 cget -foreground]
	frame .system_filters.main.row0 -height 1 -background $text_fg
	frame .system_filters.main.row2 -height 1 -background $text_fg
	frame .system_filters.main.col0 -width 1  -background $text_fg
	frame .system_filters.main.col2 -width 1  -background $text_fg
	frame .system_filters.main.col4 -width 1  -background $text_fg
	frame .system_filters.main.col6 -width 1  -background $text_fg
	frame .system_filters.main.col8 -width 1  -background $text_fg
	grid .system_filters.main.row0 -row 0 -column 0 -columnspan 9 -sticky ew
	grid .system_filters.main.header1 -row 1 -column 1 -sticky news
	grid .system_filters.main.header2 -row 1 -column 3 -sticky news
	grid .system_filters.main.header3 -row 1 -column 5 -sticky news
	grid .system_filters.main.header4 -row 1 -column 7 -sticky news
	grid .system_filters.main.row2 -row 2 -column 0 -columnspan 9 -sticky ew

	set row 3
	foreach filter $synth::_system_filter_list {
	    set synth::_system_filter_new_settings($filter,hide) $synth::_system_filter_settings($filter,hide)
	    set synth::_system_filter_new_settings($filter,foreground) $synth::_system_filter_settings($filter,foreground)
	    set synth::_system_filter_new_settings($filter,background) $synth::_system_filter_settings($filter,background)
	    
	    set synth::_system_filter_widgets($filter,label) \
		    [label .system_filters.main.filter_name_$row -text $synth::_system_filter_settings($filter,text)]
	    set synth::_system_filter_widgets($filter,hide) \
		    [checkbutton .system_filters.main.filter_hide_$row -borderwidth 2 -indicatoron false -selectcolor "" \
		    -image $synth::image_tick_no -selectimage $synth::image_tick_yes -variable synth::_system_filter_new_settings($filter,hide)]
	    set synth::_system_filter_widgets($filter,foreground) [button .system_filters.main.filter_foreground_$row -borderwidth 2 \
		    -command [list synth::_menu_view_system_filters_choose_foreground $filter]]
	    set synth::_system_filter_widgets($filter,background) [button .system_filters.main.filter_background_$row -borderwidth 2 \
		    -command [list synth::_menu_view_system_filters_choose_background $filter]]

	    grid .system_filters.main.filter_name_$row       -row $row -column 1 -sticky news
	    grid .system_filters.main.filter_hide_$row       -row $row -column 3 -sticky news
	    grid .system_filters.main.filter_foreground_$row -row $row -column 5 -sticky news
	    grid .system_filters.main.filter_background_$row -row $row -column 7 -sticky news

	    incr row
	    frame .system_filters.main.row$row -height 1 -background $text_fg
	    grid .system_filters.main.row$row -row $row -column 0 -columnspan 9 -sticky ew
	    incr row
	}
	grid .system_filters.main.col0 -row 0 -column 0 -rowspan $row -sticky ns
	grid .system_filters.main.col2 -row 0 -column 2 -rowspan $row -sticky ns
	grid .system_filters.main.col4 -row 0 -column 4 -rowspan $row -sticky ns
	grid .system_filters.main.col6 -row 0 -column 6 -rowspan $row -sticky ns
	grid .system_filters.main.col8 -row 0 -column 8 -rowspan $row -sticky ns

	for { set i 0 } { $i < $row } { incr i 2 } {
	    grid rowconfigure .system_filters.main $i -weight 0
	}
	for { set i 1 } { $i < $row } { incr i 2 } {
	    grid rowconfigure .system_filters.main $i -weight 1
	}
	for { set i 0 } { $i < 9 } { incr i 2 } {
	    grid columnconfigure .system_filters.main $i -weight 0
	}
	for { set i 1 } { $i < 9 } { incr i 2 } {
	    grid columnconfigure .system_filters.main $i -weight 1
	}
	
	pack .system_filters.main -side top -fill both -expand 1 -pady 4 -padx 4

	# FIXME: add try and revert buttons
	frame .system_filters.buttons
	button .system_filters.buttons.ok     -text "OK"     -command [list synth::_menu_view_system_filters_ok]
	button .system_filters.buttons.cancel -text "Cancel" -command [list synth::_menu_view_system_filters_cancel]
	pack .system_filters.buttons.ok .system_filters.buttons.cancel -side left -expand 1
	pack .system_filters.buttons -side bottom -fill x -pady 4

	frame .system_filters.separator -height 2 -borderwidth 1 -relief sunken
	pack .system_filters.separator -side bottom -fill x -pady 4

	bind .system_filters <KeyPress-Return> [list synth::_menu_view_system_filters_ok]
	bind .system_filters <KeyPress-Escape> [list synth::_menu_view_system_filters_cancel]

	synth::_menu_view_system_filters_reset
    }

    proc _menu_view_system_filters_reset { } {
	foreach filter $synth::_system_filter_list {
	    set synth::_system_filter_new_settings($filter,hide) $synth::_system_filter_settings($filter,hide)
	    set synth::_system_filter_new_settings($filter,foreground) $synth::_system_filter_settings($filter,foreground)
	    set synth::_system_filter_new_settings($filter,background) $synth::_system_filter_settings($filter,background)

	    set colour $synth::_system_filter_new_settings($filter,foreground)
	    if { "<default>" == $colour } {
		set colour [.system_filters.main.header1 cget -foreground]
	    }
	    $synth::_system_filter_widgets($filter,label) configure -foreground $colour 
	    $synth::_system_filter_widgets($filter,foreground) configure -background $colour -activebackground $colour

	    set colour $synth::_system_filter_new_settings($filter,background)
	    if { "<default>" == $colour } {
		set colour [.system_filters.main.header1 cget -background]
	    }
	    $synth::_system_filter_widgets($filter,label) configure -background $colour
	    $synth::_system_filter_widgets($filter,background) configure -background $colour -activebackground $colour
	}
    }

    # Change a colour. For now this involves calling Tk's chooseColor utility.
    # This is simple but not quite right: it would be much better to allow
    # the foreground and background to be modified in the same dialog, providing
    # immediate feedback on how the text will actually appear; it should also
    # provide some simple way of reverting to the default.
    proc _menu_view_system_filters_choose_foreground { filter } {
	set current_colour $synth::_system_filter_new_settings($filter,foreground)
	if { "<default>" == $current_colour } {
	    set current_colour [.system_filters.main.header1 cget -foreground]
	}
	set new_colour [tk_chooseColor -parent .system_filters -title "$synth::_system_filter_settings($filter,text) foreground" \
		-initialcolor $current_colour]
	if { "" != $new_colour } {
	    set synth::_system_filter_new_settings($filter,foreground) $new_colour
	    $synth::_system_filter_widgets($filter,label) configure -foreground $new_colour
	    $synth::_system_filter_widgets($filter,foreground) configure -background $new_colour -activebackground $new_colour
	}
    }
    
    proc _menu_view_system_filters_choose_background { filter } {
	set current_colour $synth::_system_filter_new_settings($filter,background)
	if { "<default>" == $current_colour } {
	    set current_colour [.system_filters.main.header1 cget -background]
	}
	set new_colour [tk_chooseColor -parent .system_filters -title "$synth::_system_filter_settings($filter,text) background" \
		-initialcolor $current_colour]
	if { "" != $new_colour } {
	    set synth::_system_filter_new_settings($filter,background) $new_colour
	    $synth::_system_filter_widgets($filter,label) configure -background $new_colour
	    $synth::_system_filter_widgets($filter,background) configure -background $new_colour -activebackground $new_colour
	}
    }
    
    proc _menu_view_system_filters_ok { } {
	wm withdraw .system_filters
	foreach filter $synth::_system_filter_list {
	    if { $synth::_system_filter_settings($filter,hide) != $synth::_system_filter_new_settings($filter,hide) } {
		set hide $synth::_system_filter_new_settings($filter,hide)
		set synth::_system_filter_settings($filter,hide) $hide
		if { $hide } {
		    .main.centre.text tag configure $filter -elide 1
		} else {
		    .main.centre.text tag configure $filter -elide 0
		}
	    }
	    if { $synth::_system_filter_settings($filter,foreground) != $synth::_system_filter_new_settings($filter,foreground) } {
		set foreground $synth::_system_filter_new_settings($filter,foreground)
		set synth::_system_filter_settings($filter,foreground) $foreground
		.main.centre.text tag configure $filter -foreground $foreground
	    }
	    if { $synth::_system_filter_settings($filter,background) != $synth::_system_filter_new_settings($filter,background) } {
		set background $synth::_system_filter_new_settings($filter,background)
		set synth::_system_filter_settings($filter,background) $background
		.main.centre.text tag configure $filter -background $background
	    }
	}
    }

    proc _menu_view_system_filters_cancel { } {
	wm withdraw .system_filters
	synth::_menu_view_system_filters_reset
    }

    # Now add a suitable entry to the View menu.
    .menubar.view add command -label "System filters..." -command [list synth::_menu_view_system_filters]

    # User filters.
    # FIXME: implement
    # .menubar.view add command -label "User filters..." -command [list synth::_menu_view_filters] -state disabled
}

# }}}
# {{{  Text window                                              

# ----------------------------------------------------------------------------
# The central text window is expected to provide the bulk of the information
# to the user. Various filtering mechanisms are desirable. For example the
# user should be able to control whether or not eCos trace messages are
# currently visible, not to mention other characteristics such as font
# and colours. The text widget's tag mechanism makes this relatively simple.

namespace eval synth {

    # Should the user be able to edit the text window, e.g. to add annotations?
    # This is disabled by default but can be enabled.
    variable flag_read_only 1
    
    # By default disable wrapping. Possibly it should be possible to
    # enable this on a per-tag basis.
    .main.centre.text configure -wrap "none"

    # Give the text widget the focus by default. That way operations
    # like page-up work immediately.
    focus .main.centre.text
    
    # If editing is currently disallowed, do not accept any input.
    # The code below is probably not quite sufficient, it is
    # ASCII-centric. A separate binding for Alt- sequences ensures
    # that the top-level menu processing continues to work.
    # Similarly a separate binding for Control- sequences ensures
    # that the shortcuts continue to work.
    bind .main.centre.text <Alt-KeyPress> {
	continue
    }
    bind .main.centre.text <Control-KeyPress> {
	continue
    }
    bind .main.centre.text <KeyPress> {
	if { !$synth::flag_read_only } {
	    continue
	} elseif { 0 != [llength %A] } {
	    break
	} elseif { ("Return" == "%K") || ("Tab" == "%K") || ("space" == "%K") } {
	    break
	} else {
	    continue
	}
    }
    # There are a few special control- bindings built in to the Tk text
    # widget which perform editing operations
    bind .main.centre.text <Control-KeyPress-h> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <Control-KeyPress-d> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <Control-KeyPress-k> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <Control-KeyPress-o> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <Control-KeyPress-t> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    
    # Implement support for the normal edit menu operations.
    # FIXME: add a search facility
    .menubar.edit insert end command -label "Cut"        -command [list synth::_handle_edit_cut]        -underline 2 -accelerator "Ctrl-X" -state disabled
    .menubar.edit insert end command -label "Copy"       -command [list synth::_handle_edit_copy]       -underline 0 -accelerator "Ctrl-C"
    .menubar.edit insert end command -label "Paste"      -command [list synth::_handle_edit_paste]      -underline 0 -accelerator "Ctrl-V" -state disabled
    .menubar.edit insert end command -label "Clear"      -command [list synth::_handle_edit_clear]      -underline 3 -accelerator "Del"    -state disabled
    .menubar.edit insert end command -label "Select All" -command [list synth::_handle_edit_select_all] -underline 9 -accelerator "Ctrl-A"
    .menubar.edit insert end checkbutton -label "Read Only" -variable synth::flag_read_only
    .menubar.edit insert end separator
    proc _trace_read_only { name1 name2 op } {
	if { !$synth::flag_read_only } {
	    .menubar.edit entryconfigure "Cut"   -state normal
	    .menubar.edit entryconfigure "Paste" -state normal
	    .menubar.edit entryconfigure "Clear" -state normal
	    .toolbar.cut configure -state normal
	    .toolbar.paste configure -state normal
	} else {
	    .menubar.edit entryconfigure "Cut"   -state disabled
	    .menubar.edit entryconfigure "Paste" -state disabled
	    .menubar.edit entryconfigure "Clear" -state disabled
	    .toolbar.cut configure -state disabled
	    .toolbar.paste configure -state disabled
	}
    }
    trace variable synth::flag_read_only "w" synth::_trace_read_only

    # Support for cut'n'paste etc. The widget does most of the hard
    # work, but this code has to distinguish between read-only and
    # read-write windows.
    #
    # Some operations such as clear may operate on everything in the
    # selection, including hidden text that happens to be in the
    # range. That may or may not be the right thing to do. It is right
    # if the intent is to get rid of all events during a period of
    # time, but wrong if the user wants to get rid of specific text.
    bind . <Control-KeyPress-x> [list synth::_handle_edit_cut]
    bind . <Control-KeyPress-c> [list synth::_handle_edit_copy]
    bind . <Control-KeyPress-v> [list synth::_handle_edit_paste]
    bind . <KeyPress-Delete>    [list synth::_handle_edit_clear]
    bind . <Control-KeyPress-a> [list synth::_handle_edit_select_all]
    
    bind .main.centre.text <<Paste>> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <<Cut>> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    bind .main.centre.text <<Clear>> {
	if { !$synth::flag_read_only } {
	    continue
	} else {
	    break
	}
    }
    
    proc _handle_edit_cut { } {
	event generate .main.centre.text "<<Cut>>"
    }

    proc _handle_edit_copy { } {
	event generate .main.centre.text "<<Copy>>"
    }

    proc _handle_edit_paste { } {
	event generate .main.centre.text "<<Paste>>"
    }

    proc _handle_edit_clear { } {
	event generate .main.centre.text "<<Clear>>"
    }

    proc _handle_edit_select_all { } {
	.main.centre.text tag add sel 1.0 "end - 1 chars"
    }

    # Most output to the text window goes through this routine. It inserts
    # some text with an appropriate tag. In addition it will ensure that
    # the new text is visible if appropriate, and if a logfile has been
    # specified then that will be updated as well.
    proc output { msg tag } {
	set ytail [lindex [.main.centre.text yview] 1]
	set xhead [lindex [.main.centre.text xview] 0]
	.main.centre.text insert end $msg $tag
	if { (1.0 == $ytail) && (0.0 == $xhead) } {
	    .main.centre.text see end
	}
	if { "" != $synth::_logfd } {
	    puts -nonewline $synth::_logfd $msg
	}
    }

    # Text output is now possible, so flush anything that is still buffered.
    # xview and yview may not give the right results until the window
    # is mapped, so always make the last text visible.
    set synth::flag_gui_ready 1
    synth::_flush_output
    .main.centre.text see end

    
    # Support for saving the current document. Save applies only to
    # the currently visible text. SaveAll gives the hidden text as
    # well.
    variable _savefile ""
    proc _handle_file_save { } {
	if { "" == $synth::_savefile } {
	    set synth::_savefile [tk_getSaveFile -parent .]
	    if { "" == $synth::_savefile } {
		return
	    }
	}
	set msg ""
	if { 0 != [catch { set fd [open $synth::_savefile "w"] } msg] } {
	    synth::report_error "$msg\n"
	    if { $synth::_system_filter_settings(error,hide) } {
		tk_messageBox -type "ok" -icon "error" -parent . -message "$msg\n"
	    }
	    return
	}
	set number_lines [expr int([.main.centre.text index end])]
	for { set i 1 } { $i < $number_lines } { incr i } {
	    set tags [.main.centre.text tag names "[set i].0"]
	    if {[llength $tags] > 0 } {
		set tag [lindex $tags 0]
		if { [info exists synth::_system_filter_settings($tag,hide)] &&
		     $synth::_system_filter_settings($tag,hide) } {
		    continue
		}
	    }
	    puts $fd [.main.centre.text get "[set i].0" "[set i].end"]
	}
	close $fd
    }

    proc _handle_file_save_as { } {
	set new_savefile [tk_getSaveFile -parent .]
	if { "" == $new_savefile } {
	    return
	}
	set synth::_savefile $new_savefile
	synth::_handle_file_save
    }

    proc _handle_file_save_all { } {
	set new_savefile [tk_getSaveFile -parent .]
	if { "" == $new_savefile } {
	    return
	}
	set msg ""
	if { 0 != [catch { set fd [open $new_savefile "w"] } msg] } {
	    synth::report_error "$msg\n"
	    if { $synth::_system_filter_settings(error,hide) } {
		tk_messageBox -type "ok" -icon "error" -parent . -message "$msg\n"
	    }
	    return
	}
	puts -nonewline $fd [.main.centre.text get 1.0 end]
	close $fd
    }
}

# }}}
# {{{  Heartbeat and status                                     

# ----------------------------------------------------------------------------
# This code manages a status line at the bottom of the main window.
# This involves a little heartbeat window, a label with the
# text Running or Exited, some padding, and an additional status
# line for use by other code.
#
# A variety of heartbeats have been attempted. The current one is
# still not very good, but will do for now. Others are if 0'd out.
# Note that these others may require additional images to be
# preloaded.

namespace eval synth {
    frame .status -borderwidth 1 -relief groove
    
    if { 1 } {
	# The eCos logo, bouncing horizontally
	variable _heartbeat_image_width [image width $synth::image_running1]
	variable _heartbeat_offset 0
	variable _heartbeat_ltor   1

	frame .status.heartbeat -width $synth::_heartbeat_image_width -height [image height $synth::image_running1]
	pack  .status.heartbeat -side left
	label .status.heartbeat.image -image $synth::image_running1 -anchor w -borderwidth 0
	place .status.heartbeat.image -x $synth::_heartbeat_offset -y 0

	proc _heartbeat_update { } {
	    if { ! $synth::ecos_running } {
		place configure .status.heartbeat.image -x 0 -y 0
	    } else {
		if { $synth::_heartbeat_ltor } {
		    incr synth::_heartbeat_offset 4
		} else {
		    incr synth::_heartbeat_offset -4
		}
		place configure .status.heartbeat.image -x $synth::_heartbeat_offset

		if { $synth::_heartbeat_offset < (5 - $synth::_heartbeat_image_width) } {
		    set synth::_heartbeat_ltor 1
		} elseif { $synth::_heartbeat_offset > ( $synth::_heartbeat_image_width -5) } {
		    set synth::_heartbeat_ltor 0
		}
		after 100 synth::_heartbeat_update
	    }
	}
	after 100 synth::_heartbeat_update
	
    } elseif { 0 } {
	# The eCos logo, alternating between a normal and an inverse version
	variable _heartbeat_image_width [image width $synth::image_running1]
	variable _heartbeat_inverse ""
	variable _heartbeat_normal  ""
	variable _heartbeat_inverse_width     1
	variable _heartbeat_normal_width      1
	
	canvas .status.heartbeat_canvas -width [image width $synth::image_running1] -height [image height $synth::image_running1]
	pack .status.heartbeat_canvas -side left
	label .status.heartbeat_canvas.background -image $synth::image_running1 -anchor w -borderwidth 0
	label .status.heartbeat_canvas.inverse    -image $synth::image_running2 -anchor w -borderwidth 0
	label .status.heartbeat_canvas.normal     -image $synth::image_running1 -anchor w -borderwidth 0
	.status.heartbeat_canvas create window 0 0 -anchor nw -window .status.heartbeat_canvas.background
	set synth::_heartbeat_inverse [.status.heartbeat_canvas create window 0 0 -anchor nw -window .status.heartbeat_canvas.inverse]
	raise .status.heartbeat_canvas.inverse .status.heartbeat_canvas.background
	set synth::_heartbeat_normal  [.status.heartbeat_canvas create window 0 0 -anchor nw -window .status.heartbeat_canvas.normal]
	raise .status.heartbeat_canvas.normal .status.heartbeat_canvas.inverse

	.status.heartbeat_canvas itemconfigure $synth::_heartbeat_inverse -width $synth::_heartbeat_inverse_width
	.status.heartbeat_canvas itemconfigure $synth::_heartbeat_normal  -width $synth::_heartbeat_normal_width

	proc _heartbeat_update { } {
	    if { ! $synth::ecos_running } {
		.status.heartbeat_canvas delete $synth::_heartbeat_inverse
		.status.heartbeat_canvas delete $synth::_heartbeat_normal
	    } else {
		if { $synth::_heartbeat_inverse_width < $synth::_heartbeat_image_width } {
		    incr synth::_heartbeat_inverse_width 2
		    .status.heartbeat_canvas itemconfigure $synth::_heartbeat_inverse -width $synth::_heartbeat_inverse_width
		} elseif { $synth::_heartbeat_normal_width < $synth::_heartbeat_image_width } {
		    incr synth::_heartbeat_normal_width 2
		    .status.heartbeat_canvas itemconfigure $synth::_heartbeat_normal -width $synth::_heartbeat_normal_width
		} else {
		    set synth::_heartbeat_inverse_width 1
		    set synth::_heartbeat_normal_width 1
		    .status.heartbeat_canvas itemconfigure $synth::_heartbeat_inverse -width $synth::_heartbeat_inverse_width
		    .status.heartbeat_canvas itemconfigure $synth::_heartbeat_normal  -width $synth::_heartbeat_normal_width
		}
		after 100 synth::_heartbeat_update
	    }
	}
	after 100 synth::_heartbeat_update
    
    } elseif { 0 } {
	# The eCos logo moving left to right, then replaced by a slightly smaller
	# mirror version moving right to left, sort of as if rotating around a torus
	variable _heartbeat_image_width [image width $synth::image_running1]
	variable _heartbeat_offset [expr -1 * [image width $synth::image_running1]]
	variable _heartbeat_ltor 1
	
	frame .status.heartbeat -width $synth::_heartbeat_image_width -height [image height $synth::image_running1]
	pack  .status.heartbeat -side left
	label .status.heartbeat.label -image $synth::image_running1 -anchor w -borderwidth 0

	place .status.heartbeat.label -x $synth::_heartbeat_offset -y 0

	proc _heartbeat_update { } {
	    if { ! $synth::ecos_running } {
		.status.heartbeat.label configure -image $synth::image_running1
		place configure .status.heartbeat.label -x 0
	    } else {
		if { $synth::_heartbeat_ltor } {
		    incr synth::_heartbeat_offset 4
		} else {
		    incr synth::_heartbeat_offset -4
		}
		place configure .status.heartbeat.label -x $synth::_heartbeat_offset
		if { $synth::_heartbeat_offset < (0 - $synth::_heartbeat_image_width) } {
		    .status.heartbeat.label configure -image $synth::image_running1
		    set synth::_heartbeat_ltor 1
		} elseif { $synth::_heartbeat_offset > $synth::_heartbeat_image_width } {
		    .status.heartbeat.label configure -image $synth::image_running3
		    set synth::_heartbeat_ltor 0
		}
		after 100 synth::_heartbeat_update
	    }
	}
	after 100 synth::_heartbeat_update
    }

    label .status.running -width 10 -text "Running" -anchor w
    pack .status.running -side left
    proc _heartbeat_exit_hook { arg_list } {
	.status.running configure -text "Exited"
    }
    synth::hook_add "ecos_exit" synth::_heartbeat_exit_hook
    
    label .status.text -text "" -anchor w
    pack .status.text -side left -fill x -expand 1
    pack .status -side bottom -fill x
}

# }}}
# {{{  Preferences                                              

namespace eval synth {

    if { $synth::flag_debug } {
	synth::report "Setting up preferences window.\n"
    }

    variable _pref_browser1 ""
    variable _pref_browser2 ""
    variable _pref_browser3 ""
    
    toplevel .preferences
    wm title .preferences "ecosynth preferences"
    wm withdraw .preferences
    wm protocol .preferences "WM_DELETE_WINDOW" [list synth::_menu_edit_preferences_cancel]
    wm group .preferences .

    # NOTE: the fixed-size padx/pady arguments should probably be determined 
    # using a font calculation. The fixed width for the column 0 entries is also
    # a cheat.
    set _pref_col0_width 24
    
    frame .preferences.help
    frame .preferences.help.frame -borderwidth 2 -relief groove
    pack .preferences.help.frame -fill both -expand 1 -pady 10
    frame .preferences.help.frame.blank -height 10
    label .preferences.help.frame.label1 -text "Preferred browser"  -width $synth::_pref_col0_width -anchor w
    label .preferences.help.frame.label2 -text "First alternative"  -width $synth::_pref_col0_width -anchor w
    label .preferences.help.frame.label3 -text "Second alternative" -width $synth::_pref_col0_width -anchor w
    entry .preferences.help.frame.entry1 -width 40 -relief sunken -textvariable synth::_pref_browser1
    entry .preferences.help.frame.entry2 -width 40 -relief sunken -textvariable synth::_pref_browser2
    entry .preferences.help.frame.entry3 -width 40 -relief sunken -textvariable synth::_pref_browser3
    grid .preferences.help.frame.blank -row 0 -column 0
    grid .preferences.help.frame.label1  -row 1 -column 0 -sticky w
    grid .preferences.help.frame.label2  -row 2 -column 0 -sticky w
    grid .preferences.help.frame.label3  -row 3 -column 0 -sticky w
    grid .preferences.help.frame.entry1  -row 1 -column 1 -sticky ew
    grid .preferences.help.frame.entry2  -row 2 -column 1 -sticky ew
    grid .preferences.help.frame.entry3  -row 3 -column 1 -sticky ew
    grid columnconfigure .preferences.help.frame 0 -weight 0
    grid columnconfigure .preferences.help.frame 1 -weight 1
    
    label .preferences.help.title -text "Help"
    place .preferences.help.title -in .preferences.help.frame -relx .1 -x -5 -y -10 -bordermode outside
    pack .preferences.help -fill both -expand 1 -padx 10

    frame .preferences.buttons
    button .preferences.buttons.ok     -text "OK"     -command [list synth::_menu_edit_preferences_ok]
    button .preferences.buttons.cancel -text "Cancel" -command [list synth::_menu_edit_preferences_cancel]
    pack .preferences.buttons.ok .preferences.buttons.cancel -side left -expand 1
    pack .preferences.buttons -side bottom -fill x -pady 4

    frame .preferences.separator -height 2 -borderwidth 1 -relief sunken
    pack .preferences.separator -side bottom -fill x -pady 4

    bind .preferences <KeyPress-Return> [list synth::_menu_edit_preferences_ok]
    bind .preferences <KeyPress-Escape> [list synth::_menu_edit_preferences_cancel]

    variable _saved_focus ""
    proc _menu_edit_preferences { } {
	set synth::_saved_focus [focus]
	set synth::_pref_browser1 $synth::_browser1
	set synth::_pref_browser2 $synth::_browser2
	set synth::_pref_browser3 $synth::_browser3
	if { "normal" == [wm state .preferences] } {
	    raise .preferences
	} else {
	    wm deiconify .preferences
	}
	focus .preferences.help.frame.entry1
    }

    proc _menu_edit_preferences_ok { } {
	if { $synth::_browser1 != $synth::_pref_browser1 } {
	    set synth::_browser1 $synth::_pref_browser1
	}
	if { $synth::_browser2 != $synth::_pref_browser2 } {
	    set synth::_browser2 $synth::_pref_browser2
	}
	if { $synth::_browser3 != $synth::_pref_browser3 } {
	    set synth::_browser3 $synth::_pref_browser3
	}

	wm withdraw .preferences
	catch { focus $synth::_saved_focus }
    }

    proc _menu_edit_preferences_cancel { } {
	wm withdraw .preferences
	catch { focus $synth::_saved_focus }
    }
    
    .menubar.edit add command -label "Preferences..." -command [list synth::_menu_edit_preferences]
}

# }}}
# {{{  Clean-up                                                 

# ----------------------------------------------------------------------------
# GUI clean-up.
#
# Once all the device-specific scripts have been loaded and initialized, it
# is time to go through the various components of the GUI and clean up 
# anything that is not actually required.
namespace eval synth {

    proc _cleanup_gui { } {

	if { $synth::flag_debug } {
	    synth::report "Cleaning up unused GUI items.\n"
	}
	
	# File, Edit, View and Help should always have contents, unless
	# the user has deleted entries via the mainrc file. The Windows
	# menu will be empty unless contents have been added. There is
	# always a global binding for ctrl-Q, and the window manager
	# should always provide a way of killing off the application,
	# so there is no need to treat File specially.
	if { 0 == [.menubar.file index end] } {
	    .menubar delete "File"
	}
	if { 0 == [.menubar.edit index end] } {
	    .menubar delete "Edit"
	}
	if { 0 == [.menubar.view index end] } {
	    .menubar delete "View"
	}
	if { 0 == [.menubar.windows index end] } {
	    .menubar delete "Windows"
	}
	if { 0 == [.menubar.help index end] } {
	    .menubar delete "Help"
	}

	# If the toolbar is empty get rid of it.
	if { 0 == [llength [winfo children .toolbar]] } {
	    pack forget .toolbar
	    destroy .toolbar
	}
	
	set can_destroy [list]
	# Remove some or all of the top, left hand, right hand or bottom
	# sets of frames, if nobody is using them.
	if { (0 == [llength [pack slaves .main.nw]]) &&
	     (0 == [llength [pack slaves .main.n]]) &&
	     (0 == [llength [pack slaves .main.ne]]) } {
            lappend can_destroy .main.nw .main.border_nw_n .main.n .main.border_n_ne .main.ne
	    lappend can_destroy .main.border_nw_w .main.border_n_centre .main.border_ne_e
        }
	if { (0 == [llength [pack slaves .main.nw]]) &&
	     (0 == [llength [pack slaves .main.w]]) &&
	     (0 == [llength [pack slaves .main.sw]]) } {
            lappend can_destroy .main.nw .main.border_nw_w .main.w .main.border_w_sw .main.sw
	    lappend can_destroy .main.border_nw_n .main.border_w_centre .main.border_w_sw
        }
	if { (0 == [llength [pack slaves .main.ne]]) &&
	     (0 == [llength [pack slaves .main.e]]) &&
	     (0 == [llength [pack slaves .main.se]]) } {
            lappend can_destroy .main.ne .main.border_ne_e .main.e .main.border_e_se .main.se
	    lappend can_destroy .main.border_n_ne .main.border_centre_e .main.border_s_se
        }
	if { (0 == [llength [pack slaves .main.sw]]) &&
	     (0 == [llength [pack slaves .main.s]]) &&
	     (0 == [llength [pack slaves .main.se]]) } {
            lappend can_destroy .main.sw .main.border_sw_s .main.s .main.border_s_se .main.se
	    lappend can_destroy .main.border_w_sw .main.border_centre_s .main.border_e_se
        }

 	foreach frame [lsort -unique $can_destroy] {
	    grid forget $frame
	}
 	foreach frame [lsort -unique $can_destroy] {
	    destroy $frame
	}

	# Now that the full window layout is known the .main frame can be
	# packed. Doing this before now could cause problems because the
	# desired sizes of the subwindows are not known.
	pack .main -expand 1 -fill both
    }
}

# }}}
# {{{  Screen dump support                                      

# Create screen dumps for the main window or for various subwindows.
# Normally disabled, but useful when generating documentation.
# FIXME: there seem to be problems getting the desired info about
# transient windows, e.g. sizes. Hence the generated dumps still
# require a lot of hand editing for now.
if { 0 } {

    bind . <Alt-w> {
	exec xwd -out main.xwd -frame -id [winfo id .]
    }

    bind . <Alt-f> {
	.menubar invoke "File"
	after 100 exec xwd -out menu_file.xwd -frame -id [winfo id .]
    }

    bind . <Alt-e> {
	.menubar invoke "Edit"
	after 100 exec xwd -out menu_edit.xwd -frame -id [winfo id .]
    }

    bind . <Alt-v> {
	.menubar invoke "View"
	after 100 exec xwd -out menu_view.xwd -frame -id [winfo id .]
    }

    # The Help menu will extend beyond the window boundaries
    bind . <Alt-h> {
	.menubar invoke "Help"
	after 100 exec xwd -out menu_help.xwd -root
    }
}

# }}}

# }}}
}

# {{{  Device instantiation                                     

# ----------------------------------------------------------------------------
# This code handles the loading of device-specific scripts in response
# to requests from the eCos application, and the instantiation of devices.
# The application's request provides four pieces of information, held in
# null-terminated strings in the request buffer:
#
#   package name      e.g. hal/synth/arch
#   package version   e.g. current
#   device type       e.g. console or ethernet
#   device instance   e.g. eth0, or an empty string
#   device data       e.g. 1024x768 for frame buffer resolution
#
# The first two pieces of information can be concatenated to give a
# path to the install location. The third identifies a suitable
# tcl script, e.g. console.tcl. This is sufficient to locate and load
# the tcl script. It should return an instantiation procedure which will
# be invoked with the instance name (or an empty string if there will only
# ever be one instance of this device type). The instantiation procedure
# will then be called with a number and the device instance string, and
# should return a handler for all requests intended for that device.
#
# If the package name and version are empty strings then an application-specific
# device is being initialized, and the code will search in the current
# directory and in ~/.ecos/synth

namespace eval synth {
    # Map package/version/type on to an instantiation procedure
    array set _instantiation_procs [list]

    # Map device instances on to handlers.
    array set _device_handlers [list]
    array set _device_names    [list]
    variable _next_device_id 1

    # Let scripts know their install location and their source dir
    variable device_install_dir ""
    variable device_src_dir     ""
    
    # One handler is predefined.
    set synth::_device_handlers(0)    synth::_handle_ecosynth_requests
    set synth::_device_names(0)       "ecosynth I/O auxiliary"

    proc _handle_INSTANTIATE { data } {

	set list [split $data \0]
	if { [llength $list] < 5 } {
	    synth::send_reply -1 0 ""
	    return
	}
	set package_dir     [lindex $list 0]
	set package_version [lindex $list 1]
	set device_type     [lindex $list 2]
	set device_instance [lindex $list 3]
	set device_data     [lindex $list 4]

	if { ![info exists synth::_instantiation_procs($package_dir,$package_version,$device_type)] } {
	    # The required script has not yet been loaded.
	    if { "" != $package_dir } {
		# The device is provided by a package
		set synth::device_install_dir [file join $synth::_ecosynth_libexecdir "ecos" $package_dir $package_version]
		set synth::device_src_dir     [file join $synth::_ecosynth_repository $package_dir $package_version]
	    
		set script [file join $::synth::device_install_dir "[set device_type].tcl"]
		if { ![file exists $script] } {
		    synth::report_error "Unable to initialize device $device_type\n    Script \"$script\" not found.\n"
		    synth::send_reply -1 0 ""
		    return
		} elseif { ![file readable $script] } {
		    synth::report_error "Unable to initialize device $device_type\n    Script \"$script\" not readable.\n"
		    synth::send_reply -1 0 ""
		    return
		}

		# Is there a more recent version in the repository
		if { [info exists ::env(ECOSYNTH_DEVEL)] } {
		    set _orig_name [file join $synth::device_src_dir "host" "[set device_type].tcl"]
		    if { [file exists $_orig_name] && [file readable $_orig_name] } {
			if { [file mtime $_orig_name] >= [file mtime $script] } {
			    puts "$_orig_name is more recent than install: executing that."
			    set script $_orig_name
			}
		    }
		}
	    } else {
		# The device is application-specific
		set script [file join [pwd] "[set device_type].tcl"]
		if { ![file exists $script] || ![file readable $script] } {
		    set script [file join "~/.ecos/synth" "[set device_type].tcl"]
		    if { ![file exists $script] || ![file readable $script] } {
			synth::report_error "Unable to initialize device $device_type\n    Script $device_type.tcl not found in [pwd] or ~/.ecos/synth\n"
			synth::send_reply -1 0 ""
			return
		    }
		}
	    }
	    
	    # The uplevel ensures that the device script operates at the global
	    # level, so any namespaces it creates are also at global level
	    # and not nested inside synth. This avoids having to add
	    # synth:: to lots of variable accesses and generally avoids confusion
	    set result [catch { uplevel #0 source $script } instantiator]
	    if { 0 != $result } {
		synth::report_error "Unable to initialize device $device_type\n  Error loading script \"$script\"\n  $instantiator\n"
		synth::send_reply -1 0 ""
		return
	    }

	    set synth::_instantiation_procs($package_dir,$package_version,$device_type) $instantiator
	}

	set handler [$synth::_instantiation_procs($package_dir,$package_version,$device_type) \
		$synth::_next_device_id $device_instance $device_data]
	if { "" == $handler } {
	    synth::send_reply -1 0 ""
	} else {
	    set result $synth::_next_device_id
	    incr synth::_next_device_id
	    
	    set synth::_device_handlers($result) $handler
	    if { "" != $device_instance } {
		set synth::_device_names($result) $device_instance
	    } else {
		set synth::_device_names($result) $device_type
	    }
	    synth::send_reply $result 0 ""
	}
    }
}

# }}}
# {{{  Interrupt handling                                       

# ----------------------------------------------------------------------------
# Interrupt handling. Device handlers can request an interrupt number
# using allocate_interrupt, and typically they will transmit this
# number to the eCos device driver during initialization. Device handlers
# can at any time call raise_interrupt with that number, which typically
# will result in SIGIO being sent to the eCos application. The latter will
# send a request to retrieve a mask of current pending interrupts.
#
# Exit handling, in the sense of the user selecting File->Exit, is also
# handled here. Such an exit request also involves raising SIGIO and
# then sending a specially format response to the get-pending request.

namespace eval synth {
  
    # The next interrupt number to be allocated. Interrupt source 0 is reserved
    # for the timer, which is handled within eCos itself via SIGALRM
    # rather than by the I/O auxiliary.
    variable _interrupt_next 1
    
    # Keep track of which interrupts belong to which devices, for display and
    # diagnostic purposes.
    array set _interrupt_names [list]
    set _interrupt_names(0) "system clock"
    
    # A mask of current pending interrupts
    variable _interrupt_pending 0

    # Is an exit request pending?
    variable _interrupt_exit_pending 0

    # Allow other code to hook into the interrupt system, e.g. to display
    # pending interrupts.
    synth::hook_define "interrupt"
    
    # For now interrupts are always allocated dynamically, which effectively
    # means in the order of C++ static constructors. This means that interrupt
    # allocation depends on the application, and may even change as the application
    # is relinked.
    #
    # An alternative approach would allow device scripts to request specific
    # interrupt numbers, making the system a bit more deterministic, but
    # introducing complications such as shared interrupt numbers. On the other
    # hand that would make it easier to test chained interrupt support and
    # the like.
    # FIXME: add support for allocating specific interrupt numbers
    proc interrupt_allocate { name } {
	if { $synth::_interrupt_next == 32 } {
	    synth::report_error "Unable to allocate an interrupt vector for $name\nAll 32 interrupt vectors are already in use.\n"
	    return -1
	}
	set result $synth::_interrupt_next
	set synth::_interrupt_names($result) $name
	incr synth::_interrupt_next
	return $result
    }

    # Allow information about the device->interrupt mappings to be retrieved
    proc interrupt_get_max { } {
	return [expr $synth::_interrupt_next - 1]
    }
    proc interrupt_get_devicename { number } {
	if { [info exists synth::_interrupt_names($number) ] } {
	    return $synth::_interrupt_names($number)
	} else {
	    return ""
	}
    }

    # Raise a specific interrupt. If the interrupt is already pending
    # this has no effect because a SIGIO will have been sent to the
    # eCos application already. Otherwise SIGIO needs to be raised.
    proc interrupt_raise { number } {
	if { $number >= $synth::_interrupt_next } {
	    error "Attempt to raise invalid interrupt $number."
	}
	if { !$synth::ecos_running } {
	    return
	}
	set or_mask [expr 0x01 << $number]
	if { 0 == ($or_mask & $synth::_interrupt_pending) } {
	    # This interrupt was not previously pending, so action is needed.
	    set synth::_interrupt_pending [expr $synth::_interrupt_pending | $or_mask]
	    synth::hook_call "interrupt" $number
	    synth::_send_SIGIO
	}
    }

    # Request application exit. This is typically called in response to
    # File->Exit.
    proc request_application_exit { } {
	set synth::_interrupt_exit_pending 1
	synth::_send_SIGIO
    }
    
    # The eCos application wants to know about pending interrupts. It maintains
    # its own set of pending interrupts, so once the information has been
    # transferred there are no pending interrupts left in the I/O auxiliary,
    # only in the eCos app. A pending exit is indicated by non-empty data,
    # the actual data does not matter.
    proc _handle_GET_IRQ_PENDING {  } {
	if { $synth::_interrupt_exit_pending } {
	    synth::send_reply $synth::_interrupt_pending 1 "x"
	} else {
	    synth::send_reply $synth::_interrupt_pending 0 ""
	}
	set synth::_interrupt_pending 0
    }
}

# }}}
# {{{  Initialization complete                                  

# ----------------------------------------------------------------------------
# This is called once all static constructors have been run, i.e. when all
# eCos devices should be initialized. It does the following:
#
# 1) invoke any "initialized" hooks set up by device scripts.
#
# 2) run the per-user mainrc.tcl script, if it exists, so that users can
#    install hooks, modify the GUI display, etc.
#
# 3) warn about any unused command line arguments
#
# 4) optionally warn about any unused entries in the target definition file
#
# 5) clean up the GUI, e.g. remove unwanted windows and borders, and display it.
#
# However if the user specified --help then, instead of all the above,
# a help message is displayed and the auxiliary exits, hopefully taking the
# eCos application with it.

namespace eval synth {

    proc _handle_CONSTRUCTORS_DONE { } {

	if { $synth::flag_help } {
	    puts "Usage : <eCos application> <options>"
	    puts "    Options are passed to the I/O auxiliary, and are not"
	    puts "    accessible to the eCos application."
	    puts "Standard options:"
	    puts " -io                         : run with I/O facilities."
	    puts " -nio                        : run with no I/O facilities at all."
	    puts " -nw, --no-windows           : run in console mode."
	    puts " -w, --windows               : run in GUI mode (default)."
	    puts " -v, --version               : display the version of the I/O auxiliary."
	    puts " -h, --help                  : show this help text."
	    puts " -k, --keep-going            : ignore errors in init scripts or the"
	    puts "                               target definition file."
	    puts " -nr, --no-rc                : do not run the user's init scripts."
	    puts " -x, --exit                  : terminate I/O auxiliary as soon as the eCos"
	    puts "                               application exits (default in console mode)."
	    puts " -nx, --no-exit              : I/O auxiliary keeps running even after eCos"
	    puts "                               application has exited (default in GUI mode)."
	    puts " -V, --verbose               : provide additional output during the run."
	    puts " -l <file>, --logfile <file> : send all output to the specified file. In"
	    puts "                               GUI mode this in addition to the main text"
	    puts "                               window. In console mode this is instead of"
	    puts "                               stdout."
	    puts " -t <file>, --target <file>  : use the specified .tdf file as the target"
	    puts "                               definition. The auxiliary will look for this"
	    puts "                               file in the current directory, ~/.ecos, and"
	    puts "                               finally the install location."
	    puts " -geometry <geometry>        : size and position for the main window."
	    synth::hook_call "help"
	    exit 1
	}
	
	synth::hook_call "ecos_initialized"
	
	# ----------------------------------------------------------------------------
	if { !$synth::flag_no_rc } {
	    set _config_file [file join "~/.ecos/synth" "mainrc.tcl"]
	    if { [file exists $_config_file] } {
		if { [file readable $_config_file] } {
		    if { [catch { source $_config_file } msg ] } {
			set error "Failed to execute user initialization file  \"$_config_file\"\n"
			append error "  $msg\n"
			if { $synth::flag_verbose } {
			    append error "------- backtrace ------------------------------------------\n"
			    append error $::errorInfo
			    append error "\n------- backtrace ends -------------------------------------\n"
			}
			synth::report_error $error
		    }
		} else {
		    synth::report_error "No read access to user initialization file \"$_config_file\"\n"
		}
	    }
	    unset _config_file
	}
	
	# ----------------------------------------------------------------------------
	# Report any arguments that have not been used up by the auxiliary itself
	# or by any device handlers
	set unconsumed_args [synth::argv_get_unconsumed]
	foreach arg $unconsumed_args { 
	    synth::report_warning "Unrecognised command line option \"$arg\", ignored.\n"
	}

	# ----------------------------------------------------------------------------
	if { $synth::flag_verbose } {
	    set unconsumed_devices [synth::tdf_get_unconsumed_devices]
	    set unconsumed_options [synth::tdf_get_unconsumed_options]
	    if { (0 != [llength $unconsumed_devices]) || (0 != [llength $unconsumed_options]) } {
		set msg "Target definition file $synth::target_definition\n"
		foreach dev $unconsumed_devices {
		    append msg "    synth_device \"$dev\" not recognised.\n"
		}
		foreach option $unconsumed_options {
		    set dev [lindex $option 0]
		    set opt [lindex $option 1]
		    append msg "    synth_device \"$dev\", option \"$opt\" not recognised.\n"
		}
		synth::report_warning $msg
	    }
	}

	#  ----------------------------------------------------------------------------
	if { $synth::flag_gui } {
	    synth::_cleanup_gui
	    wm deiconify .
	}
	
	# ----------------------------------------------------------------------------
	# Finally send a reply back to the application so it can really
	# start running. Alternatively, if any errors occurred during
	# initialization and the user did not specify --keep-going then
	# send back an error code, causing the eCos application to terminate.
	if { (0 == $synth::_error_count) || $synth::flag_keep_going } {
	    synth::send_reply 1 0 ""
	} else {
	    synth::send_reply 0 0 ""
	}
    }
}

# }}}
# {{{  Requests for the I/O auxiliary itself                    

# ----------------------------------------------------------------------------
# There are three requests which can be aimed at the I/O auxiliary itself,
# rather than at device-specific scripts. These are: INSTANTIATE to instantiate
# a device; CONSTRUCTORS_DONE to indicate when initialization is complete;
# and GET_IRQ_PENDING which deals with interrupts.

namespace eval synth {
    
    proc _handle_ecosynth_requests { devid request arg1 arg2 request_data request_len reply_len } {
	if { 0x01 == $request } {
	    synth::_handle_INSTANTIATE $request_data
	} elseif { 0x02 == $request } {
	    synth::_handle_CONSTRUCTORS_DONE
	} elseif { 0x03 == $request } {
	    synth::_handle_GET_IRQ_PENDING
	} elseif { 0x04 == $request } {
	    synth::_handle_GET_VERSION
	} else {
	    error "The eCos application has sent an invalid request sent to the I/O auxiliary"
	}
    }

    variable _SYNTH_AUXILIARY_PROTOCOL_VERSION	0x01
    proc _handle_GET_VERSION { } {
	synth::send_reply $synth::_SYNTH_AUXILIARY_PROTOCOL_VERSION 0 ""
    }
}

# }}}
# {{{  Application exit                                         

# ----------------------------------------------------------------------------
# The application has exited. This is detected by an EOF event on the pipe
# from the eCos application.
#
# First the rest of the system is informed about the event using the
# appropriate hook. This should ensure that the various device-specific
# scripts do the right thing, e.g shut down sub-processes. Next, if
# the immediate exit flag is set then that is obeyed. This flag is set by
# default when in command-line mode because there is no point in continuing
# to run if there is neither an application nor a GUI for the user to interact
# with. It also gets set if the user has explicitly requested an exit.
#
# The exit call will invoke the appropriate hooks.
namespace eval synth {
    
    proc _application_has_exited { } {

	set synth::ecos_running 0
	synth::hook_call "ecos_exit"
	
	# Depending on command-line arguments and whether or not the GUI is present,
	# the auxiliary should now exit
	if { $synth::flag_immediate_exit } {
	    exit 0
	} elseif { !$synth::flag_gui } {
	    synth::report "eCos application has exited: I/O auxiliary still running in the background.\n"
	}
    }
}

# }}}
# {{{  Communication with the eCos application                  

namespace eval synth {
    
    # ----------------------------------------------------------------------------
    # The basic communication routines between the auxiliary and the
    # eCos application. _read_request is invoked whenever there is
    # a pending event on the pipe from the eCos application, either
    # a request or an EOF. It 

    # Keep track of a couple of things to detect protocol mismatches.
    variable _reply_expected 0
    variable _expected_rxlen 0
    
    # Receive a single request from the eCos application and invoke the
    # appropriate handler.
    proc _read_request { } {
	# Read a single request from the application, or possibly EOF
	set devid   0
	set reqcode 0
	set arg1    0
	set arg2    0
	set txlen   0
	set txdata  ""
	set rxlen   0
	set request [read $synth::_channel_from_app 24]
	
	if { [eof $synth::_channel_from_app] } {
	    fileevent $synth::_channel_from_app readable ""
	    synth::_application_has_exited
	    return
	}

	# If a real request is sent then currently the application should
	# not be expecting a reply
	if { $synth::_reply_expected } {
	    error "The eCos application should not be sending a request when there is still a reply pending"
	}

	set binary_result [binary scan $request "iiiiii" devid reqcode arg1 arg2 txlen rxlen]
	if { 6 != $binary_result } {
	    error "Internal error decoding request from eCos application"
	}

	# If running on a 64-bit platform then the above numbers will have been sign-extended,
	# which could lead to confusing results
	set devid   [expr $devid   & 0x0FFFFFFFF]
	set reqcode [expr $reqcode & 0x0FFFFFFFF]
	set arg1    [expr $arg1    & 0x0FFFFFFFF]
	set arg2    [expr $arg2    & 0x0FFFFFFFF]
	set txlen   [expr $txlen   & 0x0FFFFFFFF]
	set rxlen   [expr $rxlen   & 0x0FFFFFFFF]

	# The top bit of rxlen is special and indicates whether or not a reply is expected.
	set synth::_reply_expected [expr 0 != ($rxlen & 0x080000000)]
	set synth::_expected_rxlen [expr $rxlen & 0x07FFFFFFF]
	
	# Is there additional data to be read
	if { $txlen > 0 } {
	    set txdata [read $synth::_channel_from_app $txlen]
	    if { [eof $synth::_channel_from_app] } {
		fileevent $synth::_channel_from_app readable ""
		synth::_application_has_exited
		return
	    }
	}

	# The devid can be used to get hold of a handler function, and that will do
	# the hard work.
	if { ![info exists synth::_device_handlers($devid)] } {
	    error "A request has been received for an unknown device $devid"
	}

	$synth::_device_handlers($devid) $devid $reqcode $arg1 $arg2 $txdata $txlen $synth::_expected_rxlen
    }
    
    # Register _read_request as the handler for file events on the pipe from
    # the application.
    fileevent $synth::_channel_from_app readable synth::_read_request
    
    # Send a reply back to eCos. This consists of a two-word structure,
    # result and length, followed by the data if any. Currently this
    # raises an error if there is a mismatch between the specified and
    # actual length of the data. Possibly the code should cope with
    # data strings that exceed the specified length, extracting a
    # suitable substring.
    proc send_reply { result { length 0 } { data "" } } {
	# Make sure that a reply is actually expected.
	if { !$synth::_reply_expected } {
	    error "Attempt to send reply to application when no request has been sent"
	} else {
	    set synth::_reply_expected 0
	}
	
	if { $length > $synth::_expected_rxlen } {
	    error "Reply contains more data than the application expects: $length bytes instead of $synth::_expected_rxlen"
	}
	if { ($length > 0) && ([string length $data] != $length) } {
	    error "Mismatch between specified and actual data length: $length [string length $data]"
	}
	if { !$synth::ecos_running } {
	    return
	}
	
	set struct [binary format "ii" $result $length]
	# Ignore any errors when writing down the pipe. The only likely error is
	# when the application has exited, causing a SIGPIPE which Tcl
        # will handle. The application should be waiting for this response.
	catch {
	    puts -nonewline $synth::_channel_to_app $struct
	    if { $length > 0 } {
		puts -nonewline $synth::_channel_to_app $data
	    }
	} 
    }
}

# }}}

# {{{  initrc                                                   

# ----------------------------------------------------------------------------
# Just before control is returned to the eCos application, run the per-user
# file, ~/.ecos/synth/initrc.tcl. The main GUI is now in place and the target
# definition file has been read in,  but no eCos static constructors have
# been run yet and hence no devices have been loaded or activated.
# All the various core procedures have been defined. initrc gives the user
# a chance to install hooks, redefine some internals, and so on.
# Another initialization file mainrc.tcl gets read in later, just before
# the eCos application really starts running.
#
# Possibly ecosynth should also read in a system-wide initialization
# file equivalent to emacs' site-start.el, but the extra complexity
# does not seem warranted just yet.

if { !$synth::flag_no_rc } {
    set _config_file [file join "~/.ecos/synth" "initrc.tcl"]
    if { [file exists $_config_file] } {
	if { [file readable $_config_file] } {
	    if { [catch { source $_config_file } msg ] } {
		set error "Failed to execute user initialization file  \"$_config_file\"\n"
		append error "  $msg\n"
		if { $synth::flag_verbose } {
		    append error "------- backtrace ------------------------------------------\n"
		    append error $::errorInfo
		    append error "\n------- backtrace ends -------------------------------------\n"
		}
		synth::report_error $error
	    }
	} else {
	    synth::report_error "No read access to user initialization file \"$_config_file\"\n"
	}
    }
    unset _config_file
}

# }}}
# {{{  Done      						

# ----------------------------------------------------------------------------
# The last few steps.

# Once everything has been initialized the application can be woken up again.
# It should be blocked waiting for a single byte on the pipe.
if { $synth::flag_debug } {
    synth::report "Core initialization complete, resuming the eCos application.\n"
}

puts -nonewline $synth::_channel_to_app  "."

# Enter the event loop. In console mode there is a problem if -nx has been
# specified: there may not be any event handlers left once the eCos application
# has exited, so the vwait would abort. This is avoided by a dummy after proc.
if { !$synth::flag_gui && !$synth::flag_immediate_exit } {
    namespace eval synth {
	proc _dummy_after_handler { } {
	    after 1000000 synth::_dummy_after_handler
	}
    }
    after 1000000 synth::_dummy_after_handler
}

vwait synth::_ecosynth_exit

# }}}
