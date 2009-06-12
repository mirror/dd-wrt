# ulset.tcl - Set labels based on info from gettext.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Extract underline and label info from a descriptor string.  Any
# underline in the descriptor is extracted, and the next character's
# index is used as the -underline value.  There can only be one _ in
# the label.
proc extract_label_info {option label} {
  set uList [split $label _]
  if {[llength $uList] > 2} then {
    error "too many underscores in label \"$label\""
  }

  if {[llength $uList] == 1} then {
    set ul -1
  } else {
    set ul [string length [lindex $uList 0]]
  }

  return [list $option [join $uList {}] -underline $ul]
}
