/*
 * Tachometer Widget Implementation
 *
 * Author: Kazuhiko Shutoh, 1989.
 * Revised by Shinji Sumimoto, 1989/9 (xtachos)
 * Modifications : ilham@mit.edu   (July 10 '90)
 * Cleaned up and simplified by Eric S. Raymond, December 2004.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <TachometerP.h>
#include <math.h>

#include "gpsd_config.h"
#include "gps.h"

#define D2R  0.0174532925199432957692369076848861271	/* radians = pi/180 */

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

typedef struct
{
    unsigned char digit[7];
} DigitRec;

typedef struct
{
    int nofline;
    XPoint point_list[5];
} StringRec;

/*@ +charint @*/
/*	Number character database - like an LED */
static DigitRec num_segment[] = {
    {{1, 1, 1, 1, 1, 1, 0}},
    {{0, 1, 1, 0, 0, 0, 0}},
    {{1, 1, 0, 1, 1, 0, 1}},
    {{1, 1, 1, 1, 0, 0, 1}},
    {{0, 1, 1, 0, 0, 1, 1}},
    {{1, 0, 1, 1, 0, 1, 1}},
    {{1, 0, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 0, 0, 0, 0}},
    {{1, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 1, 0, 1, 1}}
};

static XSegment offset[] = {
    {-10, -10, 10, -10},
    {10, -10, 10, 0},
    {10, 0, 10, 10},
    {10, 10, -10, 10},
    {-10, 10, -10, 0},
    {-10, 0, -10, -10},
    {-10, 0, 10, 0}
};

/*@ -initallelements @*/
/*	" X 10 %" character database	*/
static StringRec char_data[] = {
    {2,				/* "X" */
     {{-17, -5},
      {-7, 5}}},
    {2,
     {{-7, -5},
      {-17, 5}}},
    {2,				/* "1" */
     {{-2, -5},
      {-2, 5}}},
    {5,				/* "0" */
     {{2, -5},
      {12, -5},
      {12, 5},
      {2, 5},
      {2, -5}}}
};

/*@ -initallelements @*/
/*@ -charint @*/

/*@ -nullderef -immediatetrans -type -nullassign @*/
#define offst(field) XtOffset(TachometerWidget, field)
static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     offst(tachometer.scale), XtRString, "XtDefaultForeground"}
    ,
    {XtNtachometerCircleColor, XtCBorderColor, XtRPixel, sizeof(Pixel),
     offst(tachometer.circle), XtRString, "XtDefaultForeground"}
    ,
    {XtNtachometerNeedleColor, XtCBorderColor, XtRPixel, sizeof(Pixel),
     offst(tachometer.needle), XtRString, "XtDefaultForeground"}
    ,
    {XtNtachometerNeedleSpeed, XtCtachometerNeedleSpeed, XtRInt,
     sizeof(int), offst(tachometer.speed), XtRImmediate, (caddr_t) 1},
    {XtNvalue, XtCValue, XtRInt, sizeof(int),
     offst(tachometer.value), XtRImmediate, (caddr_t) 0},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
     offst(core.height), XtRImmediate, (caddr_t) 100}
    ,
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
     offst(core.width), XtRImmediate, (caddr_t) 100}
    ,
    {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
     offst(core.border_width), XtRImmediate, (caddr_t) 0}
    ,
    {XtNinternalBorderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
     offst(tachometer.internal_border), XtRImmediate, (caddr_t) 0}
    ,
};

/*@ -nullderef -immediatetrans +type +nullassign @*/

static void Initialize(Widget request, Widget new),
Realize(Widget w, Mask * valueMask, XSetWindowAttributes * attributes),
Resize(Widget w), Redisplay(Widget w, XEvent * event, Region region),
Destroy(Widget w);
static Boolean SetValues(Widget current, Widget request UNUSED, Widget new);

