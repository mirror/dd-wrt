
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>

#include <ntp.h>
#include <cy_conf.h>
#include <utils.h>

#define NTP_M_TIMER "3600"
#define NTP_N_TIMER "30"

extern void timer_cancel (timer_t timerid);
int
isRunning (char *name)
{
  return eval ("pidof", name) == 0 ? 1 : 0;
}

void
check_udhcpd (timer_t t, int arg)
{
  if (nvram_invmatch ("router_disable", "1")
      || nvram_match ("lan_proto", "dhcp"))
    {
      if (nvram_match ("dhcp_dnsmasq", "1"))
	{
	  if (!isRunning ("dnsmasq"))
	    {
	      //killps("dnsmasq","-9");
	      //killps("udhcpd","-9");
	      killall ("dnsmasq", SIGKILL);
	      killall ("udhcpd", SIGKILL);
	      sleep (1);
	      start_service ("udhcpd");
	      sleep (1);
	      start_service ("dnsmasq");
	    }
	}
      else
	{
	  if (!isRunning ("udhcpd"))
	    {
	      killall ("dnsmasq", SIGKILL);
	      killall ("udhcpd", SIGKILL);
	      sleep (1);
	      start_service ("udhcpd");
	      sleep (1);
	      start_service ("dnsmasq");
	    }
	}
    }
}

//<<tofu
int
do_ntp (void)			// called from ntp_main and process_monitor_main; called every hour!
{
  struct timeval tv;
  float fofs;
  int dst, i;
  char *servers;

  if (!nvram_match ("ntp_enable", "1"))
    return 0;

  if (sscanf (nvram_safe_get ("time_zone"), "%f %*d %d", &fofs, &dst) != 2)
    {
      fprintf (stderr, "invalid timezone\n");
      return 1;			// OFS[.5] UNK DSTIDX
    }
  if (((i = atoi (nvram_safe_get ("dstcode"))) > 0) && (i <= 5))
    dst = i;
  if (!nvram_match ("daylight_time", "1"))
    dst = 0;

  if (((servers = nvram_get ("ntp_server")) == NULL) || (*servers == 0))
    servers = "209.81.9.7 207.46.130.100 192.36.144.23 pool.ntp.org";

  char *argv[] = { "ntpclient", servers, NULL };
  if (_eval (argv, NULL, 20, NULL) != 0)
    {
//      fprintf (stderr, "ntp returned a error\n");
      return 1;
    }

  // -- probably should move to ntpclient

  gettimeofday (&tv, NULL);
  tv.tv_sec += (int) (fofs * 3600);	// <-- cast it or this will be off (?)

  if ((dst >= 1) && (dst <= 5))
    {
      struct tm *tm = localtime (&tv.tv_sec);
      int mon = tm->tm_mon + 1;
      int day = tm->tm_mday;
      int yi = tm->tm_year + 1900 - 2006;	// dst table starts at 2006
      int mbeg = dstEntry[dst].startMonth;
      int mend = dstEntry[dst].endMonth;
      int dbeg = dstEntry[dst].startDay[yi];
      int dend = dstEntry[dst].endDay[yi];

      if (((mon == mbeg) && (day >= dbeg)) ||
	  ((mon == mend) && (day <= dend)) ||
	  ((mbeg < mend) && (mon > mbeg) && (mon < mend)) ||
	  ((mbeg > mend) && ((mon > mbeg) || (mon < mend))))
	{

	  // ...in dst...

	  // if this is the end day, don't undo dst until we're past 1am to avoid the day from going back to yesterday
	  if ((mon != mend) || (day != dend) || (tm->tm_hour <= 1))
	    tv.tv_sec += dstEntry[dst].dstBias;
	}
      settimeofday (&tv, NULL);
#ifdef HAVE_GATEWORX
      eval ("hwclock", "-w");
#endif
/*              time_t now = time(0);
 *             syslog(LOG_INFO, "time updated: %s\n", ctime(&now));
 */
    }

  return 0;
}

