# bbox.tcl - Function for handling button box.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Pass this proc a frame whose children are all buttons.  It will put
# the children into the frame so that they look right on the current
# platform.  On Windows this means that they are all the same width
# and have a uniform separation.  (And currently on Unix it means this
# same thing, though that might change.)
proc standard_button_box {frame {horizontal 1}} {
  # This is half the separation we want between the buttons.  This
  # number comes from the Windows UI "standards" manual.
  set half_gap 2

  set width 0
  foreach button [winfo children $frame] {
    set bw [winfo reqwidth $button]
    if {$bw > $width} then {
      set width $bw
    }
  }

  incr width $half_gap
  incr width $half_gap

  if {$horizontal} then {
    set i 1
  } else {
    set i 0
  }
  foreach button [winfo children $frame] {
    if {$horizontal} then {
      # We set the size via the grid, and not -width on the button.
      # Why?  Because in Tk -width has different units depending on the
      # contents of the button.  And worse, the font units don't really
      # make sense when dealing with a proportional font.
      grid $button -row 0 -column $i -sticky ew \
	-padx $half_gap -pady $half_gap
      grid columnconfigure $frame $i -weight 0 -minsize $width
    } else {
      grid $button -column 0 -row $i -sticky new \
	-padx $half_gap -pady $half_gap
      grid rowconfigure $frame $i -weight 0
    }
    incr i
  }

  if {$horizontal} then {
    # Make the empty column 0 suck up all the space.
    grid columnconfigure $frame 0 -weight 1
  } else {
    grid columnconfigure $frame 0 -minsize $width
    # Make the last row suck up all the space.
    incr i -1
    grid rowconfigure $frame $i -weight 1
  }
}
