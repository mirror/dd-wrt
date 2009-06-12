#
# Panedwindow  
# ----------------------------------------------------------------------
# Implements a very general panedwindow which allows for mixing resizable
# and non-resizable panes.  It also allows limits to be set on individual
# pane sizes, both minimum and maximum.
#
# The look of this widget is much like Window, instead of the Motif-like
# iwidget panedwindow.
# ----------------------------------------------------------------------

# Portions of this code are originally from the iwidget panedwindow which
# is Copyright (c) 1995 DSC Technologies Corporation 

itk::usual PanedWindow {
  keep -background -cursor
}

# ------------------------------------------------------------------
#                            PANEDWINDOW
# ------------------------------------------------------------------
class cyg::PanedWindow {
  inherit itk::Widget

  constructor {args} {}

  itk_option define -orient orient Orient horizontal
  itk_option define -sashwidth sashWidth SashWidth 10
  itk_option define -sashcolor sashColor SashColor gray

  public {
    method index {index}
    method childsite {args}
    method add {tag args}
    method insert {index tag args}
    method delete {index}
    method hide {index}
    method replace {pane1 pane2}
    method show {index}
    method paneconfigure {index args}
    method reset {}
  }

  private {
    method _eventHandler {width height}
    method _startDrag {num}
    method _endDrag {where num}
    method _configDrag {where num}
    method _handleDrag {where num}
    method _moveSash {where num {dir ""}}

    method _resizeArray {}
    method _setActivePanes {}
    method _calcPos {where num {dir ""}}
    method _makeSashes {}
    method _placeSash {i}
    method _placePanes {{start 0} {end end} {forget 0}}

    variable _initialized 0	;# flag set when widget is first configured
    variable _sashes {}		;# List of sashes.

    # Pane information
    variable _panes {}		;# List of panes.
    variable _activePanes {}	;# List of active panes.
    variable _where		;# Array of relative positions
    variable _ploc		;# Array of pixel positions
    variable _frac		;# Array of relative pane sizes
    variable _pixels		;# Array of sizes in pixels for non-resizable panes
    variable _max		;# Array of pane maximum locations
    variable _min		;# Array of pane minimum locations
    variable _pmin		;# Array of pane minimum size
    variable _pmax		;# Array of pane maximum size

    variable _dimension 0	;# width or height of window
    variable _dir "height"	;# resizable direction, "height" or "width"
    variable _rPixels

    variable _sashloc          ;# Array of dist of sash from above/left.

    variable _minsashmoved     ;# Lowest sash moved during dragging.
    variable _maxsashmoved     ;# Highest sash moved during dragging.

    variable _width 0		;# hull's width.
    variable _height 0		;# hull's height.
    variable _unique -1		;# Unique number for pane names.
  }
}

#
# Provide a lowercased access method for the PanedWindow class.
# 
proc ::cyg::panedwindow {pathName args} {
  uplevel ::cyg::PanedWindow $pathName $args
}

#
# Use option database to override default resources of base classes.
#
option add *PanedWindow.width 10 widgetDefault
option add *PanedWindow.height 10 widgetDefault

# ------------------------------------------------------------------
#                        CONSTRUCTOR
# ------------------------------------------------------------------
body cyg::PanedWindow::constructor {args} {
  itk_option add hull.width hull.height

  pack propagate $itk_component(hull) no
  
  bind pw-config-$this <Configure> [code $this _eventHandler %w %h]
  bindtags $itk_component(hull) \
    [linsert [bindtags $itk_component(hull)] 0 pw-config-$this]
  
  eval itk_initialize $args
}

