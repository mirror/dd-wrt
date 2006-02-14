
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <broadcom.h>
#include <cy_conf.h>

void
validate_forward_upnp (webs_t wp, char *value, struct variable *v)
{
  int i, error = 0;
  char buf[1000] = "";
  struct variable forward_upnp_variables[] = {
  {longname: "UPnP Forward Application name", argv:ARGV ("25")},
  {longname: "UPnP Forward from WAN Ports", argv:ARGV ("0", "65535")},
  {longname: "UPnP Forward to LAN Ports", argv:ARGV ("0", "65535")},
  {longname:"UPnP Forward Protocol", NULL},
  {longname: "UPnP Forward LAN IP Address", argv:ARGV ("0", "254")},
  }, *which;

  for (i = 0; i < UPNP_FORWARDING_NUM; i++)
    {

      char upnp_name[] = "nameXXX";
      char upnp_from[] = "fromXXX";
      char upnp_to[] = "toXXX";
      char upnp_ip[] = "ipXXX";
      char upnp_pro[] = "proXXX";
      char upnp_enable[] = "enableXXX";
      char *name = "", new_name[200], *from = "", *to = "", *ip =
	"", *enable = "", *pro = "";
      char forward_name[] = "forward_portXXXXXXXXXX";

      snprintf (upnp_name, sizeof (upnp_name), "name%d", i);
      snprintf (upnp_from, sizeof (upnp_from), "from%d", i);
      snprintf (upnp_to, sizeof (upnp_to), "to%d", i);
      snprintf (upnp_ip, sizeof (upnp_ip), "ip%d", i);
      snprintf (upnp_pro, sizeof (upnp_pro), "pro%d", i);
      snprintf (upnp_enable, sizeof (upnp_enable), "enable%d", i);


      name = websGetVar (wp, upnp_name, "");
      from = websGetVar (wp, upnp_from, "0");
      to = websGetVar (wp, upnp_to, "0");
      ip = websGetVar (wp, upnp_ip, "0");
      pro = websGetVar (wp, upnp_pro, "");
      enable = websGetVar (wp, upnp_enable, "off");

      which = &forward_upnp_variables[0];


      if ((!strcmp (from, "0") || !strcmp (from, "")) &&
	  (!strcmp (to, "0") || !strcmp (to, "")) &&
	  (!strcmp (ip, "0") || !strcmp (ip, "")))
	continue;

      /* check name */
      if (strcmp (name, ""))
	{
	  if (!valid_name (wp, name, &which[0]))
	    {
	      error = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (name, new_name, sizeof (new_name), SET);
	    }
	}

      /* check PORT number */

      if (!valid_range (wp, from, &which[1])
	  || !valid_range (wp, to, &which[2]))
	{
	  error = 1;
	  continue;
	}

      /* check ip address */

      if (!valid_range (wp, ip, &which[4]))
	{
	  error = 1;
	  continue;
	}

      snprintf (buf, sizeof (buf), "%s-%s>%s:%s-%s,%s,%s,%s", from, from,
		get_complete_lan_ip (ip), to, to, pro, enable, name);
      //cprintf("buf=[%s]\n", buf); 

      snprintf (forward_name, sizeof (forward_name), "forward_port%d", i);
      nvram_set (forward_name, buf);
    }
}

