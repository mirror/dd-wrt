# print.tcl -- some procedures for dealing with printing.  To print
# PostScript on Windows, tkmswin.dll will need to be present.

proc send_printer { args } {
    global tcl_platform

    parse_args {
	{printer {}}
	{outfile {}}
	{parent {}}
	ascii
	file
    }

    if {[llength $args] == 0} {
	error "No filename or data provided."
    }

    if {$ascii == 1} {
	if {$tcl_platform(platform) == "windows"} then {
	    PRINT_windows_ascii -file $file -parent $parent [lindex $args 0]
	} else {
	    send_printer_ascii -printer $printer -file $file \
		    -outfile $outfile [lindex $args 0]
	}
	return
    }

    if {$outfile != ""} {
	if {$file} {
	    file copy [lindex 0 $args] $outfile
	} else {
	    set F [open $outfile w]
	    puts $F [lindex 0 $args]
	    close $F
	}
	return
    }

    if {$tcl_platform(platform) == "windows"} then {
	load tkmswin.dll

	set cmd {tkmswin print -postscript}
	if {$printer != ""} {
	    lappend cmd -printer $printer
	}
	if {$file} {
	    lappend cmd -file
	}
	lappend cmd [lindex $args 0]
	eval $cmd

    } else {

	# Unix box, assume lpr, but if it fails try lp.
	foreach prog {lpr lp} {
	    set cmd [list exec $prog]
	    if {$printer != ""} {
		if {$prog == "lpr"} {
		    lappend cmd "-P$printer"
		} else {
		    lappend cmd "-d$printer"
		}
	    }
	    if {$file} {
		lappend cmd "<"
	    } else {
		lappend cmd "<<"
	    }
	    # tack on data or filename
	    lappend cmd [lindex $args 0]
	    
	    # attempt to run the command, and exit if successful
	    if ![catch {eval $cmd} ret] {
		return
	    }
	}
	error "Couldn't run either `lpr' or `lp' to print"
    }
}

proc send_printer_ascii { args } {
    global tcl_platform

    parse_args {
	{printer {}}
	{outfile {}}
	{file 0}
	{font Courier}
	{fontsize 10}
	{pageheight 11}
	{pagewidth 8.5}
	{margin .5}
    }
    if {[llength $args] == 0} {
	error "No filename or data provided."
    }

    if {$tcl_platform(platform) == "windows"} then {
	PRINT_windows_ascii -file $file [lindex $args 0]
	return
    }

    # convert the filename or data to ascii, and then send to the printer.

    set inch 72
    set pageheight [expr $pageheight*$inch]
    set pagewidth [expr $pagewidth*$inch]
    set margin [expr $margin*$inch]

    set output "%!PS-Adobe-1.0\n"
    append output "%%Creator: libgui ASCII-to-PS converter\n"
    append output "%%DocumentFonts: $font\n"
    append output "%%Pages: (atend)\n"
    append output "/$font findfont $fontsize scalefont setfont\n"
    append output "/M{moveto}def\n"
    append output "/S{show}def\n"

    set pages 1
    set y [expr $pageheight-$margin-$fontsize]

    if {$file == 1} {
	set G [open [lindex $args 0] r]
	set strlen [gets $G str]
    } else {
	# make sure that we end with a newline
	set args [lindex $args 0]
	append args "\n"
	
	set strlen [string first "\n" $args]
	if {$strlen != -1} {
	    set str [string range $args 0 [expr $strlen-1]]
	    set args [string range $args [expr $strlen+1] end]
	}
    }
    while {$strlen != -1} {
	if {$y < $margin} {
	    append output "showpage\n"
	    incr pages
	    set y [expr $pageheight-$margin-$fontsize]
	}
	regsub -all {[()\\]} $str {\\&} str
	append output "$margin $y M ($str) S\n"
	set y [expr $y-($fontsize+1)]

	if {$file == 1} {
	    set strlen [gets $G str]
	} else {
	    set strlen [string first "\n" $args]
	    if {$strlen != -1} {
		set str [string range $args 0 [expr $strlen-1]]
		set args [string range $args [expr $strlen+1] end]
	    }
	}

    }
    append output "showpage\n"
    append output "%%Pages: $pages\n"

    if {$file == 1} {
	close $G
    }
    
    send_printer -printer $printer -outfile $outfile $output
}

# Print ASCII text on Windows.

