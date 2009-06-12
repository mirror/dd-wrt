# SText.tcl --
#
#	This file implements Scrolled Text widgets
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#



tixWidgetClass tixScrolledText {
    -classname TixScrolledText
    -superclass tixScrolledWidget
    -method {
    }
    -flag {
    }
    -static {
    }
    -configspec {
    }
    -default {
	{.scrollbar			both}
	{*borderWidth			1}
	{*text.background		#c3c3c3}
	{*text.highlightBackground	#d9d9d9}
	{*text.relief			sunken}
	{*text.takeFocus		1}
	{*Scrollbar.background		#d9d9d9}
	{*Scrollbar.relief		sunken}
	{*Scrollbar.troughColor		#c3c3c3}
	{*Scrollbar.takeFocus		0}
	{*Scrollbar.width		15}
    }
    -forcecall {
	-scrollbar
    }
}

proc tixScrolledText:ConstructWidget {w} {
    upvar #0 $w data
    global tcl_platform

    tixChainMethod $w ConstructWidget

    set data(w:text) \
	[text $w.text]
    set data(w:hsb) \
	[scrollbar $w.hsb -orient horizontal]
    set data(w:vsb) \
	[scrollbar $w.vsb -orient vertical]

    if {$data(-sizebox) && $tcl_platform(platform) == "windows"} {
        set data(w:sizebox) [ide_sizebox $w.sizebox]
    }

    set data(pw:client) $data(w:text)
}

proc tixScrolledText:SetBindings {w} {
    upvar #0 $w data

    tixChainMethod $w SetBindings

    $data(w:text) config \
	-xscrollcommand "tixScrolledText:XScroll $w"\
	-yscrollcommand "tixScrolledText:YScroll $w"

    $data(w:hsb) config -command "$data(w:text) xview"
    $data(w:vsb) config -command "$data(w:text) yview"
}

#----------------------------------------------------------------------
#
#		option configs
#----------------------------------------------------------------------
proc tixScrolledText:config-takefocus {w value} {
    upvar #0 $w data
  
    $data(w:text) config -takefocus $value
}	

proc tixScrolledText:config-scrollbar {w value} {
    upvar #0 $w data
  
    if {[string match "auto*" $value]} {
	set value "both"
    }
    set data(-scrollbar) $value

    tixChainMethod $w config-scrollbar $value

    return $value
}	

#----------------------------------------------------------------------
#
#		Widget commands
#----------------------------------------------------------------------


#----------------------------------------------------------------------
#
#		Private Methods
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# virtual functions to query the client window's scroll requirement
#----------------------------------------------------------------------
proc tixScrolledText:GeometryInfo {w mW mH} {
    upvar #0 $w data

    return [list "$data(x,first) $data(x,last)" "$data(y,first) $data(y,last)"]
}

proc tixScrolledText:XScroll {w first last} {
    upvar #0 $w data

    set data(x,first) $first
    set data(x,last)  $last

    $data(w:hsb) set $first $last

    tixWidgetDoWhenIdle tixScrolledWidget:Configure $w
}

proc tixScrolledText:YScroll {w first last} {
    upvar #0 $w data

    set data(y,first) $first
    set data(y,last)  $last
    
    $data(w:vsb) set $first $last

    tixWidgetDoWhenIdle tixScrolledWidget:Configure $w
}
