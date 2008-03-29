/* ttraff.c by Eko: 12.feb.2008
  
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



void 
write_to_nvram (int day, int month, int year, unsigned long rcvd, unsigned long sent)
{
char *next;
char var[80];
char tq[32];
char temp[64] = "";
char sbuff[256] = "";
char buffer[2048] = "";
int i = 1, d = 1;
unsigned int days = daysformonth (month, year);
unsigned long old_rcvd;
unsigned long old_sent;
char *tdata;

  sprintf (tq, "traff-%02u-%u", month, year);
  tdata = nvram_safe_get (tq);
  
  if (tdata == NULL || strlen (tdata) == 0)
  {
   for (d = 0; d < days; d++)
   {
    strcat (sbuff, "0:0 ");
   }
   nvram_set (tq, sbuff);
   tdata = nvram_safe_get (tq);
  }

    foreach (var, tdata, next)
    {
     if (i == day)
     {
	  sscanf (var, "%lu:%lu", &old_rcvd, &old_sent);
	  sprintf (temp, "%lu:%lu ", old_rcvd + rcvd, old_sent + sent);	    
	  strcat (buffer, temp);
     }
     else
     {
	  strcat (buffer, var);
	  strcat (buffer, " ");
      }    
	 i++;
    }	  

    nvram_set (tq, buffer);

  return;
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
   sleep (15);
   time (&tloc);
   currtime = localtime (&tloc);
  }
  
/* now we have time, let's start */ 
  static char wanface[32];
  char line[256];
  unsigned long in_dev = 0;
  unsigned long out_dev = 0;
  unsigned long in_diff = 0;
  unsigned long out_diff = 0;  
  unsigned long in_dev_last = 0;
  unsigned long out_dev_last = 0;
  int gotbase = 0;
  unsigned long gigcounti, gigcounto;
  int gigi = 0;
  int gigo = 0;
  int needcommit = 0;
  int commited = 0;
  int day, month, year;
  FILE *in;
  
  strncpy (wanface, get_wan_face (), sizeof (wanface));


/* now we can loop and collect data */

  syslog (LOG_DEBUG, "ttraff: data collection started\n");

  do
  {
   time (&tloc);
   currtime = localtime (&tloc);
   
   day = currtime->tm_mday;
   month = currtime->tm_mon + 1; // 1 - 12
   year = currtime->tm_year + 1900;
   
  
    if ((in = fopen ("/proc/net/dev", "rb")) != NULL)
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
    
   if (gotbase == 0)
   {
	 in_dev_last = in_dev;
	 out_dev_last = out_dev;
	 gotbase = 1;
   }	    
    
   if (in_dev_last > in_dev)  //4GB limit was reached
   {
	 gigi = 4;
	 in_diff = (4294967295 - in_dev_last + in_dev) >> 20;
	 in_dev_last = in_dev;  //we loose < 1 MB here, but we don't care	 
   }
   else
   {
	 in_diff = (in_dev - in_dev_last) >> 20;  //MB
	 in_dev_last += (in_diff << 20);
   }	   

   
   if (out_dev_last > out_dev)  //4GB limit was reached
   {
	 gigo = 4;
	 out_diff = (4294967295 - out_dev_last + out_dev) >> 20;
	 out_dev_last = out_dev;  //we loose < 1 MB here, but we don't care
   }
   else
   {
	 out_diff = (out_dev - out_dev_last) >> 20;  //MB
	 out_dev_last += (out_diff << 20);
   }
	 
   
//fprintf (stderr, "in_diff=%lu, out_diff=%lu\n", in_diff, out_diff);
  
   if (in_diff || out_diff)
   {
    write_to_nvram (day, month, year, in_diff, out_diff);
   }

   if (gigi || gigo)  // leave trace in /tmp/.gigc
   {
	 gigcounti = 0;
	 gigcounto = 0;
	 if ((in = fopen ("/tmp/.gigc", "r")) != NULL)
	 {
	  fgets (line, sizeof (line), in);
      sscanf (line, "%lu:%lu", &gigcounti, &gigcounto);
      fclose (in);
     }
     in = fopen ("/tmp/.gigc", "w"); 
	 sprintf (line, "%lu:%lu", gigcounti + gigi, gigcounto + gigo);	    
	 fputs (line, in);
	 fclose (in);
	 gigi = 0;
	 gigo = 0;
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
     syslog (LOG_DEBUG, "ttraff: data for %d-%d-%d commited to nvram\n", day, month, year);
   }
   
   sleep (58);
   
  }
  while (1);

  return 0;

}
