#!/bin/bash
# restart using a Tcl shell \
    exec sh -c 'for tclshell in tclsh tclsh83 cygtclsh80 ; do \
            ( echo | $tclshell ) 2> /dev/null && exec $tclshell "`( cygpath -w \"$0\" ) 2> /dev/null || echo $0`" "$@" ; \
        done ; \
        echo "ecosadmin.tcl: cannot find Tcl shell" ; exit 1' "$0" "$@"

# {{{  Banner

#===============================================================================
#
#	ecosadmin.tcl
#
#	A package install/uninstall tool.
#
#===============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 John Dallaway
## Copyright (C) 2004 eCosCentric Limited
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
#===============================================================================
######DESCRIPTIONBEGIN####
#
# Author(s):    jld
# Contributors: bartv
# Date:         1999-06-18
# Purpose:      To install and uninstall packages from an eCos component
#               repository
# Description:
# Usage:
#
#####DESCRIPTIONEND####
#===============================================================================
#

# }}}
# {{{  Version check

# ----------------------------------------------------------------------------
# ecosadmin.tcl requires at least version 8.0 of Tcl, since it makes use of
# namespaces. It is possible that some users still have older versions.

if { [info tclversion] < 8.0 } {
	puts "This script requires Tcl 8.0 or later. You are running Tcl [info patchlevel]."
	return
}

# }}}
# {{{  Namespace definition

# ----------------------------------------------------------------------------
# Namespaces. All code and variables in this script are kept in the namespace
# "ecosadmin". This is not really necessary for stand-alone operation, but if it
# ever becomes desirable to embed this script in a larger application then
# using a namespace is a lot easier.
#
# As a fringe benefit, all global variables can be declared inside this
# namespace and initialised.
#

namespace eval ecosadmin {

	# Is this program running under Windows ?
	variable windows_host [expr {$tcl_platform(platform) == "windows"}]
	variable null_device ""
	if { $windows_host != 0 } {
		set ecosadmin::null_device "nul"
	} else {
		set ecosadmin::null_device "/dev/null"
	}
		

	# Where is the component repository ? The following input sources
	# are available:
	# 1) the environment variable ECOS_REPOSITORY.
	# 2) $argv0 should correspond to the location of the ecosadmin.tcl
	#    script.
	#
	variable component_repository ""
	if { [info exists ::env(ECOS_REPOSITORY)] } {
		# override the calculation of the repository location using the 
		# (undocumented) ECOS_REPOSITORY environment variable
		set component_repository $::env(ECOS_REPOSITORY)
	} else {
		set component_repository [pwd]
		if { [file dirname $argv0] != "." } {
			set component_repository [file join $component_repository [file dirname $argv0]]
		}
	}

	# Details of the command line arguments, if any.
	variable list_packages_arg   0;     # list
	variable accept_license_arg  0;     # --accept_license
	variable extract_license_arg 0;     # --extract_license
	variable add_package        "";     # add FILE
	variable remove_package     "";     # remove PACKAGE
	variable merge_repository   "";     # merge REPOSITORY
	variable version_arg        "";     # --version VER
	
	# Details of all known packages, targets and templates
	# read from the ecos.db file
	variable known_packages ""
	variable known_targets ""
	variable known_templates ""
	array set package_data {};
	array set target_data {};
	array set template_data {};

	# List of packages merged from another repository
	variable merge_packages ""
	
	# What routines should be invoked for outputting fatal errors and
	# for warning messages ?
	variable fatal_error_handler ecosadmin::cli_fatal_error
	variable warning_handler     ecosadmin::cli_warning
	variable report_handler      ecosadmin::cli_report

        # Keep or remove the CVS directories?
        variable keep_cvs 0
}

# }}}
# {{{  Infrastructure

# ----------------------------------------------------------------------------
# Minimal infrastructure support.
#
# There must be some way of reporting fatal errors, of outputting warnings,
# and of generating report messages. The implementation of these things
# obviously depends on whether or not TK is present. In addition, if this
# script is being run inside a larger application then that larger
# application must be able to install its own versions of the routines.
#
# Once it is possible to report fatal errors, an assertion facility becomes
# feasible.
#
# These routines output fatal errors, warnings or miscellaneous messages.
# Their implementations depend on the mode in which this script is operating.
#
proc ecosadmin::fatal_error { msg } {
	$ecosadmin::fatal_error_handler "$msg"
}

proc ecosadmin::warning { msg } {
	$ecosadmin::warning_handler "$msg"
}

proc ecosadmin::report { msg } {
	$ecosadmin::report_handler "$msg"
}

#
# Command line versions.
# NOTE: some formatting so that there are linebreaks at ~72 columns would be
# a good idea.
#
proc ecosadmin::cli_fatal_error_handler { msg } {
	error "$msg"
}

proc ecosadmin::cli_warning_handler { msg } {
	puts "ecosadmin warning: $msg"
}

proc ecosadmin::cli_report_handler { msg } {
	puts "$msg"
}

#
# Determine the default destination for warnings and for fatal errors.
# After the first call to this function it is possible to use assertions.
#
proc ecosadmin::initialise_error_handling { } {
	set ecosadmin::fatal_error_handler ecosadmin::cli_fatal_error_handler
	set ecosadmin::warning_handler     ecosadmin::cli_warning_handler
	set ecosadmin::report_handler      ecosadmin::cli_report_handler
}

