# font.tcl - Font handling.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.


# This function is called whenever a font preference changes.  We use
# this information to update the appropriate symbolic font.
proc FONT_track_change {symbolic prefname value} {
  eval font configure [list $symbolic] $value
}

# Primary interface to font handling.
# define_font SYMBOLIC_NAME ARGS
# Define a new font, named SYMBOLIC_NAME.  ARGS is the default font
# specification; it is a list of options such as those passed to `font
# create'.
proc define_font {symbolic args} {
  # We do a little trick with the names here, by inserting `font' in
  # the appropriate place in the name.
  set split [split $symbolic /]
  set name [join [linsert $split 1 font] /]

  pref define $name $args
  eval font create [list $symbolic] [pref get $name]
  pref add_hook $name [list FONT_track_change $symbolic]
}