/*@ -fullinitblock @*/
TachometerClassRec tachometerClassRec = {
    {
/* core_class fields */
#define superclass		(&simpleClassRec)
     /* superclass               */ (WidgetClass) superclass,
     /* class_name               */ "Tachometer",
     /* widget_size              */ sizeof(TachometerRec),
     /* class_initialize         */ NULL,
     /* class_part_initialize    */ NULL,
     /* class_inited             */ FALSE,
     /* initialize               */ (XtInitProc) Initialize,
     /* initialize_hook          */ NULL,
     /* realize                  */ Realize,
     /* actions                  */ NULL,
     /* num_actions              */ 0,
     /* resources                */ resources,
     /* num_resources            */ XtNumber(resources),
     /* xrm_class                */ NULLQUARK,
     /* compress_motion          */ TRUE,
     /* compress_exposure        */ TRUE,
     /* compress_enterleave      */ TRUE,
     /* visible_interest         */ FALSE,
     /* destroy                  */ Destroy,
     /* resize                   */ Resize,
     /* expose                   */ Redisplay,
     /* set_values               */ (XtSetValuesFunc) SetValues,
     /* set_values_hook          */ NULL,
     /* set_values_almost        */ XtInheritSetValuesAlmost,
     /* get_values_hook          */ NULL,
     /* accept_focus             */ NULL,
     /* version                  */ XtVersion,
     /* callback_private         */ NULL,
     /* tm_table                 */ NULL,
     /* query_geometry           */ NULL,
     /* display_accelerator      */ XtInheritDisplayAccelerator,
     /* extension                */ NULL
     }
    ,
/* Simple class fields initialization */
    {
     /* change_sensitive         */ XtInheritChangeSensitive
     }

};

/*@ +fullinitblock @*/
WidgetClass tachometerWidgetClass = (WidgetClass) & tachometerClassRec;

/* Private procedures */

static void FastFillCircle(Display * d, Drawable w, GC gc,
			   Cardinal center_x, Cardinal center_y,
			   Cardinal radius_x, Cardinal radius_y)
{
    /*@ -compdef @*/
    XPoint points[360];
    Cardinal angle;

    for (angle = 0; angle < 360; angle++) {
	points[angle].x = (short)(sin((double)angle * D2R) *
				  (double)radius_x + (double)center_x);
	points[angle].y = (short)(cos((double)angle * D2R) *
				  (double)radius_y + (double)center_y);
    }
    (void)XFillPolygon(d, w, gc, points, 360, Complex, CoordModeOrigin);
    /*@ +compdef @*/
}

static void DrawSingleNumber(TachometerWidget w, int which, Cardinal x,
			     Cardinal y)
{
    XSegment segments[7];
    Cardinal nsegments, width, height, count;

    width = (Cardinal) ((w->core.width / 2) - w->tachometer.internal_border);
    height =
	(Cardinal) ((w->core.height / 2) - w->tachometer.internal_border);
    if ((width == 0) || (height == 0))
	return;

    /*@ +charint -compdef */
    for (count = 0, nsegments = 0; count < 7; count++)
	if (num_segment[which].digit[count] == 1) {
	    segments[nsegments].x1 = (short)
		(x + ((double)offset[count].x1 * ((double)width / 200.0)));
	    segments[nsegments].y1 = (short)
		(y + ((double)offset[count].y1 * ((double)height / 200.0)));
	    segments[nsegments].x2 = (short)
		(x + ((double)offset[count].x2 * ((double)width / 200.0)));
	    segments[nsegments].y2 = (short)
		(y + ((double)offset[count].y2 * ((double)height / 200.0)));
	    nsegments++;
	}

    (void)XDrawSegments(XtDisplay(w), XtWindow(w),
			w->tachometer.scale_GC, segments, (int)nsegments);
    /*@ -charint +compdef */
}

static void DrawNumbers(TachometerWidget w, int which, Cardinal x, Cardinal y)
{
    if (which == 10) {
	DrawSingleNumber(w, 1, (Cardinal) ((double)x * 0.9), y);
	DrawSingleNumber(w, 0, x, y);
    } else
	DrawSingleNumber(w, which, x, y);
}

static void DrawLabelString(TachometerWidget w)
{
    XPoint points[5];
    int char_count, data_count;
    Cardinal ry, center_x, center_y, radius_x, radius_y;
    GC gc;

    center_x = (Cardinal) (w->core.width / 2);
    center_y = (Cardinal) (w->core.height / 2);
    radius_x = center_x - w->tachometer.internal_border;
    radius_y = center_y - w->tachometer.internal_border;
    if (!(center_x != 0 && center_y != 0 && (radius_x > 0) && (radius_y > 0)))
	return;

    ry = (Cardinal) (radius_y * 0.35 + center_y);
    gc = w->tachometer.scale_GC;
    /*@ -compdef @*/
    for (char_count = 0; char_count < 4; char_count++) {
	for (data_count = 0; data_count < char_data[char_count].nofline;
	     data_count++) {
	    points[data_count].x = (short)
		((char_data[char_count].point_list[data_count].x) *
		 (double)radius_x * 0.01 + center_x);
	    points[data_count].y = (short)
		((char_data[char_count].point_list[data_count].y) *
		 (double)radius_y * 0.01 + ry);
	}
	(void)XDrawLines(XtDisplay(w), XtWindow(w), gc, points,
			 char_data[char_count].nofline, CoordModeOrigin);
    }
    /*@ +compdef @*/
}