#
# These routines can be used by containing programs to provide their
# own error handling.
#
proc ecosadmin::set_fatal_error_handler { fn } {
	ASSERT { $fn != "" }
	set ecosadmin::fatal_error_handler $fn
}

proc ecosadmin::set_warning_handler { fn } {
	ASSERT { $fn != "" }
	set ecosadmin::warning_handler $fn
}

proc ecosadmin::set_report_handler { fn } {
	ASSERT { $fn != "" }
	set ecosadmin::report_handler $fn
}

#
# A very simple assertion facility. It takes a single argument, an expression
# that should be evaluated in the calling function's scope, and on failure it
# should generate a fatal error.
#
proc ecosadmin::ASSERT { condition } {
	set result [uplevel 1 [list expr $condition]]
	
	if { $result == 0 } {
		fatal_error "assertion predicate \"$condition\"\nin \"[info level -1]\""
	}
}

# }}}
# {{{  Utilities

# ----------------------------------------------------------------------------
# cdl_compare_version. This is a partial implementation of the full
# cdl_compare_version facility defined in the product specification. Its
# purpose is to order the various versions of a given package with
# the most recent version first. As a special case, "current" is
# always considered the most recent.
#
# There are similarities between cdl_compare_version and with Tcl's
# package vcompare, but cdl_compare_version is more general.
#

proc ecosadmin::cdl_compare_version { arg1 arg2 } {

	if { $arg1 == $arg2 } {
		return 0
	}
	if { $arg1 == "current"} {
		return -1
	}
	if { $arg2 == "current" } {
		return 1
	}

	set index1 0
	set index2 0
	set ch1    ""
	set ch2    ""
	set num1   ""
	set num2   ""
	
	while { 1 } {

		set ch1 [string index $arg1 $index1]
		set ch2 [string index $arg2 $index2]
		set num1 ""
		set num2 ""

		if { ($ch1 == "") && ($ch2 == "") } {
		
			# Both strings have terminated at the same time. There may have
			# been some spurious leading zeroes in numbers.
			return 0
		
		} elseif { $ch1 == "" } {

			# The first string has ended first. If ch2 is a separator then
			# arg2 is a derived version, e.g. v0.3.p1 and hence newer. Otherwise ch2
			# is an experimental version v0.3beta and hence older.
			if { [string match \[-._\] $ch2] } {
				return 1
			} else {
				return -1
			}
		} elseif { $ch2 == "" } {

			# Equivalent to the above.
			if { [string match \[-._\] $ch1] } {
				return -1
			} else {
				return 1
			}
		}

		# There is still data to be processed.
		# Check for both strings containing numbers at the current index.
		if { ( [string match \[0-9\] $ch1] ) && ( [string match \[0-9\] $ch2] ) } {

			# Extract the entire numbers from the version string.
			while { [string match \[0-9\] $ch1] } {
				set  num1 "$num1$ch1"
				incr index1
				set  ch1 [string index $arg1 $index1]
			}
			while { [string match \[0-9\] $ch2] } {
				set  num2 "$num2$ch2"
				incr index2
				set ch2 [string index $arg2 $index2]
			}

			if { $num1 < $num2 } {
				return 1
			} elseif { $num1 > $num2 } {
				return -1
			}
			continue
		}

		# This is not numerical data. If the two characters are the same then
		# move on.
		if { $ch1 == $ch2 } {
			incr index1
			incr index2
			continue
		}
	
		# Next check if both strings are at a separator. All separators can be
		# used interchangeably.
		if { ( [string match \[-._\] $ch1] ) && ( [string match \[-._\] $ch2] ) } {
			incr index1
			incr index2
			continue
		}

		# There are differences in the characters and they are not interchangeable.
		# Just return a standard string comparison.
		return [string compare $ch1 $ch2]
	}
}

# }}}
# {{{  Argument parsing

# ----------------------------------------------------------------------------
# The argv0 argument should be the name of this script. It can be used
# to get at the component repository location. If this script has been
# run incorrectly then currently it will fail: in future it may be
# desirable to check an environment variable instead.
#
# The argv argument is a string containing the rest of the arguments.
# If any of the arguments contain spaces then this argument will be
# surrounded by braces. If any of the arguments contain braces then
# things will break.
#

