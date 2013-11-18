/* $Id: cgps.c 4074 2006-12-04 21:55:45Z jfrancis $ */
/*
 * Copyright (c) 2005 Jeff Francis <jeff@gritch.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
  Jeff Francis
  jeff@gritch.org

  Kind of a curses version of xgps for use with gpsd.
*/

/* ==================================================================
   These #defines should be modified if changing the number of fields
   to be displayed.
   ================================================================== */

/* This defines how much overhead is contained in the 'datawin' window
   (eg, box around the window takes two lines). */
#define DATAWIN_OVERHEAD 2

/* This defines how much overhead is contained in the 'satellites'
   window (eg, box around the window takes two lines, plus the column
   headers take another line). */
#define SATWIN_OVERHEAD 3

/* This is how many display fields are output in the 'datawin' window
   when in GPS mode.  Change this value if you add or remove fields
   from the 'datawin' window for the GPS mode. */
#define DATAWIN_GPS_FIELDS 9

/* This is how many display fields are output in the 'datawin' window
   when in COMPASS mode.  Change this value if you add or remove fields
   from the 'datawin' window for the COMPASS mode. */
#define DATAWIN_COMPASS_FIELDS 6

/* This is how far over in the 'datawin' window to indent the field
   descriptions. */
#define DATAWIN_DESC_OFFSET 5

/* This is how far over in the 'datawin' window to indent the field
   values. */
#define DATAWIN_VALUE_OFFSET 17

/* This is the width of the 'datawin' window.  It's recommended to
   keep DATAWIN_WIDTH + SATELLITES_WIDTH <= 80 so it'll fit on a
   "standard" 80x24 screen. */
#define DATAWIN_WIDTH 45

/* This is the width of the 'satellites' window.  It's recommended to
   keep DATAWIN_WIDTH + SATELLITES_WIDTH <= 80 so it'll fit on a
   "standard" 80x24 screen. */
#define SATELLITES_WIDTH 35

/* This is the title to put at the top of the screen. */
#define TITLE "GPSD Test Client"

/* ================================================================
   You shouldn't have to modify any #define values below this line.
   ================================================================ */

/* This is the minimum size we'll accept for the 'datawin' window in
   GPS mode. */
#define MIN_GPS_DATAWIN_SIZE (DATAWIN_GPS_FIELDS + DATAWIN_OVERHEAD)

/* This is the minimum size we'll accept for the 'datawin' window in
   COMPASS mode. */
#define MIN_COMPASS_DATAWIN_SIZE (DATAWIN_COMPASS_FIELDS + DATAWIN_OVERHEAD)

/* This is the maximum number of satellites gpsd can track. */
#define MAX_POSSIBLE_SATS (MAXCHANNELS - 2)

/* This is the maximum size we need for the 'satellites' window. */
#define MAX_SATWIN_SIZE (MAX_POSSIBLE_SATS + SATWIN_OVERHEAD)

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

#include <curses.h>
#include <signal.h>

#include "gpsd_config.h"
#include "gps.h"

static struct gps_data_t *gpsdata;
static time_t status_timer;    /* Time of last state change. */
static time_t misc_timer;    /* Misc use timer. */
static int state = 0;   /* or MODE_NO_FIX=1, MODE_2D=2, MODE_3D=3 */
static float altfactor = METERS_TO_FEET;
static float speedfactor = MPS_TO_MPH;
static char *altunits = "ft";
static char *speedunits = "mph";

static WINDOW *datawin, *satellites, *messages;

static int title_flag=0;
static int raw_flag=0;
static int silent_flag=0;
static int fixclear_flag=0;
static int compass_flag=0;
static int got_gps_type=0;
static char gps_type[26];
static int window_length;
static int display_sats;

/* Function to call when we're all done.  Does a bit of clean-up. */
static void die(int sig UNUSED) 
{
  /* Ignore signals. */
  (void)signal(SIGINT,SIG_IGN);
  (void)signal(SIGHUP,SIG_IGN);

  /* Move the cursor to the bottom left corner. */
  (void)mvcur(0,COLS-1,LINES-1,0);

  /* Put input attributes back the way they were. */
  (void)echo();

  /* Done with curses. */
  (void)endwin();

  /* We're done talking to gpsd. */
  (void)gps_close(gpsdata);

  /* Bye! */
  exit(0);
}