static void DrawGauge(TachometerWidget w)
{
    XPoint points[4];
    Cardinal in_gauge_x, in_gauge_y, out_gauge_x, out_gauge_y;
    Cardinal number_x, number_y, center_x, center_y, radius_x, radius_y;
    GC gc;
    double step, jump = 1.0;

    /*@ -type -unsignedcompare -compdef @*/
    center_x = w->core.width / 2;
    center_y = w->core.height / 2;
    radius_x = center_x - w->tachometer.internal_border;
    radius_y = center_y - w->tachometer.internal_border;
    if ((center_x == 0) || (center_y == 0) || (radius_x <= 0)
	|| (radius_y <= 0))
	return;			/* Can't draw anything */

    gc = w->tachometer.scale_GC;
    for (step = 330.0; step >= 30.0; step -= jump) {
	if ((Cardinal) (step) % 30 == 0) {
	    points[0].x =
		sin((step + 1.0) * D2R) * radius_x * 0.75 + center_x;
	    points[0].y =
		cos((step + 1.0) * D2R) * radius_y * 0.75 + center_y;
	    points[1].x =
		sin((step - 1.0) * D2R) * radius_x * 0.75 + center_x;
	    points[1].y =
		cos((step - 1.0) * D2R) * radius_y * 0.75 + center_y;
	    points[2].x =
		sin((step - 1.0) * D2R) * radius_x * 0.85 + center_x;
	    points[2].y =
		cos((step - 1.0) * D2R) * radius_y * 0.85 + center_y;
	    points[3].x =
		sin((step + 1.0) * D2R) * radius_x * 0.85 + center_x;
	    points[3].y =
		cos((step + 1.0) * D2R) * radius_y * 0.85 + center_y;
	    (void)XFillPolygon(XtDisplay(w), XtWindow(w), gc, points, 4,
			       Complex, CoordModeOrigin);

	    number_x = sin((step + 1.0) * D2R) * radius_x * 0.65 + center_x;
	    number_y = cos((step + 1.0) * D2R) * radius_y * 0.65 + center_y;
	    if ((int)((330.0 - step) / 30.0) == 1)
		jump = 3.0;
	    DrawNumbers(w, (unsigned char)((330.0 - step) / 30.0),
			number_x, number_y);
	} else {
	    in_gauge_x = sin(step * D2R) * radius_x * 0.8 + center_x;
	    in_gauge_y = cos(step * D2R) * radius_y * 0.8 + center_y;
	    out_gauge_x = sin(step * D2R) * radius_x * 0.85 + center_x;
	    out_gauge_y = cos(step * D2R) * radius_y * 0.85 + center_y;
	    (void)XDrawLine(XtDisplay(w), XtWindow(w), gc, in_gauge_x,
			    in_gauge_y, out_gauge_x, out_gauge_y);
	}
    }
    /*@ +type +unsignedcompare +compdef @*/

    DrawLabelString(w);
}

static void DrawNeedle(TachometerWidget w, int load)
{
    XPoint points[6];
    double cur_theta1, cur_theta2, cur_theta3, cur_theta4, cur_theta5;
    Cardinal center_x, center_y, radius_x, radius_y;

    /*@ -type -unsignedcompare -compdef @*/
    center_x = w->core.width / 2;
    center_y = w->core.height / 2;
    radius_x = center_x - w->tachometer.internal_border;
    radius_y = center_y - w->tachometer.internal_border;
    if ((center_x == 0) || (center_y == 0) || (radius_x <= 0)
	|| (radius_y <= 0))
	return;			/* can't draw anything */

    cur_theta1 = (double)(330 - (load * 3)) * D2R;
    cur_theta2 = (double)(330 - (load * 3) + 1) * D2R;
    cur_theta3 = (double)(330 - (load * 3) - 1) * D2R;
    cur_theta4 = (330.0 - ((double)load * 3.0) + 7.0) * D2R;
    cur_theta5 = (330.0 - ((double)load * 3.0) - 7.0) * D2R;

    points[0].x = (short)(sin(cur_theta1) * radius_x * 0.75 + center_x);
    points[0].y = (short)(cos(cur_theta1) * radius_y * 0.75 + center_y);
    points[1].x = (short)(sin(cur_theta2) * radius_x * 0.7 + center_x);
    points[1].y = (short)(cos(cur_theta2) * radius_y * 0.7 + center_y);
    points[2].x = (short)(sin(cur_theta4) * radius_x * 0.1 + center_x);
    points[2].y = (short)(cos(cur_theta4) * radius_y * 0.1 + center_y);
    points[3].x = (short)(sin(cur_theta5) * radius_x * 0.1 + center_x);
    points[3].y = (short)(cos(cur_theta5) * radius_y * 0.1 + center_y);
    points[4].x = (short)(sin(cur_theta3) * radius_x * 0.7 + center_x);
    points[4].y = (short)(cos(cur_theta3) * radius_y * 0.7 + center_y);
    /*@ -usedef @*/
    points[5].x = points[0].x;
    points[5].y = points[0].y;
    /*@ +usedef @*/

    (void)XDrawLines(XtDisplay(w), XtWindow(w),
		     w->tachometer.needle_GC, points, 6, CoordModeOrigin);
    /*@ +type +unsignedcompare +compdef @*/
}

