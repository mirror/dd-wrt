# mono.tcl - Dealing with monochrome.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# It is safe to run this any number of times, so it is ok to have it
# here.  Defined as true if the user wants monochrome display.
pref define global/monochrome 0

# Return 1 if monochrome, 0 otherwise.  This should be used to make
# the application experience more friendly for colorblind users as
# well as those stuck on mono displays.
proc monochrome_p {} {
  return [expr {[pref get global/monochrome] || [winfo depth .] == 1}]
}