/*
	int mon;
	int day;
	int dst = 4;
	for (mon = 1; mon <= 12; ++mon) {
		printf("[%02d] ", mon);
		for (day = 1; day <= 31; ++day) {
			int yi = 2005 - 2006; // dst table starts at 2006
			int mbeg = dstEntry[dst].startMonth;
			int mend = dstEntry[dst].endMonth;
			int dbeg = dstEntry[dst].startDay[yi];
			int dend = dstEntry[dst].endDay[yi];

			if (((mon == mbeg) && (day >= dbeg)) ||
				((mon == mend) && (day <= dend)) ||
				((mbeg < mend) && (mon > mbeg) && (mon < mend)) ||
				((mbeg > mend) && ((mon > mbeg) || (mon < mend)))) {
				printf("%d,", day);
				if ((mon == mend) && (day == dend)) {
					printf("***");
				}
			}
		}
		printf("\n");
	}
*/

void
ntp_main (timer_t t, int arg)
{
  if (check_action () != ACT_IDLE)
    return;			// don't execute while upgrading
  if (!check_wan_link (0) && nvram_invmatch ("wan_proto", "disabled"))
    return;			// don't execute if not online
/* #ifdef HAVE_SNMP
  struct timeval now;
  gettimeofday (&now, NULL);
#endif */
//              syslog(LOG_INFO, "time updated: %s\n", ctime(&now));
  stop_service ("ntp");
  if (do_ntp () == 0)
    {
      if (arg == FIRST)
	timer_cancel (t);
      eval ("filtersync");
      nvram_set ("timer_interval", NTP_M_TIMER);	// are these used??
    }
  else
    {
      nvram_set ("timer_interval", NTP_N_TIMER);
    }
/* #ifdef HAVE_SNMP
  struct timeval then;
  gettimeofday (&then, NULL);

  if (abs (now.tv_sec - then.tv_sec) > 100000000)
    {
      startstop("snmp");
    }
#endif */
}

//tofu>>


#if 0
				// ================ old ================
