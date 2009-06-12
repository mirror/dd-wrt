# wframe.tcl - Frame with a widget on its border.
# Copyright (C) 1997 Cygnus Solutions.
# Written by Tom Tromey <tromey@cygnus.com>.

itcl_class Widgetframe {
  # Where to put the widget.  For now, we don't support many anchors.
  # Augment as you like.
  public anchor nw {
    if {$anchor != "nw" && $anchor != "n"} then {
      error "anchors nw and n are the only ones supported"
    }
    _layout
  }

  # The name of the widget to put on the frame.  This is set by some
  # subclass calling the _add method.  Private variable.
  protected _widget {}

  constructor {config} {
    # The standard widget-making trick.
    set class [$this info class]
    set hull [namespace tail $this]
    set old_name $this
    ::rename $this $this-tmp-
    ::frame $hull -class $class -relief flat -borderwidth 0
    ::rename $hull $old_name-win-
    ::rename $this $old_name

    frame [namespace tail $this].iframe -relief groove -borderwidth 2
    grid [namespace tail $this].iframe -row 1 -sticky news
    grid rowconfigure  [namespace tail $this] 1 -weight 1
    grid columnconfigure  [namespace tail $this] 0 -weight 1

    # Make an internal frame so that user stuff isn't obscured.  Note
    # that we can't use the placer, because it doesn't set the
    # geometry of the parent.
    frame [namespace tail $this].iframe.frame -borderwidth 4 -relief flat
    grid [namespace tail $this].iframe.frame -row 1 -sticky news
    grid rowconfigure [namespace tail $this].iframe 1 -weight 1
    grid columnconfigure [namespace tail $this].iframe 0 -weight 1

    bind [namespace tail $this].iframe <Destroy> [list $this delete]
  }

  destructor {
    catch {destroy $this}
  }

  # Return name of internal frame.
  method get_frame {} {
    return [namespace tail $this].iframe.frame
  }

  # Name a certain widget to be put on the frame.  This should be
  # called by some subclass after making the widget.  Protected
  # method.
  method _add {widget} {
    set _widget $widget
    set height [expr {int ([winfo reqheight $_widget] / 2)}]
    grid rowconfigure  [namespace tail $this] 0 -minsize $height -weight 0
    grid rowconfigure [namespace tail $this].iframe 0 -minsize $height -weight 0
    _layout
  }

  # Re-layout according to the anchor.  Private method.
  method _layout {} {
    if {$_widget == "" || ! [winfo exists $_widget]} then {
      return
    }

    switch -- $anchor {
      n {
	# Put the label over the border, in the center.
	place $_widget -in [namespace tail $this].iframe -relx 0.5 -rely 0 -y -2 \
	  -anchor center
      }
      nw {
	# Put the label over the border, at the top left.
	place $_widget -in [namespace tail $this].iframe -relx 0 -x 6 -rely 0 -y -2 \
	  -anchor w
      }
      default {
	error "unsupported anchor \"$anchor\""
      }
    }
  }
}
