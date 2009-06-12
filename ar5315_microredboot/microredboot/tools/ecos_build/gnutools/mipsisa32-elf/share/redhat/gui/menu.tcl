# menu.tcl - Useful proc for dealing with menus.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# This proc computes the "desired width" of a menu.  It can be used to
# determine the minimum width for a toplevel whose -menu option is
# set.
proc compute_menu_width {menu} {
  set width 0
  set last [$menu index end]
  if {$last != "end"} then {
    # Start at borderwidth, but also preserve borderwidth on the
    # right.
    incr width [expr {2 * [$menu cget -borderwidth]}]

    set deffont [$menu cget -font]
    set abw [expr {2 * [$menu cget -activeborderwidth]}]
    for {set i 0} {$i <= $last} {incr i} {
      if {[catch {$menu entrycget $i -font} font]} then {
	continue
      }
      if {$font == ""} then {
	set font $deffont
      }
      incr width [font measure $font [$menu entrycget $i -label]]
      incr width $abw
      # "10" was chosen by reading tkUnixMenu.c.
      incr width 10
      # This is arbitrary.  Apparently I can't read tkUnixMenu.c well
      # enough to understand why the naive calculation above doesn't
      # work.
      incr width 2
    }
    # Another hack.
    incr width 2
  }

  return $width
}
