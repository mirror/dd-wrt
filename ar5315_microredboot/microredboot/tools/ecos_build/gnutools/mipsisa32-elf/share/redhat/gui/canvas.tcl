# canvas.tcl - Handy canvas-related commands.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Set scroll region on canvas.
proc set_scroll_region {canvas} {
  set bbox [$canvas bbox all]
  if {[llength $bbox]} then {
    set sr [lreplace $bbox 0 1 0 0]
  } else {
    set sr {0 0 0 0}
  }

  # Don't include borders in the scrollregion.
  set delta [expr {2 * ([$canvas cget -borderwidth]
			+ [$canvas cget -highlightthickness])}]

  set ww [winfo width $canvas]
  if {[lindex $sr 2] < $ww} then {
    set sr [lreplace $sr 2 2 [expr {$ww - $delta}]]
  }

  set wh [winfo height $canvas]
  if {[lindex $sr 3] < $wh} then {
    set sr [lreplace $sr 3 3 [expr {$wh - $delta}]]
  }

  $canvas configure -scrollregion $sr
}
