# Copyright (c) 1998, Bryan Oakley
# All Rights Reservered
#
# Bryan Oakley
# oakley@channelpoint.com
#
# combobox v1.05 August 17, 1998
# a dropdown combobox widget
#
# this code is freely distributable without restriction, but is 
# provided as-is with no waranty expressed or implied. 
#
# Standard Options:
#
# -background -borderwidth -font -foreground -highlightthickness 
# -highlightbackground -relief -state -textvariable 
# -selectbackground -selectborderwidth -selectforeground
# -cursor
#
# Custom Options:
# -command         a command to run whenever the value is changed. 
#                  This command will be called with two values
#                  appended to it -- the name of the widget and the 
#                  new value. It is run at the global scope.
# -editable        if true, user can type into edit box; false, she can't
# -height          specifies height of dropdown list, in lines
# -image           image for the button to pop down the list...
# -maxheight       specifies maximum height of dropdown list, in lines
# -value           duh
# -width           treated just like the -width option to entry widgets
#
#
# widget commands:
#
# (see source... there's a bunch; duplicates of most of the entry
# widget commands, plus commands to manipulate the listbox and a couple
# unique to the combobox as a whole)
# 
# to create a combobox:
#
# namespace import combobox::combobox
# combobox .foo ?options?
#
#
# thanks to the following people who provided beta test support or
# patches to the code:
#
# Martin M. Hunt (hunt@cygnus.com)

package require Tk 8.0
package provide combobox 1.05

namespace eval ::combobox {
    global tcl_platform
    # this is the public interface
    namespace export combobox

    if {$tcl_platform(platform) != "windows"} {
        set sbtest ".          "
        radiobutton $sbtest
        set disabledfg [$sbtest cget -disabledforeground]
        set enabledfg [$sbtest cget -fg]
    } else {
        set disabledfg SystemDisabledText
        set enabledfg SystemWindowText
    }

    # the image used for the button...
    image create bitmap ::combobox::bimage -data  {
        #define down_arrow_width 15
        #define down_arrow_height 15
        static char down_arrow_bits[] = {
          0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0xf8,0x8f,0xf0,0x87,0xe0,
          0x83,0xc0,0x81,0x80,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80
        }; 
    }
}

# this is the command that gets exported, and creates a new 
# combobox widget. It works like other widget commands in that
# it takes as its first argument a widget path, and any remaining
# arguments are option/value pairs for the widget
proc ::combobox::combobox {w args} {

    # build it...
    eval build $w $args

    # set some bindings...
    setBindings $w

    # and we are done!
    return $w
}