static void DrawTachometer(TachometerWidget w)
{
    Cardinal center_x, center_y, radius_x, radius_y;

    /*@ -type -unsignedcompare -compdef @*/
    center_x = w->core.width / 2;
    center_y = w->core.height / 2;
    radius_x = center_x - w->tachometer.internal_border;
    radius_y = center_y - w->tachometer.internal_border;
    if ((center_x == 0) || (center_y == 0) || (radius_x <= 0)
	|| (radius_y <= 0))
	return;			/* Can't draw anything -- no room */

    /* Big circle */
    FastFillCircle(XtDisplay(w), XtWindow(w), w->tachometer.circle_GC,
		   center_x, center_y, radius_x, radius_y);
    /* Inner circle same color as the background */
    FastFillCircle(XtDisplay(w), XtWindow(w), w->tachometer.background_GC,
		   center_x, center_y, (Cardinal) (radius_x * 0.95),
		   (Cardinal) (radius_y * 0.95));
    /* Small circle */
    FastFillCircle(XtDisplay(w), XtWindow(w), w->tachometer.circle_GC,
		   center_x, center_y, (Cardinal) (radius_x * 0.1),
		   (Cardinal) (radius_y * 0.1));
    /* Draw the details */
    DrawGauge(w);
    DrawNeedle(w, w->tachometer.value);
    /*@ +type +unsignedcompare +compdef @*/
}

static void MoveNeedle(TachometerWidget w, int new)
{
    int step, old, loop;

    old = w->tachometer.value;
    if (new > 100)
	new = 100;
    if (old == new)
	return;
    else if (old < new)
	step = (w->tachometer.speed ? w->tachometer.speed : new - old);
    else
	step = (w->tachometer.speed ? -w->tachometer.speed : new - old);

    /*@ -usedef @*/
    if (old < new) {
	for (loop = old; loop < new; loop += step)
	    DrawNeedle(w, loop);
	for (loop = old + step; loop <= new; loop += step)
	    DrawNeedle(w, loop);
    } else {
	for (loop = old; loop > new; loop += step)
	    DrawNeedle(w, loop);
	for (loop = old + step; loop >= new; loop += step)
	    DrawNeedle(w, loop);
    }

    if (loop != new + step)	/* The final needle wasn't printed */
	DrawNeedle(w, new);
    /*@ +usedef @*/

    w->tachometer.value = new;
}

static void GetneedleGC(TachometerWidget ta)
{
    XGCValues values;

    values.background = ta->core.background_pixel;
    values.foreground = ta->tachometer.needle ^ ta->core.background_pixel;
    values.function = GXxor;
    /*@ -type -compdef -mustfreeonly @*/
    ta->tachometer.needle_GC = XtGetGC((Widget) ta,
				       (unsigned)GCFunction | GCBackground |
				       GCForeground, &values);
    /*@ +type +compdef +mustfreeonly @*/
}

static void GetscaleGC(TachometerWidget ta)
{
    XGCValues values;

    values.foreground = ta->tachometer.scale;
    values.background = ta->core.background_pixel;
    /*@ -type -compdef -mustfreeonly @*/
    ta->tachometer.scale_GC = XtGetGC((Widget) ta,
				      (unsigned)GCForeground | GCBackground,
				      &values);
    /*@ +type +compdef +mustfreeonly @*/
}

