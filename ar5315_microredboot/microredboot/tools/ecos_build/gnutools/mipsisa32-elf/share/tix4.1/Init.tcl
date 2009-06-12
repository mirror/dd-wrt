# Init.tcl --
#
#	Initializes the Tix library and performes version checking to ensure
#	the Tcl, Tk and Tix script libraries loaded matches with the binary
#	of the respective packages.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

if ![tixStrEq $tix_library ""] {
    global auto_path
    lappend auto_path $tix_library
}

if [catch {file join a a}] {
    proc tixFileJoin {args} {
	set p [join $args /]
	regsub -all {/+} $p / p
	return $p
    }
} else {
    proc tixFileJoin {args} {
	return [eval file join $args]
   }
}

proc __tixError {errorMsg} {
    error [concat $errorMsg \
       "Please check your TIX_LIBRARY environment variable and " \
       "your Tix installation."]
}

proc __tixInit {} {
    global tix tixPriv env tix_version tix_patchLevel tk_version tix_library
    global tcl_version

    if [info exists tix(initialized)] {
	return
    }
    if {[info command "@scope"] != ""} {
	set hasItcl 1
    } else {
	set hasItcl 0
    }

    # STEP 0: Version checking using the Tcl7.5 package mechanism. This is not
    #	      done if we are linked to Tcl 7.4.
    #
    if [string compare [info command package] ""] {
	if {![string comp [info command tixScriptVersion] ""] && 
		![auto_load tixScriptVersion]} {
	    __tixError [concat "Cannot determine version of Tix script " \
		"library. Requires version $tix_version."]
	}

	if !$hasItcl {
	    set pkgVersion  $tix_version.$tcl_version
	} else {
	    # The extra .1 indicates that the Tix binary is specially
	    # compiled for Itcl. This is necessary for the "package
	    # require" command to load in the correct shared library
	    # file.
	    set pkgVersion  $tix_version.$tcl_version.1
	}

	package provide Tix $pkgVersion
	if [tixStrEq $tix_library ""] {
	    package provide Tixsam $pkgVersion
	}
    }

    # STEP 1: Version checking
    #
    #
    if {![string compare [info command tixScriptVersion] ""] && 
	    ![auto_load tixScriptVersion]} {
	__tixError [concat "Cannot determine version of Tix script library. "\
	    "Requires version $tix_version."]

    } elseif [string compare [tixScriptVersion] $tix_version] {
	__tixError [concat "Tix script library version ([tixScriptVersion]) "\
	    "does not match binary version ($tix_version)"]

    } elseif [string compare [tixScriptPatchLevel] $tix_patchLevel] {
	__tixError [concat "Tix script library patch-level "\
	    "([tixScriptPatchLevel]) does not match binary patch-level "\
	    "($tix_patchLevel)"]
    }

    if [info exists errorMsg] {
	error $errorMsg
    }

    # STEP 2: Initialize file compatibility modules
    #
    #
    if [info exists tixPriv(isWindows)] {
	tixInitFileCmpt:Win
    } elseif [info exists env(WINDOWS_EMU_DEBUG)] {
	tixInitFileCmpt:Win
	tixWinFileEmu
    } else {
	tixInitFileCmpt:Unix
    }

    # STEP 3: Initialize the Tix application context
    #
    #

    tixAppContext tix

    # STEP 4: Initialize the bindings for widgets that are implemented in C
    #
    #
    if [string compare [info command tixHList] ""] {
	tixHListBind
    }
    if [string compare [info command tixTList] ""] {
	tixTListBind
    }
    if [string compare [info command tixGrid]  ""] {
	tixGridBind
    }
    tixComboBoxBind
    tixControlBind
    tixFloatEntryBind
    tixLabelEntryBind
    tixScrolledGridBind
    tixScrolledListBoxBind

    # STEP 5: Some ITCL compatibility stuff
    #
    #
    if $hasItcl {
	rename update __update

	# We use $colon$colon as a hack here. The reason is, starting from
	# Tix 4.0.6/4.1b1, all the double colons in Tix classnames have
	# been replaced by a single colon by a sed script. This modification
	# makes it possible to use Tix with ITCL without having to modify
	# the ITCL core.  However, we don't want the real double colon
	# (which means the global scope in ITCL) to be replaced.  The
	# $colon$colon just by-passes the sed script.
	#
	proc update {args} {
	    set colon :
	    @scope $colon$colon eval __update $args
	}

	rename tkwait __tkwait

	proc tkwait {args} {
	    set colon :
	    @scope $colon$colon eval __tkwait $args
	}
    }

    rename __tixError ""
    rename __tixInit ""
}
