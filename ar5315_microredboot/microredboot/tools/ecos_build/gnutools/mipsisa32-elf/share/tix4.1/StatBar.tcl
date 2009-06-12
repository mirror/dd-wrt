# StatBar.tcl --
#
#	The StatusBar of an application.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

tixWidgetClass tixStatusBar {
    -classname TixStatusBar
    -superclass tixPrimitive
    -method {
    }
    -flag {
	-fields
    }
    -static {
	-fields
    }
    -configspec {
	{-fields fields Fields ""}
    }
}

#--------------------------
# Create Widget
#--------------------------
proc tixStatusBar:ConstructWidget {w} {
    upvar #0 $w data

    tixChainMethod $w ConstructWidget

    foreach field $data(-fields) {
	set name  [lindex $field 0]
	set width [lindex $field 1]

	set data(w:width) [label $w.$name -width $width]
    }
}


#----------------------------------------------------------------------
#                         Public methods
#----------------------------------------------------------------------


#----------------------------------------------------------------------
#                         Internal commands
#----------------------------------------------------------------------