static void GetcircleGC(TachometerWidget ta)
{
    XGCValues values;

    values.foreground = ta->tachometer.circle;
    values.background = ta->core.background_pixel;
    /*@ -type -compdef -mustfreeonly @*/
    ta->tachometer.circle_GC = XtGetGC((Widget) ta,
				       (unsigned)GCForeground | GCBackground,
				       &values);
    /*@ +type +compdef +mustfreeonly @*/
}

static void GetbackgroundGC(TachometerWidget ta)
{
    XGCValues values;

    values.foreground = ta->core.background_pixel;
    values.background = ta->core.background_pixel;
    /*@ -type -compdef -mustfreeonly @*/
    ta->tachometer.background_GC = XtGetGC((Widget) ta,
					   (unsigned)GCForeground |
					   GCBackground, &values);
    /*@ +type +compdef +mustfreeonly @*/
}

static void Initialize(Widget request UNUSED, Widget new)
{
    TachometerWidget ta = (TachometerWidget) new;

    GetneedleGC(ta);
    GetcircleGC(ta);
    GetscaleGC(ta);
    GetbackgroundGC(ta);
    ta->tachometer.width = ta->tachometer.height = 0;
}				/* Initialize */

static void Realize(Widget w, Mask * valueMask,
		    XSetWindowAttributes * attributes)
{
    *valueMask |= CWBitGravity;
    attributes->bit_gravity = NorthWestGravity;
    (*superclass->core_class.realize) (w, valueMask, attributes);
}				/* Realize */

static void Redisplay(Widget w, XEvent * event, Region region UNUSED)
{
    if (event->xexpose.count == 0)
	DrawTachometer((TachometerWidget) w);
}				/* Redisplay */

static void Resize(Widget w)
{
    TachometerWidget ta = (TachometerWidget) w;

    if ((ta->core.width == ta->tachometer.width) &&
	(ta->core.height == ta->tachometer.height))
	/* What resize?  We don't see a resize! */
	return;

    (void)XClearWindow(XtDisplay(w), XtWindow(w));

    if ((ta->core.width <= ta->tachometer.width) &&
	(ta->core.height <= ta->tachometer.height))
	/* Only redraw here if no expose events are going to be */
	/* generated, i.e. if the window has not grown horizontally */
	/* or vertically. */
	DrawTachometer(ta);

    ta->tachometer.width = ta->core.width;
    ta->tachometer.height = ta->core.height;
}				/* Resize */

static Boolean SetValues(Widget current, Widget request UNUSED, Widget new)
/* Set specified arguments into widget */
{
    /*@ -type -boolops -predboolothers @*/
    Boolean back, changed = (Boolean) False;

    TachometerWidget curta = (TachometerWidget) current;
    TachometerWidget newta = (TachometerWidget) new;

    back = (curta->core.background_pixel != newta->core.background_pixel);
    if (back || (curta->tachometer.needle != newta->tachometer.needle)) {
	(void)XtReleaseGC(new, newta->tachometer.needle_GC);
	GetneedleGC(newta);
	changed = True;
    }
    if (back || (curta->tachometer.scale != newta->tachometer.scale)) {
	(void)XtReleaseGC(new, newta->tachometer.scale_GC);
	GetscaleGC(newta);
	changed = True;
    }
    if (back || (curta->tachometer.circle != newta->tachometer.circle)) {
	(void)XtReleaseGC(new, newta->tachometer.circle_GC);
	GetcircleGC(newta);
	changed = True;
    }
    if (back) {
	(void)XtReleaseGC(new, newta->tachometer.background_GC);
	GetbackgroundGC(newta);
	changed = True;
    }
    if (curta->tachometer.value != newta->tachometer.value) {
	MoveNeedle(newta, newta->tachometer.value);
	changed = True;
    }
    /*@ +type +boolops +predboolothers @*/

    return (changed);
}

static void Destroy(Widget w)
{
    TachometerWidget ta = (TachometerWidget) w;

    (void)XtReleaseGC(w, ta->tachometer.needle_GC);
    (void)XtReleaseGC(w, ta->tachometer.circle_GC);
    (void)XtReleaseGC(w, ta->tachometer.scale_GC);
    (void)XtReleaseGC(w, ta->tachometer.background_GC);
}

/* Exported Procedures */

int TachometerGetValue(Widget w)
{
    return (((TachometerWidget) w)->tachometer.value);
}

int TachometerSetValue(Widget w, int i)
{
    int old;
    TachometerWidget ta = (TachometerWidget) w;

    old = ta->tachometer.value;
    MoveNeedle(ta, i);
    return (old);
}