static enum deg_str_type deg_type = deg_dd;

/* This gets called once for each new sentence until we figure out
   what's going on and switch to either update_gps_panel() or
   update_compass_panel(). */
static void update_probe(struct gps_data_t *gpsdata, 
		       char *message,
		       size_t len UNUSED , 
		       int level UNUSED)
{
  /* Send an 'i' once per second until we figure out what the GPS
     device is. */
  if(time(NULL)-misc_timer > 2) {
    (void)gps_query(gpsdata, "i\n");
    (void)fprintf(stderr,"Probing...\n");
    misc_timer=time(NULL);
  }

  assert(message != NULL);
  if(strncmp(message,"GPSD,I=",6)==0) {
    message+=7;
    (void)strlcpy(gps_type, message, sizeof(gps_type));
    got_gps_type=1;
    /* If we're hooked to a compass, we display an entirely different
       screen and label the data much differently. */
    if(strstr(message,"True North")) {
      compass_flag=1;
    }
  }
}


/* This gets called once for each new compass sentence. */
static void update_compass_panel(struct gps_data_t *gpsdata, 
                         char *message,
                         size_t len UNUSED , 
                         int level UNUSED)
{
  char *s;

  /* Print time/date. */
  (void)wmove(datawin, 1, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.time)==0) {
    char scr[128];
    (void)wprintw(datawin,"%s",unix_to_iso8601(gpsdata->fix.time, scr, (int)sizeof(s)));
  } else
    (void)wprintw(datawin,"n/a                    ");

  /* Fill in the heading. */
  (void)wmove(datawin, 2, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.track)==0) {
    (void)wprintw(datawin,"%.1f     ", gpsdata->fix.track);
  } else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the pitch. */
  (void)wmove(datawin, 3, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.climb)==0) {
    (void)wprintw(datawin,"%.1f     ", gpsdata->fix.climb);
  } else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the roll. */
  (void)wmove(datawin, 4, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.speed)==0)
    (void)wprintw(datawin,"%.1f     ",gpsdata->fix.speed);
  else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the speed. */
  (void)wmove(datawin, 5, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.altitude)==0)
    (void)wprintw(datawin,"%.1f     ", gpsdata->fix.altitude);
  else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in receiver type. */
  (void)wmove(datawin, 6, DATAWIN_VALUE_OFFSET);
  if(got_gps_type==1) {
    (void)wprintw(datawin,"%s",gps_type);
  } else {
    (void)wprintw(datawin,"unknown     ");
  }

  /* Be quiet if the user requests silence. */
  if(silent_flag==0 && raw_flag==1) {
    (void)wprintw(messages, "%s\n", message);
  }

  (void)wrefresh(datawin);
  if(raw_flag==1) {
    (void)wrefresh(messages);
  }
}


