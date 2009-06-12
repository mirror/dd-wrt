# Shell.tcl --
#
#	This is the base class to all shell widget
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

#
# type : normal, transient, overrideredirect
#
tixWidgetClass tixShell {
    -superclass tixPrimitive
    -classname  TixShell
    -flag {
	-title
    }
    -configspec {
	{-title title Title ""}
    }
    -forcecall {
	-title
    }
}

#----------------------------------------------------------------------
# ClassInitialization:
#----------------------------------------------------------------------
proc tixShell:CreateRootWidget {w args} {
    upvar #0 $w data
    upvar #0 $data(className) classRec

    toplevel $w -class $data(ClassName)
    wm withdraw $w
}

proc tixShell:config-title {w value} {
    wm title $w $value
}