int
ej_forward_upnp (int eid, webs_t wp, int argc, char_t ** argv)
{
  char name[] = "forward_portXXXXXXXXXX", value[1000];
  char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
  char *enable, *desc;

  char *type;
  int which;

  if (ejArgs (argc, argv, "%s %d", &type, &which) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  snprintf (name, sizeof (name), "forward_port%d", which);
  if (!nvram_invmatch (name, ""))
    goto def;
  strncpy (value, nvram_get (name), sizeof (value));

  /* Check for LAN IP address specification */
  lan_ipaddr = value;
  wan_port0 = strsep (&lan_ipaddr, ">");
  if (!lan_ipaddr)
    return FALSE;

  /* Check for LAN destination port specification */
  lan_port0 = lan_ipaddr;
  lan_ipaddr = strsep (&lan_port0, ":");
  if (!lan_port0)
    return FALSE;

  /* Check for protocol specification */
  proto = lan_port0;
  lan_port0 = strsep (&proto, ":,");
  if (!proto)
    return FALSE;

  /* Check for enable specification */
  enable = proto;
  proto = strsep (&enable, ":,");
  if (!enable)
    return FALSE;

  /* Check for description specification (optional) */
  desc = enable;
  enable = strsep (&desc, ":,");

  /* Check for WAN destination port range (optional) */
  wan_port1 = wan_port0;
  wan_port0 = strsep (&wan_port1, "-");
  if (!wan_port1)
    wan_port1 = wan_port0;

  /* Check for LAN destination port range (optional) */
  lan_port1 = lan_port0;
  lan_port0 = strsep (&lan_port1, "-");
  if (!lan_port1)
    lan_port1 = lan_port0;

  if (!strcmp (type, "name"))
    return websWrite (wp, "%s", desc);
  else if (!strcmp (type, "from"))
    return websWrite (wp, "%s", wan_port0);
  else if (!strcmp (type, "to"))
    return websWrite (wp, "%s", lan_port0);

  else if (!strcmp (type, "tcp"))
    {				// use checkbox 
      if (!strcmp (proto, "udp"))
	return websWrite (wp, "");
      else
	return websWrite (wp, "checked");
    }
  else if (!strcmp (type, "udp"))
    {				//use checkbox 
      if (!strcmp (proto, "tcp"))
	return websWrite (wp, "");
      else
	return websWrite (wp, "checked");
    }
  else if (!strcmp (type, "sel_tcp"))
    {				// use select 
      if (!strcmp (proto, "udp"))
	return websWrite (wp, "");
      else
	return websWrite (wp, "selected");
    }
  else if (!strcmp (type, "sel_udp"))
    {				//use select 
      if (!strcmp (proto, "tcp"))
	return websWrite (wp, "");
      else
	return websWrite (wp, "selected");
    }
  else if (!strcmp (type, "sel_both"))
    {				//use select 
      if (!strcmp (proto, "both"))
	return websWrite (wp, "selected");
      else
	return websWrite (wp, "");
    }
  else if (!strcmp (type, "ip"))
    return websWrite (wp, "%d", get_single_ip (lan_ipaddr, 3));
  else if (!strcmp (type, "enable"))
    if (!strcmp (enable, "on"))
      return websWrite (wp, "checked");
  return websWrite (wp, " ");

def:
  if (!strcmp (type, "from") || !strcmp (type, "to") || !strcmp (type, "ip"))
    return websWrite (wp, "0");
  else if (!strcmp (type, "udp"))
    return websWrite (wp, "checked");
  else
    return 1;

}


/* Example:
 * name:[on|off]:[tcp|udp|both]:8000:80>100
 */

void
validate_forward_proto (webs_t wp, char *value, struct variable *v)
{
  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable forward_proto_variables[] = {
  {longname: "Port Forward Application name", argv:ARGV ("12")},
  {longname: "Port Forward from WAN Port", argv:ARGV ("0", "65535")},
  {longname: "Port Forward to LAN Port", argv:ARGV ("0", "65535")},
  {longname:"Port Forward Protocol", NULL},
  }, *which;
  buf = nvram_safe_get ("forward_entries");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 128) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  buf[0] = 0;

  for (i = 0; i < count; i++)
    {

      char forward_name[] = "nameXXX";
      char forward_from[] = "fromXXX";
      char forward_to[] = "toXXX";
      char forward_ip[] = "ipXXX";
      char forward_tcp[] = "tcpXXX";	// for checkbox
      char forward_udp[] = "udpXXX";	// for checkbox
      char forward_pro[] = "proXXX";	// for select, cisco style UI
      char forward_enable[] = "enableXXX";
      char *name = "", new_name[200] = "", *from = "", *to = "", *ip =
	"", *tcp = "", *udp = "", *enable = "", proto[10], *pro = "";

      snprintf (forward_name, sizeof (forward_name), "name%d", i);
      snprintf (forward_from, sizeof (forward_from), "from%d", i);
      snprintf (forward_to, sizeof (forward_to), "to%d", i);
      snprintf (forward_ip, sizeof (forward_ip), "ip%d", i);
      snprintf (forward_tcp, sizeof (forward_tcp), "tcp%d", i);
      snprintf (forward_udp, sizeof (forward_udp), "udp%d", i);
      snprintf (forward_enable, sizeof (forward_enable), "enable%d", i);
      snprintf (forward_pro, sizeof (forward_pro), "pro%d", i);

      name = websGetVar (wp, forward_name, "");
      from = websGetVar (wp, forward_from, "0");
      to = websGetVar (wp, forward_to, "0");
      ip = websGetVar (wp, forward_ip, "0");
      tcp = websGetVar (wp, forward_tcp, NULL);	// for checkbox
      udp = websGetVar (wp, forward_udp, NULL);	// for checkbox
      pro = websGetVar (wp, forward_pro, NULL);	// for select option
      enable = websGetVar (wp, forward_enable, "off");

      which = &forward_proto_variables[0];


      if (!*from && !*to && !*ip)
	continue;
      if (!strcmp (ip, "0") || !strcmp (ip, ""))
	continue;
      if ((!strcmp (from, "0") || !strcmp (from, "")) &&
	  (!strcmp (to, "0") || !strcmp (to, "")) &&
	  (!strcmp (ip, "0") || !strcmp (ip, "")))
	{
	  continue;
	}

      /* check name */
      if (strcmp (name, ""))
	{
	  if (!valid_name (wp, name, &which[0]))
	    {
	      error_value = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (name, new_name, sizeof (new_name), SET);
	    }
	}

      if (!strcmp (from, ""))
	from = to;
      if (!strcmp (to, ""))
	to = from;

      if (atoi (from) > atoi (to))
	{
	  SWAP (from, to);
	}

      if (!valid_range (wp, from, &which[1])
	  || !valid_range (wp, to, &which[2]))
	{
	  error_value = 1;
	  continue;
	}


      if (pro)
	{			// use select option
	  strcpy (proto, pro);
	}
      else
	{			// use checkbox
	  if (tcp && udp)
	    strcpy (proto, "both");
	  else if (tcp && !udp)
	    strcpy (proto, "tcp");
	  else if (!tcp && udp)
	    strcpy (proto, "udp");
	}
      /* check ip address */

      if (!*ip)
	{
	  error = 1;
	  //      websWrite(wp, "Invalid <b>%s</b> : must specify a ip<br>",which[4].longname);
	  continue;
	}

      /* Sveasoft add - new format allows full IP address */
      if (sv_valid_ipaddr (ip))
	{
	  cur += snprintf (cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s",
			   cur == buf ? "" : " ", new_name, enable, proto,
			   atoi (from), atoi (to), ip);
	}
      /* Sveasoft - for backwords compatability allow single number */
      else if (sv_valid_range (ip, 0, 254))
	{
	  char fullip[16] = { 0 };
	  int f_ip[4];

	  sscanf (nvram_safe_get ("lan_ipaddr"), "%d.%d.%d.%d", &f_ip[0],
		  &f_ip[1], &f_ip[2], &f_ip[3]);
	  snprintf (fullip, 15, "%d.%d.%d.%d", f_ip[0], f_ip[1], f_ip[2],
		    atoi (ip));
	  cur +=
	    snprintf (cur, buf + sof - cur, "%s%s:%s:%s:%d:%d>%s",
		      cur == buf ? "" : " ", new_name, enable, proto,
		      atoi (from), atoi (to), fullip);


	}
      else
	{
	  error = 1;
	  continue;
	}

    }
  if (!error)
    nvram_set (v->name, buf);
  free (buf);
}



