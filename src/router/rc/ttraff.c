/* ttraff.c by Eko: 11.feb.2008
  
  used for collecting and storing WAN traffic info to nvram
  
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>
#include <syslog.h>
#include <utils.h>
#include <wlutils.h>


unsigned long get_todays_rcvd (int day, int month, int year)
{
//fprintf (stderr, "entering get_todays_rcvd\n");
char *next;
char var[80];
char tq[32];
int i = 1;
unsigned long rcvd = 0;

  sprintf (tq, "traff-%02u-%u", month, year);
  char *tdata = nvram_safe_get (tq);
  if (tdata != NULL || strlen(tdata))
   {
    foreach (var, tdata, next)
    {
     if (i == day)
	    sscanf (var, "%lu:%*lu", &rcvd);
	 i++;
    }
   }
//fprintf (stderr, "leaving get_todays_rcvd: rcvd=%lu\n", rcvd);
  return rcvd;
}

unsigned long get_todays_sent (int day, int month, int year)
{
//fprintf (stderr, "entering get_todays_sent\n");
char *next;
char var[80];
char tq[32];
int i = 1;
unsigned long sent = 0;

  sprintf (tq, "traff-%02u-%u", month, year);
  char *tdata = nvram_safe_get (tq);
  if (tdata != NULL || strlen(tdata))
   {
    foreach (var, tdata, next)
    {
     if (i == day)
	    sscanf (var, "%*lu:%lu", &sent);
	 i++;
    }
   }
//fprintf (stderr, "leaving get_todays_sent: sent=%lu\n", sent);
  return sent;
}

int write_to_nvram (int day, int month, int year, unsigned long rcvd, unsigned long sent)
{
//fprintf (stderr, "entering write_to_nvram\n");
char *next;
char var[80];
char tq[32];
char temp[64] = "";
char buffer[2048] = "";
int i;
int days = daysformonth (month, year);

  sprintf (tq, "traff-%02u-%u", month, year);

  for (i = 1; i <= days; i++)
  {
   if (i == day)	  
   { 
	sprintf (temp, "%lu:%lu", rcvd, sent);
   }
   else
   {
	sprintf (temp, "%lu:%lu", get_todays_rcvd (i, month, year) , get_todays_sent (i, month, year));
   }
   strcat (buffer, temp);
   if (i < days) strcat (buffer, " ");
  }
  
  nvram_set (tq, buffer);
//fprintf (stderr, "leaving write_to_nvram\n");
  return 1;
}

int
ttraff_main (void)
{

  struct tm *currtime;
  long tloc;

  time (&tloc);		// get time in seconds since epoch
  currtime = localtime (&tloc);	// convert seconds to date structure 
  
  while (currtime->tm_year < 100) //loop until ntp time is set (year >= 2000)
  {
   sleep (60);
   time (&tloc);
   currtime = localtime (&tloc);
  }
  
/* now we have time, let's start */ 
  static char wanface[32];
  char line[256];
  unsigned long trin = 0;
  unsigned long trout = 0;
  unsigned long in_dev = 0;
  unsigned long out_dev = 0;
  unsigned long in_diff = 0;
  unsigned long out_diff = 0;  
  unsigned long in_dev_last = 0;
  unsigned long out_dev_last = 0;
  int needcommit = 0;
  int commited = 0;
  int needbase = 1;
  int day, month, year;
  
  strncpy (wanface, get_wan_face (), sizeof (wanface));


/* now we can loop and collect data */

  do
  {
   time (&tloc);
   currtime = localtime (&tloc);
   
   day = currtime->tm_mday;
   month = currtime->tm_mon + 1; // 1 - 12
   year = currtime->tm_year + 1900;
   
  
   FILE *in = fopen ("/proc/net/dev", "rb");

    if (in != NULL)
    {
     while (fgets (line, sizeof (line), in) != NULL)
     {
      int ifl = 0;
      if (!strchr (line, ':'))
       continue;
      while (line[ifl] != ':')
       ifl++;
      line[ifl] = 0;
      if (strstr (line, wanface))
      { 
	  sscanf (line + ifl + 1,
		      "%lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld %lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld",
		      &in_dev,  &out_dev);
      }
     }
    
  fclose (in);
    }
    
   if (needbase)
   { 
    in_dev_last = in_dev;
    out_dev_last = out_dev;
    needbase = 0;
    sleep (2);
    continue;
   }
   
   if (in_dev_last > in_dev || out_dev_last > out_dev)  // forget this data and get new base
   {
	 needbase = 1;
	 sleep (2);
	 continue;
   }
   
	   
   in_diff = (in_dev - in_dev_last) / (1024 * 1024);  //MBytes
   out_diff = (out_dev - out_dev_last) / (1024 * 1024);  //MBytes
   
//fprintf (stderr, "in_diff=%lu, out_diff=%lu\n", in_diff, out_diff);
  
   if (in_diff || out_diff)
   { 
    write_to_nvram (day, month, year, get_todays_rcvd (day, month, year) + in_diff, get_todays_sent (day, month, year) + out_diff);
    in_dev_last = in_dev_last + in_diff * 1024 * 1024;
    out_dev_last = out_dev_last + out_diff * 1024 * 1024;    
   }
   
   if (currtime->tm_hour == 23 && currtime->tm_min == 59 && commited == 0)
   {
    needcommit = 1;
   }
   else
   {
	commited = 0;
   }   
      
   if (needcommit)  //commit only 1 time per day (at 23:59)
   {
     nvram_commit();
     commited = 1;
     needcommit = 0;
   }
   
   sleep (58);
   
  }
  while (1);

  return 0;

}
