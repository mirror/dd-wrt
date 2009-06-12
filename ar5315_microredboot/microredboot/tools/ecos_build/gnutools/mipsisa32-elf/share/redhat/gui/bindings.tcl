# bindings.tcl - Procs to handle bindings.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Reorder the bindtags so that the tag appears before the widget.
# Tries to preserve other relative orderings as much as possible.  In
# particular, nothing changes if the widget is already after the tag.
proc bind_widget_after_tag {w tag} {
  set seen_tag 0
  set seen_widget 0
  set new_list {}
  foreach tag [bindtags $w] {
    if {$tag == $tag} then {
      lappend new_list $tag
      if {$seen_widget} then {
	lappend new_list $w
      }
      set seen_tag 1
    } elseif {$tag == $w} then {
      if {$seen_tag} then {
	lappend new_list $tag
      }
      set seen_widget 1
    } else {
      lappend new_list $tag
    }
  }

  if {! $seen_widget} then {
    lappend new_list $w
  }

  bindtags $w $new_list
}

# Reorder the bindtags so that the class appears before the widget.
# Tries to preserve other relative orderings as much as possible.  In
# particular, nothing changes if the widget is already after the
# class.
proc bind_widget_after_class {w} {
  bind_widget_after_tag $w [winfo class $w]
}

# Make the specified binding for KEY and empty bindings for common
# modifiers for KEY.  This can be used to ensure that a binding won't
# also be triggered by (eg) Alt-KEY.  This proc also makes the binding
# case-insensitive.  KEY is either the name of a key, or a key with a
# single modifier.
proc bind_plain_key {w key binding} {
  set l [split $key -]
  if {[llength $l] == 1} then {
    set mod {}
    set part $key
  } else {
    set mod "[lindex $l 0]-"
    set part [lindex $l 1]
  }

  set modifiers {Meta- Alt- Control-}

  set part_list [list $part]
  # If we just have a single letter, then we can't look for
  # Shift-PART; we must use the uppercase equivalent.
  if {[string length $part] == 1} then {
    # This is nasty: if we bind Control-L, we won't see the events we
    # want.  Instead we have to bind Shift-Control-L.  Actually, we
    # must also bind Control-L so that we'll see the event if the Caps
    # Lock key is down.
    if {$mod != ""} then {
      lappend part_list "Shift-[string toupper $part]"
    }
    lappend part_list [string toupper $part]
  } else {
    lappend modifiers Shift-
  }

  foreach part $part_list {
    # Bind the key itself (with modifier if required).
    bind $w <${mod}${part}> $binding

    # Ignore any modifiers other than the one we like.
    foreach onemod $modifiers {
      if {$onemod != $mod} then {
	bind $w <${onemod}${part}> {;}
      }
    }
  }
}