proc ecosadmin::parse_arguments { argv0 argv } {

	if { $argv != "" } {

		# There are arguments. If any of the arguments contained
		# spaces then these arguments will have been surrounded
		# by braces, which is a nuisance. So start by turning the
		# arguments into a numerically indexed array.

		set argc 0
		array set args { }
		foreach arg $argv {
			set args([incr argc]) $arg
		}

		# Now examine each argument with regular expressions. It is
		# useful to have some variables filled in by the regexp
		# matching.
		set dummy  ""
		set match1 ""
		set match2 ""
		for { set i 1 } { $i <= $argc } { incr i } {

			# Check for --list and the other simple ones.
			if { [regexp -- {^-?-?list$} $args($i)] == 1 } {
				set ecosadmin::list_packages_arg 1
				continue
			}

			# check for --version
			if { [regexp -- {^-?-version=?(.*)$} $args($i) dummy match1] == 1 } {
				if { $match1 != "" } {
					set ecosadmin::version_arg $match1
				} else {
					if { $i == $argc } {
						fatal_error "missing argument after --version"
					} else {
						set ecosadmin::version_arg $args([incr i])
					}
				}
				continue
			}
		
			# check for --accept_license
			if { [regexp -- {^-?-accept_license$} $args($i)] == 1 } {
				set ecosadmin::accept_license_arg 1
				continue
			}
		
			# check for --extract_license
			if { [regexp -- {^-?-extract_license$} $args($i)] == 1 } {
				set ecosadmin::extract_license_arg 1
				continue
			}
		
			# check for the add command
			if { [regexp -- {^-?-?add=?(.*)$} $args($i) dummy match1] == 1 } {
				if { $match1 != "" } {
					set ecosadmin::add_package $match1
				} else {
					if { $i == $argc } {
						fatal_error "missing argument after add"
					} else {
						set ecosadmin::add_package $args([incr i])
					}
				}
				continue
			}
		
			# check for the merge command
			if { [regexp -- {^-?-?merge=?(.*)$} $args($i) dummy match1] == 1 } {
				if { $match1 != "" } {
					set ecosadmin::merge_repository $match1
				} else {
					if { $i == $argc } {
						fatal_error "missing argument after merge"
					} else {
						set ecosadmin::merge_repository $args([incr i])
					}
				}
				continue
			}
		
			# check for the remove command
			if { [regexp -- {^-?-?remove=?(.*)$} $args($i) dummy match1] == 1 } {
				if { $match1 != "" } {
					set ecosadmin::remove_package $match1
				} else {
					if { $i == $argc } {
						fatal_error "missing argument after remove"
					} else {
						set ecosadmin::remove_package $args([incr i])
					}
				}
				continue
			}
		
			# Check for --srcdir
			if { [regexp -- {^-?-srcdir=?([ \.\\/:_a-zA-Z0-9-]*)$} $args($i) dummy match1] == 1 } {
				if { $match1 == "" } {
					if { $i == $argc } {
						puts "ecosrelease: missing argument after --srcdir"
						exit 1
					} else {
						set match1 $args([incr i])
					}
				}
				set ecosadmin::component_repository $match1
				continue
			}
	    
			# An unrecognised argument.
			fatal_error "invalid argument $args($i)"
		}
	} 

	# Convert user-specified UNIX-style Cygwin pathnames to Windows Tcl-style as necessary
	set ecosadmin::component_repository [get_pathname_for_tcl $ecosadmin::component_repository]
	set ecosadmin::add_package [get_pathname_for_tcl $ecosadmin::add_package]
	set ecosadmin::merge_repository [get_pathname_for_tcl $ecosadmin::merge_repository]
}

#
# Display help information if the user has typed --help, -H, --H, or -help.
# The help text uses two hyphens for consistency with configure.
# Arguably this should change.

proc ecosadmin::argument_help { } {

	puts "Usage: ecosadmin \[ command \]"
	puts "  commands are:"
	puts "    list                                   : list packages"
	puts "    add FILE                               : add packages"
	puts "    remove PACKAGE \[ --version VER \]       : remove a package"
}

# }}}
# {{{  Packages file

