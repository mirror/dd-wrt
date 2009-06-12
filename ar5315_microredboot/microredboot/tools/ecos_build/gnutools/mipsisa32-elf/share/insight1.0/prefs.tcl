# Local preferences functions for Insight.
# Copyright 1997, 1998, 1999, 2002 Red Hat
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


# On STARTUP:
# 1. Options database (.Xdefaults on Unix  or ? on Windows) is read
# 2. GDB prefs file is read ("gdbtk.ini" on Windows; ".gdbtkinit" on Unix)
# 3. GDB init script is read
#
# Normally all preferences will be set in the prefs file, which is
# a generated file.  Hand-editing, if necessary, should be done to the
# GDB init script.
#
# when "save options" is selected, the contents of the
# preferences array is written out to the GDB prefs file.
#
# Usage:
#   pref_save
#   pref_read
# ----------------------------------------------------------------------
#

proc pref_read {} {
  global prefs_init_filename env gdb_ImageDir GDBTK_LIBRARY GDBStartup
  global tcl_platform

  if {[info exists env(HOME)]} {
    if {$tcl_platform(platform) == "windows"} {
      set home [ide_cygwin_path to_win32 $env(HOME)]
    } else {
      set home $env(HOME)
    }
  } else {
    set home ""
  }

  if {$tcl_platform(platform) == "windows"} {
    set prefs_init_filename "gdbtk.ini"
  } else {
    set prefs_init_filename ".gdbtkinit"
  }

  if {!$GDBStartup(inhibit_prefs)} {
    set file_opened 0
    if {[file exists $prefs_init_filename]} {
      if {[catch {open $prefs_init_filename r} fd]} {
	dbug E "$fd"
	return
      }
      set file_opened 1
    } elseif {$home != ""} {
      set name [file join $home $prefs_init_filename]
      if {[file exists $name]} {
	if {[catch {open $name r} fd]} {
	  dbug E "$fd"
	  return
	}
	set prefs_init_filename $name
	set file_opened 1
      }
    }

    if {$file_opened == "1"} {
      set section gdb
      set version 0
      while {[gets $fd line] >= 0} {
	switch -regexp -- $line {
	  {^[ \t\n]*#.*} {
	    # Comment.  We recognize one magic comment that includes
	    # the version number.
	    if {[regexp -- "^# GDBtkInitVersion: (\[0-9\]+)\$" $line \
		   dummy v]} {
	      set version $v
	    }
	  }

	  {^[ \t\n]*$} {
	    ;# empty line; ignore it
	  }

	  {\[.*\]} {
	    regexp {\[(.*)\]} $line match section
	  }

	  {[ \t\n]*option.*} {
	    set line [string trimleft $line]
	    eval $line
	  }

	  default {
	    set a ""
	    set name ""
	    set val ""
	    regexp "\[ \t\n\]*\(.+\)=\(.+\)" $line a name val
	    if {$a == "" || $name == ""} {
	      dbug W "Cannot parse line: $line"
	    } else {
	      # Must unescape equal signs in val
	      set val [unescape_value $val $version]
	      if {$section == "gdb"} {
		pref setd gdb/$name $val
	      } elseif {$section == "global" && [regexp "^font/" $name]} {
		set name [split $name /]
		set f global/
		append f [join [lrange $name 1 end] /]
		if {[lsearch [font names] $f] == -1} {
		  # new font
		  eval define_font $f $val
		} else {
		  # existing font
		  pref set global/font/[join [lrange $name 1 end] /] $val
		}
	      } elseif {$section == "global"} {
		pref setd $section/$name $val
	      } else {
		# backwards compatibility. recognize old src-font name
		if {$val == "src-font"} {set val "global/fixed"}
		pref setd gdb/$section/$name $val
	      }
	    }
	  }
	}
      }
      close $fd
    } elseif {$home != ""} {
      set prefs_init_filename [file join $home $prefs_init_filename]
    }
  
    # now set global options
    set gdb_ImageDir [file join $GDBTK_LIBRARY [pref get gdb/ImageDir]]

  }

  # finally set colors, from system if possible
  pref_set_colors
}

