# topbind.tcl - Put a binding on a toplevel.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.
#
# Put a binding on a toplevel.  This needs a separate proc because by
# default the toplevel's name is put into the bindtags list for all
# its descendents.  Eg Destroy bindings typically don't want to be run
# more than once.
#

# FIXME: should catch destroy operations and remove all bindings for
# our tag.

# Make the binding.  Return nothing.
proc bind_for_toplevel_only {toplevel sequence script} {
  set tagList [bindtags $toplevel]
  set tag _DBind_$toplevel
  if {[lsearch -exact $tagList $tag] == -1} then {
    # Always put our new binding first in case the other bindings run
    # break.
    bindtags $toplevel [concat $tag $tagList]
  }

  # Use "+" binding in case there are multiple calls to this.  FIXME
  # should just use gensym.
  bind $tag $sequence +$script

  return {}
}
