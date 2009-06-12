#
# internet.tcl - tcl interface to various internet functions
#
# Copyright (C) 1998 Cygnus Solutions
# 

# ------------------------------------------------------------------
#  send_mail - send email
# ------------------------------------------------------------------

proc send_mail {to subject body} {
  global tcl_platform

  switch -- $tcl_platform(platform) {
    windows {
      ide_mapi simple-send $to $subject $body
    }    
    unix {
      exec echo $body | mail -s $subject $to &
    }
    default {
      error "platform \"$tcl_platform(platform)\" not supported"
    }
  }
}

# ------------------------------------------------------------------
#  open_url - open a URL in a browser
#  Netscape must be available for Unix.
# ------------------------------------------------------------------

proc open_url {url} {
  global tcl_platform
  switch -- $tcl_platform(platform) {
    windows {
      ide_shell_execute open $url
      # FIXME.  can we detect errors?
    }
    unix {
      if {[catch "exec netscape -remote [list openURL($url,new-window)]" result]} {
	if {[string match {*not running on display*} $result]} {
	  # Netscape is not running.  Try to start it.
	  if {[catch "exec netscape [list $url] &" result]} {
	    tk_dialog .warn "Netscape Error" "$result" error 0 Ok
	    return 0
	  }
	} elseif {[string match {couldn't execute *} $result]} {
	  tk_dialog .warn "Netscape Error" "Cannot locate \"netscape\" on your system.\nIt must be installed and in your path." error 0 Ok
	  return 0
	} else {
	  tk_dialog .warn "Netscape Error" "$result" error 0 Ok
	  return 0
	}
      }
    }
    default {
      error "platform \"$tcl_platform(platform)\" not supported"
      return 0
    }
  }
  return 1
}


