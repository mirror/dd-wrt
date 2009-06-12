# path.tcl - Path-handling helpers.
# Copyright (C) 1998 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# This proc takes a possibly relative path and expands it to the
# corresponding fully qualified path.  Additionally, on Windows the
# result is guaranteed to be in "long" form.
proc canonical_path {path} {
  global tcl_platform

  set r [file join [pwd] $path]
  if {$tcl_platform(platform) == "windows"} then {
    # This will fail if the file does not already exist.
    if {! [catch {file attributes $r -longname} long]} then {
      set r $long
    }
  }

  return $r
}