/* for NTP */
int
do_ntp (void)
{
  char default_servers[] = "209.81.9.7 207.46.130.100 192.36.144.23";
  char servers[100];

  char buf[20], buf2[4], buf4[20];
  int TimeZone;
  struct timeval tv;
  struct timezone tz;
  struct tm tm;
  int i, j, ret;
  char startMonth;
  char endMonth;
  char diffMonth;
  float time_zone;

  if (!nvram_match ("ntp_enable", "1"))
    {
      cprintf ("Disable NTP Client");
      return 1;
    }

  if (!check_wan_link (0) && nvram_invmatch ("wan_proto", "disabled"))
    {
      cprintf ("Don't exec ntp\n");
      return 1;
    }

  if (nvram_match ("ntp_mode", "manual") && nvram_invmatch ("ntp_server", ""))
    strcpy (servers, nvram_safe_get ("ntp_server"));
  else
    strcpy (servers, default_servers);

  char *ntpclient_argv[] =
    { "ntpclient", "-h", servers, "-l", "-s", "-i", "5", "-c", "1", NULL };

  ret = -1;
  ret = _eval (ntpclient_argv, NULL, 20, NULL);

  cprintf ("return code=%d, from ntpclient\n", ret);

  if (ret == 0)			// Update Successfully
    {

      strcpy (buf4, nvram_safe_get ("time_zone"));
      strcpy (buf, nvram_safe_get ("time_zone"));
      strcpy (buf2, strtok (buf, " "));
      time_zone = atof (buf2);

      cprintf ("\n%s,%s,%s\n", buf, buf2, buf4);
      cprintf ("Time update successfully, adjust time. (adjust:%f)\n",
	       time_zone);

      gettimeofday (&tv, &tz);
      tv.tv_sec = tv.tv_sec + time_zone * 3600;
      settimeofday (&tv, &tz);

      /* DL */
      gettimeofday (&tv, &tz);
      memcpy (&tm, localtime (&tv.tv_sec), sizeof (struct tm));

      //dprintf("\nYear:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);   
      //syslog(LOG_INFO,"\nYear:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);   

      cprintf ("\nYear:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",
	       tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
	       tm.tm_min, tm.tm_sec);
      i = 0;
      TimeZone = 0;;
      while (strncmp (buf4, tzEntry[i].name, strlen (tzEntry[i].name)) != 0)
	i++;
      //printf("\ntzEntrySize=%d\n",tzEntrySize);
      if (i == tzEntrySize)
	return -1;		/* fail */

      TimeZone = i;
      startMonth = dstEntry[(int) tzEntry[TimeZone].dstFlag].startMonth;
      endMonth = dstEntry[(int) tzEntry[TimeZone].dstFlag].endMonth;
      diffMonth = dstEntry[(int) tzEntry[TimeZone].dstFlag].diffMonth;

      j = tm.tm_year + 1900 - 2002;
      cprintf ("\nTimeZone:%d,i=%d,startm=%d,endmonth=%d,diffm=%d,j=%d\n",
	       TimeZone, i, startMonth, endMonth, diffMonth, j);
      //syslog(LOG_INFO,"\nTimeZone:%d,i=%d,startm=%d,endmonth=%d,diffm=%d,j=%d\n",TimeZone,i,startMonth,endMonth,diffMonth,j);
      //dprintf("\nflag:%d,dstBias:%d\n",tzEntry[TimeZone].dstFlag,dstEntry[tzEntry[TimeZone].dstFlag].dstBias);
      //syslog(LOG_INFO,"\nflag:%d,dstBias:%d\n",tzEntry[TimeZone].dstFlag,dstEntry[tzEntry[TimeZone].dstFlag].dstBias);

      //     dprintf("\n,Year:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);    
      //syslog(LOG_INFO,"\n,Year:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);  
      if (atoi (nvram_safe_get ("daylight_time")))
	{
	  cprintf ("Adjust daylight\n");
	  if (tzEntry[TimeZone].dstFlag && (((diffMonth == 0) && ((tm.tm_mon + 1 == startMonth && tm.tm_mday >= dstEntry[(int) tzEntry[TimeZone].dstFlag].startDay[j]) || (tm.tm_mon + 1 == endMonth && tm.tm_mday < dstEntry[(int) tzEntry[TimeZone].dstFlag].endDay[j]) ||	// tofu - was <=
								  (tm.tm_mon + 1 > startMonth && tm.tm_mon + 1 < endMonth))) || ((diffMonth == 1) && ((tm.tm_mon + 1 == startMonth && tm.tm_mday >= dstEntry[(int) tzEntry[TimeZone].dstFlag].startDay[j]) || (tm.tm_mon + 1 == endMonth && tm.tm_mday < dstEntry[(int) tzEntry[TimeZone].dstFlag].endDay[j]) ||	// tofu - was <=
																		      (tm.tm_mon + 1 > startMonth || tm.tm_mon + 1 < endMonth)))))
	    {

	      tv.tv_sec =
		tv.tv_sec + dstEntry[(int) tzEntry[TimeZone].dstFlag].dstBias;
	      cprintf ("\ndstBias:%d\n",
		       dstEntry[(int) tzEntry[TimeZone].dstFlag].dstBias);
	      settimeofday (&tv, &tz);
	      gettimeofday (&tv, &tz);
	      memcpy (&tm, localtime (&tv.tv_sec), sizeof (struct tm));

	      cprintf ("\n,Year:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",
		       tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		       tm.tm_hour, tm.tm_min, tm.tm_sec);
	    }
	}
      /* firewall.c : synchronize the filter rules by TOD scheduling */
      eval ("filtersync");
      nvram_set ("timer_interval", NTP_M_TIMER);

    }				/* get gmt time successfully */
  else
    {
      cprintf ("Time update failed\n");
      nvram_set ("timer_interval", NTP_N_TIMER);
    }
  return ret;
}

void
ntp_main (timer_t t, int arg)
{
  int ret = 0;

  if (check_action () == ACT_IDLE && check_wan_link (0))
    {				// Don't execute during upgrading
      stop_service ("ntp");
      ret = do_ntp ();
      if (ret == 0 && arg == FIRST)
	{
	  cprintf ("Cancel first ntp timer\n");
	  timer_cancel (t);
	}
    }
  else
    fprintf (stderr, "ntp: nothing to do...\n");
}




#endif
