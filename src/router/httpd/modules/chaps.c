#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <broadcom.h>
#include <cy_conf.h>



/* Example:
 * name:pass:ip:on
 */


void
validate_chaps (webs_t wp, char *value, struct variable *v)
{

  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable chaps_variables[] = {
  {argv:ARGV ("30")},
  {argv:ARGV ("30")},
  {NULL},
  }, *which;
  buf = nvram_safe_get ("pppoeserver_chapsnum");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 128) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  buf[0] = 0;

  for (i = 0; i < count; i++)
    {

      char chap_user[] = "userXXX";
      char chap_pass[] = "passXXX";
      char chap_ip[] = "ipXXX";
      char chap_enable[] = "enableXXX";
      char *user = "", new_user[200] = "", *pass = "", new_pass[200] =
	"", *ip = "", *enable = "";

      snprintf (chap_user, sizeof (chap_user), "user%d", i);
      snprintf (chap_pass, sizeof (chap_pass), "pass%d", i);
      snprintf (chap_ip, sizeof (chap_ip), "ip%d", i);
      snprintf (chap_enable, sizeof (chap_enable), "enable%d", i);

      user = websGetVar (wp, chap_user, "");
      pass = websGetVar (wp, chap_pass, "");
      ip = websGetVar (wp, chap_ip, "0");
      enable = websGetVar (wp, chap_enable, "off");

      which = &chaps_variables[0];


      if (!*ip)
	continue;
      if (!strcmp (ip, "0") || !strcmp (ip, ""))
	continue;

      /* check name */
      if (strcmp (user, ""))
	{
	  if (!valid_name (wp, user, &which[0]))
	    {
	      error_value = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (user, new_user, sizeof (new_user), SET);
	    }
	}

      if (strcmp (pass, ""))
	{
	  if (!valid_name (wp, pass, &which[1]))
	    {
	      error_value = 1;
	      continue;
	    }
	  else
	    {
	      httpd_filter_name (pass, new_pass, sizeof (new_pass), SET);
	    }
	}

      /* check ip address */
      if (!*ip)
	{
	  error = 1;
	  //      websWrite(wp, "Invalid <b>%s</b> : must specify a ip<br>",which[4].longname);
	  continue;
	}

      if (sv_valid_ipaddr (ip))
	{
	  cur += snprintf (cur, buf + sof - cur, "%s%s:%s:%s:%s",
			   cur == buf ? "" : " ", new_user, new_pass, ip,
			   enable);
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


void
show_chaps_table (webs_t wp, char *type, int which)
{

  static char word[256];
  char *next, *wordlist;
  char *user, *pass, *ip, *enable;
  static char new_user[200], new_pass[200];
  int temp;
  wordlist = nvram_safe_get ("pppoeserver_chaps");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
	pass = word;
	user = strsep (&pass, ":");
	if (!user || !pass)
	  continue;

	ip = pass;
	pass = strsep (&ip, ":");
	if (!pass || !ip)
	  continue;

	enable = ip;
	ip = strsep (&enable, ":");
	if (!ip || !enable)
	  continue;


	if (!strcmp (type, "user"))
	  {
	    httpd_filter_name (user, new_user, sizeof (new_user), GET);
	    websWrite (wp, "%s", new_user);
	  }
	else if (!strcmp (type, "pass"))
	  {
	    httpd_filter_name (pass, new_pass, sizeof (new_pass), GET);
	    websWrite (wp, "%s", new_pass);
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
  if (!strcmp (type, "ip"))
    websWrite (wp, "0.0.0.0");
  else
    websWrite (wp, "");

}


#define CHAPSHOW(a,b) sprintf(buffer,a,b); do_ej_buffer(buffer,wp);

void
ej_show_chaps (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char buffer[1024], *count;
  int c = 0;
  count = nvram_safe_get ("pppoeserver_chapsnum");
  if (count == NULL || strlen (count) == 0)
    {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  c = atoi (count);
  if (c <= 0)
    {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      CHAPSHOW
	("<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
	 i);
      show_chaps_table (wp, "user", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      CHAPSHOW
	("<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
	 i);
      show_chaps_table (wp, "pass", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      CHAPSHOW
	("<input class=\"num\" maxlength=\"15\" size=\"26\" name=\"ip%d\" value=\"",
	 i);
      show_chaps_table (wp, "ip", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      CHAPSHOW ("<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ",
		i);
      show_chaps_table (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}