proc ecosadmin::read_data { silentflag } {

	ASSERT { $ecosadmin::component_repository != "" }

	set ecosadmin::known_packages ""
	set ecosadmin::known_targets ""
	set ecosadmin::known_templates ""

	# A safe interpreter is used to process the packages file.
	# This is somewhat overcautious, but it is also harmless.
	# The following two commands are made accessible to the slave
	# interpreter and are responsible for updating the actual data.
	proc add_known_package { name } {
		lappend ::ecosadmin::known_packages $name
	}
	proc add_known_target { name } {
		lappend ::ecosadmin::known_targets $name
	}
	proc add_known_template { name } {
		lappend ::ecosadmin::known_templates $name
	}
	proc set_package_data { name value } {
		set ::ecosadmin::package_data($name) $value
	}
	proc set_target_data { name value } {
		set ::ecosadmin::target_data($name) $value
	}
	proc set_template_data { name value } {
		set ::ecosadmin::template_data($name) $value
	}

	# Create the parser, add the aliased commands, and then define
	# the routines that do the real work.
	set parser [interp create -safe]
	$parser alias add_known_package ecosadmin::add_known_package
	$parser alias add_known_target ecosadmin::add_known_target
	$parser alias add_known_template ecosadmin::add_known_template
	$parser alias set_package_data  ecosadmin::set_package_data
	$parser alias set_target_data  ecosadmin::set_target_data
	$parser alias set_template_data  ecosadmin::set_template_data
	
	$parser eval {
	
	set current_package ""
	set current_target ""
	set current_template ""
	
	proc package { name body } {
		add_known_package $name
		set_package_data "$name,alias" ""
		set_package_data "$name,versions" ""
		set_package_data "$name,dir" ""
		set_package_data "$name,hardware" 0
		set ::current_package $name
		eval $body
		set ::current_package ""
	}

	proc target { name body } {
		add_known_target $name
		set_target_data "$name,packages" ""
		set ::current_target $name
		eval $body
		set ::current_target ""
	}

#if 0
	# templates are no longer specified in the package database
	proc template { name body } {
		add_known_template $name
		set_template_data "$name,packages" ""
		set ::current_template $name
		eval $body
		set ::current_template ""
	}
#endif

	proc packages { str } {
		if { $::current_template != "" } {
			set_template_data "$::current_template,packages" $str
		} elseif { $::current_target != "" } {
			set_target_data "$::current_target,packages" $str
		} else {
			ASSERT 0
		}
	}

	proc directory { dir } {
		set_package_data "$::current_package,dir" $dir
	}

	proc alias { str } {
		if { $::current_package != "" } {
			set_package_data "$::current_package,alias" $str
		}
	}

	proc hardware { } {
		set_package_data "$::current_package,hardware" 1
	}

	proc description { str } { }
	proc disable { str } { }
	proc enable { str } { }
	proc script { str } { }
	proc set_value { str1 str2 } { }
	}

	# The parser is ready to evaluate the script. To avoid having to give the
	# safe interpreter file I/O capabilities, the file is actually read in
	# here and then evaluated.
	set filename [file join $ecosadmin::component_repository "ecos.db"]
	set status [ catch {
		set fd [open $filename r]
		set script [read $fd]
		close $fd
		$parser eval $script
} message ]

	if { $status != 0 } {
		ecosadmin::fatal_error "parsing $filename:\n$message"
	}

	# The interpreter and the aliased commands are no longer required.
	rename set_package_data {}
	rename set_target_data {}
	rename set_template_data {}
	rename add_known_package {}
	interp delete $parser
	
	# At this stage the packages file has been read in. It is a good idea to
	# check that all of these packages are present and correct, and incidentally
	# figure out which versions are present.
	foreach pkg $ecosadmin::known_packages {

		set pkgdir [file join $ecosadmin::component_repository $ecosadmin::package_data($pkg,dir)]
		if { ![file exists $pkgdir] || ![file isdir $pkgdir] } {
			if { "" == $silentflag } {
				warning "package $pkg at $pkgdir missing"
			}
		} else {
			# Each subdirectory should correspond to a release. A utility routine
			# is available for this.
			set ecosadmin::package_data($pkg,versions) [locate_subdirs $pkgdir]
			if { $ecosadmin::package_data($pkg,versions) == "" } {
				fatal_error "package $pkg has no version directories"
			}
		}
		# Sort all the versions using a version-aware comparison version
		set ecosadmin::package_data($pkg,versions) [
			lsort -command ecosadmin::cdl_compare_version $ecosadmin::package_data($pkg,versions)
		]
	}
}

#
# Given a package name as supplied by the user, return the internal package name.
# This involves searching through the list of aliases.
#
proc ecosadmin::find_package { name } {

	foreach pkg $ecosadmin::known_packages {
		if { [string toupper $pkg] == [string toupper $name] } {
			return $pkg
		}

		foreach alias $ecosadmin::package_data($pkg,alias) {
			if { [string toupper $alias] == [string toupper $name] } {
				return $pkg
			}
		}
	}

	return ""
}

# }}}
# {{{  Directory and file utilities

# ----------------------------------------------------------------------------
# Start with a number of utility routines to access all files in
# a directory, stripping out well-known files such as makefile.am.
# The routines take an optional pattern argument if only certain
# files are of interest.
#
# Note that symbolic links are returned as well as files.
#
proc ecosadmin::locate_files { dir { pattern "*"} } {

	ASSERT { $dir != "" }

	# Start by getting a list of all the files.
	set filelist [glob -nocomplain -- [file join $dir $pattern]]

        if { $pattern == "*" } {
                # For "everything", include ".*" files, but excluding .
                # and .. directories
                lappend filelist [glob -nocomplain -- [file join $dir ".\[a-zA-Z0-9\]*"]]
        }

	# Eliminate the pathnames from all of these files
	set filenames ""
	foreach file $filelist {
		if { [string range $file end end] != "~" } {
			lappend filenames [file tail $file]
		}
	}

	# Eliminate any subdirectories.
	set subdirs ""
	foreach name $filenames {
		if { [file isdir [file join $dir $name]] } {
			lappend subdirs $name
		}
	}
	foreach subdir $subdirs {
		set index [lsearch -exact $filenames $subdir]
		set filenames [lreplace $filenames $index $index]
	}

	return $filenames
}

#
# This utility returns all sub-directories, as opposed to all files.
# A variant glob pattern is used here. This version is not recursive.
proc ecosadmin::locate_subdirs { dir { pattern "*" }} {

	ASSERT { $dir != "" }

	set dirlist [glob -nocomplain -- [file join $dir $pattern "."]]

	# Eliminate the pathnames and the spurious /. at the end of each entry
	set dirnames ""
	foreach dir $dirlist {
		lappend dirnames [file tail [file dirname $dir]]
	}

	# Get rid of the CVS directory, if any
        if { $ecosadmin::keep_cvs == 0 } {
                set index [lsearch -exact $dirnames "CVS"]
                if { $index != -1 } {
                        set dirnames [lreplace $dirnames $index $index]
                }
        }

	# That should be it.
	return $dirnames
}