# builds the combobox...
proc ::combobox::build {w args } {
    global tcl_platform
    if {[winfo exists $w]} {
	error "window name \"$w\" already exists"
    }

    # create the namespace...
    namespace eval ::combobox::$w {

	variable widgets
	variable options
	variable oldValue
	variable ignoreTrace
	variable this

	array set widgets {}
	array set options {}

	set oldValue {}
	set ignoreTrace 0
    }

    # import the widgets and options arrays into this proc
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options

    # ok, everything we create should exist in the namespace
    # we create for this widget. This is to hide all the internal
    # foo from prying eyes. If they really want to get at the 
    # internals, they know where they can find it...

    # see... I'm pretending to be a Java programmer here...
    set this $w
    namespace eval ::combobox::$w "set this $this"

    # the basic, always-visible parts of the combobox. We do these
    # here, because we want to query some of them for their default
    # values, which we want to juggle to other widgets. I suppose
    # I could use the options database, but I choose not to...
    set widgets(this)   [frame  $this -class Combobox -takefocus 0]
    set widgets(entry)  [entry  $this.entry -takefocus {}]
    set widgets(button) [label  $this.button -takefocus 0] 

    # we will later rename the frame's widget proc to be our
    # own custom widget proc. We need to keep track of this
    # new name, so we'll store it here...
    set widgets(frame) .$this

    pack $widgets(button) -side right -fill y -expand n
    pack $widgets(entry)  -side left -fill both -expand y

    # we need these to be defined, regardless if the user defined
    # them for us or not...
    array set options [list \
	    -height       0 \
	    -maxheight    10 \
	    -command      {} \
	    -image        {} \
	    -textvariable {} \
	    -editable     1 \
	    -state        normal 
    ]
    # now, steal some attributes from the entry widget...
    foreach option [list -background -foreground -relief \
	    -borderwidth -highlightthickness -highlightbackground \
	    -font -width -selectbackground -selectborderwidth \
	    -selectforeground] {
	set options($option) [$widgets(entry) cget $option] 
    }

    # I should probably do this in a catch, but for now it's
    # good enough... What it does, obviously, is put all of
    # the option/values pairs into an array. Make them easier
    # to handle later on...
    array set options $args

    # now, the dropdown list... the same renaming nonsense
    # must go on here as well...
    set widgets(popup)   [toplevel $this.top]
    set widgets(listbox) [listbox $this.top.list]
    set widgets(vsb)     [scrollbar $this.top.vsb]

    pack $widgets(listbox) -side left -fill both -expand y

    # fine tune the widgets based on the options (and a few
    # arbitrary values...)

    # NB: we are going to use the frame to handle the relief
    # of the widget as a whole, so the entry widget will be 
    # flat.
    $widgets(vsb) configure \
	    -command "$widgets(listbox) yview" \
	    -highlightthickness 0

    set width [expr [winfo reqwidth $widgets(vsb)] - 2]
    $widgets(button) configure \
	    -highlightthickness 0 \
	    -borderwidth 1 \
	    -relief raised \
	    -width $width

    $widgets(entry) configure \
	    -borderwidth 0 \
	    -relief flat \
	    -highlightthickness 0 

    $widgets(popup) configure \
	    -borderwidth 1 \
	    -relief sunken
    $widgets(listbox) configure \
	    -selectmode browse \
	    -background [$widgets(entry) cget -bg] \
	    -yscrollcommand "$widgets(vsb) set" \
	    -borderwidth 0

    #Windows look'n'feel: black boarder around listbox
    if {$tcl_platform(platform)=="windows"} {
        $widgets(listbox) configure -highlightbackground black
    }


    # do some window management foo. 
    wm overrideredirect $widgets(popup) 1
    wm transient $widgets(popup) [winfo toplevel $this]
    wm group $widgets(popup) [winfo parent $this]
    wm resizable $widgets(popup) 0 0
    wm withdraw $widgets(popup)
    
    # this moves the original frame widget proc into our
    # namespace and gives it a handy name
    rename ::$this $widgets(frame)

    # now, create our widget proc. Obviously (?) it goes in
    # the global namespace
    
    proc ::$this {command args} \
	    "eval ::combobox::widgetProc $this \$command \$args"
#    namespace export $this
#    uplevel \#0 namespace import ::combobox::${this}::$this

    # ok, the thing exists... let's do a bit more configuration:
    foreach opt [array names options] {
	::combobox::configure $widgets(this) set $opt $options($opt)
    }
}

# here's where we do most of the binding foo. I think there's probably
# a few bindings I ought to add that I just haven't thought about...
proc ::combobox::setBindings {w} {
    namespace eval ::combobox::$w {
	variable widgets
	variable options

	# make sure we clean up after ourselves...
	bind $widgets(this) <Destroy> [list ::combobox::destroyHandler $this]

	# this closes the listbox if we get hidden
	bind $widgets(this) <Unmap> "$widgets(this) close"

	# this helps (but doesn't fully solve) focus issues. 
	bind $widgets(this) <FocusIn> [list focus $widgets(entry)]

	# this makes our "button" (which is actually a label) 
	# do the right thing
	bind $widgets(button) <ButtonPress-1> [list $widgets(this) toggle]

	# this lets the autoscan of the listbox work, even if they
	# move the cursor over the entry widget. 
	bind $widgets(entry) <B1-Enter> "break"
	bind $widgets(entry) <FocusIn> \
		[list ::combobox::entryFocus $widgets(this) "<FocusIn>"]
	bind $widgets(entry) <FocusOut> \
		[list ::combobox::entryFocus $widgets(this) "<FocusOut>"]

	# this will (hopefully) close (and lose the grab on) the
	# listbox if the user clicks anywhere outside of it. Note
	# that on Windows, you can click on some other app and 
	# the listbox will still be there, because tcl won't see
	# that button click
	bind $widgets(this) <Any-ButtonPress> [list $widgets(this) close]
	bind $widgets(this) <Any-ButtonRelease> [list $widgets(this) close]

	bind $widgets(listbox) <ButtonRelease-1> \
	"::combobox::select $widgets(this) \[$widgets(listbox) nearest %y\]; break"

	bind $widgets(listbox) <Any-Motion> {
	    %W selection clear 0 end
	    %W activate @%x,%y
	    %W selection anchor @%x,%y
	    %W selection set @%x,%y @%x,%y
	    # need to do a yview if the cursor goes off the top
	    # or bottom of the window... (or do we?)
	}

	# these events need to be passed from the entry
	# widget to the listbox, or need some sort of special
	# handling....
	foreach event [list <Up> <Down> <Tab> <Return> <Escape> \
		<Next> <Prior> <Double-1> <1> <Any-KeyPress> \
		<FocusIn> <FocusOut>] {
	    bind $widgets(entry) $event \
		    "::combobox::handleEvent $widgets(this) $event"
	}

    }
}