# ------------------------------------------------------------------
#                             OPTIONS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# OPTION: -orient
#
# Specifies the orientation of the sashes.  Once the paned window
# has been mapped, set the sash bindings and place the panes.
# ------------------------------------------------------------------
configbody cyg::PanedWindow::orient {
  #puts "orient $_initialized"
  if {$_initialized} {
    set orient $itk_option(-orient)
    if {$orient != "vertical" && $orient != "horizontal"} {
      error "bad orientation option \"$itk_option(-orient)\":\
	        should be horizontal or vertical"
    }
    if {[string compare $orient "vertical"]} {
      set _dimension $_height
      set _dir "height"
    } else {
      set _dimension $_width
      set _dir "width"
    }
    _resizeArray
    _makeSashes
    _placePanes 0 end 1
  }
}

# ------------------------------------------------------------------
# OPTION: -sashwidth
#
# Specifies the width of the sash.
# ------------------------------------------------------------------
configbody cyg::PanedWindow::sashwidth {
  set pixels [winfo pixels $itk_component(hull) $itk_option(-sashwidth)]
  set itk_option(-sashwidth) $pixels
  
  if {$_initialized} {
    # FIXME
    for {set i 1} {$i < [llength $_panes]} {incr i} {
      $itk_component(sash$i) configure \
	-width $itk_option(-sashwidth) -height $itk_option(-sashwidth) \
	-borderwidth 2
    }
    for {set i 1} {$i < [llength $_panes]} {incr i} {
      _placeSash $i
    }
  }
}

# ------------------------------------------------------------------
# OPTION: -sashcolor
#
# Specifies the color of the sash.
# ------------------------------------------------------------------
configbody cyg::PanedWindow::sashcolor {
  if {$_initialized} {
    for {set i 1} {$i < [llength $_panes]} {incr i} {
      $itk_component(sash$i) configure -background $itk_option(-sashcolor)
    }
  }
}

# ------------------------------------------------------------------
#                            METHODS
# ------------------------------------------------------------------

# ------------------------------------------------------------------
# METHOD: index index
#
# Searches the panes in the paned window for the one with the 
# requested tag, numerical index, or keyword "end".  Returns the pane's 
# numerical index if found, otherwise error.
# ------------------------------------------------------------------    
body cyg::PanedWindow::index {index} {
  if {[llength $_panes] > 0} {
    if {[regexp {(^[0-9]+$)} $index]} {
      if {$index < [llength $_panes]} {
	return $index
      } else {
	error "PanedWindow index \"$index\" is out of range"
      }
    } elseif {$index == "end"} {
      return [expr [llength $_panes] - 1]
    } else {
      if {[set idx [lsearch $_panes $index]] != -1} {
	return $idx
      }
      error "bad PanedWindow index \"$index\": must be number, end,\
		    or pattern"
    }
  } else {
    error "PanedWindow \"$itk_component(hull)\" has no panes"
  }
}

# ------------------------------------------------------------------
# METHOD: childsite ?index?
#
# Given an index return the specifc childsite path name.  Invoked 
# without an index return a list of all the child site panes.  The 
# list is ordered from the near side (left/top).
# ------------------------------------------------------------------
body cyg::PanedWindow::childsite {args} {
  #puts "childsite $args ($_initialized)"
  
  if {[llength $args] == 0} {
    set children {}
    foreach pane $_panes {
      lappend children [$itk_component($pane) childSite]
    }
    return $children
  } else {
    set index [index [lindex $args 0]]
    return [$itk_component([lindex $_panes $index]) childSite]
  }
}


# ------------------------------------------------------------------
# METHOD: add tag ?option value option value ...?
#
# Add a new pane to the paned window to the far (right/bottom) side.
# The method takes additional options which are passed on to the 
# pane constructor.  These include -margin, and -minimum.  The path 
# of the pane is returned.
# ------------------------------------------------------------------
body cyg::PanedWindow::add {tag args} {
  itk_component add $tag {
    eval cyg::Pane $itk_interior.pane[incr _unique] $args
  } {
    keep -background -cursor
  }
  
  lappend _panes $tag
  lappend _activePanes $tag
  reset  
  return $itk_component($tag)
}

