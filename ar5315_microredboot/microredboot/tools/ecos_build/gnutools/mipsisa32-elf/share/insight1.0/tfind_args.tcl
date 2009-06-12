# TfindArgs
# Copyright 1998, 1999 Cygnus Solutions
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (GPL) as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.


# ----------------------------------------------------------------------
# Implements tfind arguments dialogs
#
#   PUBLIC ATTRIBUTES:
#     
#     Type .........Type of dialog (tfind pc, tfind line, tfind tracepoint)
#
#     config ....... used to change public attributes
#
#     PRIVATE METHODS
#
#   X11 OPTION DATABASE ATTRIBUTES
#
#
# ----------------------------------------------------------------------
 
itcl_class TfindArgs {
  # ------------------------------------------------------------------
  #  CONSTRUCTOR - create new tfind arguments dialog
  # ------------------------------------------------------------------
  constructor {config} {
    #
    #  Create a window with the same name as this object
    #
    set class [$this info class]
    set hull [namespace tail $this]
    set old_name $this
    ::rename $this $this-tmp-
    ::frame $hull -class $class
    ::rename $hull $old_name-win-
    ::rename $this $old_name
    build_win
  }
 
  # ------------------------------------------------------------------
  #  METHOD:  build_win - build the dialog
  # ------------------------------------------------------------------
  method build_win {} {

    frame $hull.f
    frame $hull.f.a
    frame $hull.f.b
    set f $hull.f.a

    switch $Type {
      LN   { 
	set text "Enter argument: " 
      }
      PC   { 
	set text "Enter PC value: " 
      }
      TP   { 
	set text "Enter Tracepoint No.: " 
      }
      FR  {
	set text "Enter Frame No.:"
    }
    
    if {[string compare $Type $last_type]} != 0} {
      global argument
      set argument ""
    }

    set last_type $Type

    label $f.1 -text $text
    entry $f.2 -textvariable argument -width 10
    $f.2 selection range 0 end
    grid $f.1 $f.2 -padx 4 -pady 4 -sticky nwe
    
    button $hull.f.b.ok -text OK -command "$this ok" -width 7 -default active
    button $hull.f.b.quit -text Cancel -command "delete object $this" -width 7
    grid $hull.f.b.ok $hull.f.b.quit  -padx 4 -pady 4  -sticky ews

    pack $hull.f.a $hull.f.b  
    grid $hull.f

    focus $f.2
    bind $f.2 <Return> "$this.f.b.ok flash; $this.f.b.ok invoke"

  }

  # ------------------------------------------------------------------
  #  DESTRUCTOR - destroy window containing widget
  # ------------------------------------------------------------------
  destructor {
    set top [winfo toplevel $hull]
    manage delete $this 1
    destroy $this
      destroy $top
  }
 


  # ------------------------------------------------------------------
  #  METHOD:  ok - do it and quit
  # ------------------------------------------------------------------
  method ok {} {
    do_it 
    delete
  }


  # ------------------------------------------------------------------
  #  METHOD:  do_it - call the gdb command
  # ------------------------------------------------------------------
  method do_it {} {
    global argument
    
    
    switch $Type {
      LN  { tfind_cmd "tfind line $argument"} 
      PC  { tfind_cmd "tfind pc $argument"}
      TP  { tfind_cmd "tfind tracepoint $argument"} 
      FR  { tfind_cmd "tfind $argument"}
    }
  }


  public Type
  common last_type {}
  private hull


}