/* Example:
 * name:[on|off]:[tcp|udp|both]:8000:80>100
 */

void
validate_forward_spec (webs_t wp, char *value, struct variable *v)
{
  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable forward_proto_variables[] = {
  {longname: "Port Forward Application name", argv:ARGV ("12")},
  {longname: "Port Forward from WAN Port", argv:ARGV ("0", "65535")},
  {longname: "Port Forward to LAN Port", argv:ARGV ("0", "65535")},
  {longname:"Port Forward Protocol", NULL},
  }, *which;
  buf = nvram_safe_get ("forwardspec_entries");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 128) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  buf[0] = 0;

  for (i = 0; i < count; i++)
    {

      char forward_name[] = "nameXXX";
      char forward_from[] = "fromXXX";
      char forward_to[] = "toXXX";
      char forward_ip[] = "ipXXX";
      char forward_tcp[] = "tcpXXX";	// for checkbox
      char forward_udp[] = "udpXXX";	// for checkbox
      char forward_pro[] = "proXXX";	// for select, cisco style UI
      char forward_enable[] = "enableXXX";
      char *name = "", new_name[200] = "", *from = "", *to = "", *ip =
	"", *tcp = "", *udp = "", *enable = "", proto[10], *pro = "";

      snprintf (forward_name, sizeof (forward_name), "name%d", i);
      snprintf (forward_from, sizeof (forward_from), "from%d", i);
      snprintf (forward_to, sizeof (forward_to), "to%d", i);
      snprintf (forward_ip, sizeof (forward_ip), "ip%d", i);
      snprintf (forward_tcp, sizeof (forward_tcp), "tcp%d", i);
      snprintf (forward_udp, sizeof (forward_udp), "udp%d", i);
      snprintf (forward_enable, sizeof (forward_enable), "enable%d", i);
      snprintf (forward_pro, sizeof (forward_pro), "pro%d", i);

      name = websGetVar (wp, forward_name, "");
      from = websGetVar (wp, forward_from, "0");
      to = websGetVar (wp, forward_to, "0");
      ip = websGetVar (wp, forward_ip, "0");
      tcp = websGetVar (wp, forward_tcp, NULL);	// for checkbox
      udp = websGetVar (wp, forward_udp, NULL);	// for checkbox
      pro = websGetVar (wp, forward_pro, NULL);	// for select option
      enable = websGetVar (wp, forward_enable, "off");

      which = &forward_proto_variables[0];


      if (!*from && !*to && !*ip)
	continue;
      if (!strcmp (ip, "0") || !strcmp (ip, ""))
	continue;
      if ((!strcmp (from, "0") || !strcmp (from, "")) &&
	  (!strcmp (to, "0") || !strcmp (to, "")) &&
	  (!strcmp (ip, "0") || !strcmp (ip, "")))
	{
	  continue;
	}

      /* check name */
      if (strcmp (name, ""))
	{
	  if (!valid_name (wp, name, &which[0]))
	    {
	      error_value = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (name, new_name, sizeof (new_name), SET);
	    }
	}

      if (!strcmp (from, ""))
	from = to;
      if (!strcmp (to, ""))
	to = from;

      /*if(atoi(from) > atoi(to)){
         SWAP(from, to);
         } */

      if (!valid_range (wp, from, &which[1])
	  || !valid_range (wp, to, &which[2]))
	{
	  error_value = 1;
	  continue;
	}


      if (pro)
	{			// use select option
	  strcpy (proto, pro);
	}
      else
	{			// use checkbox
	  if (tcp && udp)
	    strcpy (proto, "both");
	  else if (tcp && !udp)
	    strcpy (proto, "tcp");
	  else if (!tcp && udp)
	    strcpy (proto, "udp");
	}
      /* check ip address */

      if (!*ip)
	{
	  error = 1;
	  //      websWrite(wp, "Invalid <b>%s</b> : must specify a ip<br>",which[4].longname);
	  continue;
	}

      /* Sveasoft add - new format allows full IP address */
      if (sv_valid_ipaddr (ip))
	{
	  cur += snprintf (cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d",
			   cur == buf ? "" : " ", new_name, enable, proto,
			   atoi (from), ip, atoi (to));
	}
      /* Sveasoft - for backwords compatability allow single number */
      else if (sv_valid_range (ip, 0, 254))
	{
	  char fullip[16] = { 0 };
	  int f_ip[4];

	  sscanf (nvram_safe_get ("lan_ipaddr"), "%d.%d.%d.%d", &f_ip[0],
		  &f_ip[1], &f_ip[2], &f_ip[3]);
	  snprintf (fullip, 15, "%d.%d.%d.%d", f_ip[0], f_ip[1], f_ip[2],
		    atoi (ip));
	  cur +=
	    snprintf (cur, buf + sof - cur, "%s%s:%s:%s:%d>%s:%d",
		      cur == buf ? "" : " ", new_name, enable, proto,
		      atoi (from), fullip, atoi (to));


	}
      else
	{
	  error = 1;
	  continue;
	}

    }
  if (!error)
    nvram_set (v->name, buf);
  free (buf);
}