/* This gets called once for each new GPS sentence. */
static void update_gps_panel(struct gps_data_t *gpsdata, 
                         char *message,
                         size_t len UNUSED , 
                         int level UNUSED)
{
  int i,n,c;
  int newstate;
  char *s;

  /* This is for the satellite status display.  Originally lifted from
     xgps.c.  Note that the satellite list may be truncated based on
     available screen size, or may only show satellites used for the
     fix.  */
  if (gpsdata->satellites!=0) {
    if (display_sats >= MAX_POSSIBLE_SATS) {
      for (i = 0; i < MAX_POSSIBLE_SATS; i++) {
	(void)wmove(satellites, i+2, 1);
	if (i < gpsdata->satellites) {
	  (void)wprintw(satellites," %3d    %02d    %03d    %02d      %c  ",
			gpsdata->PRN[i],
			gpsdata->elevation[i], gpsdata->azimuth[i],
			gpsdata->ss[i], gpsdata->used[i] ? 'Y' : 'N');
	} else {
	  for(c = 0; c < SATELLITES_WIDTH - 3; c++) {
	    (void)wprintw(satellites," ");
	  }
	}
      }
    } else {
      n=0;
      for (i = 0; i < MAX_POSSIBLE_SATS; i++) {
	if (n < display_sats) {
	  (void)wmove(satellites, n+2, 1);
	  if ((i < gpsdata->satellites) && ((gpsdata->used[i]!=0) || (gpsdata->satellites <= display_sats))) {
	    n++;
	    (void)wprintw(satellites," %3d    %02d    %03d    %02d      %c  ",
			  gpsdata->PRN[i],
			  gpsdata->elevation[i], gpsdata->azimuth[i],
			  gpsdata->ss[i], gpsdata->used[i] ? 'Y' : 'N');
	  }
	}
      }
      
      if(n < display_sats) {
	for(i = n; i <= display_sats; i++) {
	  (void)wmove(satellites, i+2, 1);
	  for(c = 0; c < SATELLITES_WIDTH - 3; c++) {
	    (void)wprintw(satellites," ");
	  }
	}
      }
      
    }
  }
  
  /* Print time/date. */
  (void)wmove(datawin, 1, DATAWIN_VALUE_OFFSET);
  if (isnan(gpsdata->fix.time)==0) {
    char scr[128];
    (void)wprintw(datawin,"%s",unix_to_iso8601(gpsdata->fix.time, scr, (int)sizeof(s)));
  } else
    (void)wprintw(datawin,"n/a                    ");

  /* Fill in the latitude. */
  (void)wmove(datawin, 2, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.latitude)==0) {
    s = deg_to_str(deg_type,  fabs(gpsdata->fix.latitude));
    (void)wprintw(datawin,"%s %c     ", s, (gpsdata->fix.latitude < 0) ? 'S' : 'N');
  } else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the longitude. */
  (void)wmove(datawin, 3, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.longitude)==0) {
    s = deg_to_str(deg_type,  fabs(gpsdata->fix.longitude));
    (void)wprintw(datawin,"%s %c     ", s, (gpsdata->fix.longitude < 0) ? 'W' : 'E');
  } else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the altitude. */
  (void)wmove(datawin, 4, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode == MODE_3D && isnan(gpsdata->fix.altitude)==0)
    (void)wprintw(datawin,"%.1f %s     ",gpsdata->fix.altitude*altfactor, altunits);
  else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the speed. */
  (void)wmove(datawin, 5, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.track)==0)
    (void)wprintw(datawin,"%.1f %s     ", gpsdata->fix.speed*speedfactor, speedunits);
  else
    (void)wprintw(datawin,"n/a         ");

  /* Fill in the heading. */
  (void)wmove(datawin, 6, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.track)==0)
    (void)wprintw(datawin,"%.1f degrees     ", gpsdata->fix.track);
  else
    (void)wprintw(datawin,"n/a          ");

  /* Fill in the rate of climb. */
  (void)wmove(datawin, 7, DATAWIN_VALUE_OFFSET);
  if (gpsdata->fix.mode == MODE_3D && isnan(gpsdata->fix.climb)==0)
    (void)wprintw(datawin,"%.1f %s/min     "
                  , gpsdata->fix.climb * altfactor * 60, altunits);
  else
    (void)wprintw(datawin,"n/a         ");
  
  /* Fill in the GPS status and the time since the last state
     change. */
  (void)wmove(datawin, 8, DATAWIN_VALUE_OFFSET);
  if (gpsdata->online == 0) {
    newstate = 0;
    (void)wprintw(datawin,"OFFLINE          ");
  } else {
    newstate = gpsdata->fix.mode;
    switch (gpsdata->fix.mode) {
    case MODE_2D:
      (void)wprintw(datawin,"2D %sFIX (%d secs)   ",(gpsdata->status==STATUS_DGPS_FIX)?"DIFF ":"", (int) (time(NULL) - status_timer));
      break;
    case MODE_3D:
      (void)wprintw(datawin,"3D %sFIX (%d secs)   ",(gpsdata->status==STATUS_DGPS_FIX)?"DIFF ":"", (int) (time(NULL) - status_timer));
      break;
    default:
      (void)wprintw(datawin,"NO FIX (%d secs)     ", (int) (time(NULL) - status_timer));
      break;
    }
  }

  /* Fill in the receiver type. */
  (void)wmove(datawin, 9, DATAWIN_VALUE_OFFSET);
  if(got_gps_type==1) {
    (void)wprintw(datawin,"%s",gps_type);
  } else {
    (void)wprintw(datawin,"unknown     ");
  }

    /* Note that the following four fields are exceptions to the
       sizing rule.  The minimum window size does not include these
       fields, if the window is too small, they get excluded.  This
       may or may not change if/when the output for these fields is
       fixed and/or people request their permanance.  They're only
       there in the first place because I arbitrarily thought they
       sounded interesting. ;^) */

    if(window_length >= (MIN_GPS_DATAWIN_SIZE + 4)) {

      /* Fill in the estimated horizontal position error. */
      (void)wmove(datawin, 10, DATAWIN_VALUE_OFFSET + 5);
      if (isnan(gpsdata->fix.eph)==0)
	(void)wprintw(datawin,"+/- %d %s     ", (int) (gpsdata->fix.eph * altfactor), altunits);
      else
	(void)wprintw(datawin,"n/a         ");
      
      /* Fill in the estimated vertical position error. */
      (void)wmove(datawin, 11, DATAWIN_VALUE_OFFSET + 5);
      if (isnan(gpsdata->fix.epv)==0)
	(void)wprintw(datawin,"+/- %d %s     ", (int)(gpsdata->fix.epv * altfactor), altunits);
      else
	(void)wprintw(datawin,"n/a         ");
      
      /* Fill in the estimated track error. */
      (void)wmove(datawin, 12, DATAWIN_VALUE_OFFSET + 5);
      if (isnan(gpsdata->fix.epd)==0)
	(void)wprintw(datawin,"+/- %.1f deg     ", (gpsdata->fix.epd));
      else
	(void)wprintw(datawin,"n/a          ");
      
      /* Fill in the estimated speed error. */
      (void)wmove(datawin, 13, DATAWIN_VALUE_OFFSET + 5);
      if (isnan(gpsdata->fix.eps)==0)
	(void)wprintw(datawin,"+/- %d %s     ", (int)(gpsdata->fix.eps * speedfactor), speedunits);
      else
	(void)wprintw(datawin,"n/a            ");
    }

  /* Be quiet if the user requests silence. */
  if(silent_flag==0 && raw_flag==1) {
    (void)wprintw(messages, "%s\n", message);
  }

  /* Reset the status_timer if the state has changed. */
  if (newstate != state) {
    status_timer = time(NULL);
    state = newstate;
  }

  (void)wrefresh(datawin);
  (void)wrefresh(satellites);
  if(raw_flag==1) {
    (void)wrefresh(messages);
  }
}

