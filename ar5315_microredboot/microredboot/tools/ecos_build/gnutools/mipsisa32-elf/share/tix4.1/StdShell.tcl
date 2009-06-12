# StdShell.tcl --
#
#	Standard Dialog Shell.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

tixWidgetClass tixStdDialogShell {
    -classname TixStdDialogShell
    -superclass tixDialogShell
    -method {}
    -flag   {
	-cached
    }
    -configspec {
	{-cached cached Cached ""}
    }
}

proc tixStdDialogShell:ConstructWidget {w} {
    upvar #0 $w data

    tixChainMethod $w ConstructWidget
    set data(w:btns)   [tixStdButtonBox $w.btns]
    set data(w_tframe) [frame $w.tframe]

    pack $data(w_tframe) -side top -expand yes -fill both
    pack $data(w:btns) -side bottom -fill both

    tixCallMethod $w ConstructTopFrame $data(w_tframe)
}


# Subclasses of StdDialogShell should override this method instead of
# ConstructWidget.
#
# Override : always
# chain    : before
proc tixStdDialogShell:ConstructTopFrame {w frame} {
    # Do nothing
}