# this proc handles events from the entry widget that we want handled
# specially (typically, to allow navigation of the list even though
# the focus is in the entry widget)
proc ::combobox::handleEvent {w event} {
    upvar ::combobox::${w}::widgets  widgets
    upvar ::combobox::${w}::options  options
    upvar ::combobox::${w}::oldValue oldValue

    # for all of these events, if we have a special action we'll
    # do that and do a "return -code break" to keep additional 
    # bindings from firing. Otherwise we'll let the event fall
    # on through. 
    switch $event {
	"<Any-KeyPress>" {
	    set editable [::combobox::getBoolean $options(-editable)]
	    # if the widget is editable, clear the selection. 
	    # this makes it more obvious what will happen if the 
	    # user presses <Return> (and helps our code know what
	    # to do if the user presses return)
	    if {$editable} {
		$widgets(listbox) see 0
		$widgets(listbox) selection clear 0 end
		$widgets(listbox) selection anchor 0
		$widgets(listbox) activate 0
	    }
	}

	"<FocusIn>" {
	    set oldValue [$widgets(entry) get]
	}

	"<FocusOut>" {
	    $widgets(entry) delete 0 end
	    $widgets(entry) insert 0 $oldValue
	}

	"<1>" {
	    set editable [::combobox::getBoolean $options(-editable)]
	    if {!$editable} {
		if {[winfo ismapped $widgets(popup)]} {
		    $widgets(this) close
		    return -code break;

		} else {
		    if {$options(-state) != "disabled"} {
			$widgets(this) open
			return -code break;
		    }
		}
	    }
	}

	"<Double-1>" {
	    if {$options(-state) != "disabled"} {
		$widgets(this) toggle
		return -code break;
	    }
	}
	"<Tab>" {
	    if {[winfo ismapped $widgets(popup)]} {
		::combobox::find $widgets(this)
		return -code break;
	    }
	}
	"<Escape>" {
	    $widgets(entry) delete 0 end
	    $widgets(entry) insert 0 $oldValue
	    if {[winfo ismapped $widgets(popup)]} {
		$widgets(this) close
		return -code break;
	    }
	}

	"<Return>" {
	    set editable [::combobox::getBoolean $options(-editable)]
	    if {$editable} {
		# if there is something in the list that is selected,
		# we'll pick it. Otherwise, use whats in the 
		# entry widget...
		set index [$widgets(listbox) curselection]
		if {[winfo ismapped $widgets(popup)] && \
			[llength $index] > 0} {

		    ::combobox::select $widgets(this) \
			    [$widgets(listbox) curselection]
		    return -code break;

		} else {
		    ::combobox::setValue $widgets(this) [$widgets(this) get]
		    $widgets(this) close
		    return -code break;
		}
	    }

	    if {[winfo ismapped $widgets(popup)]} {
		::combobox::select $widgets(this) \
			[$widgets(listbox) curselection]
		return -code break;
	    } 

	}

	"<Next>" {
	    $widgets(listbox) yview scroll 1 pages
	    set index [$widgets(listbox) index @0,0]
	    $widgets(listbox) see $index
	    $widgets(listbox) activate $index
	    $widgets(listbox) selection clear 0 end
	    $widgets(listbox) selection anchor $index
	    $widgets(listbox) selection set $index

	}

	"<Prior>" {
	    $widgets(listbox) yview scroll -1 pages
	    set index [$widgets(listbox) index @0,0]
	    $widgets(listbox) activate $index
	    $widgets(listbox) see $index
	    $widgets(listbox) selection clear 0 end
	    $widgets(listbox) selection anchor $index
	    $widgets(listbox) selection set $index
	}

	"<Down>" {
	    if {![winfo ismapped $widgets(popup)]} {
		if {$options(-state) != "disabled"} {
		    $widgets(this) open
		    return -code break;
		}
	    } else {
		tkListboxUpDown $widgets(listbox) 1
		return -code break;
	    }
	}
	"<Up>" {
	    if {![winfo ismapped $widgets(popup)]} {
		if {$options(-state) != "disabled"} {
		    $widgets(this) open
		    return -code break;
		}
	    } else {
		tkListboxUpDown $widgets(listbox) -1
		return -code break;
	    }
	}
    }
}