# ------------------------------------------------------------------
# METHOD: insert index tag ?option value option value ...?
#
# Insert the specified pane in the paned window just before the one 
# given by index.  Any additional options which are passed on to the 
# pane constructor.  These include -margin, -minimum.  The path of 
# the pane is returned.
# ------------------------------------------------------------------
body cyg::PanedWindow::insert {index tag args} {
  itk_component add $tag {
    eval cyg::Pane $itk_interior.pane[incr _unique] $args
  } {
    keep -background -cursor
  }
  
  set index [index $index]
  set _panes [linsert $_panes $index $tag]
  lappend _activePanes $tag  
  reset
  return $itk_component($tag)
}

# ------------------------------------------------------------------
# METHOD: delete index
#
# Delete the specified pane.
# ------------------------------------------------------------------
body cyg::PanedWindow::delete {index} {
  set index [index $index]
  set tag [lindex $_panes $index]

  # remove the itk component
  destroy $itk_component($tag)
  # remove it from panes list
  set _panes [lreplace $_panes $index $index]  
  
  # remove its _frac value
  set ind [lsearch -exact $_activePanes $tag]
  if {$ind != -1 && [info exists _frac($ind)]} {
    unset _frac($ind)
  }
  
  # this will reset _activePane and resize things
  reset
}

# ------------------------------------------------------------------
# METHOD: hide index
#
# Remove the specified pane from the paned window. 
# ------------------------------------------------------------------
body cyg::PanedWindow::hide {index} {
  set index [index $index]
  set tag [lindex $_panes $index]
  
  if {[set idx [lsearch -exact $_activePanes $tag]] != -1} {
    set _activePanes [lreplace $_activePanes $idx $idx]
    if {[info exists _frac($idx)]} {unset _frac($idx)}
  }

  reset
}

body cyg::PanedWindow::replace {pane1 pane2} {
  set ind1 [lsearch -exact $_activePanes $pane1]
  if {$ind1 == -1} {
    error "$pane1 is not an active pane name."
  }
  set ind2 [lsearch -exact $_panes $pane2]
  if {$ind2 == -1} {
    error "Pane $pane2 does not exist."
  }
  set _activePanes [lreplace $_activePanes $ind1 $ind1 $pane2]
  _placePanes 0 $ind1 1
}

# ------------------------------------------------------------------
# METHOD: show index
#
# Display the specified pane in the paned window.
# ------------------------------------------------------------------
body cyg::PanedWindow::show {index} {
  set index [index $index]
  set tag [lindex $_panes $index]
  
  if {[lsearch -exact $_activePanes $tag] == -1} {
    lappend _activePanes $tag
  }

  reset
}

# ------------------------------------------------------------------
# METHOD: paneconfigure index ?option? ?value option value ...?
#
# Configure a specified pane.  This method allows configuration of
# panes from the PanedWindow level.  The options may have any of the 
# values accepted by the add method.
# ------------------------------------------------------------------
body cyg::PanedWindow::paneconfigure {index args} {
  set index [index $index]
  set tag [lindex $_panes $index]
  return [uplevel $itk_component($tag) configure $args]
}