# ------------------------------------------------------------------
#  PROC:  pref_save - save preferences to a file and delete window
# ------------------------------------------------------------------
proc pref_save {{win {}}} {
  global prefs_init_filename GDBStartup

  if {!$GDBStartup(inhibit_prefs)} {
    debug "pref_save $prefs_init_filename"

    if {[catch {open $prefs_init_filename w} fd]} {
      debug "ERROR: $fd"
      return
    }
  
    puts $fd "\# GDBtk Init file"
    puts $fd {# GDBtkInitVersion: 1}

    set plist [pref list]
    # write out global options
    puts $fd "\[global\]"
    foreach var $plist {
      set t [split $var /]
      if {[lindex $t 0] == "global"} {
	set x [join [lrange $t 1 end] /]
	set v [escape_value [pref get $var]]

	if {$x != "" && $v != ""} {
	  puts $fd "\t$x=$v"
	}
      }
    }

    # write out gdb-global options
    puts $fd "\[gdb\]"
    foreach var $plist {
      set t [split $var /]
      # We use the funny join/lreplace code because the session code
      # can generate a key where [lindex $t 2] is empty but there is
      # still stuff after that.  This happens because the session code
      # uses filenames, which can start with `/'.
      if {[lindex $t 0] == "gdb"
	  && [string compare [join [lreplace $t 0 1] /] ""] == 0} {
	set x [lindex $t 1]
	if {$x != ""} {
	  set v [escape_value [pref get $var]]
	  puts $fd "\t$x=$v"
	}
      }
    }

    # now loop through all sections writing out values
    # FIXME: this is broken.  We should discover the list
    # dynamically.
    lappend secs load console src reg stack locals watch bp search \
      process geometry help browser kod window session mem

    foreach section $secs {
      puts $fd "\[$section\]"
      foreach var $plist {
	set t [split $var /]
	if {[lindex $t 0] == "gdb" && [lindex $t 1] == $section} {
	  set x [join [lrange $t 2 end] /]
	  set v [escape_value [pref get $var]]
	  if {$x != "" && $v != ""} {
	    puts $fd "\t$x=$v"
	  }
	}
      }
    }
    close $fd
  }

  if {$win != ""} {
    $win delete
  }
}

# -------------------------------------------------------
#  PROC: escape_value - escape all equal signs for saving
#         prefs to a file
# -------------------------------------------------------
proc escape_value {val} {
  # We use a URL-style quoting.  We encode `=', `%', the `[]'
  # characters and newlines.  We use a cute trick here: we regsub in
  # command expressions which we then expand using subst.
  if {[info tclversion] >= 8.1} {
    set expr {([\[\]=%\n])}
  } else {
    set expr "(\[\]\[=%\n\])"
  }
  regsub -all -- $expr $val \
    {[format "%%%02x" [scan {\1} %c x; set x]]} newval
  return [subst -nobackslashes -novariables $newval]
}

# -------------------------------------------------------
#  PROC: unescape_value - unescape all equal signs for
#         reading prefs from a file.  VERSION is the version
#         number of the encoding.
#  version 0 only encoded `='.
#  version 1 correctly encoded more values
# -------------------------------------------------------
proc unescape_value {val version} {
  switch -exact -- $version {
    0 {
      # Old-style encoding.
      if {[regsub -all -- {!%} $val = newval]} {
	return $newval
      }
    }

    1 {
      # Version 1 uses URL encoding.
      regsub -all -- "%(..)" $val \
	{[format %c 0x\1]} newval
      return [subst -nobackslashes -novariables $newval]
    }

    default {
      error "Unknown encoding version $version"
    }
  }

  return $val  
}

# ------------------------------------------------------------------
#  PROC:  pref_set_defaults - set up default values
# ------------------------------------------------------------------
proc pref_set_defaults {} {
  global GDBTK_LIBRARY tcl_platform gdb_ImageDir

  # Gdb global defaults
  pref define gdb/ImageDir                images2
  set gdb_ImageDir [file join $GDBTK_LIBRARY [pref get gdb/ImageDir]]
  pref define gdb/font_cache              ""
  pref define gdb/mode                    0;     # 0 no tracing, 1 tracing enabled
  pref define gdb/control_target          1;     # 0 can't control target (EMC), 1 can
  pref define gdb/B1_behavior             1;     # 0 means set/clear breakpoints,
                                                 # 1 means set/clear tracepoints.
  pref define gdb/use_icons		  1;	 # For Unix, use gdbtk_icon.gif as an icon
						 # some window managers can't deal with it.    
  # set download and execution options
  pref define gdb/load/verbose 0
  pref define gdb/load/main 1
  pref define gdb/load/exit 1
  pref define gdb/load/check 0
  pref define gdb/load/bp_at_func 0
  pref define gdb/load/bp_func ""
  pref define gdb/load/baud 38400
  if {$tcl_platform(platform) == "windows"}  {
    pref define gdb/load/port com1
  } else {
    pref define gdb/load/port "/dev/ttyS0"
  }

  # The list of active windows:
  pref define gdb/window/active           {}

  # Console defaults
  pref define gdb/console/prompt          "(gdb) "
  pref define gdb/console/deleteLeft      1
  pref define gdb/console/wrap            0
  pref define gdb/console/prompt_fg       DarkGreen
  pref define gdb/console/error_fg        red
  pref define gdb/console/log_fg          green 
  pref define gdb/console/target_fg       blue
  pref define gdb/console/font            global/fixed

  # Source window defaults
  pref define gdb/src/PC_TAG              green
  pref define gdb/src/STACK_TAG           gold
  pref define gdb/src/BROWSE_TAG          \#9595e2
  pref define gdb/src/handlebg            red
  pref define gdb/src/bp_fg               red
  pref define gdb/src/temp_bp_fg          orange
  pref define gdb/src/disabled_fg         black
  pref define gdb/src/font                global/fixed
  pref define gdb/src/break_fg            black
  pref define gdb/src/source2_fg          navy
  pref define gdb/src/variableBalloons    1
  pref define gdb/src/trace_fg            magenta
  pref define gdb/src/tab_size            8
  pref define gdb/src/linenums		  1
  pref define gdb/src/thread_fg           pink
  pref define gdb/src/top_control	  1;	# 1 srctextwin controls on top, 0 bottom

  # Define the run button's functions. These are defined here in case
  # we do a "run" with an exec target (which never causes target.tcl to 
  # source)...
  pref define gdb/src/run_attach          0
  pref define gdb/src/run_load            0
  pref define gdb/src/run_run             1
  pref define gdb/src/run_cont            0

  # This is the disassembly flavor.  For now this is only supported on x86
  # machines.

  pref define gdb/src/disassembly-flavor  ""

  # Variable Window defaults
  pref define gdb/variable/font           global/fixed
  pref define gdb/variable/disabled_fg    gray

  # Stack Window
  pref define gdb/stack/font              global/fixed

  # Register Window
  pref define gdb/reg/rows                16

  # Global Prefs Dialogs
  pref define gdb/global_prefs/save_fg    red
  pref define gdb/global_prefs/message_fg white
  pref define gdb/global_prefs/message_bg red

  # Browser Window Search
  pref define gdb/search/last_symbol      ""
  pref define gdb/search/filter_mode     "starts with"

  pref define gdb/browser/hide_h          0
  pref define gdb/browser/width           0
  pref define gdb/browser/top_height       0
  pref define gdb/browser/view_height      -1
  pref define gdb/browser/view_is_open    0

  # BP (breakpoint)
  pref define gdb/bp/show_threads         0

  # Help
  pref define gdb/help/browser		  0

  # Kernel Objects (kod)
  pref define gdb/kod/show_icon           0

  # Various possible "main" functions. What's for Java?
  pref define gdb/main_names              [list MAIN___ MAIN__ main cyg_user_start cyg_start ]

  # These are the classes of warning dialogs, and whether the user plans
  # to ignore them.
  pref define gdb/warnings/signal         0

  # Memory window.
  pref define gdb/mem/size 4
  pref define gdb/mem/numbytes 0
  pref define gdb/mem/format x
  pref define gdb/mem/ascii 1
  pref define gdb/mem/ascii_char .
  pref define gdb/mem/bytes_per_row 16
  pref define gdb/mem/color green

  # External editor.
  pref define gdb/editor ""
}

proc pref_set_colors {} {
  # set color palette
  
  # In a normal tk app, most of this is not necessary.  Unfortunately
  # Insight is a mixture of widgets from all over and was coded first
  # in tcl and later in itcl.  So lots of color inheritance is broken or wrong.
  # To enable us to fix that without hardcoding colors, we create a color
  # array here and use it as needed to force widgets to the correct colors.
  
  global Colors tcl_platform
  
  debug

  if {$tcl_platform(platform) == "windows"} {
    option add *foreground SystemButtonText
    set Colors(fg) SystemButtonText
    
    option add *background SystemButtonFace
    set Colors(bg) SystemButtonFace
    
    option add *Entry*foreground SystemWindowText
    option add *Text*foreground SystemWindowText
    set Colors(textfg) SystemWindowText
    
    option add *Entry*background SystemWindow
    option add *Text*background SystemWindow
    set Colors(textbg) SystemWindow
    
    option add *selectForeground SystemHighlightText
    set Colors(sfg) SystemHighlightText
    
    option add *selectBackground SystemHighlight
    set Colors(sbg) SystemHighlight
    
    option add *highlightBackground SystemButtonFace
    set Colors(hbg) SystemButtonFace
    return
  }

  # UNIX colors
  
  # For KDE3 (and probably earlier versions) when the user sets
  # a color scheme from the KDE control center, the appropriate color 
  # information is set in the X resource database.  Well, most of it 
  # is there but it is missing some settings, so we will carefully 
  # adjust things.
  #
  # For GNOME, you can use a program called grdb update the X resource database
  # with your current color scheme.
  #
  # If there is no information in the X rdb, we provide reasonable defaults.
  
  # create an empty entry widget so we can query its colors
  entry .e
  
  # text background
  set Colors(textbg) [option get .e background {}]
  if {$Colors(textbg) == ""} {set Colors(textbg) white}
  
  # text foreground
  set Colors(textfg) [option get .e foreground {}]
  if {$Colors(textfg) == ""} {set Colors(textfg) black}
  
  # background
  set Colors(bg) [option get . background {}]
  if {$Colors(bg) == ""} {set Colors(bg) lightgray}
  
  # foreground
  set Colors(fg) [option get . foreground {}]
  if {$Colors(fg) == ""} {set Colors(fg) black}
  
  # now reset resource database so all widgets are consistent
  option add *background $Colors(bg)
  option add *Text*background $Colors(textbg)
  option add *Entry*background $Colors(textbg)
  option add *foreground $Colors(fg)
  option add *Text*foreground $Colors(textfg)
  option add *Entry*foreground $Colors(textfg)
  
  
  # highlightBackground.  Set to background for now.
  set Colors(hbg) $Colors(bg)
  option add *highlightBackground $Colors(hbg)
  
  # selectBackground
  set Colors(sbg) [option get .e selectBackground {}]
  if {$Colors(sbg) == ""} {set Colors(sbg) blue}
  option add *selectBackground $Colors(sbg)
  
  # selectForeground
  set Colors(sfg) [option get .e selectForeground {}]
  if {$Colors(sfg) == ""} {set Colors(sfg) white}
  option add *selectForeground $Colors(sfg)
  
  # compute a slightly darker background color
  # and use for activeBackground and troughColor
  set bg2 [winfo rgb . $Colors(bg)]
  set dbg [format #%02x%02x%02x [expr {(9*[lindex $bg2 0])/2560}] \
	   [expr {(9*[lindex $bg2 1])/2560}] [expr {(9*[lindex $bg2 2])/2560}]]
  option add *activeBackground $dbg
  option add *troughColor $dbg
  
  # Change the default select color for checkbuttons, etc to match 
  # selectBackground.
  option add *selectColor $Colors(sbg)
  
  destroy .e
}
