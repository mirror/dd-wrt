# postghost.tcl - Ghost a menu item at post time.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.


# Helper proc.
proc GHOST_helper {menu index predicate} {
  if {[eval $predicate]} then {
    set state normal
  } else {
    set state disabled
  }
  $menu entryconfigure $index -state $state
}

# Add a -postcommand to a menu.  This is careful not to stomp other
# postcommands.
proc add_post_command {menu callback} {
  set old [$menu cget -postcommand]
  # We use a "\n" and not a ";" to separate so that people can put
  # comments into their -postcommands without fear.
  $menu configure -postcommand "$old\n$callback"
}

# Run this to make a menu item which ghosts or unghosts depending on a
# predicate that is run at menu-post time.  The NO_CACHE option
# prevents the index from being looked up statically; this is useful
# if you want to use an entry name as the index and you have a very
# dynamic menu (ie one where the numeric index of a named item is not
# constant over time).  If PREDICATE returns 0 at post time, then the
# item will be ghosted.
proc ghosting_menu_item {menu index predicate {no_cache 0}} {
  if {! $no_cache} then {
    set index [$menu index $index]
  }

  add_post_command $menu [list GHOST_helper $menu $index $predicate]
}
