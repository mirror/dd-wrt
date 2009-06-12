# Warning dialog for GDBtk.
# Copyright 1998, 1999 Cygnus Solutions
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


# -----------------------------------------------------------------------------
# NAME:
#       class WarningDlg
#
# DESC:
#       This class implements a warning dialog.  It has an optional checkbox
#       that the user can select to disable all warnings of the same type.
#
# ARGS:
#	-ignorable "class"	- Causes an ignorable dialog to be created.
#				"class" is the warning class that will be either
#				displayed or ignored.  It may be any string, so
#				long as the same string is used for all related
#				warning messages.
#
#	-message "msg"		- Message to be displayed.
# -----------------------------------------------------------------------------
#

class WarningDlg {
  inherit ManagedWin ModalDialog

  public {
    variable ignorable ""
    variable message ""
    method constructor {args}
  }

  protected common ignore
}
 
# -----------------------------------------------------------------------------
# NAME: 
#       WarningDlg::constructor
#
# DESC: 
#       Creates the warning dialog.
# -----------------------------------------------------------------------------
body WarningDlg::constructor {args} {
  debug $args
  window_name "Warning"
  eval itk_initialize $args

  if {$ignorable == ""} {
    tk_messageBox -message $message -type ok -icon warning -default ok \
      -parent [winfo toplevel $itk_interior]
    delete
    return
  } else {
    if {[info exists ignore($ignorable)]} {
      if {$ignore($ignorable)} { 
	delete
	return 
      }
    } else {
      set ignore($ignorable) 0
    }
  }
  
  frame $itk_interior.f
  frame $itk_interior.f.a -relief raised -bd 1
  frame $itk_interior.f.b -relief raised -bd 1
  set f $itk_interior.f.a
  
 
  label $f.bitmap -bitmap warning
  label $f.lab -text $message
  pack $f.bitmap $f.lab -side left -padx 10 -pady 10

  if {$ignorable != ""} {
    checkbutton $itk_interior.f.b.ignore -text "Don't show this warning again" \
      -variable [scope ignore($ignorable)] -anchor w 
  }
  
  button $itk_interior.f.b.ok -text OK -underline 0 -command [code $this unpost]
  bind $itk_interior.f.b.ok <Return> \
    "$itk_interior.f.b.ok flash; $itk_interior.f.b.ok invoke"
  focus $itk_interior.f.b.ok

  if {$ignorable != ""} {
    pack $itk_interior.f.b.ignore
  }

  pack $itk_interior.f.b.ok -expand yes -side left 
  pack $itk_interior.f.a 
  pack $itk_interior.f.b  -fill x
  pack $itk_interior.f
}