# ------------------------------------------------------------------
# METHOD: reset
#
# Redisplay the panes based on the default percentages of the panes.
# ------------------------------------------------------------------
body cyg::PanedWindow::reset {} {
  if {$_initialized && [llength $_panes]} {
    #puts RESET
    _setActivePanes
    _resizeArray
    _makeSashes
    _placePanes 0 end 1
  }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _setActivePanes
#
# Resets the active pane list.
# ------------------------------------------------------------------
body cyg::PanedWindow::_setActivePanes {} {
  set _prevActivePanes $_activePanes
  set _activePanes {}
  
  foreach pane $_panes {
    if {[lsearch -exact $_prevActivePanes $pane] != -1} {
      lappend _activePanes $pane
    }
  }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _eventHandler
#
# Performs operations necessary following a configure event.  This
# includes placing the panes.
# ------------------------------------------------------------------
body cyg::PanedWindow::_eventHandler {width height} {
  #puts "Event $width $height"
  set _width $width
  set _height $height
  if {[string compare $itk_option(-orient) "vertical"]} {
    set _dimension $_height
    set _dir "height"
  } else {
    set _dimension $_width
    set _dir "width"
  }
  
  if {$_initialized} {
    _resizeArray
    _placePanes
  } else {
    set _initialized 1
    reset
  }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _resizeArray
#
# Recalculates the sizes and positions of all the panes.
# This is only done at startup, when the window size changes, when
# a new pane is added, or the orientation is changed.
#
# _frac($i) contains:
#		% of resizable space when pane$i is resizable
# _pixels($i) contains
#		pixels when pane$i is not resizable
#
# _where($i) contains the relative position of the top of pane$i
# ------------------------------------------------------------------
body cyg::PanedWindow::_resizeArray {} {
  set numpanes 0
  set _rPixels 0
  set totalFrac 0.0
  set numfreepanes 0

  #puts "sresizeArray dim=$_dimension dir=$_dir"

  # first pass. Count the number of resizable panes and
  # the pixels reserved for non-resizable panes.
  set i 0
  foreach p $_activePanes {
    set _resizable($i) [$itk_component($p) cget -resizable]
    if {$_resizable($i)} {
      # remember pane min and max
      set _pmin($i) [$itk_component($p) cget -minimum]
      set _pmax($i) [$itk_component($p) cget -maximum]

      incr numpanes
      if {[info exists _frac($i)]} {
	# sum up all the percents
	set totalFrac [expr $totalFrac + $_frac($i)]
      } else {
	# number of new windows not yet sized
	incr numfreepanes
      }
    } else {
      set _pixels($i) [winfo req$_dir $itk_component($p)]
      set _pmin($i) $_pixels($i)
      set _pmax($i) $_pixels($i)
      incr _rPixels $_pixels($i)
    }
    incr i
  }
  set totalpanes $i

  #puts "numpanes=$numpanes nfp=$numfreepanes _rPixels=$_rPixels  totalFrac=$totalFrac"

  if {$numfreepanes} {
    # set size for the new window(s) to average size
    if {$totalFrac > 0.0} {
      set freepanesize [expr $totalFrac / ($numpanes - $numfreepanes)]
    } else {
      set freepanesize [expr 1.0 / $numpanes.0]
    }
    for {set i 0} {$i < $totalpanes} {incr i} {
      if {$_resizable($i) && ![info exists _frac($i)]} {
	set _frac($i) $freepanesize
	set totalFrac [expr $totalFrac + $_frac($i)]
      }
    }
  }

  set done 0

  while {!$done} {
    # force to a reasonable value
    if {$totalFrac <= 0.0} { set totalFrac 1.0 }

    # scale the _frac array
    if {$totalFrac > 1.01 || $totalFrac < 0.99} {
      set cor [expr 1.0 / $totalFrac]
      set totalFrac 0.0
      for {set i 0} {$i < $totalpanes} {incr i} {
	if {$_resizable($i)} {
	  set _frac($i) [expr $_frac($i) * $cor]
	  set totalFrac [expr $totalFrac + $_frac($i)]
	}
      }
    }
    
    # bounds checking; look for panes that are too small or too large
    # if one is found, fix its size at the min or max and mark the
    # window non-resizable. Adjust percents and try again.
    set done 1
    for {set i 0} {$i < $totalpanes} {incr i} {
      if {$_resizable($i)} {
	set _pixels($i) [expr int($_frac($i) * ($_dimension - $_rPixels.0))]
	if {$_pixels($i) < $_pmin($i)} {
	  set _resizable($i) 0
	  set totalFrac [expr $totalFrac - $_frac($i)]
	  set _pixels($i) $_pmin($i)
	  incr  _rPixels $_pixels($i)
	  set done 0
	  break
	} elseif {$_pmax($i) && $_pixels($i) > $_pmax($i)} {
	  set _resizable($i) 0
	  set totalFrac [expr $totalFrac - $_frac($i)]
	  set _pixels($i) $_pmax($i)
	  incr  _rPixels $_pixels($i)
	  set done 0
	  break
	}
      }
    }
  }

  # Done adjusting. Now build pane position arrays.  These are designed
  # to minimize calculations while resizing.
  # Note: position of sash $i = position of top of pane $i
  # _where($i): relative (0.0 - 1.0) position of sash $i
  # _ploc($i): position in pixels of sash $i
  # _max($i): maximum position in pixels of sash $i (0 = no max)
  set _where(0) 0.0
  set _ploc(0) 0
  set _max(0) 0
  set _min(0) 0

  # calculate the percentage of resizable space
  set resizePerc [expr 1.0 - ($_rPixels.0 / $_dimension)]

  # now set the pane positions
  for {set i 1; set n 0} {$i < $totalpanes} {incr i; incr n} {
    if {$_resizable($n)} {
      set _where($i) [expr $_where($n) + ($_frac($n) * $resizePerc)]
    } else {
      set _where($i) [expr $_where($n) + [expr $_pixels($n).0 / $_dimension]]
    }
    set _ploc($i) [expr $_ploc($n) + $_pixels($n)]
    set _max($i) [expr $_max($n) + $_pmax($n)]
    if {($_max($n) == 0 || $_pmax($n) == 0) && $n != 0} {
      set _max($i) 0
    }
    set _min($i) [expr $_min($n) + $_pmin($n)]
    #puts "where($i)=$_where($i)"
    #puts "ploc($i)=$_ploc($i)"
    #puts "min($i)=$_min($i)"
    #puts "pmin($i)=$_pmin($i)"
    #puts "pmax($i)=$_pmax($i)"
    #puts "pixels($i)=$_pixels($i)"
  }
  set _ploc($i) $_dimension
  set _where($i) 1.0

  # finally, starting at the bottom,
  # check the _max and _min arrays
  set _max($totalpanes) $_dimension
  set _min($totalpanes) $_dimension
  #puts "_max($totalpanes) = $_max($totalpanes)"
  for {set i [expr $totalpanes - 1]} {$i > 0} {incr i -1} {
    set n [expr $i + 1]
    set m [expr $_max($n) - $_pmin($i)]
    if {$_max($i) > $m || !$_max($i)} { set _max($i) $m }
    if {$_pmax($i)} {
      set m [expr $_min($n) - $_pmax($i)]
      if {$_min($i) < $m} {set _min($i) $m }
    }
    #puts "$i $_max($i) $_min($i)"
  }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _startDrag num
#
# Starts the sash drag and drop operation.  At the start of the drag
# operation all the information is known as for the upper and lower
# limits for sash movement.  The calculation is made at this time and
# stored in protected variables for later access during the drag
# handling routines.
# ------------------------------------------------------------------
body cyg::PanedWindow::_startDrag {num} {
  #puts "startDrag $num"
  
  set _minsashmoved $num
  set _maxsashmoved $num

  grab  $itk_component(sash$num)
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _endDrag where num
#
# Ends the sash drag and drop operation.
# ------------------------------------------------------------------
body cyg::PanedWindow::_endDrag {where num} {
  #puts "endDrag $where $num"

  grab release $itk_component(sash$num)
  
  # set new _frac values
  for {set i [expr $_minsashmoved-1]} {$i <= $_maxsashmoved} {incr i} {
    set _frac($i) \
      [expr ($_ploc([expr $i+1]).0 - $_ploc($i)) / ($_dimension - $_rPixels)]
  }
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _configDrag where num
#
# Configure  action for sash.
# ------------------------------------------------------------------
body cyg::PanedWindow::_configDrag {where num} {
  set _sashloc($num) $where
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _handleDrag where num
#
# Motion action for sash.
# ------------------------------------------------------------------
body cyg::PanedWindow::_handleDrag {where num} {
  #puts "handleDrag $where $num"
  _moveSash [expr $where + $_sashloc($num)] $num
  _placePanes [expr $_minsashmoved - 1] $_maxsashmoved
}

# ------------------------------------------------------------------
# PROTECTED METHOD: _moveSash where num
#
# Move the sash to the absolute pixel location
# ------------------------------------------------------------------
body cyg::PanedWindow::_moveSash {where num {dir ""}} {
  #puts "moveSash $where $num"
  set _minsashmoved [expr ($_minsashmoved<$num)?$_minsashmoved:$num]
  set _maxsashmoved [expr ($_maxsashmoved>$num)?$_maxsashmoved:$num]
  _calcPos $where $num $dir
}


# ------------------------------------------------------------------
# PRIVATE METHOD: _calcPos where num
#
# Determines the new position for the sash.  Make sure the position does
# not go past the minimum for the pane on each side of the sash.
# ------------------------------------------------------------------
body cyg::PanedWindow::_calcPos {where num {direction ""}} {
  set dir [expr $where - $_ploc($num)]
  #puts "calcPos $where $num $dir $direction"
  if {$dir == 0} { return }
  
  # simplify expressions by computing these now
  set m [expr $num-1]
  set p [expr $num+1]

  # we have squeezed the pane below us to the limit
  set lower1 [expr $_ploc($m) + $_pmin($m)]
  set lower2 0
  if {$_pmax($num)} {
    # we have stretched the pane above us to the limit
    set lower2 [expr $_ploc($p) - $_pmax($num)]
  }

  set upper1 9999 ;# just a large number
  if {$_pmax($m)} {
    # we have stretched the pane above us to the limit
    set upper1 [expr $_ploc($m) + $_pmax($m)]
  }

  # we have squeezed the pane below us to the limit
  set upper2 [expr $_ploc($p) - $_pmin($num)]

  set done 0
  
  #puts "lower1=$lower1 lower2=$lower2 _min($num)=$_min($num)"
  #puts "upper1=$upper1 upper2=$upper2 _max($num)=$_max($num)"
  if {$dir < 0 && $where > $_min($num)} {
    if {$where < $lower2 && $direction != "down"} {
      set done 1
      if {$p == [llength $_activePanes]} {
	set _ploc($num) $upper2
      } else {
	_moveSash [expr $where + $_pmax($num)] $p up
	set _ploc($num) [expr $_ploc($p) - $_pmax($num)]
      }
    }
    if {$where < $lower1 && $direction != "up"} {
      set done 1
      if {$num == 1} {
	set _ploc($num) $lower1
      } else {
	_moveSash [expr $where - $_pmin($m)] $m down
	set _ploc($num) [expr $_ploc($m) + $_pmin($m)]
      }
    }
  } elseif {$dir > 0 && ($_max($num) == 0 || $where < $_max($num))} {
    if {$where > $upper1 && $direction != "up"} {
      set done 1
      if {$num == 1} {
	set _ploc($num) $upper1
      } else {
	_moveSash [expr $where - $_pmax($m)] $m down
	set _ploc($num) [expr $_ploc($m) + $_pmax($m)]
      }
    }
    if {$where > $upper2 && $direction != "down"} {
      set done 1
      if {$p == [llength $_activePanes]} {
	set _ploc($num) $upper2
      } else {
	_moveSash [expr $where + $_pmin($num)] $p up
	set _ploc($num) [expr $_ploc($p) - $_pmin($num)]
      }
    }
  }

  if {!$done} {
    if {!($_max($num) > 0 && $where > $_max($num)) && $where >= $_min($num)} {
      set _ploc($num) $where
    }
  }
  set _where($num) [expr $_ploc($num).0 / $_dimension]
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _makeSashes
#
# Removes any previous sashes and creates new ones.
# ------------------------------------------------------------------
body cyg::PanedWindow::_makeSashes {} {
  #
  # Remove any existing sashes.
  #
  foreach sash $_sashes {
    destroy $itk_component($sash)
  }
  
  set _sashes {}
  set skipped_first 0
  #
  # Create necessary number of sashes
  #
  for {set id 0} {$id < [llength $_activePanes]} {incr id} {
    set p [lindex $_activePanes $id]
    if {[$itk_component($p) cget -resizable]} {
      if {$skipped_first == 0} {
	# create the first sash when we see the 2nd resizable pane
	incr skipped_first
      } else {
	# create sash

	itk_component add sash$id {
	  frame $itk_interior.sash$id -relief raised \
	    -height $itk_option(-sashwidth) \
	    -width $itk_option(-sashwidth) \
	    -borderwidth 2
	} {
	  keep -background
	}
	lappend _sashes sash$id
	
	set com $itk_component(sash$id)
	$com configure -background $itk_option(-sashcolor)
	bind $com <Button-1> [code $this _startDrag $id]
	
	switch $itk_option(-orient) {
	  vertical {
	    bind $com <B1-Motion> \
	      [code $this _handleDrag %x $id]
	    bind $com <B1-ButtonRelease-1> \
	      [code $this _endDrag %x $id]
	    bind $com <Configure> \
	      [code $this _configDrag %x $id]
	    # FIXME Windows should have a different cirsor
	    $com configure -cursor sb_h_double_arrow
	  }
	  
	  horizontal {
	    bind $com <B1-Motion> \
	      [code $this _handleDrag %y $id]
	    bind $com <B1-ButtonRelease-1> \
	      [code $this _endDrag %y $id]
	    bind $com <Configure> \
	      [code $this _configDrag %y $id]
	    # FIXME Windows should have a different cirsor
	    $com configure -cursor sb_v_double_arrow
	  }
	}
      }
    }
  }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _placeSash i
#
# Places the position of the sash
# ------------------------------------------------------------------
body cyg::PanedWindow::_placeSash {i} {
  if {[string compare $itk_option(-orient) "vertical"]} {
    place $itk_component(sash$i) -in $itk_component(hull) \
      -x 0 -relwidth 1 -rely $_where($i) -anchor w \
      -height $itk_option(-sashwidth)
  } else {
    place $itk_component(sash$i) -in $itk_component(hull) \
      -y 0 -relheight 1 -relx $_where($i) -anchor n \
      -width $itk_option(-sashwidth)
  }
}

# ------------------------------------------------------------------
# PRIVATE METHOD: _placePanes
#
# Resets the panes of the window following movement of the sash.
# ------------------------------------------------------------------
body cyg::PanedWindow::_placePanes {{start 0} {end end} {forget 0}} {
  #puts "placeplanes $start $end"

  if {!$_initialized} {
    return 
  }

  if {$end=="end"} { set end [expr [llength $_activePanes] - 1] }
  set _updatePanes [lrange $_activePanes $start $end]

  if {$forget} {
    if {$_updatePanes == $_activePanes} {
      set _forgetPanes $_panes
    } else {
      set _forgetPanes $_updatePanes
    }
    foreach pane $_forgetPanes {
      place forget $itk_component($pane)
    }
  }
  
  if {[string compare $itk_option(-orient) "vertical"]} {
    set i $start
    foreach pane $_updatePanes {
      place $itk_component($pane) -in $itk_component(hull) \
	-x 0 -rely $_where($i) -relwidth 1 \
	-relheight [expr $_where([expr $i + 1]) - $_where($i)]
      incr i
    }
  } else {
    set i $start
    foreach pane $_updatePanes {
      place $itk_component($pane) -in $itk_component(hull) \
	-y 0 -relx $_where($i) -relheight 1 \
	-relwidth [expr $_where([expr $i + 1]) - $_where($i)]
      incr i
    }    
  }

  for {set i [expr $start+1]} {$i <= $end} {incr i} {
    if {[lsearch -exact $_sashes sash$i] != -1} {
      _placeSash $i
    }
  }
}
