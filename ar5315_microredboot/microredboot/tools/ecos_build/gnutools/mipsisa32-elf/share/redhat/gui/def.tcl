# def.tcl - Definining commands.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Define a global array.
proc defarray {name {value {}}} {
  upvar \#0 $name ary

  if {! [info exists ary]} then {
    set ary(_) {}
    unset ary(_)
    array set ary $value
  }
}

# Define a global variable.
proc defvar {name {value {}}} {
  upvar \#0 $name var
  if {! [info exists var]} then {
    set var $value
  }
}

# Define a "constant".  For now this is just a pretty way to declare a
# global variable.
proc defconst {name value} {
  upvar \#0 $name var
  set var $value
}
