# list.tcl - Some handy list procs.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.
# FIXME: some are from TclX; we should probably just use the C
# implementation that is in S-N.

proc lvarpush {listVar element {index 0}} {
  upvar $listVar var
  if {![info exists var]} then {
    lappend var $element
  } else {
    set var [linsert $var $index $element]
  }
}

proc lvarpop {listVar {index 0}} {
  upvar $listVar var
  set result [lindex $var $index]
  # NOTE lreplace can fail if list is empty.
  if {! [catch {lreplace $var $index $index} new]} then {
    set var $new
  }
  return $result
}

proc lassign {list args} {
  set len [expr {[llength $args] - 1}]

  # Special-case last element: if LIST is longer than ARGS, assign a
  # list of leftovers to the last variable.
  if {[llength $list] - 1 > $len} then {
    upvar [lindex $args $len] local
    set local [lrange $list $len end]
    incr len -1
  }

  while {$len >= 0} {
    upvar [lindex $args $len] local
    set local [lindex $list $len]
    incr len -1
  }
}

# Remove duplicates and sort list.  ARGS are arguments to lsort, eg
# --increasing.
proc lrmdups {list args} {
  set slist [eval lsort $args [list $list]]
  set last [lvarpop slist]
  set result [list $last]
  foreach item $slist {
    if {$item != $last} then {
      set last $item
      lappend result $item
    }
  }
  return $result
}

proc lremove {list element} {
  set index [lsearch -exact $list $element]
  if {$index == -1} then {
    return $list
  }
  return [lreplace $list $index $index]
}

# replace element with new element
proc lrep {list element new} {
  set index [lsearch -exact $list $element]
  if {$index == -1} {
    return $list
  }
  return [lreplace $list $index $index $new]
}

# FIXME: this isn't precisely like the C lvarcat.  It is slower.
proc lvarcat {listVar args} {
  upvar $listVar var
  if {[join $args] != ""} then {
    # Yuck!
    eval eval lappend var $args
  }
}
