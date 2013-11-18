/* $Id: Tachometer.h 3486 2006-09-21 00:58:22Z ckuethe $ */
/* Tachometer.h -- tachometer widget interface */
#ifndef _Tachometer_h
#define _Tachometer_h

#include <X11/Xaw/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 circleColor	     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	0
 cursor		     Cursor		Cursor		None
 destroyCallback     Callback		XtCallbackList	NULL
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	100
 insensitiveBorder   Insensitive	Pixmap		Gray
 internalBorderWidth BorderWidth	Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 needleColor	     BorderColor	Pixel		XtDefaultForeground
 needleSpeed	     NeedleSpeed	int		1
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	100
 value		     Value		int		0
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define XtNinternalBorderWidth "internalBorderWidth"
#define XtNtachometerNeedleSpeed "needleSpeed"
#define XtNtachometerCircleColor "circleColor"
#define XtNtachometerNeedleColor "needleColor"
#define XtCtachometerNeedleSpeed "NeedleSpeed"

extern int TachometerGetValue(Widget);
extern int TachometerSetValue(Widget, int);

/* Class record constants */

extern WidgetClass tachometerWidgetClass;

typedef struct _TachometerClassRec *TachometerWidgetClass;
typedef struct _TachometerRec      *TachometerWidget;

#endif /* _Tachometer_h */
