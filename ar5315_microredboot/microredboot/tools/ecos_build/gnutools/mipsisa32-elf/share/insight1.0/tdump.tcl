# Trace dump window for Insight
# Copyright 1998, 1999, 2001, 2002 Red Hat, Inc.
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
# Implements Tdump window for gdb
#
#   PUBLIC ATTRIBUTES:
#
#
#   METHODS:
#
#     reconfig ....... called when preferences change
#
#
#   X11 OPTION DATABASE ATTRIBUTES
#
#
# ----------------------------------------------------------------------

class TdumpWin {
  inherit ManagedWin GDBWin

  # ------------------------------------------------------------------
  #  CONSTRUCTOR - create new tdump window
  # ------------------------------------------------------------------
  constructor {args} {
    window_name "Trace Dump"
    build_win
    eval itk_initialize $args
  }


  # ------------------------------------------------------------------
  #  METHOD:  build_win - build the main tdump window
  # ------------------------------------------------------------------
  method build_win {} {
    global tcl_platform

    if {[string compare $tcl_platform(platform) "windows"] == 0} {
      set mode static
      ide_sizebox $itk_interior.sbox
      place $itk_interior.sbox -relx 1.0 -rely 1.0 -anchor se
    } else {
      set mode dynamic
    }

    itk_component add stext {
      iwidgets::scrolledtext $itk_interior.stext -hscrollmode $mode \
	-vscrollmode $mode -textfont global/fixed \
	-background $::Colors(bg)
    } {}
    [$itk_component(stext) component text] configure \
      -background $::Colors(bg)
    pack $itk_component(stext) -side left -expand yes -fill both
    update dummy
  }


  # ------------------------------------------------------------------
  #  METHOD:  update - update widget when PC changes
  # ------------------------------------------------------------------
  method update {event} {
    #debug "tdump: update"
    gdbtk_busy
    set tframe_num [gdb_get_trace_frame_num]

    if { $tframe_num!=-1 } {
      debug "doing tdump"
      $itk_component(stext) delete 1.0 end

      if {[catch {gdb_cmd "tdump $tframe_num" 0} tdump_output]} {
	tk_messageBox -title "Error" -message $tdump_output -icon error \
	  -type ok
      } else {
	#debug "tdum output is $tdump_output"
	
	$itk_component(stext) insert end $tdump_output
	$itk_component(stext) see insert
      }
    }
    gdbtk_idle
  }

  # ------------------------------------------------------------------
  #  METHOD:  reconfig - used when preferences change
  # ------------------------------------------------------------------
  method reconfig {} {
    if {[winfo exists $itk_interior.sbox]} { destroy $itk_interior.sbox }
    if {[winfo exists $itk_interior.stext]} { destroy $itk_interior.stext }
    build_win
  }
}

