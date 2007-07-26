
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>



void
ej_show_routeif (webs_t wp, int argc, char_t ** argv)
{
  char *arg;
  int which, count;
  char word[256], *next, *page;
  char name[50] =
    "", *ipaddr, *netmask, *gateway, *metric, *ifname, ifnamecopy[32];
  int temp;
  char new_name[200];
  char bufferif[512];
  page = websGetVar (wp, "route_page", NULL);
  if (!page)
    page = "0";
  which = atoi (page);

  foreach (word, nvram_safe_get ("static_route"), next)
  {
    if (which-- == 0)
      {
	netmask = word;
	ipaddr = strsep (&netmask, ":");
	if (!ipaddr || !netmask)
	  continue;
	gateway = netmask;
	netmask = strsep (&gateway, ":");
	if (!netmask || !gateway)
	  continue;
	metric = gateway;
	gateway = strsep (&metric, ":");
	if (!gateway || !metric)
	  continue;
	ifname = metric;
	metric = strsep (&ifname, ":");
	if (!metric || !ifname)
	  continue;
	break;
      }
  }
  if (!ifname)
    ifname = "br0";
  strcpy (ifnamecopy, ifname);

  memset (bufferif, 0, 512);
  getIfList (bufferif, NULL);
  websWrite (wp, "<option value=\"lan\" %s >LAN &amp; WLAN</option>\n",
	     nvram_match ("lan_ifname",
			  ifname) ? "selected=\"selected\"" : "");
  websWrite (wp, "<option value=\"wan\" %s >WAN</option>\n",
	     nvram_match ("wan_ifname",
			  ifname) ? "selected=\"selected\"" : "");
  memset (word, 0, 256);
  next = NULL;
  foreach (word, bufferif, next)
  {
    if (nvram_match ("lan_ifname", word))
      continue;
    if (nvram_match ("wan_ifname", word))
      continue;
    websWrite (wp, "<option value=\"%s\" %s >%s</option>\n", word,
	       strcmp (word, ifnamecopy) == 0 ? "selected=\"selected\"" : "",
	       word);
  }
}

/*
 * Example: 
 * static_route=192.168.2.0:255.255.255.0:192.168.1.2:1:br0
 * <% static_route("ipaddr", 0); %> produces "192.168.2.0"
 * <% static_route("lan", 0); %> produces "selected" if nvram_match("lan_ifname", "br0")
 */