#
# A variant which is recursive. This one does not support a pattern.
#
proc ecosadmin::locate_all_subdirs { dir } {

	ASSERT { $dir != "" }

	set result ""
	foreach subdir [locate_subdirs $dir] {
		lappend result $subdir
		foreach x [locate_all_subdirs [file join $dir $subdir]] {
			lappend result [file join $subdir $x]
		}
	}
	return $result
}

#
# This routine returns a list of all the files in a given directory and in
# all subdirectories, preserving the subdirectory name.
#
proc ecosadmin::locate_all_files { dir { pattern "*" } } {

	ASSERT { $dir != "" }

	set files   [locate_files $dir $pattern]
	set subdirs [locate_subdirs $dir]

	foreach subdir $subdirs {
		set subfiles [locate_all_files [file join $dir $subdir] $pattern]
		foreach file $subfiles {
			lappend files [file join $subdir $file]
		}
	}

	return $files
}

#
# Sometimes a directory may be empty, or contain just a CVS subdirectory,
# in which case there is no point in copying it across.
#
proc ecosadmin::is_empty_directory { dir } {

	ASSERT { $dir != "" }

	set contents [glob -nocomplain -- [file join $dir "*"]]
	if { [llength $contents] == 0 } {
		return 1
	}
	if { ([llength $contents] == 1) && [string match {*CVS} $contents] } {
		return 1
	}
	return 0
}

#
# ----------------------------------------------------------------------------
# Take a cygwin32 filename such as //d/tmp/pkgobj and turn it into something
# acceptable to Tcl, i.e. d:/tmp/pkgobj. There are a few other complications...

proc ecosadmin::get_pathname_for_tcl { name } {

	if { ( $ecosadmin::windows_host ) && ( $name != "" ) } {

		# If there is no logical drive letter specified
		if { [ string match "?:*" $name ] == 0 } {

			# Invoke cygpath to resolve the POSIX-style path
			if { [ catch { exec cygpath -w $name } result ] != 0 } {
				fatal_error "processing filepath $name:\n$result"
			}
		} else {
			set result $name
		}

		# Convert backslashes to forward slashes
		regsub -all -- {\\} $result "/" name
	}

	return $name
}

# ----------------------------------------------------------------------------
# Make sure that a newly created or copied file is writable. This operation
# is platform-specific. Under Unix at most the current user is given
# permission, since there does not seem to be any easy way to get hold
# of the real umask.

proc ecosadmin::make_writable { name } {

	ASSERT { $name != "" }
	ASSERT { [file isfile $name] }
	
	if { [file writable $name] == 0 } {
		if { $ecosadmin::windows_host != 0 } {
			file attributes $name -readonly 0
		} else {
			set mask [file attributes $name -permissions]
			set mask [expr $mask | 0200]
			file attributes $name -permissions $mask
		}
	}
}

# }}}
# {{{  main()

#-----------------------------------------------------------------------
# Procedure target_requires_missing_package determines whether a
# target entry is dependent on missing packages. It is called when
# filtering templates out of the database

proc ecosadmin::target_requires_missing_package { target } {
	foreach package $ecosadmin::target_data($target,packages) {
		if { [ lsearch $ecosadmin::known_packages $package ] == -1 } {
			return 1
		}
	}
	return 0
}

#-----------------------------------------------------------------------
# Procedure template_requires_missing_package determines whether a
# template entry is dependent on missing packages. It is called when
# filtering templates out of the database

proc ecosadmin::template_requires_missing_package { template } {
	foreach package $ecosadmin::template_data($template,packages) {
		if { [ lsearch $ecosadmin::known_packages $package ] == -1 } {
			return 1
		}
	}
	return 0
}

#-----------------------------------------------------------------------
# Procedure target_requires_any_package determines whether a target entry
# is dependent on specified packages. It is called when removing packages
# to determine whether a target should also be removed

proc ecosadmin::target_requires_any_package { target packages } {
	foreach package $packages {
		if { [ lsearch $ecosadmin::target_data($target,packages) $package ] != -1 } {
			return 1
		}
	}
	return 0
}

#-----------------------------------------------------------------------
# Procedure template_requires_any_package determines whether a template entry
# is dependent on specified packages. It is called when removing packages
# to determine whether a template should also be removed

proc ecosadmin::template_requires_any_package { template packages } {
	foreach package $packages {
		if { [ lsearch $ecosadmin::template_data($template,packages) $package ] != -1 } {
			return 1
		}
	}
	return 0
}

#-----------------------------------------------------------------------
# Procedure merge_new_packages adds any entries in the specified data
# file to the eCos repository database iff they are not already present

