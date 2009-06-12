# LabFrame.tcl --
#
# 	TixLabelFrame Widget: a frame box with a label
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

tixWidgetClass tixLabelFrame {
    -classname TixLabelFrame
    -superclass tixLabelWidget
    -method {
	frame
    }
    -flag {}
    -static {}
    -configspec {
	{-labelside labelSide LabelSide acrosstop}
	{-padx padX Pad 2}
	{-pady padY Pad 2}
    }
    -alias {}
    -default {
	{*Label.font            -Adobe-Helvetica-Bold-R-Normal--*-120-*}
	{*Label.anchor          c}
	{.frame.borderWidth	2}
	{.frame.relief		groove}
	{.border.borderWidth	2}
	{.border.relief		groove}
	{.borderWidth 	 	2}
	{.padX 		 	2}
	{.padY 		 	2}
	{.anchor 	 	sw}
    }
}

#----------------------------------------------------------------------
# Public methods
#----------------------------------------------------------------------
proc tixLabelFrame:frame {w args} {

    return [eval tixCallMethod $w subwidget frame $args]
}