void
ej_static_route_setting (webs_t wp, int argc, char_t ** argv)
{
  char *arg;
  int which, count;
  char word[256], *next, *page;
  char name[50] = "", *ipaddr, *netmask, *gateway, *metric, *ifname;
  int temp;
  char new_name[200];

  if (ejArgs (argc, argv, "%s %d", &arg, &count) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  page = websGetVar (wp, "route_page", NULL);
  if (!page)
    page = "0";

  which = atoi (page);

  temp = which;

  if (!strcmp (arg, "name"))
    {
      foreach (word, nvram_safe_get ("static_route_name"), next)
      {
	if (which-- == 0)
	  {
	    find_match_pattern (name, sizeof (name), word, "$NAME:", "");
	    httpd_filter_name (name, new_name, sizeof (new_name), GET);
	    websWrite (wp, new_name);
	    return;
	  }

      }
    }

  foreach (word, nvram_safe_get ("static_route"), next)
  {
    if (which-- == 0)
      {
	netmask = word;
	ipaddr = strsep (&netmask, ":");
	if (!ipaddr || !netmask)
	  continue;
	gateway = netmask;
	netmask = strsep (&gateway, ":");
	if (!netmask || !gateway)
	  continue;
	metric = gateway;
	gateway = strsep (&metric, ":");
	if (!gateway || !metric)
	  continue;
	ifname = metric;
	metric = strsep (&ifname, ":");
	if (!metric || !ifname)
	  continue;
	if (!strcmp (arg, "ipaddr"))
	  {
	    websWrite (wp, "%d", get_single_ip (ipaddr, count));
	    return;
	  }
	else if (!strcmp (arg, "netmask"))
	  {
	    websWrite (wp, "%d", get_single_ip (netmask, count));
	    return;
	  }
	else if (!strcmp (arg, "gateway"))
	  {
	    websWrite (wp, "%d", get_single_ip (gateway, count));
	    return;
	  }
	else if (!strcmp (arg, "metric"))
	  {
	    websWrite (wp, metric);
	    return;
	  }
	else if (!strcmp (arg, "lan") && nvram_match ("lan_ifname", ifname))
	  {
	    websWrite (wp, "selected=\"selected\"");
	    return;
	  }
	else if (!strcmp (arg, "wan") && nvram_match ("wan_ifname", ifname))
	  {
	    websWrite (wp, "selected=\"selected\"");
	    return;
	  }
      }
  }

  if (!strcmp (arg, "ipaddr") || !strcmp (arg, "netmask")
      || !strcmp (arg, "gateway"))
    websWrite (wp, "0");
  else if (!strcmp (arg, "metric"))
    websWrite (wp, "0");

  return;
}


void
addDeletion (char *word)
{
  char *oldarg = nvram_get ("action_service_arg1");
  if (oldarg && strlen (oldarg) > 0)
    {
      char *newarg = malloc (strlen (oldarg) + strlen (word) + 2);
      sprintf (newarg, "%s %s", oldarg, word);
      nvram_set ("action_service_arg1", newarg);
      free (newarg);
    }
  else
    nvram_set ("action_service_arg1", word);
}
extern int save_olsrd (webs_t wp);

void
validate_static_route (webs_t wp, char *value, struct variable *v)
{
#ifdef HAVE_OLSRD
  save_olsrd (wp);
#endif

  int i, tmp = 1;
  char word[256], *next;
  char buf[1000] = "", *cur = buf;
  char buf_name[1000] = "", *cur_name = buf_name;
  char old[STATIC_ROUTE_PAGE][60];
  char old_name[STATIC_ROUTE_PAGE][30];
  char backuproute[256];
  struct variable static_route_variables[] = {
  {argv:NULL},
  {argv:NULL},
  {argv:NULL},
  {argv:ARGV ("lan", "wan")},
  };

  char *name, ipaddr[20], netmask[20], gateway[20], *metric =
    "0", *ifname, *page;
  char new_name[80];
  char temp[30], *val = NULL;

  name = websGetVar (wp, "route_name", "");	// default empty if no find route_name

  /* validate ip address */
  strcpy (ipaddr, "");
  for (i = 0; i < 4; i++)
    {
      snprintf (temp, sizeof (temp), "%s_%d", "route_ipaddr", i);
      val = websGetVar (wp, temp, NULL);
      if (val)
	{
	  strcat (ipaddr, val);
	  if (i < 3)
	    strcat (ipaddr, ".");
	}
      else
	{
//        free (ipaddr);
	  return;
	}
    }

  /* validate netmask */
  strcpy (netmask, "");
  for (i = 0; i < 4; i++)
    {
      snprintf (temp, sizeof (temp), "%s_%d", "route_netmask", i);
      val = websGetVar (wp, temp, NULL);
      if (val)
	{
	  strcat (netmask, val);
	  if (i < 3)
	    strcat (netmask, ".");
	}
      else
	{
//        free (netmask);
//        free (ipaddr);
	  return;
	}
    }

  /* validate gateway */
  strcpy (gateway, "");
  for (i = 0; i < 4; i++)
    {
      snprintf (temp, sizeof (temp), "%s_%d", "route_gateway", i);
      val = websGetVar (wp, temp, NULL);
      if (val)
	{
	  strcat (gateway, val);
	  if (i < 3)
	    strcat (gateway, ".");
	}
      else
	{
//        free (gateway);
//        free (netmask);
//        free (ipaddr);
	  return;
	}
    }

  page = websGetVar (wp, "route_page", NULL);
  ifname = websGetVar (wp, "route_ifname", NULL);


  if (!page || !ipaddr || !netmask || !gateway || !metric || !ifname)
    return;

// Allow Defaultroute here

  if (!strcmp (ipaddr, "0.0.0.0") && !strcmp (netmask, "0.0.0.0")
      && strcmp (gateway, "0.0.0.0"))
    {
      tmp = 1;
      goto write_nvram;
    }
  if ((!strcmp (ipaddr, "0.0.0.0") || !strcmp (ipaddr, "")) &&
      (!strcmp (netmask, "0.0.0.0") || !strcmp (netmask, "")) &&
      (!strcmp (gateway, "0.0.0.0") || !strcmp (gateway, "")))
    {
      tmp = 0;
      goto write_nvram;
    }

//  if (!valid_choice (wp, ifname, &static_route_variables[3]))
//    {
//      free (gateway);
//      free (netmask);
//      free (ipaddr);

//      return;
//    }

  if (!*ipaddr)
    {
      websDebugWrite (wp, "Invalid <b>%s</b>: must specify an IP Address<br>",
		      v->longname);
//      free (gateway);
//      free (netmask);
//      free (ipaddr);

      return;
    }
  if (!*netmask)
    {
      websDebugWrite (wp, "Invalid <b>%s</b>: must specify a Subnet Mask<br>",
		      v->longname);
//      free (gateway);
//      free (netmask);
//      free (ipaddr);

      return;
    }
  if (!valid_ipaddr (wp, ipaddr, &static_route_variables[0]) ||
      !valid_netmask (wp, netmask, &static_route_variables[1]) ||
      !valid_ipaddr (wp, gateway, &static_route_variables[2]))
    {
//      free (gateway);
//      free (netmask);
//      free (ipaddr);

      return;
    }

  /* save old value in nvram */

write_nvram:
  if (!strcmp (ifname, "lan"))
    {
      ifname = nvram_safe_get ("lan_ifname");
      static_route_variables[2].argv = NULL;
    }
  if (!strcmp (ifname, "wan"))
    {
      ifname = nvram_safe_get ("wan_ifname");
      static_route_variables[2].argv = NULL;
    }
  else
    {
      static_route_variables[2].argv = NULL;
    }

  for (i = 0; i < STATIC_ROUTE_PAGE; i++)
    {
      strcpy (old[i], "");
      strcpy (old_name[i], "");
    }
  i = 0;
  foreach (word, nvram_safe_get ("static_route"), next)
  {
    strcpy (old[i], word);
    i++;
  }
  i = 0;
  foreach (word, nvram_safe_get ("static_route_name"), next)
  {
    strcpy (old_name[i], word);
    i++;
  }
  
  
  
  strcpy(backuproute,old[atoi (page)]);
  if (!tmp)
    {
      char met[16];
      char ifn[16];
      sscanf (old[atoi (page)], "%s:%s:%s:%s:%s", ipaddr, netmask, gateway,
	      met, ifn);
      fprintf (stderr, "deleting %s %s %s %s %s\n", ipaddr, netmask, gateway,
	       met, ifn);
      route_del (ifn, atoi (met) + 1, ipaddr, gateway, netmask);

      snprintf (old[atoi (page)], sizeof (old[0]), "%s", "");
      snprintf (old_name[atoi (page)], sizeof (old_name[0]), "%s", "");
    }
  else
    {
      snprintf (old[atoi (page)], sizeof (old[0]), "%s:%s:%s:%s:%s", ipaddr,
		netmask, gateway, metric, ifname);
      httpd_filter_name (name, new_name, sizeof (new_name), SET);
      snprintf (old_name[atoi (page)], sizeof (old_name[0]), "$NAME:%s$$",
		new_name);
    }
  if (strcmp(backuproute,old[atoi (page)]))
  {
  if (strlen (backuproute) > 0)
    {
      addAction ("static_route_del");
      addDeletion (backuproute);
    }
  }

  for (i = 0; i < STATIC_ROUTE_PAGE; i++)
    {
      if (strcmp (old[i], ""))
	cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
			 cur == buf ? "" : " ", old[i]);
      if (strcmp (old_name[i], ""))
	cur_name +=
	  snprintf (cur_name, buf_name + sizeof (buf_name) - cur_name, "%s%s",
		    cur_name == buf_name ? "" : " ", old_name[i]);
    }

  nvram_set (v->name, buf);
  nvram_set ("static_route_name", buf_name);

//  if (ipaddr)
//    free (ipaddr);
//  if (netmask)
//    free (netmask);
//  if (gateway)
//    free (gateway);
}