# this cleans up the mess that is left behind when the widget goes away 
proc ::combobox::destroyHandler {w} {

    # kill any trace or after we may have started...
    namespace eval ::combobox::$w {
	variable options
        variable widgets

	if {[string length $options(-textvariable)]} {
	    trace vdelete $options(-textvariable) w \
		    [list ::combobox::vTrace $widgets(this)]
	}
        
        # CYGNUS LOCAL - kill any after command that may be registered.
        if {[info exists widgets(after)]} {
            after cancel $widgets(after)
	    unset widgets(after)
        }
    }

#    catch {rename ::combobox::${w}::$w {}}
    # kill the namespace
    catch {namespace delete ::combobox::$w}
}

# finds something in the listbox that matches the pattern in the
# entry widget
#
# I'm not convinced this is working the way it ought to. It works,
# but is the behavior what is expected? I've also got a gut feeling
# that there's a better way to do this, but I'm too lazy to figure
# it out...
proc ::combobox::find {w {exact 0}} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options

    ## *sigh* this logic is rather gross and convoluted. Surely
    ## there is a more simple, straight-forward way to implement
    ## all this. As the saying goes, I lack the time to make it
    ## shorter...

    # use what is already in the entry widget as a pattern
    set pattern [$widgets(entry) get]

    if {[string length $pattern] == 0} {
	# clear the current selection
	$widgets(listbox) see 0
	$widgets(listbox) selection clear 0 end
	$widgets(listbox) selection anchor 0
	$widgets(listbox) activate 0
	return
    }

    # we're going to be searching this list...
    set list [$widgets(listbox) get 0 end]

    # if we are doing an exact match, try to find,
    # well, an exact match
    if {$exact} {
	set exactMatch [lsearch -exact $list $pattern]
    }

    # search for it. We'll try to be clever and not only
    # search for a match for what they typed, but a match for
    # something close to what they typed. We'll keep removing one
    # character at a time from the pattern until we find a match
    # of some sort.
    set index -1
    while {$index == -1 && [string length $pattern]} {
	set index [lsearch -glob $list "$pattern*"]
	if {$index == -1} {
	    regsub {.$} $pattern {} pattern
	}
    }

    # this is the item that most closely matches...
    set thisItem [lindex $list $index]

    # did we find a match? If so, do some additional munging...
    if {$index != -1} {

	# we need to find the part of the first item that is 
	# unique wrt the second... I know there's probably a
	# simpler way to do this... 

	set nextIndex [expr $index + 1]
	set nextItem [lindex $list $nextIndex]

	# we don't really need to do much if the next
	# item doesn't match our pattern...
	if {[string match $pattern* $nextItem]} {
	    # ok, the next item matches our pattern, too
	    # now the trick is to find the first character
	    # where they *don't* match...
	    set marker [string length $pattern]
	    while {$marker <= [string length $pattern]} {
		set a [string index $thisItem $marker]
		set b [string index $nextItem $marker]
		if {[string compare $a $b] == 0} {
		    append pattern $a
		    incr marker
		} else {
		    break
		}
	    }
	} else {
	    set marker [string length $pattern]
	}
	
    } else {
	set marker end
	set index 0
    }

    # ok, we know the pattern and what part is unique;
    # update the entry widget and listbox appropriately
    if {$exact && $exactMatch == -1} {
	$widgets(listbox) selection clear 0 end
	$widgets(listbox) see $index
    } else {
	$widgets(entry) delete 0 end
	$widgets(entry) insert end $thisItem
	$widgets(entry) selection clear
	$widgets(entry) selection range $marker end
	$widgets(listbox) activate $index
	$widgets(listbox) selection clear 0 end
	$widgets(listbox) selection anchor $index
	$widgets(listbox) selection set $index
	$widgets(listbox) see $index
    }
}

