# advice.tcl - Generic advice package.
# Copyright (C) 1998 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# Please note that I adapted this from some code I wrote elsewhere,
# for non-Cygnus reasons.  Don't complain to me if you see something
# like it somewhere else.


# Internal state.
defarray ADVICE_state

# This is a helper proc that does all the actual work.
proc ADVICE_do {command argList} {
  global ADVICE_state

  # Run before advice.
  if {[info exists ADVICE_state(before,$command)]} {
    foreach item $ADVICE_state(before,$command) {
      # We purposely let errors in advice go uncaught.
      uplevel $item $argList
    }
  }

  # Run the command itself.
  set code [catch \
	      [list uplevel \#0 $ADVICE_state(original,$command) $argList] \
	      result]

  # Run the after advice.
  if {[info exists ADVICE_state(after,$command)]} {
    foreach item $ADVICE_state(after,$command) {
      # We purposely let errors in advice go uncaught.
      uplevel $item [list $code $result] $argList
    }
  }

  # Return just as the original command would.
  return -code $code $result
}

# Put some advice on a proc or command.
#  WHEN says when to run the advice - `before' or `after' the
#     advisee is run.
#  WHAT is the name of the proc or command to advise.
#  ADVISOR is the advice.  It is passed the arguments to the advisee
#     call as its arguments.  In addition, `after' advisors are
#     passed the return code and return value of the proc as their
#     first and second arguments.
proc advise {when what advisor} {
  global ADVICE_state

  if {! [info exists ADVICE_state(original,$what)]} {
    set newName [gensym]
    rename $what $newName
    set ADVICE_state(original,$what) $newName

    # Create a new proc which just runs our internal command with the
    # correct arguments.
    uplevel \#0 [list proc $what args \
		   [format {ADVICE_do %s $args} $what]]
  }

  lappend ADVICE_state($when,$what) $advisor
}

# Remove some previously-set advice.  Note that we could undo the
# `rename' when the last advisor is removed.  This adds complexity,
# though, and there isn't much reason to.
proc unadvise {when what advisor} {
  global ADVICE_state

  if {[info exists ADVICE_state($when,$what)]} {
    set newList {}
    foreach item $ADVICE_state($when,$what) {
      if {[string compare $advisor $item]} {
	lappend newList $item
      }
    }
    set ADVICE_state($when,$what) $newList
  }
}
