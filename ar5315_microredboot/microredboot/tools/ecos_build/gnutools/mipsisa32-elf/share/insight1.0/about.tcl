# About window for GDBtk.
# Copyright 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (GPL) as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.


# ----------------------------------------------------------------------
# Implements About window
# ----------------------------------------------------------------------

class About {
  inherit ManagedWin ModalDialog
  constructor {args} {
    global gdb_ImageDir
    set f [frame $itk_interior.f]
    label $f.image1 -bg #ee0000 -image \
      [image create photo -file [file join $gdb_ImageDir insight.gif]]
    message $f.m -bg #ee0000 -fg white -text [gdb_cmd {show version}] \
      -aspect 500 -relief flat
    pack $f.image1 $f.m $itk_interior.f -fill both -expand yes
    pack  $itk_interior
    bind $f.image1 <1> [code $this unpost]
    bind $f.m <1> [code $this unpost]
    window_name "About Red Hat Insight"
  }

  # Don't quit if this is the last window.  The only way that this can
  # happen is if we are the splash screen. 

  method quit_if_last {} { 
    return 0
  }

}

