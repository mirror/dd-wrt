/* 
 * gpiowatcher.c - a program that is allowed to have help and debug
 *
 * Copyright (C) NewMedia-NET GmbH Christian Scheele <chris@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>
#include <stdarg.h>
#include <dirent.h>
#include <syslog.h>

#define NORMAL_INTERVAL		1	/* second */
#define URGENT_INTERVAL		100 * 1000	/* microsecond */

static int debug = 0;
static int count = 0;
static int oldstate = 0;
static int firstrun = 1;
static int gpio = -1;
static int interval = 0;
static int use_interval = 0;
static int exit_only = 0;
static int use_exit_only = 0;
static int use_syslog = 0;
char *syslog_text = "GPIOWATCHER";


static void alarmtimer(unsigned long sec, unsigned long usec)
{
	struct itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;

	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

void check_exit(int val)
	{
			if (!use_exit_only)
				{
   				if (debug) 
  					fprintf (stderr,"Gpio %d changed from %d to  %d\n", gpio, oldstate, val);
				if (use_syslog)
					dd_syslog(LOG_INFO,
					  "%s gpio %d changed from %d to %d\n",syslog_text,gpio,oldstate,val);
				fprintf(stdout, "%d",val);
				exit(val);
				}
			else
				{
				if ( val == exit_only )
					{
   					if (debug) 
  						fprintf (stderr,"Gpio %d changed from %d to %d (exit_only)\n", gpio, oldstate, val);
					if (use_syslog)
						dd_syslog(LOG_INFO,
						  "%s gpio %d changed from %d to %d\n",syslog_text,gpio,oldstate,val);
					fprintf(stdout, "%d",val); 
					exit(val);
					}
				else
					{
   					if (debug) 
  						fprintf (stderr,"Gpio %d changed from %d to %d, but no exit cause exit_only does not match\n", gpio, oldstate, val);
					oldstate=val;
					}
				}

	}

void period_check(int sig)
{
	unsigned int val = 0;
	if (firstrun)
			{
			oldstate=get_gpio(gpio);
			firstrun=0;
					if (debug) 
  				fprintf (stderr,"Firstrun: actual-state of gpio %d is %d\n", gpio, oldstate);
			}
	val = get_gpio(gpio);
	if (val != oldstate )
		{
		if (use_interval)
			{
			alarmtimer(0, URGENT_INTERVAL);
			if (++count > (interval * 10 )) 
					{ 
   					if (debug) 
  						fprintf (stderr,"Gpio %d changed from %d to %d (%d seconds)                            \n", gpio, oldstate, val,interval);
					check_exit(val);
					}
			else
					{
					if (use_exit_only &&  val != exit_only )
						{
   						if (debug) 
  							fprintf (stderr,"Gpio %d changed from %d to %d (%d seconds)\r", gpio, oldstate, val,interval-count/10);
						}
					else
						{
   						if (debug) 
  							fprintf (stderr,"Gpio %d changed from %d to %d (exit in %d seconds)\r", gpio, oldstate, val,interval-count/10);
						}
					}
			}
		else
			{
			check_exit(val);
			}
		}
	else
		{
		if (use_interval && count)
			{
			if (use_exit_only &&  val != exit_only )
				{
   				if (debug) 
					fprintf (stderr,"now waiting for exit_only value change\n");
				}
			else
				{
   				if (debug) 
					fprintf (stderr,"Gpio %d fall back to oldstate %d  before interval ended\n", gpio, oldstate);
				}
			alarmtimer(NORMAL_INTERVAL, 0);
			count=0;
			}
		}
}

void usage(void) {
    fprintf(stderr,"\nUsage: gpiowatcher [-d] [-s] [-t <syslog_text>] [-i interval] [-o exit_only_on_value] -g gpio  \n\nuse -d for debug\n    -s to log changes syslog\n    -t <text> to change syslog loging text (default GPIOWATCHER) \n\n exit-value is the new gpio state, that is also printed\n");
    exit(1);
	}


int main(int argc, char *argv[])
{

	int c;
	while ((c = getopt (argc, argv, "dst:g:i:o:")) != -1)
         switch (c)
           {
           case 'd':
             debug = 1;
             break;
           case 'g':
			 gpio = atoi(optarg);
             break;
           case 'i':
			 interval = atoi(optarg);
			 use_interval=1;
             break;
           case 'o':
			 exit_only = atoi(optarg);
			 use_exit_only=1;
             break;
           case 't':
			 syslog_text = optarg;
             break;
           case 's':
			 use_syslog=1;
             break;
           default:
		   	usage();
             abort ();
           }
   if (gpio == -1)
		{
  		fprintf (stderr,"Option -g is necessary, no gpio no action\n");
		usage();
		exit (-1);
		}

   if (debug) 
  	fprintf (stderr,"g = %d, i = %d, o= %d\n", gpio, interval,exit_only);

#if 0
	switch (fork()) {
	case -1:
		DEBUG("can't fork\n");
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		DEBUG("fork ok\n");
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
#endif

	/* 
	 * set the signal handler 
	 */
	signal(SIGALRM, period_check);

	/* 
	 * set timer 
	 */
	alarmtimer(NORMAL_INTERVAL, 0);

	/* 
	 * Most of time it goes to sleep 
	 */
	while (1)
		pause();

	return 0;
}
