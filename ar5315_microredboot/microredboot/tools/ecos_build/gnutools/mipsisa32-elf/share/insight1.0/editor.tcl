# Editor
# Copyright 2001 Red Hat, Inc.
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
# Implements a set of editor commands
# ----------------------------------------------------------------------

namespace eval Editor {
  namespace export edit

  proc edit {loc_info} {
    global external_editor_command

    if {[info exists external_editor_command]} {
      if {[catch {uplevel \#0 "$external_editor_command edit $loc_info"} \
	     err]} {
	tk_dialog .warn-sn "Edit" $err error 0 Ok
      }
      return
    }

    lassign $loc_info baseName fnName fileName lineNum addr pc

    set newCmd [pref get gdb/editor]
    if {! [string compare $newCmd ""]} {
      tk_dialog .warn "Edit" "No editor command specified" error 0 Ok
    }

    # Replace %s with file name and %d with line number.
    regsub -all -- %s $newCmd $fileName newCmd
    regsub -all -- %d $newCmd $lineNum newCmd

    if {[catch "exec $newCmd &" err]} {
      tk_dialog .warn "Edit" $err error 0 Ok
    }
  }
}
