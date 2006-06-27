/*
 * Reset button 
 *
 * Copyright 2002, Cybertan Corporation
 * All Rights Reserved.
 *
 * Description:
 *   This program checks the Reset-Button status periodically.
 *   There is two senaria, whenever the button is pushed
 *     o  Less than 3 seconds : reboot.
 *     o  Greater than 3 seconds : factory default restore, and reboot. 
 *
 *   The reset-button is connected to the GPIO pin, it has character
 *   device driver to manage it by file operation read/write.
 *
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cybertan Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Cybertan Corporation.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>
#include <stdarg.h>

#define BCM47XX_SOFTWARE_RESET  0x40	/* GPIO 6 */
#define BCM47XX_SW_PUSH         0x10	/* GPIO 4 */

#define WHR_SOFTWARE_RESET 0x10	//GPIO 4  , should work with Buffalo WBR-G54 too
#define WHR_SW_PUSH 0		//GPIO 0, code unknown

#define WBR2_SOFTWARE_RESET 0x80	//GPIO 7
#define WBR2_SW_PUSH 0		//GPIO 0, code unknown

#define	SES_LED_CHECK_TIMES	"9999"	/* How many times to check? */
#define	SES_LED_CHECK_INTERVAL	"1"	/* Wait interval seconds */
#define RESET_WAIT		3	/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10	/* 10 times a second */

#define NORMAL_INTERVAL		1	/* second */
#define URGENT_INTERVAL		100 * 1000	/* microsecond */
						/* 1/10 second */
#define GPIO_FILE		"/dev/gpio/in"

#if 0
#define DEBUG printf
#else
#define DEBUG(format, args...)
#endif


static int mode = 0;		/* mode 1 : pushed */
static int ses_mode = 0;	/* mode 1 : pushed */
static int count = 0;

static int brand;

static void
alarmtimer (unsigned long sec, unsigned long usec)
{
  struct itimerval itv;

  itv.it_value.tv_sec = sec;
  itv.it_value.tv_usec = usec;

  itv.it_interval = itv.it_value;
  setitimer (ITIMER_REAL, &itv, NULL);
}

/*
void system_reboot(void)
{
	DEBUG("resetbutton: reboot\n");
	alarmtimer(0, 0);
	eval("reboot");
}
*/

void
service_restart (void)
{
  DEBUG ("resetbutton: restart\n");
  /* Stop the timer alarm */
  alarmtimer (0, 0);
  /* Reset the Diagnostic LED */
  diag_led (DIAG, START_LED);	/* call from service.c */
  /* Restart all of services */
  eval ("rc", "restart");
}

int
check_ses_led_main (int argc, char **argv)
{
  int i;
  int times;
  int interval;

  times = atoi (argv[1]);
  interval = atoi (argv[2]);

  for (i = 0; i < times; i++)
    {

      /* White led */
      diag_led (SES_LED1, START_LED);
      usleep (1000000 * interval);
      diag_led (SES_LED1, STOP_LED);

      /* Orange led */
      diag_led (SES_LED2, START_LED);
      usleep (1000000 * interval);
      diag_led (SES_LED2, STOP_LED);
    }
  return 0;
}

