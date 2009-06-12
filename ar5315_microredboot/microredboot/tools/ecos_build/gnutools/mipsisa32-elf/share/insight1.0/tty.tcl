# tty.tcl - xterm as tty for the inferior
# Copyright (C) 1996, 2000 Red Hat, Inc
# Written by Tom Tromey <tromey@cygnus.com>
#
# Interface to the inferior's terminal.  This is very rough, and is
# guaranteed to only work on Unix machines (if even there).
#

namespace eval tty {
  namespace export create

  variable _xterm_fd {}

  proc create {} {
    variable _xterm_fd

    destroy

    # Tricky: we exec /bin/cat so that the xterm will exit whenever we
    # close the write end of the pipe.  Note that the stdin
    # redirection must come after tty is run; tty looks at its stdin.
    set shcmd {/bin/sh -c 'exec 1>&7; tty; exec /bin/cat 0<&6'}

    set fg [option get . foreground Foreground]
    if {$fg == ""} then {
      set fg black
    }

    set bg [. cget -background]
    if {$bg == ""} then {
      set bg [lindex [. configure -background] 3]
    }

    set xterm [list /bin/sh -c "exec xterm -T 'Gdb Child' -n Gdb -bg '$bg' -fg '$fg' -e $shcmd 6<&0 7>&1"]

    # Need both read and write access to xterm process.
    set _xterm_fd [open "| $xterm" w+]
    set tty [gets $_xterm_fd]

    # On failure we don't try the tty command.
    if {$tty != ""} {
      gdb_cmd "tty $tty"
    }
  }

  proc destroy {} {
    variable _xterm_fd

    if {$_xterm_fd != ""} then {
      # We don't care if this fails.
      catch {close $_xterm_fd}
    }
    set _xterm_fd {}
  }
}
