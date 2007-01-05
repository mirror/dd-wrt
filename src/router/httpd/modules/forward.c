
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <broadcom.h>
#include <cy_conf.h>


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

void
port_forward_table (webs_t wp, char *type, int which)
{
  static char word[256];
  char *next, *wordlist;
  char *name, *from, *to, *proto, *ip, *enable;
  static char new_name[200];
  int temp;

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
	    websWrite (wp, "%s", new_name);
	  }
	else if (!strcmp (type, "from"))
	  websWrite (wp, "%s", from);
	else if (!strcmp (type, "to"))
	  websWrite (wp, "%s", to);
	else if (!strcmp (type, "tcp"))
	  {			// use checkbox
	    if (!strcmp (proto, "udp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "checked=\"checked\"");
	  }
	else if (!strcmp (type, "udp"))
	  {			//use checkbox
	    if (!strcmp (proto, "tcp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "checked=\"checked\"");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      websWrite (wp, "selected=\\\"selected\\\"");
	    else
	      websWrite (wp, "");
	  }
	else if (!strcmp (type, "ip"))
	  websWrite (wp, "%s", ip);
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      websWrite (wp, "checked=\"checked\"");
	    else
	      websWrite (wp, "");
	  }
	return;
      }
  }
  if (!strcmp (type, "from") || !strcmp (type, "to"))
    websWrite (wp, "0");
  else if (!strcmp (type, "ip"))
    websWrite (wp, "0.0.0.0");
  else if (!strcmp (type, "sel_both"))
    websWrite (wp, "selected=\\\"selected\\\"");
  else
    websWrite (wp, "");
}


void
port_forward_spec (webs_t wp, char *type, int which)
{
  static char word[256];
  char *next, *wordlist;
  char *name, *from, *to, *proto, *ip, *enable;
  static char new_name[200];
  int temp;
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
	    websWrite (wp, "%s", new_name);
	  }
	else if (!strcmp (type, "from"))
	  websWrite (wp, "%s", from);
	else if (!strcmp (type, "to"))
	  websWrite (wp, "%s", to);
	else if (!strcmp (type, "tcp"))
	  {			// use checkbox
	    if (!strcmp (proto, "udp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "checked=\"checked\"");
	  }
	else if (!strcmp (type, "udp"))
	  {			//use checkbox
	    if (!strcmp (proto, "tcp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "checked=\"checked\"");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      websWrite (wp, "selected=\\\"selected\\\"");
	    else
	      websWrite (wp, "");
	  }
	else if (!strcmp (type, "ip"))
	  websWrite (wp, "%s", ip);
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      websWrite (wp, "checked=\"checked\"");
	    else
	      websWrite (wp, "");
	  }
	return;
      }
  }
  if (!strcmp (type, "from") || !strcmp (type, "to"))
    websWrite (wp, "0");
  else if (!strcmp (type, "ip"))
    websWrite (wp, "0.0.0.0");
  else if (!strcmp (type, "sel_both"))
    websWrite (wp, "selected=\\\"selected\\\"");
  else
    websWrite (wp, "");
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

void
ej_port_trigger_table (webs_t wp, int argc, char_t ** argv)
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
      return;
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
		websWrite (wp, "%s", new_name);
	      }
	  }
	else if (!strcmp (type, "enable"))
	  {
	    if (!strcmp (enable, "on"))
	      websWrite (wp, "checked=\\\"checked\\\"");
	    else
	      websWrite (wp, "");
	  }
	else if (!strcmp (type, "sel_tcp"))
	  {			// use select
	    if (!strcmp (proto, "udp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_udp"))
	  {			//use select
	    if (!strcmp (proto, "tcp"))
	      websWrite (wp, "");
	    else
	      websWrite (wp, "selected=\"selected\"");
	  }
	else if (!strcmp (type, "sel_both"))
	  {			//use select
	    if (!strcmp (proto, "both"))
	      websWrite (wp, "selected=\"selected\"");
	    else
	      websWrite (wp, "");
	  }
	else if (!strcmp (type, "i_from"))
	  websWrite (wp, "%s", i_from);
	else if (!strcmp (type, "i_to"))
	  websWrite (wp, "%s", i_to);
	else if (!strcmp (type, "o_from"))
	  websWrite (wp, "%s", o_from);
	else if (!strcmp (type, "o_to"))
	  websWrite (wp, "%s", o_to);
	return;
      }
  }
  if (!strcmp (type, "name"))
    websWrite (wp, "");
  else
    websWrite (wp, "0");
  return;
}
