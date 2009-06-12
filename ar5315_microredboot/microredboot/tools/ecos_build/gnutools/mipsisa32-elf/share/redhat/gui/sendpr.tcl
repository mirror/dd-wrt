# sendpr.tcl - GUI to send-pr.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# FIXME:
# * consider adding ability to set various options from outside,
#   eg via the configure method.
# * Have explanatory text at the top
# * if synopsis not set, don't allow PR to be sent
# * at least one text field must have text in it before PR can be sent
# * see other fixme comments in text.

# FIXME: shouldn't have global variable.
defarray SENDPR_state

itcl_class Sendpr {
  inherit Ide_window

  # This array holds information about this site.  It is a private
  # common array.  Once initialized it is never changed.
  common _site

  # Initialize the _site array.
  global Paths tcl_platform

  # On Windows, there is no `send-pr' program.  For now, we just
  # hard-code things there to work in the most important case.
  if {$tcl_platform(platform) == "windows"} then {
    set _site(header) ""
    set _site(to) bugs@cygnus.com
    set _site(field,Submitter-Id) cygnus
    set _site(field,Originator) Nobody
    set _site(field,Release) "Internal"
    set _site(field,Organization) "Red Hat, Inc."
    set _site(field,Environment) ""
    foreach item {byteOrder machine os osVersion platform} {
      append _site(field,Environment) "$item = $tcl_platform($item)\n"
    }
    set _site(categories) foundry
  } else {
    set _site(sendpr) [file join $Paths(bindir) send-pr]
    # If it doesn't exist, try the user's path.  This is a hack for
    # developers.
    if {! [file exists $_site(sendpr)]} then {
      set _site(sendpr) send-pr
    }

    set _site(header) {}
    set outList [split [exec $_site(sendpr) -P] \n]
    set lastField {}
    foreach line $outList {
      if {[string match SEND-PR* $line]} then {
	# Nothing.
      } elseif {[regexp {^$} $line] || [regexp "^\[ \t\]" $line]} then {
	# Empty lines and lines starting with a blank are skipped.
      } elseif {$lastField == "" &&
		[regexp [format {^[^>]([^:]+):[ %s]+(.+)$} \t] \
		   $line dummy field value]} then {
	# A non-empty mail header line.  This can only occur when there
	# is no last field.
	if {[string tolower $field] == "to"} then {
	  set _site(to) $value
	}
      } elseif {[regexp {^>([^:]*):(.*)$} $line dummy field value]} then {
	# Found a field.  Set it.
	set lastField $field
	if {$value != "" && ![string match <*> [string trim $value]]} then {
	  set _site(field,$lastField) $value
	}
      } elseif {$lastField == ""} then {
	# No last field.
      } else {
	# Stuff into last field.
	if {[info exists _site(field,$lastField)]} then {
	  append _site(field,$lastField) \n
	}
	append _site(field,$lastField) $line
      }
    }
    # Now find the categories.
    regsub -all -- {[()\"]} [exec $_site(sendpr) -CL] \
      "" _site(categories)
    set _site(categories) [lrmdups [concat foundry $_site(categories)]]
  }

  # Internationalize some text.  We have to do this because of how
  # Tk's optionmenu works.  Indices here are the names that GNATS
  # wants; this is important.
  set _site(sw-bug) [gettext "Software bug"]
  set _site(doc-bug) [gettext "Documentation bug"]
  set _site(change-request) [gettext "Change request"]
  set _site(support) [gettext "Support"]
  set _site(non-critical) [gettext "Non-critical"]
  set _site(serious) [gettext "Serious"]
  set _site(critical) [gettext "Critical"]
  set _site(low) [gettext "Low"]
  set _site(medium) [gettext "Medium"]
  set _site(high) [gettext "High"]

  # Any text passed to constructor is saved and put into Description
  # section of output.
  constructor {{text ""}} {
    Ide_window::constructor [gettext "Report Bug"]
  } {
    global SENDPR_state

    # The standard widget-making trick.
    set class [$this info class]
    set hull [namespace tail $this]
    set old_name $this
    ::rename $this $this-tmp-
    # For now always make a toplevel.  Number 7 comes from Windows
    ::rename $hull $old_name-win-
    ::rename $this $old_name
    ::rename $this $this-win-
    ::rename $this-tmp- $this

    wm withdraw  [namespace tail $this]
###FIXME - this constructor callout will cause the parent constructor to be called twice

    ::set SENDPR_state($this,desc) $text

    #
    # The Classification frame.
    #

    Labelledframe [namespace tail $this].cframe -text [gettext "Classification"]
    set parent [[namespace tail $this].cframe get_frame]

    tixComboBox $parent.category -dropdown 1 -editable 0 \
      -label [gettext "Category"] -variable SENDPR_state($this,category)
    foreach item $_site(categories) {
      $parent.category insert end $item
    }
    # FIXME: allow user of this class to set default category.
    ::set SENDPR_state($this,category) foundry

    ::set SENDPR_state($this,secret) no
    checkbutton $parent.secret -text [gettext "Confidential"] \
      -variable SENDPR_state($this,secret) -onvalue yes -offvalue no \
      -anchor w

    # FIXME: put labels on these?
    set m1 [_make_omenu $parent.class class 0 \
	      sw-bug doc-bug change-request support]
    set m2 [_make_omenu $parent.severity severity 1 \
	      non-critical serious critical]
    set m3 [_make_omenu $parent.priority priority 1 \
	      low medium high]
    if {$m1 > $m2} then {
      set m2 $m1
    }
    if {$m2 > $m3} then {
      set m3 $m2
    }
    $parent.class configure -width $m3
    $parent.severity configure -width $m3
    $parent.priority configure -width $m3

    grid $parent.category $parent.severity -sticky nw -padx 2
    grid $parent.secret $parent.class -sticky nw -padx 2
    grid x $parent.priority -sticky nw -padx 2

    #
    # The text and entry frames.
    #

    Labelledframe [namespace tail $this].synopsis -text [gettext "Synopsis"]
    set parent [[namespace tail $this].synopsis get_frame]
    entry $parent.synopsis -textvariable SENDPR_state($this,synopsis)
    pack $parent.synopsis -expand 1 -fill both

    # Text fields.  Each is wrapped in its own label frame.
    # We decided to eliminate all the frames but one; the others are
    # just confusing.
    ::set SENDPR_state($this,repeat) [_make_text [namespace tail $this].desc \
					[gettext "Description"]]

    # Some buttons.
    frame [namespace tail $this].buttons -borderwidth 0 -relief flat
    button [namespace tail $this].buttons.send -text [gettext "Send"] \
      -command [list $this _send]
    button [namespace tail $this].buttons.cancel -text [gettext "Cancel"] \
      -command [list destroy $this]
    button [namespace tail $this].buttons.help -text [gettext "Help"] -state disabled
    standard_button_box [namespace tail $this].buttons

    # FIXME: we'd really like to have sashes between the text widgets.
    # iwidgets or tix will provide that for us.
    grid [namespace tail $this].cframe -sticky ew -padx 4 -pady 4
    grid [namespace tail $this].synopsis -sticky ew -padx 4 -pady 4
    grid [namespace tail $this].desc -sticky news -padx 4 -pady 4
    grid [namespace tail $this].buttons -sticky ew -padx 4

    grid rowconfigure  [namespace tail $this] 0 -weight 0
    grid rowconfigure  [namespace tail $this] 1 -weight 0
    grid rowconfigure  [namespace tail $this] 2 -weight 1
    grid rowconfigure  [namespace tail $this] 3 -weight 1
    grid columnconfigure  [namespace tail $this] 0 -weight 1

    bind [namespace tail $this].buttons <Destroy> [list $this delete]

    wm deiconify  [namespace tail $this]
  }

  destructor {
    global SENDPR_state
    foreach item [array names SENDPR_state $this,*] {
      ::unset SENDPR_state($item)
    }
    catch {destroy $this}
  }

  method configure {config} {}

  # Create an optionmenu and fill it.  Also, go through all the items
  # and find the one that makes the menubutton the widest.  Return the
  # max width.  Private method.
  method _make_omenu {name index def_index args} {
    global SENDPR_state

    set max 0
    set values {}
    # FIXME: we can't actually examine which one makes the menubutton
    # widest.  Why not?  Because the menubutton's -width option is in
    # characters, but we can only look at the width in pixels.
    foreach item $args {
      lappend values $_site($item)
      if {[string length $_site($item)] > $max} then {
	set max [string length $_site($item)]
      }
    }

    eval tk_optionMenu $name SENDPR_state($this,$index) $values

    ::set SENDPR_state($this,$index) $_site([lindex $args $def_index])

    return $max
  }

  # Create a labelled frame and put a text widget in it.  Private
  # method.
  method _make_text {name text} {
    Labelledframe $name -text $text
    set parent [$name get_frame]
    text $parent.text -width 80 -height 15 -wrap word \
      -yscrollcommand [list $parent.vb set]
    scrollbar $parent.vb -orient vertical -command [list $parent.text yview]
    grid $parent.text -sticky news
    grid $parent.vb -row 0 -column 1 -sticky ns
    grid rowconfigure $parent 0 -weight 1
    grid columnconfigure $parent 0 -weight 1
    grid columnconfigure $parent 1 -weight 0
    return $parent.text
  }

  # This takes a text string and finds the element of site which has
  # the same value.  It returns the corresponding key.  Private
  # method.
  method _invert {text values} {
    foreach item $values {
      if {$_site($item) == $text} then {
	return $item
      }
    }
    error "couldn't find \"$text\""
  }

  # Send the PR.  Private method.
  method _send {} {
    global SENDPR_state

    set email {}

    if {[info exists _site(field,Submitter-Id)]} then {
      set _site(field,Customer-Id) $_site(field,Submitter-Id)
      unset _site(field,Submitter-Id)
    }

    foreach field {Customer-Id Originator Release} {
      append email ">$field: $_site(field,$field)\n"
    }
    foreach field {Organization Environment} {
      append email ">$field:\n$_site(field,$field)\n"
    }

    append email ">Confidential: "
    if {$SENDPR_state($this,secret)} then {
      append email yes\n
    } else {
      append email no\n
    }

    append email ">Synopsis: $SENDPR_state($this,synopsis)\n"

    foreach field {Severity Priority Class} \
            values {{non-critical serious critical} {low medium high}
	      {sw-bug doc-bug change-request support}} {
      set name [string tolower $field]
      set value [_invert $SENDPR_state($this,$name) $values]
      append email ">$field: $value\n"
    }

    append email ">Category: $SENDPR_state($this,category)\n"

    # Now big things.
    append email ">How-To-Repeat:\n"
    append email "[$SENDPR_state($this,repeat) get 1.0 end]\n"

    # This isn't displayed to the user, but can be set by the caller.
    append email ">Description:\n$SENDPR_state($this,desc)\n"

    send_mail $_site(to) $SENDPR_state($this,synopsis) $email

    destroy $this
  }

  # Override from Ide_window.
  method idew_save {} {
    global SENDPR_state

    foreach name {category secret severity priority class synopsis} {
      set result($name) $SENDPR_state($this,$name)
    }
    # Stop just before `end'; otherwise we add a newline each time.
    set result(repeat) [$SENDPR_state($this,repeat) get 1.0 {end - 1c}]
    set result(desc) $SENDPR_state($this,desc)

    return [list Sendpr :: _restore [array get result]]
  }

  # This is used to restore a bug report window.  Private proc.
  proc _restore {alist x y width height visibility} {
    global SENDPR_state

    array set values $alist

    set name .[gensym]
    Sendpr $name $values(desc)
    foreach name {category secret severity priority class synopsis} {
      ::set $SENDPR_state($this,$name) $values($name)
    }
    $SENDPR_state($name,repeat) insert end $desc

    $name idew_set_geometry $x $y $width $height
    $name idew_set_visibility $visibility
  }
}
