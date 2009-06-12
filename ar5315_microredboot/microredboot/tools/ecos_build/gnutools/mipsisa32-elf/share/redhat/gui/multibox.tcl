# multibox.tcl - Multi-column listbox.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

# FIXME:
# * Should support sashes so user can repartition widget sizes.
# * Should support itemcget, itemconfigure.

itcl_class Multibox {
  # The selection mode.
  public selectmode browse {
    _apply_all configure [list -selectmode $selectmode]
  }

  # The height.
  public height 10 {
    _apply_all configure [list -height $height]
  }

  # This is a list of all the listbox widgets we've created.  Private
  # variable.
  protected _listboxen {}

  # Tricky: take the class bindings for the Listbox widget and turn
  # them into Multibox bindings that directly run our bindings.  That
  # way any binding on any of our children will automatically work the
  # right way.
  # FIXME: this loses if any Listbox bindings are added later.
  # To really fix we need Uhler's change to support megawidgets.
  foreach seq [bind Listbox] {
    regsub -all -- %W [bind Listbox $seq] {[winfo parent %W]} sub
    bind Multibox $seq $sub
  }

  constructor {config} {
    # The standard widget-making trick.
    set class [$this info class]
    set hull [namespace tail $this]
    set old_name $this
    ::rename $this $this-tmp-
    ::frame $hull -class $class -relief flat -borderwidth 0
    ::rename $hull $old_name-win-
    ::rename $this $old_name

    scrollbar [namespace tail $this].vs -orient vertical
    bind [namespace tail $this].vs <Destroy> [list $this delete]

    grid rowconfigure  [namespace tail $this] 0 -weight 0
    grid rowconfigure  [namespace tail $this] 1 -weight 1
  }

  destructor {
    destroy $this
  }

  #
  # Our interface.
  #

  # Add a new column.
  method add {args} {
    # The first array set sets up the default values, and the second
    # overwrites with what the user wants.
    array set opts {-width 20 -fix 0 -title Zardoz}
    array set opts $args

    set num [llength $_listboxen]
    listbox [namespace tail $this].box$num -exportselection 0 -height $height \
      -selectmode $selectmode -width $opts(-width)
    if {$num == 0} then {
      [namespace tail $this].box$num configure -yscrollcommand [list [namespace tail $this].vs set]
      [namespace tail $this].vs configure -command [list $this yview]
    }
    label [namespace tail $this].label$num -text $opts(-title) -anchor w

    # No more class bindings.
    set tag_list [bindtags [namespace tail $this].box$num]
    set index [lsearch -exact $tag_list Listbox]
    bindtags [namespace tail $this].box$num [lreplace $tag_list $index $index Multibox]

    grid [namespace tail $this].label$num -row 0 -column $num -sticky new
    grid [namespace tail $this].box$num -row 1 -column $num -sticky news
    if {$opts(-fix)} then {
      grid columnconfigure  [namespace tail $this] $num -weight 0 \
	-minsize [winfo reqwidth [namespace tail $this].box$num]
    } else {
      grid columnconfigure  [namespace tail $this] $num -weight 1
    }

    lappend _listboxen [namespace tail $this].box$num

    # Move the scrollbar over.
    incr num
    grid [namespace tail $this].vs -row 1 -column $num -sticky nsw
    grid columnconfigure  [namespace tail $this] $num -weight 0
  }

  method configure {config} {}

  # FIXME: should handle automatically.
  method cget {option} {
    switch -- $option {
      -selectmode {
	return $selectmode
      }
      -height {
	return $height
      }

      default {
	error "option $option not supported"
      }
    }
  }

  # FIXME: this isn't ideal.  But we want to support adding bindings
  # at least.  A "bind" method might be better.
  method get_boxes {} {
    return $_listboxen
  }


  #
  # Methods that duplicate Listbox interface.
  #

  method activate index {
    _apply_all activate [list $index]
  }

  method bbox index {
    error "bbox method not supported"
  }

  method curselection {} {
    return [_apply_first curselection {}]
  }

  # FIXME: In itcl 1.5, can't have a method name "delete".  Sigh.
  method delete_hack {args} {
    _apply_all delete $args
  }

  # Return some contents.  We return each item as a list of the
  # columns.
  method get {first {last {}}} {
    if {$last == ""} then {
      set r {}
      foreach l $_listboxen {
	lappend r [$l get $first]
      }
      return $r
    } else {
      # We do things this way so that we don't have to specially
      # handle the index "end".
      foreach box $_listboxen {
	set seen(var-$box) [$box get $first $last]
      }

      # Tricky: we use the array indices as variable names and the
      # array values as values.  This lets us "easily" construct the
      # result lists.
      set r {}
      eval foreach [array get seen] {{
	set elt {}
	foreach box $_listboxen {
	  lappend elt [set var-$box]
	}
	lappend r $elt
      }}
      return $r
    }
  }

  method index index {
    return [_apply_first index [list $index]]
  }

  # Insert some items.  Each new item is a list of items for all
  # columns.
  method insert {index args} {
    if {[llength $args]} then {
      set seen(_) {}
      unset seen(_)

      foreach value $args {
	foreach columnvalue $value lname $_listboxen {
	  lappend seen($lname) $columnvalue
	}
      }

      foreach box $_listboxen {
	eval $box insert $index $seen($box)
      }
    }
  }

  method nearest y {
    return [_apply_first nearest [list $y]]
  }

  method scan {option args} {
    _apply_all scan $option $args
  }

  method see index {
    _apply_all see [list $index]
  }

  method selection {option args} {
    if {$option == "includes"} then {
      return [_apply_first selection [concat $option $args]]
    } else {
      return [_apply_all selection [concat $option $args]]
    }
  }

  method size {} {
    return [_apply_first size {}]
  }

  method xview args {
    error "xview method not supported"
  }

  method yview args {
    if {! [llength $args]} then {
      return [_apply_first yview {}]
    } else {
      return [_apply_all yview $args]
    }
  }


  #
  # Private methods.
  #

  # This applies METHOD to every listbox.
  method _apply_all {method argList} {
    foreach l $_listboxen {
      eval $l $method $argList
    }
  }

  # This applies METHOD to the first listbox, and returns the result.
  method _apply_first {method argList} {
    set l [lindex $_listboxen 0]
    return [eval $l $method $argList]
  }
}