# selects an item from the list and sets the value of the combobox
# to that value
proc ::combobox::select {w index} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options

    catch {
	set data [$widgets(listbox) get [lindex $index 0]]
	::combobox::setValue $widgets(this) $data
    }

    $widgets(this) close
}

# computes the geometry of the popup list based on the size of the
# combobox. Compute size of popup by requested size of listbox
# plus twice the bordersize of the popup.
proc ::combobox::computeGeometry {w} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options
    
    if {$options(-height) == 0 && $options(-maxheight) != "0"} {
	# if this is the case, count the items and see if
	# it exceeds our maxheight. If so, set the listbox
	# size to maxheight...
	set nitems [$widgets(listbox) size]
	if {$nitems > $options(-maxheight)} {
	    # tweak the height of the listbox
	    $widgets(listbox) configure -height $options(-maxheight)
	} else {
	    # un-tweak the height of the listbox
	    $widgets(listbox) configure -height 0
	}
	update idletasks
    }
    set bd [$widgets(popup) cget -borderwidth]
    set height [expr [winfo reqheight $widgets(listbox)] + $bd + $bd]
    #set height [winfo reqheight $widgets(popup)]

    set width [winfo reqwidth $widgets(this)]

    # Compute size of listbox, allowing larger entries to expand
    # the listbox, clipped by the screen
    set x [winfo rootx $widgets(this)]
    set sw [winfo screenwidth $widgets(this)]
    if {$width > $sw - $x} {
        # The listbox will run off the side of the screen, so clip it
        # (and keep a 10 pixel margin).
	set width [expr {$sw - $x - 10}]
    }
    set size [format "%dx%d" $width $height]
    set y [expr {[winfo rooty $widgets(this)]+[winfo reqheight $widgets(this)] + 1}]
    if {[expr $y + $height] >= [winfo screenheight .]} {
	set y [expr [winfo rooty $widgets(this)] - $height]
    }
    set location "+[winfo rootx $widgets(this)]+$y"
    set geometry "=${size}${location}"
    return $geometry
}

# perform an internal widget command, then mung any error results
# to look like it came from our megawidget. A lot of work just to
# give the illusion that our megawidget is an atomic widget
proc ::combobox::doInternalWidgetCommand {w subwidget command args} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options

    set subcommand $command
    set command [concat $widgets($subwidget) $command $args]

    if {[catch $command result]} {
	# replace the subwidget name with the megawidget name
	regsub $widgets($subwidget) $result $widgets($w) result

	# replace specific instances of the subwidget command
	# with out megawidget command
	switch $subwidget,$subcommand {
	    listbox,index  {regsub "index"  $result "list index"  result}
	    listbox,insert {regsub "insert" $result "list insert" result}
	    listbox,delete {regsub "delete" $result "list delete" result}
	    listbox,get    {regsub "get"    $result "list get"    result}
	    listbox,size   {regsub "size"   $result "list size"   result}
	    listbox,curselection   {regsub "curselection"   $result "list curselection"   result}
	}
	error $result

    } else {
	return $result
    }
}