/* Example:
 * name:[on|off]:[tcp|udp|both]:8000:80>100
 */

int
ej_port_forward_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  int which;

  static char word[256];
  char *next, *wordlist;
  char *name, *from, *to, *proto, *ip, *enable;
  static char new_name[200];
  int temp;

  if (ejArgs (argc, argv, "%s %d", &type, &which) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  wordlist = nvram_safe_get ("forward_port");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
	enable = word;
	name = strsep (&enable, ":");
	if (!name || !enable)
	  continue;
	proto = enable;
	enable = strsep (&proto, ":");
	if (!enable || !proto)
	  continue;
	from = proto;
	proto = strsep (&from, ":");
	if (!proto || !from)
	  continue;
	to = from;
	from = strsep (&to, ":");
	if (!to || !from)
	  continue;
	ip = to;
	to = strsep (&ip, ">");
	if (!ip || !to)
	  continue;

	if (!strcmp (type, "name"))
	  {
	    httpd_filter_name (name, new_name, sizeof (new_name), GET);
	    return websWrite (wp, "%s", new_name);
	  }
	else if (!strcmp (type, "from"))
	  return websWrite (wp, "%s", from);
	else if (!strcmp (type, "to"))
	  return websWrite (wp, "%s", to);
	else if (!strcmp (type, "tcp"))
	  {			// use checkbox
	    if (!strcmp (proto, "udp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "checked");
	  }
	else if (!strcmp (type, "udp"))
	  {			//use checkbox
	    if (!strcmp (proto, "tcp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "checked");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      return websWrite (wp, "selected");
	    else
	      return websWrite (wp, "");
	  }
	else if (!strcmp (type, "ip"))
	  return websWrite (wp, "%s", ip);
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      return websWrite (wp, "checked");
	    else
	      return websWrite (wp, "");
	  }
      }
  }
  if (!strcmp (type, "from") || !strcmp (type, "to") || !strcmp (type, "ip"))
    return websWrite (wp, "0");
  else if (!strcmp (type, "sel_both"))
    return websWrite (wp, "selected");
  else
    return websWrite (wp, "");
}


int
ej_port_forward_spec (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  int which;

  static char word[256];
  char *next, *wordlist;
  char *name, *from, *to, *proto, *ip, *enable;
  static char new_name[200];
  int temp;

  if (ejArgs (argc, argv, "%s %d", &type, &which) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  wordlist = nvram_safe_get ("forward_spec");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
	enable = word;
	name = strsep (&enable, ":");
	if (!name || !enable)
	  continue;
	proto = enable;
	enable = strsep (&proto, ":");
	if (!enable || !proto)
	  continue;
	from = proto;
	proto = strsep (&from, ":");
	if (!proto || !from)
	  continue;
	ip = from;
	from = strsep (&ip, ">");
	if (!from || !ip)
	  continue;
	to = ip;
	ip = strsep (&to, ":");
	if (!ip || !to)
	  continue;


	if (!strcmp (type, "name"))
	  {
	    httpd_filter_name (name, new_name, sizeof (new_name), GET);
	    return websWrite (wp, "%s", new_name);
	  }
	else if (!strcmp (type, "from"))
	  return websWrite (wp, "%s", from);
	else if (!strcmp (type, "to"))
	  return websWrite (wp, "%s", to);
	else if (!strcmp (type, "tcp"))
	  {			// use checkbox
	    if (!strcmp (proto, "udp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "checked");
	  }
	else if (!strcmp (type, "udp"))
	  {			//use checkbox
	    if (!strcmp (proto, "tcp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "checked");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      return websWrite (wp, "selected");
	    else
	      return websWrite (wp, "");
	  }
	else if (!strcmp (type, "ip"))
	  return websWrite (wp, "%s", ip);
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      return websWrite (wp, "checked");
	    else
	      return websWrite (wp, "");
	  }
      }
  }
  if (!strcmp (type, "from") || !strcmp (type, "to") || !strcmp (type, "ip"))
    return websWrite (wp, "0");
  else if (!strcmp (type, "sel_both"))
    return websWrite (wp, "selected");
  else
    return websWrite (wp, "");
}


/* Example:
 * name:on:both:1000-2000>3000-4000
 */

void
validate_port_trigger (webs_t wp, char *value, struct variable *v)
{
  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable trigger_variables[] = {
  {longname: "Port Trigger Application name", argv:ARGV ("12")},
  {longname: "Port Trigger from WAN Port", argv:ARGV ("0", "65535")},
  {longname: "Port Trigger from WAN Port", argv:ARGV ("0", "65535")},
  {longname: "Port Trigger to LAN Port", argv:ARGV ("0", "65535")},
  {longname: "Port Trigger to LAN Port", argv:ARGV ("0", "65535")},
  }, *which;

  buf = nvram_safe_get ("trigger_entries");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 46) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  memset (buf, 0, sof);

  for (i = 0; i < count; i++)
    {

      char trigger_name[] = "nameXXX";
      char trigger_enable[] = "enableXXX";
      char trigger_i_from[] = "i_fromXXX";
      char trigger_i_to[] = "i_toXXX";
      char trigger_o_from[] = "o_fromXXX";
      char trigger_o_to[] = "o_toXXX";
      char *name = "", *enable, new_name[200] = "", *i_from = "", *i_to =
	"", *o_from = "", *o_to = "";

      snprintf (trigger_name, sizeof (trigger_name), "name%d", i);
      snprintf (trigger_enable, sizeof (trigger_enable), "enable%d", i);
      snprintf (trigger_i_from, sizeof (trigger_i_from), "i_from%d", i);
      snprintf (trigger_i_to, sizeof (trigger_i_to), "i_to%d", i);
      snprintf (trigger_o_from, sizeof (trigger_o_from), "o_from%d", i);
      snprintf (trigger_o_to, sizeof (trigger_o_to), "o_to%d", i);

      name = websGetVar (wp, trigger_name, "");
      enable = websGetVar (wp, trigger_enable, "off");
      i_from = websGetVar (wp, trigger_i_from, NULL);
      i_to = websGetVar (wp, trigger_i_to, NULL);
      o_from = websGetVar (wp, trigger_o_from, NULL);
      o_to = websGetVar (wp, trigger_o_to, NULL);

      which = &trigger_variables[0];

      if (!i_from || !i_to || !o_from || !o_to)
	continue;


      if ((!strcmp (i_from, "0") || !strcmp (i_from, "")) &&
	  (!strcmp (i_to, "0") || !strcmp (i_to, "")) &&
	  (!strcmp (o_from, "0") || !strcmp (o_from, "")) &&
	  (!strcmp (o_to, "0") || !strcmp (o_to, "")))
	continue;

      if (!strcmp (i_from, "0") || !strcmp (i_from, ""))
	i_from = i_to;
      if (!strcmp (i_to, "0") || !strcmp (i_to, ""))
	i_to = i_from;
      if (!strcmp (o_from, "0") || !strcmp (o_from, ""))
	o_from = o_to;
      if (!strcmp (o_to, "0") || !strcmp (o_to, ""))
	o_to = o_from;

      if (atoi (i_from) > atoi (i_to))
	SWAP (i_from, i_to);

      if (atoi (o_from) > atoi (o_to))
	SWAP (o_from, o_to);

      if (strcmp (name, ""))
	{
	  if (!valid_name (wp, name, &which[0]))
	    {
	      error = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (name, new_name, sizeof (new_name), SET);
	    }
	}

      if (!valid_range (wp, i_from, &which[1])
	  || !valid_range (wp, i_to, &which[2])
	  || !valid_range (wp, o_from, &which[3])
	  || !valid_range (wp, o_to, &which[4]))
	{
	  error = 1;
	  continue;
	}

      cur += snprintf (cur, buf + sof - cur, "%s%s:%s:both:%s-%s>%s-%s",
		       cur == buf ? "" : " ", new_name, enable, i_from, i_to,
		       o_from, o_to);

    }

  if (!error)
    nvram_set (v->name, buf);
  free (buf);
}

/* Example:
 * name:on:both:1000-2000>3000-4000
 */

int
ej_port_trigger_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  int which;

  static char word[256];
  char *next, *wordlist;
  char *name = NULL, *enable = NULL, *proto = NULL, *i_from = NULL, *i_to =
    NULL, *o_from = NULL, *o_to = NULL;
  static char new_name[200];
  int temp;

  if (ejArgs (argc, argv, "%s %d", &type, &which) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  wordlist = nvram_safe_get ("port_trigger");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
	enable = word;
	name = strsep (&enable, ":");
	if (!name || !enable)
	  continue;
	proto = enable;
	enable = strsep (&proto, ":");
	if (!enable || !proto)
	  continue;
	i_from = proto;
	proto = strsep (&i_from, ":");
	if (!proto || !i_from)
	  continue;
	i_to = i_from;
	i_from = strsep (&i_to, "-");
	if (!i_from || !i_to)
	  continue;
	o_from = i_to;
	i_to = strsep (&o_from, ">");
	if (!i_to || !o_from)
	  continue;
	o_to = o_from;
	o_from = strsep (&o_to, "-");
	if (!o_from || !o_to)
	  continue;

	if (!strcmp (type, "name"))
	  {
	    if (strcmp (name, ""))
	      {
		httpd_filter_name (name, new_name, sizeof (new_name), GET);
		return websWrite (wp, "%s", new_name);
	      }
	  }
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      return websWrite (wp, "checked");
	    else
	      return websWrite (wp, "");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      return websWrite (wp, "");
	    else
	      return websWrite (wp, "selected");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      return websWrite (wp, "selected");
	    else
	      return websWrite (wp, "");;
	  }
	else if (!strcmp (type, "i_from"))
	  return websWrite (wp, "%s", i_from);
	else if (!strcmp (type, "i_to"))
	  return websWrite (wp, "%s", i_to);
	else if (!strcmp (type, "o_from"))
	  return websWrite (wp, "%s", o_from);
	else if (!strcmp (type, "o_to"))
	  return websWrite (wp, "%s", o_to);
      }
  }
  if (!strcmp (type, "name"))
    return websWrite (wp, "");
  else
    return websWrite (wp, "0");
}
