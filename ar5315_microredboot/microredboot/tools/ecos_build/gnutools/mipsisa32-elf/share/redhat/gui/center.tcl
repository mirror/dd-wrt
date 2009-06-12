# center.tcl - Center a window on the screen or over another window
# Copyright (C) 1997, 1998, 2001 Red Hat, Inc.
# Written by Tom Tromey <tromey@cygnus.com>.

# Call this after the TOPLEVEL has been filled in, but before it has
# been mapped.  This proc will center the toplevel on the screen or
# over another window.
proc center_window {top args} {
  parse_args {{over ""}}

  update idletasks
  if {$over != ""} {
    set cx [expr {int ([winfo rootx $over] + [winfo width $over] / 2)}]
    set cy [expr {int ([winfo rooty $over] + [winfo height $over] / 2)}]
    set x [expr {$cx - int ([winfo reqwidth $top] / 2)}]
    set y [expr {$cy - int ([winfo reqheight $top] / 2)}]
  } else {
    set x [expr {int (([winfo screenwidth $top] - [winfo reqwidth $top]) / 2)}]
    set y [expr {int (([winfo screenheight $top] - [winfo reqheight $top]) / 2)}]
  }
  wm geometry $top +${x}+${y}
  wm positionfrom $top user

  # We run this update here because Tk updates toplevel geometry
  # (position) info in an idle handler on Windows, but doesn't force
  # the handler to run before mapping the window.
  update idletasks
}
