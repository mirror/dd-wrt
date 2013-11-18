/* $Id: TachometerP.h 3486 2006-09-21 00:58:22Z ckuethe $ */
/* TachometerP.h -- Tachometer widget private data */
#ifndef _TachometerP_h
#define _TachometerP_h

#include <Tachometer.h>
#include <X11/Xaw/SimpleP.h>

/* New fields for the Tachometer widget class record */
typedef struct {int foo;} TachometerClassPart;

/* Full class record declaration */
typedef struct _TachometerClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TachometerClassPart	label_class;
} TachometerClassRec;

extern TachometerClassRec tachometerClassRec;

/* New fields for the Tachometer widget record */
typedef struct {
    /* resources */
    Pixel	needle, scale, circle;
    int		value, speed;
    /* private state */
    GC		needle_GC, scale_GC, circle_GC,	background_GC;
    /* We need to store the width and height separately, because when */
    /* we get a resize request, we need to know if the window has     */
    /* gotten bigger.						      */
    Dimension	width, height, internal_border;
} TachometerPart;

/* Full instance record declaration */
typedef struct _TachometerRec {
    CorePart		core;
    SimplePart		simple;
    TachometerPart	tachometer;
} TachometerRec;

#endif /* _TachometerP_h */
