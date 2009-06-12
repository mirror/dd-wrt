# EFileDlg.tcl --
#
#	Implements the Extended File Selection Dialog widget.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

tixWidgetClass tixExFileSelectDialog {
    -classname TixExFileSelectDialog
    -superclass tixDialogShell
    -method {}
    -flag   {
	-command
    }
    -configspec {
	{-command command Command ""}

	{-title title Title "Select A File"}
    }
}

proc tixExFileSelectDialog:ConstructWidget {w} {
    upvar #0 $w data

    tixChainMethod $w ConstructWidget
    set data(w:fsbox) [tixExFileSelectBox $w.fsbox -dialog $w \
	-command $data(-command)]
    pack $data(w:fsbox) -expand yes -fill both
}

proc tixExFileSelectDialog:config-command {w value} {
    upvar #0 $w data

    $data(w:fsbox) config -command $value
}

proc tixExFileSelectDialog:SetBindings {w} {
    upvar #0 $w data

    tixChainMethod $w SetBindings

    bind $w <Alt-Key-f> "focus [$data(w:fsbox) subwidget file]"
    bind $w <Alt-Key-t> "focus [$data(w:fsbox) subwidget types]"
    bind $w <Alt-Key-d> "focus [$data(w:fsbox) subwidget dir]"
    bind $w <Alt-Key-o> "tkButtonInvoke [$data(w:fsbox) subwidget ok]"
    bind $w <Alt-Key-c> "tkButtonInvoke [$data(w:fsbox) subwidget cancel]"
    bind $w <Alt-Key-s> "tkButtonInvoke [$data(w:fsbox) subwidget hidden]"
}
