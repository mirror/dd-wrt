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
static int no_exit = 0;
static int invert = 0;
static int interval = 0;
static int use_interval = 0;
static int use_wait = 0;
static int exit_only = 0;
static int use_exit_only = 0;
static int use_syslog = 0;
char *syslog_text = "GPIOWATCHER";
static int use_call = 0;
char *call = NULL;


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
				if (use_wait) 
					val=1-val;
   				if (debug) 
  					fprintf (stderr,"Gpio %d changed from %d to  %d\n", gpio, oldstate, val);
				if (use_syslog)
					{
					if (use_wait)
						{
						dd_syslog(LOG_INFO,
					  		"%s gpio %d changed from %d to %d for %d seconds\n",syslog_text,gpio,oldstate,val,count/10);
						}
					else
						{
						dd_syslog(LOG_INFO,
					  		"%s gpio %d changed from %d to %d\n",syslog_text,gpio,oldstate,val);
						}
					}
				fprintf(stdout, "%d",val);
				if (use_wait) 
					{
						fprintf(stdout, " %d",count/10);
					}
				if (use_call)
					{
					char call_script[1024];
					if (invert)
						sprintf(call_script,"%s %d %d %d",call,gpio,val,count/10);
					else
						sprintf(call_script,"%s %d %d %d",call,gpio,1-val,count/10);
   					if (debug) 
  						fprintf (stderr,"CALL SCRIPT: %s\n", call_script);

					system(call_script);
					if (use_syslog)
						dd_syslog(LOG_INFO,
					  		"%s CALL SCRIPT: %s\n",syslog_text,call_script);
					}
				if (!no_exit) 
					exit(val);
				else
					fprintf(stdout, "\n");
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
					if (!no_exit) 
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
		if (use_wait)
			{
			alarmtimer(0, URGENT_INTERVAL);
   			if (debug) 
  					fprintf (stderr,"Gpio %d changed from %d to %d (%d seconds)\r", gpio, oldstate, val,count/10);
			++count;
			}
		else if (use_interval)
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
		if (use_wait && count) 
			{
   			if (debug) 
  					fprintf (stderr,"Gpio %d released to %d was %d seconds on %d                     \n", gpio, oldstate,count/10,1-oldstate);
			check_exit(val);
			// no exit might be set
			count=0;
			alarmtimer(NORMAL_INTERVAL, 0);
			}
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
			count=0;
			alarmtimer(NORMAL_INTERVAL, 0);
			}
		}
}

void usage(void) {
    fprintf(stderr,"\nUsage: gpiowatcher [-h] [-d] [-s] [-t <syslog_text>] -[-I] [-i interval] [-w] [-o exit_only_on_value] [-c <script>] -g gpio  \n\n-h this ugly text \nuse -d for debug\n   -n will not exit, just report on stdout/syslog\n    -w reports how long the status change (as a second value on stdout)\n    -c <script> call a script with gpio value time\n   -I invert values (right now for script calls only!)\n    -s log to syslog\n    -t <text> to change syslog loging text (default GPIOWATCHER) \n\n exit-value is the new gpio state, that is also printed\n\nEverybody who does not like this help -> make it nicer and send it to me\n");
    exit(1);
	}


int main(int argc, char *argv[])
{

	int c;
	while ((c = getopt (argc, argv, "hIdnswt:g:i:o:c:")) != -1)
         switch (c)
           {
           case 'h':
             usage();
			exit (-1);
             break;
           case 'd':
             debug = 1;
             break;
           case 'n':
			 no_exit = 1;
             break;
           case 'g':
			 gpio = atoi(optarg);
             break;
           case 'I':
			 invert=1;
             break;
           case 'i':
			 interval = atoi(optarg);
			 use_interval=1;
             break;
           case 'w':
			 use_wait=1;
             break;
           case 'o':
			 exit_only = atoi(optarg);
			 use_exit_only=1;
             break;
           case 't':
			 syslog_text = optarg;
             break;
           case 'c':
		   	 if (strlen(optarg) > 1000 )
				{
  				fprintf (stderr,"1000 Bytes for script including path should do the job. nenenene\n");
				usage();
				exit (-1);
				}
			 call = optarg;
			 use_call=1;
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

	if (use_interval && use_wait)
			{
  			fprintf (stderr,"Option -i and -w does not really make sense\n");
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