proc ecosadmin::merge_new_packages { datafile } {

	# open the eCos database file for appending
	set ecosfile [ file join $ecosadmin::component_repository "ecos.db" ]
	variable outfile [ open $ecosfile a+ ]

	# initialize the list of merged packages
	set ecosadmin::merge_packages ""

	# this procedure is called when the interpreter encounters a
	# package command in the datafile
	proc merge { command name body } {
		ecosadmin::report "processing $command $name"
		# append the new package/target/template only if it is not already known
		if { ( ( $command == "package" ) && ( [ lsearch -exact $ecosadmin::known_packages $name ] == -1 ) ) ||
			( ( $command == "target" ) && ( [ lsearch -exact $ecosadmin::known_targets $name ] == -1 ) ) ||
			( ( $command == "template" ) && ( [ lsearch -exact $ecosadmin::known_templates $name ] == -1 ) ) } {
			puts $ecosadmin::outfile "$command $name {$body}\n"
		}
		
		# add new packages to the list of merged packages
		if { ( "package" == $command ) } {
			lappend ecosadmin::merge_packages $name
		}
	}

	# Create the parser, add the aliased commands, and then define
	# the routines that do the real work.
	set parser [ interp create -safe ]
	$parser alias merge ecosadmin::merge
	$parser eval {
		proc package { name body } {
			merge "package" $name $body
		}

		proc template { name body } {
			merge "template" $name $body
		}

		proc target { name body } {
			merge "target" $name $body
		}
	}

	# The parser is ready to evaluate the script. To avoid having to give the
	# safe interpreter file I/O capabilities, the file is actually read in
	# here and then evaluated.
	set status [ catch {
		set fd [ open $datafile r ]
		set script [ read $fd ]
		close $fd
		$parser eval $script
	} message ]

	# The interpreter and the aliased commands are no longer required.
	rename merge {}
	interp delete $parser

	# close the eCos database file
	close $outfile

	# report errors
	if { $status != 0 } {
		ecosadmin::fatal_error "parsing $datafile:\n$message"
	}
}

#-----------------------------------------------------------------------
# Procedure filter_old_packages removes the specified packages/versions
# from the eCos repository database. Any targets and templates dependent
# on the removed packages are also removed.

proc ecosadmin::filter_old_packages { old_packages } {

	# open the new eCos database file for writing
	set ecosfile [ file join $ecosadmin::component_repository "ecos.db.new" ]
	variable outfile [ open $ecosfile w ]
	variable filter_list $old_packages
	variable removed_packages ""

	# this procedure is called when the interpreter encounters a command in the datafile on the first pass
	# it generates a list of packages which will be removed on the second pass
	proc removelist { command name body } {
		if { [ lsearch $ecosadmin::filter_list $name ] != -1 } {
			# the package is in the filter list
			if { ( $ecosadmin::version_arg == "" ) || ( [ llength $ecosadmin::package_data($name,versions) ] == 1 ) } {
				# there is no version argument or only one version so add the package to the remove list
				set ::ecosadmin::removed_packages [ lappend ::ecosadmin::removed_packages $name ]
			}			
		}
	}

	# this procedure is called when the interpreter encounters a command in the datafile on the second pass
	proc filter { command name body } {
		if { ( $command == "target" ) && ( ( [ target_requires_any_package $name $ecosadmin::removed_packages ] != 0 ) || ( [ target_requires_missing_package $name ] != 0 ) ) } {
			# the target requires a package which has been removed so remove the target
			ecosadmin::report "removing target $name"
		} elseif { ( $command == "template" ) && ( ( [ template_requires_any_package $name $ecosadmin::removed_packages ] != 0 ) || ( [ template_requires_missing_package $name ] != 0 ) ) } {
			# the template requires a package which has been removed so remove the template
			ecosadmin::report "removing template $name"
		} elseif { [ lsearch $ecosadmin::filter_list $name ] == -1 } {
			# the package is not in the filter list so copy the data to the new database
			puts $ecosadmin::outfile "$command $name {$body}\n"
		} else {
			# the package is in the filter list
			set package_dir [ file join $ecosadmin::component_repository $ecosadmin::package_data($name,dir) ]
			if { ( $ecosadmin::version_arg != "" ) && ( [ llength $ecosadmin::package_data($name,versions) ] > 1 ) } {
				# there are multiple versions and only one version will be removed
				# so copy the data to the new database and only remove one version directory
				set package_dir [ file join $package_dir $ecosadmin::version_arg ]
				ecosadmin::report "removing package $name $ecosadmin::version_arg"
				puts $ecosadmin::outfile "$command $name {$body}\n"
			} else {
				# there is no version argument or only one version so delete the package directory
				ecosadmin::report "removing package $name"
			}
			if { [ catch { file delete -force -- $package_dir } message ] != 0 } {
				# issue a warning if package deletion failed - this is not fatal
				ecosadmin::warning $message
			}
			set dir [ file dirname $package_dir ]
			while { [ llength [ glob -nocomplain -- [ file join $dir "*" ] ] ] == 0 } {
				# the parent of the deleted directory is now empty so delete it
				if { [ catch { file delete -- $dir } message ] != 0 } {
					# issue a warning if empty directory deletion failed - this is not fatal
					ecosadmin::warning $message
				}
				set dir [ file dirname $dir ]
			}
		}
	}

	# Create the parser, add the aliased commands, and then define
	# the routines that do the real work.
	set parser [ interp create -safe ]
	$parser eval {
		proc package { name body } {
			filter "package" $name $body
		}

		proc template { name body } {
			filter "template" $name $body
		}

		proc target { name body } {
			filter "target" $name $body
		}
	}

	# The parser is ready to evaluate the script. To avoid having to give the
	# safe interpreter file I/O capabilities, the file is actually read in
	# here and then evaluated.
	set filename [ file join $ecosadmin::component_repository "ecos.db" ]
	set status [ catch {
		set fd [ open $filename r ]
		set script [ read $fd ]
		close $fd

		# first pass to generate a list of packages which will be removed
		$parser alias filter ecosadmin::removelist
		$parser eval $script

		# second pass to remove the packages, targets and templates
		$parser alias filter ecosadmin::filter
		$parser eval $script
	} message ]

	# The interpreter and the aliased commands are no longer required.
	rename filter {}
	interp delete $parser

	# close the new eCos database file
	close $outfile

	# report errors
	if { $status != 0 } {
		ecosadmin::fatal_error "parsing $filename:\n$message"
	}

	# replace the old eCos database file with the new one
	file rename -force $ecosfile $filename
}