static void usage( char *prog) 
{
  (void)fprintf(stderr, 
                "Usage: %s [-h] [-V] [-l {d|m|s}] [server[:port:[device]]]\n\n"
                "  -h          Show this help, then exit\n"
                "  -V          Show version, then exit\n"
                "  -s          Be silent (don't print raw gpsd data)\n"
                "  -l {d|m|s}  Select lat/lon format\n"
                "                d = DD.dddddd\n"
                "                m = DD MM.mmmm'\n"
                "                s = DD MM' SS.sss\"\n"
                , prog);

  exit(1);
}

int main(int argc, char *argv[])
{
  int option;
  char *arg = NULL, *colon1, *colon2, *device = NULL, *server = NULL, *port = DEFAULT_GPSD_PORT;
  char *err_str = NULL;
  int c;

  int xsize, ysize;

  struct timeval timeout;
  fd_set rfds;
  int data;

  /* Process the options.  Print help if requested. */
  while ((option = getopt(argc, argv, "hVl:sj")) != -1) {
    switch (option) {
    case 's':
      silent_flag=1;
      break;
    case 'j':
      fixclear_flag=1;
      break;
    case 'V':
      (void)fprintf(stderr, "SVN ID: $Id: cgps.c 4074 2006-12-04 21:55:45Z jfrancis $ \n");
      exit(0);
    case 'l':
      switch ( optarg[0] ) {
      case 'd':
        deg_type = deg_dd;
        continue;
      case 'm':
        deg_type = deg_ddmm;
        continue;
      case 's':
        deg_type = deg_ddmmss;
        continue;
      default:
        (void)fprintf(stderr, "Unknown -l argument: %s\n", optarg);
        /*@ -casebreak @*/
      }
    case 'h': default:
      usage(argv[0]);
      break;
    }
  }

  /* Grok the server, port, and device. */
  /*@ -branchstate @*/
  if (optind < argc) {
    arg = strdup(argv[optind]);
    /*@i@*/colon1 = strchr(arg, ':');
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

  /*@ -observertrans @*/
  switch (gpsd_units())
    {
    case imperial:
      altfactor = METERS_TO_FEET;
      altunits = "ft";
      speedfactor = MPS_TO_MPH;
      speedunits = "mph";
      break;
    case nautical:
      altfactor = METERS_TO_FEET;
      altunits = "ft";
      speedfactor = MPS_TO_KNOTS;
      speedunits = "knots";
      break;
    case metric:
      altfactor = 1;
      altunits = "m";
      speedfactor = MPS_TO_KPH;
      speedunits = "kph";
      break;
    default:
      /* leave the default alone */
      break;
    }
  /*@ +observertrans @*/

  /* Open the stream to gpsd. */
  /*@i@*/gpsdata = gps_open(server, port);
  if (!gpsdata) {
    switch ( errno ) {
    case NL_NOSERVICE:  err_str = "can't get service entry"; break;
    case NL_NOHOST:     err_str = "can't get host entry"; break;
    case NL_NOPROTO:    err_str = "can't get protocol entry"; break;
    case NL_NOSOCK:     err_str = "can't create socket"; break;
    case NL_NOSOCKOPT:  err_str = "error SETSOCKOPT SO_REUSEADDR"; break;
    case NL_NOCONNECT:  err_str = "can't connect to host"; break;
    default:                    err_str = "Unknown"; break;
    }
    (void)fprintf( stderr, 
                   "cgps: no gpsd running or network error: %d, %s\n", 
                   errno, err_str);
    exit(2);
  }

  /* Set both timers to now. */
  status_timer = time(NULL);
  misc_timer = status_timer;

  /* If the user requested a specific device, try to change to it. */
  if (device)
      (void)gps_query(gpsdata, "F=%s\n", device);

  /* Here's where updates go until we figure out what we're dealing
     with. */
  gps_set_raw_hook(gpsdata, update_probe);

  /* Tell me what you are... */
  (void)gps_query(gpsdata, "i\n");

  /* Loop for ten seconds looking for a device.  If none found, give
     up and assume "unknown" device type. */
  while(got_gps_type==0) {

    /* Sleep for one second. */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    (int)select(0,NULL,NULL,NULL,&timeout);

    /* Give up after ten seconds. */
    if(time(NULL)-status_timer >= 10) {
      (void)strlcpy(gps_type, "unknown", sizeof(gps_type));
      got_gps_type=1;
    }
  }

  /* If the user has requested the 'j' option (buffering), make the
     request of gpsd before we continue. */
  if(fixclear_flag==1 && compass_flag==0) {
    (void)gps_query(gpsdata, "j=1\n");
  }

  /* Fire up curses. */
  (void)initscr();
  (void)noecho();
  (void)signal(SIGINT,die);
  (void)signal(SIGHUP,die);

  /* Set the window sizes per the following criteria:

     1.  Set the window size to display the maximum number of
     satellites possible, but not more than the size required to
     display the maximum number of satellites gpsd is capable of
     tracking (MAXCHANNELS - 2).

     2.  If the screen size will not allow for the full complement of
     satellites to be displayed, set the windows sizes smaller, but
     not smaller than the number of lines necessary to display all of
     the fields in the 'datawin'.  The list of displayed satellites
     will be truncated to fit the available window size.  (TODO: If
     the satellite list is truncated, omit the satellites not used to
     obtain the current fix.)

     3.  If the criteria in #2 is one line short of what is necessary
     to display the 'datawin' data, drop the title at the top of the
     screen and move everything up one line, also expanding the
     satellite list by one.

     4.  If the screen is large enough to display all possible
     satellites (MAXCHANNELS - 2) with space still left at the bottom,
     add a window at the bottom in which to scroll raw gpsd data.
  */
  (void)getmaxyx(stdscr,ysize,xsize);

  if(compass_flag==1) {
    if(ysize == MIN_COMPASS_DATAWIN_SIZE) {
      title_flag = 0;
      raw_flag = 0;
      window_length = MIN_COMPASS_DATAWIN_SIZE;
    } else if(ysize == MIN_COMPASS_DATAWIN_SIZE + 1) {
      title_flag = 1;
      raw_flag = 0;
      window_length = MIN_COMPASS_DATAWIN_SIZE;
    } else if(ysize >= MIN_COMPASS_DATAWIN_SIZE + 2) {
      title_flag = 1;
      raw_flag = 1;
      window_length = MIN_COMPASS_DATAWIN_SIZE;
    } else {
      (void)mvprintw(0, 0, "Your screen must be at least 80x%d to run cgps.",MIN_COMPASS_DATAWIN_SIZE);
      /*@ -nullpass @*/
      (void)refresh();
      /*@ +nullpass @*/
      (void)sleep(5);
      die(0);
    }
  } else {
    if(ysize == MAX_SATWIN_SIZE) {
      title_flag = 0;
      raw_flag = 0;
      window_length = MAX_SATWIN_SIZE;
      display_sats = MAX_POSSIBLE_SATS;
    } else if(ysize == MAX_SATWIN_SIZE + 1) {
      title_flag = 0;
      raw_flag = 1;
      window_length = MAX_SATWIN_SIZE;
      display_sats = MAX_POSSIBLE_SATS;
    } else if(ysize > MAX_SATWIN_SIZE + 2) {
      title_flag = 0;
      raw_flag = 1;
      window_length = MAX_SATWIN_SIZE;
      display_sats = MAX_POSSIBLE_SATS;
    } else if(ysize > MIN_GPS_DATAWIN_SIZE) {
      title_flag = 1;
      raw_flag = 0;
      window_length = ysize - title_flag - raw_flag;
      display_sats = window_length - SATWIN_OVERHEAD - title_flag - raw_flag;
    } else if(ysize == MIN_GPS_DATAWIN_SIZE) {
      title_flag = 0;
      raw_flag = 0;
      window_length = MIN_GPS_DATAWIN_SIZE;
      display_sats = window_length - SATWIN_OVERHEAD - 1;
    } else {
      (void)mvprintw(0, 0, "Your screen must be at least 80x%d to run cgps.",MIN_GPS_DATAWIN_SIZE);
      /*@ -nullpass @*/
      (void)refresh();
      /*@ +nullpass @*/
      (void)sleep(5);
      die(0);
    }
  }

  /* Set up the screen for either a compass or a gps receiver. */
  if(compass_flag==1) {
    /* We're a compass, set up accordingly. */

    /*@ -onlytrans @*/
    datawin    = newwin(window_length, DATAWIN_WIDTH, title_flag, 0);
    (void)nodelay(datawin,(bool)TRUE);
    if(raw_flag==1) {
      messages   = newwin(0, 0, title_flag + window_length, 0);

      /*@ +onlytrans @*/
      (void)scrollok(messages, true);
      (void)wsetscrreg(messages, 0, ysize - (window_length + title_flag));
    }

    if(title_flag==1) {
	(void)mvprintw(0, (int)(((DATAWIN_WIDTH + SATELLITES_WIDTH) / 2) - (strlen(TITLE) / 2)), TITLE);
    }

    /*@ -nullpass @*/
    (void)refresh();
    /*@ +nullpass @*/
    
    /* Do the initial field label setup. */
    (void)mvwprintw(datawin, 1, DATAWIN_DESC_OFFSET, "Time:");
    (void)mvwprintw(datawin, 2, DATAWIN_DESC_OFFSET, "Heading:");
    (void)mvwprintw(datawin, 3, DATAWIN_DESC_OFFSET, "Pitch:");
    (void)mvwprintw(datawin, 4, DATAWIN_DESC_OFFSET, "Roll:");
    (void)mvwprintw(datawin, 5, DATAWIN_DESC_OFFSET, "Dip:");
    (void)mvwprintw(datawin, 6, DATAWIN_DESC_OFFSET, "Rcvr Type:");
    (void)wborder(datawin, 0, 0, 0, 0, 0, 0, 0, 0);

  } else {
    /* We're a GPS, set up accordingly. */

    /*@ -onlytrans @*/
    datawin    = newwin(window_length, DATAWIN_WIDTH, title_flag, 0);
    satellites = newwin(window_length, SATELLITES_WIDTH, title_flag, DATAWIN_WIDTH);
    (void)nodelay(datawin,(bool)TRUE);
    if(raw_flag==1) {
      messages   = newwin(ysize - (window_length + title_flag), xsize, title_flag + window_length, 0);

      /*@ +onlytrans @*/
      (void)scrollok(messages, true);
      (void)wsetscrreg(messages, 0, ysize - (window_length + title_flag));
    }

    if(title_flag==1) {
	(void)mvprintw(0, (int)(((DATAWIN_WIDTH + SATELLITES_WIDTH) / 2) - (strlen(TITLE) / 2)), TITLE);
    }
    /*@ -nullpass @*/
    (void)refresh();
    /*@ +nullpass @*/
    
    /* Do the initial field label setup. */
    (void)mvwprintw(datawin, 1, DATAWIN_DESC_OFFSET, "Time:");
    (void)mvwprintw(datawin, 2, DATAWIN_DESC_OFFSET, "Latitude:");
    (void)mvwprintw(datawin, 3, DATAWIN_DESC_OFFSET, "Longitude:");
    (void)mvwprintw(datawin, 4, DATAWIN_DESC_OFFSET, "Altitude:");
    (void)mvwprintw(datawin, 5, DATAWIN_DESC_OFFSET, "Speed:");
    (void)mvwprintw(datawin, 6, DATAWIN_DESC_OFFSET, "Heading:");
    (void)mvwprintw(datawin, 7, DATAWIN_DESC_OFFSET, "Climb:");
    (void)mvwprintw(datawin, 8, DATAWIN_DESC_OFFSET, "Status:");
    (void)mvwprintw(datawin, 9, DATAWIN_DESC_OFFSET, "GPS Type:");

    /* Note that the following four fields are exceptions to the
       sizing rule.  The minimum window size does not include these
       fields, if the window is too small, they get excluded.  This
       may or may not change if/when the output for these fields is
       fixed and/or people request their permanance.  They're only
       there in the first place because I arbitrarily thought they
       sounded interesting. ;^) */

    if(window_length >= (MIN_GPS_DATAWIN_SIZE + 4)) {
      (void)mvwprintw(datawin, 10, DATAWIN_DESC_OFFSET, "Horizontal Err:");
      (void)mvwprintw(datawin, 11, DATAWIN_DESC_OFFSET, "Vertical Err:");
      (void)mvwprintw(datawin, 12, DATAWIN_DESC_OFFSET, "Course Err:");
      (void)mvwprintw(datawin, 13, DATAWIN_DESC_OFFSET, "Speed Err:");
    }

    (void)wborder(datawin, 0, 0, 0, 0, 0, 0, 0, 0);
    (void)mvwprintw(satellites, 1,1, "PRN:   Elev:  Azim:  SNR:  Used:");
    (void)wborder(satellites, 0, 0, 0, 0, 0, 0, 0, 0);
  }

  /* Here's where updates go now that things are established. */
  if(compass_flag==1) {
    gps_set_raw_hook(gpsdata, update_compass_panel);
  } else {
    gps_set_raw_hook(gpsdata, update_gps_panel);
  }

  /* Request "w+x" data from gpsd. */
  (void)gps_query(gpsdata, "w+x\n");

  /* heart of the client */
  for (;;) {
    
    /* watch to see when it has input */
    FD_ZERO(&rfds);
    FD_SET(gpsdata->gps_fd, &rfds);

    /* wait up to five seconds. */
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    /* check if we have new information */
    data = select(gpsdata->gps_fd + 1, &rfds, NULL, NULL, &timeout);
        
    if (data == -1) {
      fprintf( stderr, "cgps: socket error\n");
      exit(2);
    }
    else if( data ) {
      /* code that calls gps_poll(gpsdata) */
      if (gps_poll(gpsdata) != 0)
        die(1);
    }

    /* Check for user input.

     TODO: Restrict the allowed keys based on device type (ie, 'j'
     makes no sense for a True North compass).  No doubt there will be
     other examples as cgps grows more bells and whistles. */
    c=wgetch(datawin);
        
    switch ( c ) {
      /* Quit */
    case 'q':
      die(0);
      break;

      /* Toggle spewage of raw gpsd data. */
    case 's':
      if(silent_flag==0) {
        silent_flag=1;
      } else {
        silent_flag=0;
      }
      break;

      /* Toggle fix clear. */
    case 'j':
      if(fixclear_flag==0) {
        fixclear_flag=1;
        (void)gps_query(gpsdata, "j=1\n");
      } else {
        fixclear_flag=0;
        (void)gps_query(gpsdata, "j=0\n");
      }
      break;

      /* Clear the spewage area. */
    case 'c':
      (void)werase(messages);
      break;

    default:
      break;
    }

  }
 
}