# this is the widget proc that gets called when you do something like
# ".checkbox configure ..."
proc ::combobox::widgetProc {w command args} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options

    # this is just shorthand notation...
    set doWidgetCommand \
	    [list ::combobox::doInternalWidgetCommand $widgets(this)]

    if {$command == "list"} {
	# ok, the next argument is a list command; we'll 
	# rip it from args and append it to command to
	# create a unique internal command
	#
	# NB: because of the sloppy way we are doing this,
	# we'll also let the user enter our secret command
	# directly (eg: listinsert, listdelete), but we
	# won't document that fact
	set command "list[lindex $args 0]"
	set args [lrange $args 1 end]
    }

    # many of these commands are just synonyms for specific
    # commands in one of the subwidgets. We'll get them out
    # of the way first, then do the custom commands.
    switch $command {
	bbox 	{eval $doWidgetCommand entry bbox $args}
	delete	{eval $doWidgetCommand entry delete $args}
	get 	{eval $doWidgetCommand entry get $args}
	icursor 	{eval $doWidgetCommand entry icursor $args}
	index       {eval $doWidgetCommand entry index $args}
	insert 	{eval $doWidgetCommand entry insert $args}
	listinsert 	{
            eval $doWidgetCommand listbox insert $args
            # pack the scrollbar if the number of items exceeds
            # the maximum
            if {$options(-height) == 0 && $options(-maxheight) != 0
              && ([$widgets(listbox) size] > $options(-maxheight))} {
                pack $widgets(vsb) -before $widgets(listbox) -side right \
                  -fill y -expand n
            }
        }
	listdelete 	{
            eval $doWidgetCommand listbox delete $args
            # unpack the scrollbar if the number of items
            # decreases under the maximum
            if {$options(-height) == 0 && $options(-maxheight) != 0
              && ([$widgets(listbox) size] <= $options(-maxheight))} {
                pack forget $widgets(vsb)
            }
        }
	listget 	{eval $doWidgetCommand listbox get $args}
	listindex 	{eval $doWidgetCommand listbox index $args}
	listsize 	{eval $doWidgetCommand listbox size $args}
	listcurselection 	{eval $doWidgetCommand listbox curselection $args}

	scan 	{eval $doWidgetCommand entry scan $args}
	selection 	{eval $doWidgetCommand entry selection $args}
	xview 	{eval $doWidgetCommand entry xview $args}

        entryset {
          # update the entry field without invoking the command
	  ::combobox::setValue $widgets(this) [lindex $args 0] 0
	}

	toggle {
	    # ignore this command if the widget is disabled...
	    if {$options(-state) == "disabled"} return

	    # pops down the list if it is not, hides it
	    # if it is...
	    if {[winfo ismapped $widgets(popup)]} {
		$widgets(this) close
	    } else {
		$widgets(this) open
	    }
	}

	open {
	    # if we are disabled, we won't allow this to happen
	    if {$options(-state) == "disabled"} {
		return 0
	    }

	    # compute the geometry of the window to pop up, and set
	    # it, and force the window manager to take notice
	    # (even if it is not presently visible).
	    #
	    # this isn't strictly necessary if the window is already
	    # mapped, but we'll go ahead and set the geometry here
	    # since its harmless and *may* actually reset the geometry
	    # to something better in some weird case.
	    set geometry [::combobox::computeGeometry $widgets(this)]
	    wm geometry $widgets(popup) $geometry
	    update idletasks

	    # if we are already open, there's nothing else to do
	    if {[winfo ismapped $widgets(popup)]} {
		return 0
	    }

	    # ok, tweak the visual appearance of things and 
	    # make the list pop up
	    $widgets(button) configure -relief sunken
	    wm deiconify $widgets(popup) 
	    raise $widgets(popup) [winfo parent $widgets(this)]
	    focus -force $widgets(entry)

	    # select something by default, but only if its an
	    # exact match...
	    ::combobox::find $widgets(this) 1

	    # *gasp* do a global grab!!! Mom always told not to
	    # do things like this... :-)
	    grab -global $widgets(this)

	    # fake the listbox into thinking it has focus
	    event generate $widgets(listbox) <B1-Enter>

	    return 1
	}

	close {
	    # if we are already closed, don't do anything...
	    if {![winfo ismapped $widgets(popup)]} {
		return 0
	    }
	    # hides the listbox
	    grab release $widgets(this)
	    $widgets(button) configure -relief raised
	    wm withdraw $widgets(popup) 

	    # select the data in the entry widget. Not sure
	    # why, other than observation seems to suggest that's
	    # what windows widgets do.
	    set editable [::combobox::getBoolean $options(-editable)]
	    if {$editable} {
		$widgets(entry) selection range 0 end
		$widgets(button) configure -relief raised
	    }

	    # magic tcl stuff (see tk.tcl in the distribution 
	    # lib directory)
	    tkCancelRepeat

	    return 1
	}

	cget {
	    # tries to mimic the standard "cget" command
	    if {[llength $args] != 1} {
		error "wrong # args: should be \"$widgets(this) cget option\""
	    }
	    set option [lindex $args 0]
	    return [::combobox::configure $widgets(this) cget $option]
	}

	configure {
	    # trys to mimic the standard "configure" command
	    if {[llength $args] == 0} {
		# this isn't the same format as "real" widgets,
		# but for now its good enough
		foreach item [lsort [array names options]] {
		    lappend result [list $item $options($item)]
		}
		return $result

	    } elseif {[llength $args] == 1} {
		# they are requesting configure information...
		set option [lindex $args 0]
		return [::combobox::configure $widgets(this) get $option]
	    } else {
		array set tmpopt $args
		foreach opt [array names tmpopt] {
		    ::combobox::configure $widgets(this) set $opt $tmpopt($opt)
		}
	    }
	}
	default {
	    error "bad option \"$command\""
	}
    }
}