# ----------------------------------------------------------------------------
# Process_add_packages. This routine is responsible for installing packages
# into the eCos repository using the gzip and tar tools which must be on
# the path
#

proc ecosadmin::process_add_package { } {
	ASSERT { $ecosadmin::add_package != "" }
	ASSERT { $ecosadmin::component_repository != "" }

	# calculate the absolute path of the specified package archive
	# since we must change directory before extracting files
	# note that we cannot use "tar -C" to avoid changing directory
	# since "tar -C" only accepts relative paths
	set abs_package [ file join [ pwd ] $ecosadmin::add_package ]
	set datafile "pkgadd.db"
	set licensefile "pkgadd.txt"
	set logfile "pkgadd.log"
	cd $ecosadmin::component_repository

	# check for --extract_license on command line
	if { $ecosadmin::extract_license_arg == 1 } {
		# extract the license file (if any) from the specified gzipped tar archive
		file delete $licensefile
		catch { exec > $ecosadmin::null_device gzip -d < $abs_package | tar xf - $licensefile }
		return
	}

	# extract the package data file from the specified gzipped tar archive
	if { [ catch { exec > $ecosadmin::null_device gzip -d < $abs_package | tar xf - $datafile } message ] != 0 } {
		fatal_error "extracting $datafile:\n$message"
	}

	# obtain license acceptance
	if { [ ecosadmin::accept_license $abs_package $licensefile ] != "y" } {
		file delete $datafile
		file delete $licensefile
		fatal_error "license agreement not accepted"
	}

	# extract the remaining package contents and generate a list of extracted files
	if { [ catch { exec gzip -d < $abs_package | tar xvf - > $logfile } message ] != 0 } {
		file delete $logfile
		fatal_error "extracting files:\n$message"
	}

	# read the list of extracted files from the log file
	set fd [ open $logfile r ]
	set message [ read $fd ]
	close $fd
	file delete $logfile

	# convert extracted text files to use the line-ending convention of the host
	set filelist [ split $message "\n" ]
	set binary_extension ".bin"
	foreach filename $filelist {
		if { [ file isfile $filename ] != 0 } {
			if { [ file extension $filename ] == $binary_extension } {
				# a binary file - so remove the binary extension
				file rename -force -- $filename [ file rootname $filename ]
			} else {
				# a text file - so convert file to use native line-endings
				# read in the file (line-ending conversion is implicit)
				set fd [ open $filename "r" ]
				set filetext [ read $fd ]
				close $fd

				# write the file out again
				set fd [ open $filename "w" ]
				puts -nonewline $fd $filetext
				close $fd
			}
		}
	}

	# merge the new package information into the eCos database file as necessary
	ecosadmin::merge_new_packages [ file join $ecosadmin::component_repository $datafile ]

	# delete the database and license files
	file delete $datafile
	file delete $licensefile

	# read the revised database back in and remove any
	# targets and templates with missing packages
	read_data ""
	filter_old_packages ""
}

# ----------------------------------------------------------------------------
# Process_remove_package. This routine is responsible for uninstalling a
# package from the eCos repository
#

proc ecosadmin::process_remove_package { } {
	ASSERT { $ecosadmin::remove_package != "" }

	# get the formal package name
	set package_name [ ecosadmin::find_package $ecosadmin::remove_package ]
	if { $package_name == "" } {
		# package not found
		fatal_error "package not found"
	} elseif { $ecosadmin::version_arg == "" } {
		# version not specified
#		if { [ llength $ecosadmin::package_data($package_name,versions) ] > 1 } {
#			fatal_error "multiple versions, use --version"
#		}
	} elseif { [ lsearch $ecosadmin::package_data($package_name,versions) $ecosadmin::version_arg ] == -1 } {
		# specified version not found
		fatal_error "version not found"
	}
	
	# filter out the old package from the eCos database file
	filter_old_packages $package_name
}

# ----------------------------------------------------------------------------
# Process_merge_repository. This routine is responsible for merging packages
# from another repository into the eCos repository
#