void
ej_static_route_table (webs_t wp, int argc, char_t ** argv)
{
  int i, page;
  int which;
  char *type;
  char word[256], *next;

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  page = atoi (websGetVar (wp, "route_page", "0"));	// default to 0

  if (!strcmp (type, "select"))
    {
      for (i = 0; i < STATIC_ROUTE_PAGE; i++)
	{
	  char name[50] = " ";
	  char new_name[80] = " ";
	  char buf[80] = "";
	  which = i;
	  foreach (word, nvram_safe_get ("static_route_name"), next)
	  {
	    if (which-- == 0)
	      {
		find_match_pattern (name, sizeof (name), word, "$NAME:", " ");
		httpd_filter_name (name, new_name, sizeof (new_name), GET);
	      }
	  }
	  snprintf (buf, sizeof (buf), "(%s)", new_name);


	  websWrite (wp, "\t\t<option value=\"%d\" %s> %d %s</option>\n", i,
		     (i == page) ? "selected=\"selected\"" : "", i + 1, buf);
	}
    }

  return;
}

int
delete_static_route (webs_t wp)
{
  addAction ("routing");
  char *buf = malloc (1000);
  char *buf_name = malloc (1000);
  memset (buf, 0, 1000);
  memset (buf_name, 0, 1000);
  char *cur = buf;
  char *cur_name = buf_name;
  static char word[256], *next;
  static char word_name[256], *next_name;
  char *page = websGetVar (wp, "route_page", NULL);
  char *value = websGetVar (wp, "action", "");
  int i = 0;
  char *performance = nvram_safe_get ("static_route");
  char *performance2 = nvram_safe_get ("static_route_name");
  foreach (word, performance, next)
  {
    if (i == atoi (page))
      {
	addDeletion (word);
	i++;
	continue;
      }

    cur += snprintf (cur, buf + 1000 - cur, "%s%s",
		     cur == buf ? "" : " ", word);

    i++;
  }

  i = 0;
  foreach (word_name, performance2, next_name)
  {
    if (i == atoi (page))
      {
	i++;
	continue;
      }
    cur_name +=
      snprintf (cur_name, buf_name + 1000 - cur_name, "%s%s",
		cur_name == buf_name ? "" : " ", word_name);

    i++;
  }

  nvram_set ("static_route", buf);
  nvram_set ("static_route_name", buf_name);
  free (buf_name);
  free (buf);
  if (!strcmp (value, "ApplyTake"))
    {
      nvram_commit ();
      service_restart ();
    }
  return 0;
}
