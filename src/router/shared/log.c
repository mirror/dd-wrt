
/*
 *********************************************************
 *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/klog.h>
#include <netdb.h>

#include <broadcom.h>
#include <support.h>

#define LOG_BUF	16384		// max buf, total have 64 entries

/* Dump firewall log */
int
ej_dumplog (int eid, webs_t wp, int argc, char_t ** argv)
{
  char buf[LOG_BUF], *line, *next, *s;
  int len, ret = 0;
  char *type;

  time_t tm;
  char *verdict, *src, *dst, *proto, *spt, *dpt, *in, *out;
  char src_old[10] = "", dpt_old[10] = "", dst_old[10] = "";

  int _dport, _sport;
  char *_proto = NULL;
  struct servent *servp;
  //struct servent *d_servp;

  char skip[10];
  int debug = 0;

  char *wan_if = get_wan_face ();
  char *lan_if = nvram_safe_get ("lan_ifname");

  int count = 0;

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  //if (klogctl(3, buf, 4096) < 0) {
  if (klogctl (3, buf, LOG_BUF) < 0)
    {
      websError (wp, 400, "Insufficient memory\n");
      return -1;
    }
  cprintf ("log: %s\n", buf);
  for (next = buf; (line = strsep (&next, "\n"));)
    {
      strcpy (skip, "");
      if (!strncmp (line, "<4>DROP", 7))
	verdict = "denied";
      else if (!strncmp (line, "<4>ACCEPT", 9))
	verdict = "accepted";
      else
	continue;


      /* Parse into tokens */
      s = line;
      len = strlen (s);
      while (strsep (&s, " "));

      /* Initialize token values */
      time (&tm);
      in = out = src = dst = proto = spt = dpt = "n/a";

      /* Set token values */
      for (s = line; s < &line[len] && *s; s += strlen (s) + 1)
	{
	  if (!strncmp (s, "TIME=", 5))
	    tm = strtoul (&s[5], NULL, 10);
	  else if (!strncmp (s, "IN=", 3))
	    in = &s[3];
	  else if (!strncmp (s, "OUT=", 4))
	    out = &s[4];
	  else if (!strncmp (s, "SRC=", 4))
	    src = &s[4];
	  else if (!strncmp (s, "DST=", 4))
	    dst = &s[4];
	  else if (!strncmp (s, "PROTO=", 6))
	    proto = &s[6];
	  else if (!strncmp (s, "SPT=", 4))
	    spt = &s[4];
	  else if (!strncmp (s, "DPT=", 4))
	    dpt = &s[4];
	}

      if (!strncmp (dpt, "n/a", 3))	// example: ping
	continue;

      _dport = atoi (dpt);
      _sport = atoi (spt);

      if (!strncmp (proto, "TCP", 3))
	_proto = "tcp";
      else if (!strncmp (proto, "UDP", 3))
	_proto = "udp";

      servp = my_getservbyport (htons (_dport), _proto);

      if (!strcmp (type, "incoming"))
	{
	  if ((!strncmp (in, "ppp", 3) && !strncmp (in, wan_if, 3)) ||
	      (!strcmp (in, wan_if)))
	    {
	      if (!strcmp (src, src_old) && !strcmp (dpt, dpt_old))
		{		// skip same record
		  if (debug)
		    strcpy (skip, "(skip) ");
		  else
		    continue;
		}
	      else
		{
		  strcpy (src_old, src);
		  strcpy (dpt_old, dpt);
		}
	      ret +=
		websWrite (wp,
			   "<TR bgcolor=cccccc> <TD align=middle height=1>%s%s</TD> <TD align=middle height=1>%s</TD></TR>\n",
			   skip, src, servp ? servp->s_name : dpt);
	    }
	}
      else if (!strcmp (type, "outgoing"))
	{
	  if (!strncmp (in, lan_if, 3) &&
	      ((!strncmp (out, "ppp", 3) && !strncmp (out, wan_if, 3)) ||
	       (!strcmp (out, wan_if))))
	    {
	      if (_dport == 53)
		{		// skip DNS
		  if (debug)
		    strcpy (skip, "(skip) ");
		  else
		    continue;
		}
	      if (!strcmp (src, src_old) && !strcmp (dpt, dpt_old)
		  && !strcmp (dst, dst_old))
		{		// skip same record
		  if (debug)
		    strcpy (skip, "(skip) ");
		  else
		    continue;
		}
	      else
		{
		  strcpy (src_old, src);
		  strcpy (dpt_old, dpt);
		  strcpy (dst_old, dst);
		}
	      ret +=
		websWrite (wp,
			   "<TR bgcolor=cccccc> <TD align=middle>%s%s</TD><TD align=middle>%s</TD><TD align=middle>%s</TD></TR>\n",
			   skip, src, dst, servp ? servp->s_name : dpt);
	    }
	}
      else if (!strcmp (type, "all"))
	{
	  int dir = 0;
	  if ((!strncmp (out, "ppp", 3) && !strncmp (out, wan_if, 3)) || (!strcmp (out, wan_if)))	// incoming
	    dir = 1;
	  else if (!strncmp (in, lan_if, 3) && ((!strncmp (out, "ppp", 3) && !strncmp (out, wan_if, 3)) || (!strcmp (out, wan_if))))	// outgoing
	    dir = 2;
	  else
	    continue;

	  if (_dport == 53)
	    {			// skip DNS
	      if (debug)
		strcpy (skip, "(skip) ");
	      else
		continue;
	    }
	  if (!strcmp (src, src_old) && !strcmp (dpt, dpt_old)
	      && !strcmp (dst, dst_old))
	    {			// skip same record
	      if (debug)
		strcpy (skip, "(skip) ");
	      else
		continue;
	    }
	  else
	    {
	      strcpy (src_old, src);
	      strcpy (dpt_old, dpt);
	      strcpy (dst_old, dst);
	    }
	  ret +=
	    websWrite (wp, "%c'%s','%s','%s','%s','%d'\n", count ? ',' : ' ',
		       proto, src, dst, servp ? servp->s_name : dpt, dir);
	  count++;
	}
      //if(s_service) free(s_service);
      //if(d_service) free(d_service);

    }

  return ret;
}