# handles all of the configure and cget foo
proc ::combobox::configure {w action {option ""} {newValue ""}} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options
    set namespace "::combobox::${w}"

    if {$action == "get"} {
	# this really ought to do more than just get the value,
	# but for the time being I don't fully support the configure
	# command in all its glory...
	if {$option == "-value"} {
	    return [list "-value" [$widgets(entry) get]]
	} else {
	    return [list $option $options($option)]
	}

    } elseif {$action == "cget"} {
	if {$option == "-value"} {
	    return [$widgets(entry) get]
	} else {
	    return $options($option)
	}

    } else {

	if {[info exists options($option)]} {
	    set oldValue $options($option)
	    set options($option) $newValue
	} else {
	    set oldValue ""
	    set options($option) $newValue
	}

	# some (actually, most) options require us to
	# do something, like change the attributes of
	# a widget or two. Here's where we do that...
	switch -- $option {
	    -background {
		$widgets(frame)   configure -background $newValue
		$widgets(entry)   configure -background $newValue
		$widgets(listbox) configure -background $newValue
		$widgets(vsb)     configure -background $newValue
		$widgets(vsb)     configure -troughcolor $newValue
	    }

	    -borderwidth {
		$widgets(frame) configure -borderwidth $newValue
	    }

	    -command {
		# nothing else to do...
	    }

	    -cursor {
		$widgets(frame) configure -cursor $newValue
		$widgets(entry) configure -cursor $newValue
		$widgets(listbox) configure -cursor $newValue
	    }

	    -editable {
		if {$newValue} {
		    # it's editable...
		    $widgets(entry) configure -state normal
		} else {
		    $widgets(entry) configure -state disabled
		}
	    }

	    -font {
		$widgets(entry) configure -font $newValue
		$widgets(listbox) configure -font $newValue
	    }

	    -foreground {
		$widgets(entry)   configure -foreground $newValue
		$widgets(button)  configure -foreground $newValue
		$widgets(listbox) configure -foreground $newValue
	    }

	    -height {
		$widgets(listbox) configure -height $newValue
	    }

	    -highlightbackground {
		$widgets(frame) configure -highlightbackground $newValue
	    }

	    -highlightthickness {
		$widgets(frame) configure -highlightthickness $newValue
	    }
	    
	    -image {
		if {[string length $newValue] > 0} {
		    $widgets(button) configure -image $newValue
		} else {
		    $widgets(button) configure -image ::combobox::bimage
		}
	    }

	    -maxheight {
		# computeGeometry may dork with the actual height
		# of the listbox, so let's undork it
		$widgets(listbox) configure -height $options(-height)
	    }

	    -relief {
		$widgets(frame) configure -relief $newValue
	    }

	    -selectbackground {
		$widgets(entry) configure -selectbackground $newValue
		$widgets(listbox) configure -selectbackground $newValue
	    }

	    -selectborderwidth {
		$widgets(entry) configure -selectborderwidth $newValue
		$widgets(listbox) configure -selectborderwidth $newValue
	    }

	    -selectforeground {
		$widgets(entry) configure -selectforeground $newValue
		$widgets(listbox) configure -selectforeground $newValue
	    }

	    -state {
		if {$newValue == "normal"} {
		    # it's enabled
		    set editable [::combobox::getBoolean \
			    $options(-editable)]
		    if {$editable} {
			$widgets(entry) configure -state normal -takefocus 1
		    }
		    $widgets(entry) configure -fg $::combobox::enabledfg
		} else {
		    # it's disabled
		    $widgets(entry) configure -state disabled -takefocus 0\
		      -fg $::combobox::disabledfg
		}
	    }

	    -textvariable {
		# destroy our trace on the old value, if any
		if {[string length $oldValue] > 0} {
		    trace vdelete $oldValue w \
			    [list ::combobox::vTrace $widgets(this)]
		}
		# set up a trace on the new value, if any. Also, set
		# the value of the widget to the current value of
		# the variable

		set variable ::$newValue
		if {[string length $newValue] > 0} {
		    if {[info exists $variable]} {
			::combobox::setValue $widgets(this) [set $variable]
		    }
		    trace variable $variable w \
			    [list ::combobox::vTrace $widgets(this)]
		}
	    }

	    -value {
		::combobox::setValue $widgets(this) $newValue
	    }

	    -width {
		$widgets(entry) configure -width $newValue
		$widgets(listbox) configure -width $newValue
	    }

	    default {
		error "unknown option \"$option\""
	    }
	}
    }
}

