/* $Id: gpxlogger.c 3666 2006-10-26 23:11:51Z ckuethe $ */
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <math.h>
#include <time.h>
#include <signal.h>

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

#include <glib/gprintf.h>
#include "gpsd_config.h"

DBusConnection* connection;

static char *author = "Amaury Jacquot";
static char *copyright = "BSD or GPL v 2.0";

static int intrack = 0;
static int first = 1;
static time_t tracklimit = 5; /* seconds */

static struct {
	time_t	old_time;
	time_t	time;
	gint32	mode;
	gdouble	ept;
	gdouble	latitude;
	gdouble	longitude;
	gdouble eph;
	gdouble altitude;
	gdouble epv;
	gdouble	track;
	gdouble epd;
	gdouble	speed;
	gdouble	eps;
	gdouble	climb;
	gdouble epc;
	//gdouble separation;
} gpsfix;


static void print_gpx_trk_start (void) {
	fprintf (stdout, " <trk>\n");
	fprintf (stdout, "  <trkseg>\n");
	fflush (stdout);
}

static void print_gpx_trk_end (void) {
	fprintf (stdout, "  </trkseg>\n");
	fprintf (stdout, " </trk>\n");
	fflush (stdout);
}

static DBusHandlerResult handle_gps_fix (DBusMessage* message) {
	DBusError	error;
	double		temp_time;

	dbus_error_init (&error);

	dbus_message_get_args (message,
			       &error,
			       DBUS_TYPE_DOUBLE, &temp_time,
			       DBUS_TYPE_INT32,	 &gpsfix.mode,
			       DBUS_TYPE_DOUBLE, &gpsfix.ept,
			       DBUS_TYPE_DOUBLE, &gpsfix.latitude,
			       DBUS_TYPE_DOUBLE, &gpsfix.longitude,
			       DBUS_TYPE_DOUBLE, &gpsfix.eph,
			       DBUS_TYPE_DOUBLE, &gpsfix.altitude,
			       DBUS_TYPE_DOUBLE, &gpsfix.epv,
			       DBUS_TYPE_DOUBLE, &gpsfix.track,
			       DBUS_TYPE_DOUBLE, &gpsfix.epd,
			       DBUS_TYPE_DOUBLE, &gpsfix.speed,
			       DBUS_TYPE_DOUBLE, &gpsfix.eps,
			       DBUS_TYPE_DOUBLE, &gpsfix.climb,
			       DBUS_TYPE_DOUBLE, &gpsfix.epc,
			       DBUS_TYPE_INVALID);
	gpsfix.time = floor(temp_time);
	
	/* 
	 * we have a fix there - log the point
	 */
	if ((gpsfix.time!=gpsfix.old_time)&&gpsfix.mode>1) {
		struct tm 	time;

		/* Make new track if the jump in time is above
		 * tracklimit.  Handle jumps both forward and
		 * backwards in time.  The clock sometimes jump
		 * backward when gpsd is submitting junk on the
		 * dbus. */
		if (fabs(gpsfix.time - gpsfix.old_time) > tracklimit && !first) {
			print_gpx_trk_end();
			intrack = 0;
		}

		if (!intrack) {
			print_gpx_trk_start();
			intrack = 1;
			if (first)
			    first = 0;
		}
		
		gpsfix.old_time = gpsfix.time;
		fprintf (stdout, "   <trkpt lat=\"%f\" lon=\"%f\">\n", gpsfix.latitude, gpsfix.longitude);
		fprintf (stdout, "    <ele>%f</ele>\n", gpsfix.altitude);
		gmtime_r (&(gpsfix.time), &time);
		fprintf (stdout, "    <time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\n",
				time.tm_year+1900, time.tm_mon+1, time.tm_mday,
				time.tm_hour, time.tm_min, time.tm_sec);
		if (gpsfix.mode==1)
			fprintf (stdout, "    <fix>none</fix>\n");
		else
			fprintf (stdout, "    <fix>%dd</fix>\n", gpsfix.mode);
		fprintf (stdout, "   </trkpt>\n");
		fflush (stdout);
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}

static void print_gpx_header (void) {
	fprintf (stdout, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf (stdout, "<gpx version=\"1.1\" creator=\"navsys logger\"\n");
	fprintf (stdout, "        xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
	fprintf (stdout, "        xmlns=\"http://www.topografix.com/GPX/1.1\"\n");
	fprintf (stdout, "        xsi:schemaLocation=\"http://www.topografix.com/GPS/1/1\n");
	fprintf (stdout, "        http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");
	fprintf (stdout, " <metadata>\n");
	fprintf (stdout, "  <name>NavSys GPS logger dump</name>\n");
	fprintf (stdout, "  <author>%s</author>\n", author);
	fprintf (stdout, "  <copyright>%s</copyright>\n", copyright);
	fprintf (stdout, " </metadata>\n");
	fflush (stdout);
}

static void print_gpx_footer (void) {
	if (intrack)
		print_gpx_trk_end();
	fprintf (stdout, "</gpx>\n");
	fclose (stdout);
}

static void quit_handler (int signum) {
	syslog (LOG_INFO, "exiting, signal %d received", signum);
	print_gpx_footer ();
	exit (0);
}

/*
 * Message dispatching function
 *
 */
static DBusHandlerResult signal_handler (
		DBusConnection* connection, DBusMessage* message) {
	/* dummy, need to use the variable for some reason */
	connection = NULL;
	
	if (dbus_message_is_signal (message, "org.gpsd", "fix")) 
		return handle_gps_fix (message);
	/*
	 * ignore all other messages
	 */
	
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int main (int argc, char** argv) {
	GMainLoop* mainloop;
	DBusError error;

	/* initializes the gpsfix data structure */
	gps_clear_fix(&gpsfix);

	/* catch all interesting signals */
	signal (SIGTERM, quit_handler);
	signal (SIGQUIT, quit_handler);
	signal (SIGINT, quit_handler);

	//openlog ("gpxlogger", LOG_PID | LOG_NDELAY , LOG_DAEMON);
	//syslog (LOG_INFO, "---------- STARTED ----------");
	
	print_gpx_header ();
	
	mainloop = g_main_loop_new (NULL, FALSE);

	dbus_error_init (&error);
	connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set (&error)) {
		syslog (LOG_CRIT, "%s: %s", error.name, error.message);
		return 3;
	}
	
	dbus_bus_add_match (connection, "type='signal'", &error);
	if (dbus_error_is_set (&error)) {
		syslog (LOG_CRIT, "unable to add match for signals %s: %s", error.name, error.message);
		return 4;
	}

	if (!dbus_connection_add_filter (connection, (DBusHandleMessageFunction)signal_handler, NULL, NULL)) {
		syslog (LOG_CRIT, "unable to register filter with the connection");
		return 5;
	}
	
	dbus_connection_setup_with_g_main (connection, NULL);

	g_main_loop_run (mainloop);
	return 0;
}