proc ecosadmin::process_merge_repository { } {
	ASSERT { $ecosadmin::merge_repository != "" }
	ASSERT { $ecosadmin::component_repository != "" }

	# merge new package and target information into the eCos database file as necessary
	# names of packages to be merged are placed in $ecosadmin::merge_packages
	ecosadmin::merge_new_packages [ file join $ecosadmin::merge_repository "ecos.db" ]
	
	# read the revised database back in to pick up new package paths, but ignore missing package directories
	read_data "silent"
	
	# copy package directories into the repository as necessary
	# existing packages are never replaced but a another version may be added
	foreach pkg $ecosadmin::merge_packages {
		set newpkgdir [file join $ecosadmin::merge_repository $ecosadmin::package_data($pkg,dir)]
		foreach newpkgver [locate_subdirs $newpkgdir] {
			if { [lsearch $ecosadmin::package_data($pkg,versions) $newpkgver] == -1 } {
				ecosadmin::report "copying $pkg $newpkgver"
				file mkdir [ file join $ecosadmin::component_repository $ecosadmin::package_data($pkg,dir) ]
				file copy [ file join $newpkgdir $newpkgver ] [ file join $ecosadmin::component_repository $ecosadmin::package_data($pkg,dir) $newpkgver ]
			}
		}
	}

	# read the revised database again to deliver warnings of missing package directories if necessary
	read_data ""

	# copy new files from the pkgconf and templates directory hierarchies into the repository as necessary
	foreach topdir { pkgconf templates } {
		set repository_files [ ecosadmin::locate_all_files [ file join $ecosadmin::component_repository $topdir ] ]
		set merge_files [ ecosadmin::locate_all_files [ file join $ecosadmin::merge_repository $topdir ] ]
		foreach filename $merge_files {
			if { [lsearch $repository_files $filename] == -1 } {
				ecosadmin::report "copying $topdir file $filename"
				file mkdir [ file join $ecosadmin::component_repository $topdir [ file dirname $filename ] ]
				file copy [ file join $ecosadmin::merge_repository $topdir $filename ] [ file join $ecosadmin::component_repository $topdir $filename ]
			}
		}
	}

	# copy files from the top level packages directory into the repository as necessary
	foreach filename [ glob -nocomplain -directory $ecosadmin::merge_repository -type f * ] {
		set destination [ file join $ecosadmin::component_repository [ file tail $filename ] ]
		if { 0 == [ file exists $destination ] } {
			ecosadmin::report "copying file [file tail $filename]"
			file copy $filename $destination
		}
	}
}

# ----------------------------------------------------------------------------
# Accept_license. This routine is responsible for displaying the package
# license and obtaining user acceptance. It returns "y" if the license is
# accepted.
#

proc ecosadmin::accept_license { archivename filename } {
	ASSERT { $ecosadmin::add_package != "" }

	# check for --accept_license on command line
	if { $ecosadmin::accept_license_arg == 1 } {
		# --accept_license specified so do not prompt for acceptance
		return "y"
	}

	# extract the specified license file from the specified gzipped tar archive
	if { [ catch { exec > $ecosadmin::null_device gzip -d < $archivename | tar xf - $filename } message ] != 0 } {
		# no license file
		return "y"
	}

	# read in the file and output to the user
	set fd [ open $filename "r" ]
	set filetext [ read $fd ]
	close $fd
	puts $filetext

	# prompt for acceptance
	puts -nonewline "Do you accept all the terms of the preceding license agreement? (y/n) "
	flush "stdout"
	gets "stdin" response

	# return the first character of the response in lowercase
	return [ string tolower [ string index $response 0 ] ]
}

# ----------------------------------------------------------------------------
# Main(). This code only runs if the script is being run stand-alone rather
# than as part of a larger application. The controlling predicate is the
# existence of the variable ecosadmin_not_standalone which can be set by
# the containing program if any.
#

if { ! [info exists ecosadmin_not_standalone] } {

	# Decide where warnings and fatal errors should go.
	ecosadmin::initialise_error_handling

	# First, check for --help or any of the variants. If this script
	# is running in a larger program then it is assumed that the
	# containing program will not pass --help as an argument.
	if { ( $argv == "--help" ) || ( $argv == "-help" ) ||
	     ( $argv == "--H"    ) || ( $argv == "-H" ) || ($argv == "" ) } {

		ecosadmin::argument_help
		return
	}

	# catch any errors while processing the specified command
	if { [ catch {
	
		# Parse the arguments and set the global variables appropriately.
		ecosadmin::parse_arguments $argv0 $argv

		# Read in the eCos repository database.
		ecosadmin::read_data ""
	
		# Process the ecosadmin command
		if { $ecosadmin::list_packages_arg != 0 } {
			foreach pkg $ecosadmin::known_packages {
				ecosadmin::report "$pkg: $ecosadmin::package_data($pkg,versions)"
			}
		} elseif { $ecosadmin::add_package != "" } {
			ecosadmin::process_add_package
		} elseif { $ecosadmin::remove_package != "" } {
			ecosadmin::process_remove_package
		} elseif { $ecosadmin::merge_repository != "" } {
			ecosadmin::process_merge_repository
		}

	} error_message ] != 0 } { 

		# handle error message
		if { [ info exists gui_mode ] } {
			return $error_message
		}
		puts "ecosadmin error: $error_message"
	}
	return
}

# }}}