# this proc is called whenever the user changes the value of 
# the -textvariable associated with a widget
proc ::combobox::vTrace {w args} {
    upvar ::combobox::${w}::widgets widgets
    upvar ::combobox::${w}::options options
    upvar ::combobox::${w}::ignoreTrace ignoreTrace

    if {[info exists ignoreTrace]} return
    ::combobox::setValue $widgets(this) [set ::$options(-textvariable)]
}

# sets the value of the combobox and calls the -command, if defined
proc ::combobox::setValue {w newValue {call 1}} {
    upvar ::combobox::${w}::widgets     widgets
    upvar ::combobox::${w}::options     options
    upvar ::combobox::${w}::ignoreTrace ignoreTrace
    upvar ::combobox::${w}::oldValue    oldValue

    set editable [::combobox::getBoolean $options(-editable)]

    # update the widget, no matter what. This might cause a few
    # false triggers on a trace of the associated textvariable,
    # but that's a chance we'll have to take. 
    $widgets(entry) configure -state normal
    $widgets(entry) delete 0 end
    $widgets(entry) insert 0 $newValue
    if {!$editable || $options(-state) != "normal"} {
	$widgets(entry) configure -state disabled
    }

    # set the associated textvariable
    if {[string length $options(-textvariable)] > 0} {
	set ignoreTrace 1 ;# so we don't get in a recursive loop
	uplevel \#0 [list set $options(-textvariable) $newValue]
	unset ignoreTrace
    }

    # Call the -command, if it exists.
    # We could optionally check to see if oldValue == newValue
    # first, but sometimes we want to execute the command even
    # if the value didn't change...
    # CYGNUS LOCAL
    # Call it after idle, so the menu gets unposted BEFORE
    # the command gets run...  Make sure to clean up the afters
    # so you don't try to access a dead widget...

    if {$call && [string length $options(-command)] > 0} {
        if {[info exists widgets(after)]} {
            after cancel $widgets(after)
        }
        set widgets(after) [after idle $options(-command) \
                                   [list $widgets(this) $newValue]\;\
                               unset ::combobox::${w}::widgets(after)]
    }
    set oldValue $newValue
}

# returns the value of a (presumably) boolean string (ie: it should
# do the right thing if the string is "yes", "no", "true", 1, etc
proc ::combobox::getBoolean {value {errorValue 1}} {
    if {[catch {expr {([string trim $value])?1:0}} res]} {
	return $errorValue
    } else {
	return $res
    }
}

# computes the combobox widget name based on the name of one of
# it's children widgets.. Not presently used, but might come in
# handy...
proc ::combobox::widgetName {w} {
    while {$w != "."} {
	if {[winfo class $w] == "Combobox"} {
	    return $w
	}
	set w [winfo parent $w]
    }
    error "internal error: $w is not a child of a combobox"
}