proc PRINT_windows_ascii { args } {
    global tcl_platform errorInfo
    global PRINT_state

    parse_args {
	{file 0}
	{parent {}}
    }
    if {[llength $args] == 0} {
	error "No filename or data provided."
    }

    if {$tcl_platform(platform) != "windows"} then {
	error "Only works on Windows"
    }

    # Copied from tk_dialog, except that it returns.
    catch {destroy .cancelprint}
    toplevel .cancelprint -class Dialog
    wm withdraw .cancelprint
    wm title .cancelprint [gettext "Printing"]
    frame .cancelprint.bot
    frame .cancelprint.top
    pack .cancelprint.bot -side bottom -fill both
    pack .cancelprint.top -side top -fill both -expand 1
    set PRINT_state(pageno) [format [gettext "Now printing page %d"] 0]
    label .cancelprint.msg -justify left -textvariable PRINT_state(pageno)
    pack .cancelprint.msg -in .cancelprint.top -side right -expand 1 \
	    -fill both -padx 1i -pady 5
    button .cancelprint.button -text [gettext "Cancel"] \
	    -command { ide_winprint abort } -default active
    grid .cancelprint.button -in .cancelprint.bot -column 0 -row 0 \
	    -sticky ew -padx 10
    grid columnconfigure .cancelprint.bot 0

    update idletasks
    set x [expr [winfo screenwidth .cancelprint]/2 \
	    - [winfo reqwidth .cancelprint]/2 \
	    - [winfo vrootx [winfo parent .cancelprint]]]
    set y [expr [winfo screenheight .cancelprint]/2 \
	    - [winfo reqheight .cancelprint]/2 \
	    - [winfo vrooty [winfo parent .cancelprint]]]
    wm geom .cancelprint +$x+$y
    update

    # We're going to change the focus and the grab as soon as we start
    # printing, so remember them now.
    set oldFocus [focus]
    set oldGrab [grab current .cancelprint]
    if {$oldGrab != ""} then {
	set grabStatus [grab status $oldGrab]
    }

    focus .cancelprint.button

    set PRINT_state(start) 1
    set PRINT_state(file) $file
    if {$file == 1} then {
	set PRINT_state(fp) [open [lindex $args 0] r]
    } else {
	set PRINT_state(text) [lindex $args 0]
    }

    set cmd [list ide_winprint print_text PRINT_query PRINT_text \
	       -pageproc PRINT_page]
    if {$parent != {}} then {
	lappend cmd -parent $parent
    }

    set code [catch $cmd errmsg]
    set errinfo $errorInfo

    catch { focus $oldFocus }
    catch { destroy .cancelprint }
    if {$oldGrab != ""} then {
	if {$grabStatus == "global"} then {
	    grab -global $oldGrab
	} else {
	    grab $oldGrab
	}
    }

    if {$code == 1} then {
	error $errmsg $errinfo
    }
}

# The query procedure passed to ide_winprint print_text.  This should
# return one of "continue", "done", or "newpage".

proc PRINT_query { } {
    global PRINT_state

    # Fetch the next line into PRINT_state(str).

    if {$PRINT_state(file) == 1} then {
	set strlen [gets $PRINT_state(fp) PRINT_state(str)]
    } else {
	set strlen [string first "\n" $PRINT_state(text)]
	if {$strlen != -1} then {
	    set PRINT_state(str) \
		    [string range $PRINT_state(text) 0 [expr $strlen-1]]
	    set PRINT_state(text) \
		    [string range $PRINT_state(text) [expr $strlen+1] end]
	} else {
	    if {$PRINT_state(text) != ""} then {
		set strlen 0
		set PRINT_state(str) $PRINT_state(text)
		set PRINT_state(text) ""
	    }
	}
    }

    if {$strlen != -1} then {

	# Expand tabs assuming tabstops every 8 spaces and a fixed
	# pitch font.  Text written to other assumptions will have to
	# be handled by the caller.

	set str $PRINT_state(str)
	while {[set i [string first "\t" $str]] >= 0} {
	    set c [expr 8 - ($i % 8)]
	    set spaces ""
	    while {$c > 0} {
		set spaces "$spaces "
		incr c -1
	    }
	    set str "[string range $str 0 [expr $i - 1]]$spaces[string range $str [expr $i + 1] end]"
	}
	set PRINT_state(str) $str

	return "continue"
    } else {
	return "done"
    }
}

# The text procedure passed to ide_winprint print_text.  This should
# return the next line to print.

proc PRINT_text { } {
    global PRINT_state

    return $PRINT_state(str)
}

# This page procedure passed to ide_winprint print_text.  This is
# called at the start of each page.

proc PRINT_page { pageno } {
    global PRINT_state

    set PRINT_state(pageno) [format [gettext "Now printing page %d"] $pageno]

    if {$PRINT_state(start)} then {
	wm deiconify .cancelprint

	grab .cancelprint
	focus .cancelprint.button

	set PRINT_state(start) 0
    }

    update
    return "continue"
}
