
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <net/if_arp.h>
#include <stdarg.h>


#include <broadcom.h>
#include <cyutils.h>
#include <code_pattern.h>
#include <cy_conf.h>

/* Format:
 * type = SET :  " " => "&nbsp;" , ":" => "&semi;"
 * type = GET :  "&nbsp;" => " " , "&semi;" => ":"
 * Example:
 * name1 = test 123:abc
 * filter_name("name1", new_name, SET); new_name="test&nbsp;123&semi;abc"
 * name2 = test&nbsp;123&semi;abc
 * filter_name("name2", new_name, GET); new_name="test 123:abc"
 */
int
httpd_filter_name (char *old_name, char *new_name, size_t size, int type)
{
  int i, j, match;

  struct pattern
  {
    char ch;
    char *string;
  };

  struct pattern patterns[] = {
    {' ', "&nbsp;"},
    {':', "&semi;"},
  };

  struct pattern *v;

  strcpy (new_name, "");

  switch (type)
    {
    case SET:
      for (i = 0; *(old_name + i); i++)
	{
	  match = 0;
	  for (v = patterns; v < &patterns[STRUCT_LEN (patterns)]; v++)
	    {
	      if (*(old_name + i) == v->ch)
		{
		  if (strlen (new_name) + strlen (v->string) > size)
		    {		// avoid overflow
		      cprintf ("%s(): overflow\n", __FUNCTION__);
		      new_name[strlen (new_name)] = '\0';
		      return 1;
		    }
		  sprintf (new_name + strlen (new_name), "%s", v->string);
		  match = 1;
		  break;
		}
	    }
	  if (!match)
	    {
	      if (strlen (new_name) + 1 > size)
		{
		  cprintf ("%s(): overflow\n", __FUNCTION__);	// avoid overflow
		  new_name[strlen (new_name)] = '\0';
		  return 1;
		}
	      sprintf (new_name + strlen (new_name), "%c", *(old_name + i));
	    }
	}

      break;
    case GET:
      for (i = 0, j = 0; *(old_name + j); j++)
	{
	  match = 0;
	  for (v = patterns; v < &patterns[STRUCT_LEN (patterns)]; v++)
	    {
	      if (!memcmp (old_name + j, v->string, strlen (v->string)))
		{
		  *(new_name + i) = v->ch;
		  j = j + strlen (v->string) - 1;
		  match = 1;
		  break;
		}
	    }
	  if (!match)
	    *(new_name + i) = *(old_name + j);

	  i++;
	}
      *(new_name + i) = '\0';
      break;
    default:
      cprintf ("%s():Invalid type!\n", __FUNCTION__);
      break;
    }
  //cprintf("%s():new_name=[%s]\n", __FUNCTION__, new_name);

  return 1;
}

void
ej_compile_date (int eid, webs_t wp, int argc, char_t ** argv)
{
  char year[4], mon[3], day[2];
  char string[20];

  sscanf (__DATE__, "%s %s %s", mon, day, year);
  snprintf (string, sizeof (string), "%s. %s, %s", mon, day, year);

  websWrite (wp, "%s", string);
}

void
ej_compile_time (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s", __TIME__);
}

void
ej_get_firmware_version (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s%s %s", CYBERTAN_VERSION, MINOR_VERSION,
	     nvram_safe_get ("dist_type"));
}

void
ej_get_firmware_title (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "Wireless-G Broadband Router");
}

#include <revision.h>
void
ej_get_firmware_svnrev (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s", SVN_REVISION);
}

void
ej_get_web_page_name (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s.asp", websGetVar (wp, "submit_button", "index"));
}

void
ej_get_model_name (int eid, webs_t wp, int argc, char_t ** argv)
{
  //return websWrite(wp,"%s",MODEL_NAME);
  websWrite (wp, "%s", nvram_safe_get ("router_name"));
}

void
ej_get_url (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  websWrite (wp, "%s", "Invalid argument\n");
}

void
ej_show_logo (int eid, webs_t wp, int argc, char_t ** argv)
{
  return;
}


int
protocol_to_num (char *proto)
{
  if (!strcmp (proto, "icmp"))
    return 1;
  else if (!strcmp (proto, "tcp"))
    return 6;
  else if (!strcmp (proto, "udp"))
    return 17;
  else if (!strcmp (proto, "both"))
    return 23;
  else if (!strcmp (proto, "l7"))
    return 99;
  else if (!strcmp (proto, "p2p"))
    return 100;
  else
    return 0;
}

char *
num_to_protocol (int num)
{
  switch (num)
    {
    case 1:
      return "icmp";
    case 6:
      return "tcp";
    case 17:
      return "udp";
    case 23:
      return "both";
    case 99:
      return "l7";
    case 100:
      return "p2p";
    default:
      return "unknown";
    }
}