void
period_check (int sig)
{
  FILE *fp;
  unsigned int val = 0;
//      time_t t;

//      time(&t);
//      DEBUG("resetbutton: now time=%d\n", t);

  if ((fp = fopen (GPIO_FILE, "r")))
    {
      fread (&val, 4, 1, fp);
      fclose (fp);
    }
  else
    perror (GPIO_FILE);

  DEBUG ("resetbutton: GPIO = 0x%x\n", val);
  int gpio = 0;
  int state = 0;
  switch (brand)
    {
    case ROUTER_BUFFALO_WHRG54S:
    case ROUTER_BUFFALO_WBR54G:
      gpio = WHR_SOFTWARE_RESET;
      state = (val & gpio);
      break;
    case ROUTER_BUFFALO_WBR2G54S:
      gpio = WBR2_SOFTWARE_RESET;
      state = (val & gpio);
      break;
    case ROUTER_ASUS:
      gpio = BCM47XX_SOFTWARE_RESET;
      state = (val & gpio);
      break;
    default:
      gpio = BCM47XX_SOFTWARE_RESET;
      state = !(val & gpio);
      break;
    }
  /*  The value is zero during button-pushed. */
  if (state)
    {
      DEBUG ("resetbutton: mode=%d, count=%d\n", mode, count);

      if (mode == 0)
	{
	  /* We detect button pushed first time */
	  alarmtimer (0, URGENT_INTERVAL);
	  mode = 1;
	}
      else
	{			/* Whenever it is pushed steady */
	  if (++count > RESET_WAIT_COUNT)
	    {
	      if (check_action () != ACT_IDLE)
		{		// Don't execute during upgrading
		  fprintf (stderr, "resetbutton: nothing to do...\n");
		  alarmtimer (0, 0);	/* Stop the timer alarm */
		  return;
		}
	      if ((brand == ROUTER_WRT54G) || 
	      	(brand == ROUTER_WRT54G1X) ||
		  	(brand == ROUTER_LINKSYS_WRT55AG) ||
		  	(brand == ROUTER_BUFFALO_WHRG54S) ||
		  	(brand == ROUTER_BUFFALO_WBR54G) ||
		  	(brand == ROUTER_BUFFALO_WBR2G54S))
		{
		  printf ("resetbutton: factory default.\n");
		  if (brand == ROUTER_BUFFALO_WBR54G)
		    {
		      eval ("gpio", "disable", "7");	//turn on DIAG led on WBR-G54/WLA-G54
		    }
		  if (brand == ROUTER_BUFFALO_WBR2G54S)
		    {
		      eval ("gpio", "enable", "1");	//turn on DIAG led on WBR2-G54
		    }
		  ACTION ("ACT_HW_RESTORE");
		  alarmtimer (0, 0);	/* Stop the timer alarm */
		  nvram_set ("sv_restore_defaults", "1");
		  nvram_commit ();
		  kill (1, SIGTERM);
		}
	    }
	}
    }
  else if (!(val & BCM47XX_SW_PUSH) && brand==ROUTER_WRT54G)
    {
      if (ses_mode == 1)
	{
	  //enable orange led
	  eval ("gpio", "enable", "2");
	  eval ("gpio", "disable", "3");
	  ses_mode = 2;
	}
      else if (ses_mode == 0)
	{
	  //enable white led
	  eval ("gpio", "enable", "3");
	  eval ("gpio", "disable", "2");
	  ses_mode = 1;
	}
      else if (ses_mode == 2)
	{
	  eval ("gpio", "disable", "3");
	  eval ("gpio", "disable", "2");
	  ses_mode = 3;
	}
      else if (ses_mode == 3)
	{
	  eval ("gpio", "enable", "3");
	  eval ("gpio", "enable", "2");
	  ses_mode = 0;
	}
      /*
         char *led_argv[] = { "check_ses_led",
         SES_LED_CHECK_TIMES,
         SES_LED_CHECK_INTERVAL,
         NULL
         };
         pid_t pid;

         if(!is_exist("/tmp/EnablePushButton"))       
         return;

         ses_mode = 1;
         eval("killall", "check_ses_led");
         _eval(led_argv, NULL, 0, &pid); */

    }
  else
    {

      /* Although it's unpushed now, it had ever been pushed */
      if (mode == 1)
	{
	  if (check_action () != ACT_IDLE)
	    {			// Don't execute during upgrading
	      fprintf (stderr, "resetbutton: nothing to do...\n");
	      alarmtimer (0, 0);	/* Stop the timer alarm */
	      return;
	    }
	  service_restart ();
	}
      /*
         if( ses_mode == 1 ){
         cprintf("Release SES push button\n");
         eval("sendudp", "-i", nvram_safe_get("lan_ifname"),
         "-s", nvram_safe_get("lan_ipaddr"),
         "-d", nvram_safe_get("http_client_ip"),
         "-m", nvram_safe_get("lan_hwaddr"),
         "-p", "9999",
         "LED TEST FINISH");
         ses_mode = 0;
         } */
    }
}


int
resetbutton_main (int argc, char *argv[])
{

  brand = getRouterBrand ();

  	if ((brand == ROUTER_SIEMENS) || 
  		(brand == ROUTER_BELKIN) || 
  		(brand == ROUTER_RT210W) ||
  		(brand == ROUTER_MOTOROLA) || 
  		(brand == ROUTER_BUFFALO_WZRRSG54) ||
  		(brand == ROUTER_BELKIN_F5D7230) ||
  		(brand == ROUTER_MICROSOFT_MN700) ||
  		(brand == ROUTER_BUFFALO_WLAG54C) ||
  		(brand == ROUTER_ASUS_WL500G_PRE))
    {
      puts ("sorry, your unit does not support resetbutton feature");
      nvram_set ("resetbutton_enable", "0");
      return 0;
    }

  /* Run it under background */
  switch (fork ())
    {
    case -1:
      DEBUG ("can't fork\n");
      exit (0);
      break;
    case 0:
      /* child process */
      DEBUG ("fork ok\n");
      (void) setsid ();
      break;
    default:
      /* parent process should just die */
      _exit (0);
    }

  /* set the signal handler */
  signal (SIGALRM, period_check);

  /* set timer */
  alarmtimer (NORMAL_INTERVAL, 0);

  /* Most of time it goes to sleep */
  while (1)
    pause ();

  return 0;
}
