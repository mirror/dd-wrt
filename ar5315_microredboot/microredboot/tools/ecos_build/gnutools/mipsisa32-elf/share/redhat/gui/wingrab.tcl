# wingrab.tcl -- grab support for Windows.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Ian Lance Taylor <ian@cygnus.com>.

# Disable a list of windows.

proc WINGRAB_disable { args } {
  foreach w $args {
    ide_grab_support_disable [wm frame $w]
  }
}

# Disable all top level windows, other than the argument, which are
# children of `.'.  Note that if you do this, and then destroy the
# frame of the only enabled window, your application will lose the
# input focus to some other application.  Make sure that you reenable
# the windows before calling wm transient or wm withdraw or destroy on
# the only enabled window.

proc WINGRAB_disable_except { window } {
  foreach w [winfo children .] {
    if {$w != $window} then {
      ide_grab_support_disable [wm frame [winfo toplevel $w]]
    }
  }
}

# Enable a list of windows.

proc WINGRAB_enable { args } {
  foreach w $args {
    ide_grab_support_enable [wm frame $w]
  }
}

# Enable all top level windows which are children of `.'.

proc WINGRAB_enable_all {} {
  foreach w [winfo children .] {
    ide_grab_support_enable [wm frame [winfo toplevel $w]]
  }
}

# The basic routine.  All commands are subcommands of this.

proc ide_grab_support {dispatch args} {
  global tcl_platform

  if {[info commands WINGRAB_$dispatch] == ""} then {
    error "unrecognized key \"$dispatch\""
  }

  # We only need to do stuff on Windows.
  if {$tcl_platform(platform) != "windows"} then {
    return
  }

  eval WINGRAB_$dispatch $args
}
