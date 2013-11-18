/* $Id: xgpsspeed.c 3666 2006-10-26 23:11:51Z ckuethe $ */
/* GPS speedometer as a wrapper around an Athena widget Tachometer
 * - Derrick J Brashear <shadow@dementia.org>
 */
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <Xm/Xm.h>
#include <Xm/XmStrDefs.h>
#include <Tachometer.h>

#include "gpsd_config.h"
#include "gps.h"

#include "xgpsspeed.icon"

/*@ -nullassign @*/
static XrmOptionDescRec options[] = {
{"-rv",		"*reverseVideo",	XrmoptionNoArg,		"TRUE"},
{"-nc",         "*needleColor",         XrmoptionSepArg,        NULL},
{"-needlecolor","*needleColor",         XrmoptionSepArg,        NULL},
{"-speedunits", "*speedunits",          XrmoptionSepArg,        NULL},
};
String fallback_resources[] = {NULL};
/*@ +nullassign @*/

static struct gps_data_t *gpsdata;
static Widget tacho;
static double speedfactor;
static Widget toplevel;

static void update_display(struct gps_data_t *gpsdata, 
			   char *buf UNUSED, size_t len UNUSED, int level UNUSED)
{
    int temp_int = (int)rint(gpsdata->fix.speed * speedfactor);

    if (temp_int < 0) temp_int = 0;
    else if (temp_int > 100) temp_int = 100;

    (void)TachometerSetValue(tacho, temp_int);
}

static void handle_input(XtPointer client_data UNUSED,
			 int *source UNUSED, XtInputId * id UNUSED)
{
    if (gps_poll(gpsdata) < 0) {
	(void)fprintf(stderr, "Read error on server socket.");
	exit(1);
    }
}

static char *get_resource(Widget w, char *name, char *default_value)
{
  XtResource xtr;
  char *value = NULL;

  /*@ -observertrans -statictrans -immediatetrans -compdestroy -nullpass @*/
  xtr.resource_name = name;
  xtr.resource_class = "AnyClass";
  xtr.resource_type = XmRString;
  xtr.resource_size = (Cardinal)sizeof(String);
  xtr.resource_offset = 0;
  xtr.default_type = XmRImmediate;
  xtr.default_addr = default_value;
  XtGetApplicationResources(w, &value, &xtr, 1, NULL, 0);
  if (value) return value;
  /*@ +observertrans +statictrans +immediatetrans +compdestroy +nullpass @*/
  /*@i@*/return default_value;
}

/*@ -mustfreefresh @*/
int main(int argc, char **argv)
{
    Arg             args[10];
    XtAppContext app;
    int option;
    char *arg = NULL, *colon1, *colon2, *device = NULL, *server = NULL, *port = DEFAULT_GPSD_PORT;
    char *speedunits;
    Widget base;

    /*@ -compdef -nullpass -onlytrans @*/
    toplevel = XtVaAppInitialize(&app, "xgpsspeed", 
				 options, XtNumber(options),
				 &argc, argv, fallback_resources, NULL);

    /*@ +compdef +nullpass +onlytrans @*/
    speedfactor = MPS_TO_MPH;		/* Software maintained in US */
    speedunits = get_resource(toplevel, "speedunits", "mph");
    if (strcmp(speedunits, "kph")==0) 
	speedfactor = MPS_TO_KPH;
    else if (strcmp(speedunits, "knots")==0)
	speedfactor = MPS_TO_KNOTS;

    while ((option = getopt(argc, argv, "hV")) != -1) {
	switch (option) {
	case 'V':
	    (void)printf("xgpsspeed %s\n", VERSION);
	    exit(0);
	case 'h': default:
	    (void)fputs("usage: gps [-h] [-V] [-rv] [-nc] [-needlecolor] [-speedunits {mph,kph,knots}] [server[:port]]\n", stderr);
	    exit(1);
	}
    }
    /*@ -branchstate -nullpass @*/
    if (optind < argc) {
	arg = strdup(argv[optind]);
	colon1 = strchr(arg, ':');
	server = arg;
	if (colon1 != NULL) {
	    if (colon1 == arg)
		server = NULL;
	    else
		*colon1 = '\0';
	    port = colon1 + 1;
	    colon2 = strchr(port, ':');
	    if (colon2 != NULL) {
		if (colon2 == port)
		    port = NULL;
	        else
		    *colon2 = '\0';
		device = colon2 + 1;
	    }
	}
	colon1 = colon2 = NULL;
    }
    /*@ +branchstate @*/

    /*@ -immediatetrans -usedef -observertrans -statictrans @*/
    /**** Shell Widget ****/
    (void)XtSetArg(args[0], XtNiconPixmap,
	     XCreateBitmapFromData(XtDisplay(toplevel),
				   XtScreen(toplevel)->root, (char*)xgps_bits,
				   xgps_width, xgps_height));
    (void)XtSetValues(toplevel, args, 1);
    
    /**** Form widget ****/
    base = XtCreateManagedWidget("pane", panedWidgetClass, toplevel, NULL, 0);

    /**** Label widget (Title) ****/
    (void)XtSetArg(args[0], XtNlabel, "GPS Speedometer");
    (void)XtCreateManagedWidget("title", labelWidgetClass, base, args, 1);

    /**** Label widget ****/
    if (speedfactor == MPS_TO_MPH)
        (void)XtSetArg(args[0], XtNlabel, "Miles per Hour");
    else if (speedfactor == MPS_TO_KPH)
        (void)XtSetArg(args[0], XtNlabel, "Km per Hour");
    else 
        (void)XtSetArg(args[0], XtNlabel, "Knots");

    /*@ +immediatetrans +usedef +observertrans +statictrans @*/
    (void)XtCreateManagedWidget("name", labelWidgetClass, base, args, 1);
    
    /**** Tachometer widget ****/
    /*@ -onlytrans -mustfreeonly @*/
    tacho = XtCreateManagedWidget("meter", tachometerWidgetClass,base,NULL,0);
    (void)XtRealizeWidget(toplevel);

    if (!(gpsdata = gps_open(server, DEFAULT_GPSD_PORT))) {
	(void)fputs("xgpsspeed: no gpsd running or network error\n", stderr);
	exit(2);
    }
    /*@ +onlytrans +mustfreeonly @*/

    /*@ -usedef @*/
    (void)XtAppAddInput(app, gpsdata->gps_fd, (XtPointer) XtInputReadMask,
		  handle_input, NULL);
    /*@ +nullpass +usedef @*/

    gps_set_raw_hook(gpsdata, update_display);

    if (device) {
	char *channelcmd;
	size_t l;
	l = strlen(device)+4;

	if ((channelcmd = (char *)malloc(l)) != NULL){
	    /*@ -compdef @*/
	    (void)strlcpy(channelcmd, "F=", l);
	    (void)strlcpy(channelcmd+2, device, l);
	    (void)gps_query(gpsdata, channelcmd);
	    (void)free(channelcmd);
	    /*@ +compdef @*/
	}
    }

    (void)gps_query(gpsdata, "w+x\n");

    (void)XtAppMainLoop(app);

    (void)gps_close(gpsdata);
    if (arg != NULL)
	(void)free(arg);
    return 0;
}
/*@ +mustfreefresh @*/
