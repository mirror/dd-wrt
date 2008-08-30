#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>

void
ej_exec_milkfish_service (webs_t wp, int argc, char_t ** argv)
{

  FILE *fp;
  char line[254];
  char *request;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s", &request);
#else
  if (ejArgs (argc, argv, "%s", &request) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
    }
#endif

  if ((fp = popen (request, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  websWrite (wp, line);
	  websWrite (wp, "<br>");
	}
      pclose (fp);
    }

  return;
}

void
ej_exec_milkfish_phonebook (webs_t wp, int argc, char_t ** argv)
{

  FILE *fp;
  char line[254];
  char *request;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s", &request);
#else
  if (ejArgs (argc, argv, "%s", &request) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
    }
#endif

  if ((fp = popen (request, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
        {
          websWrite (wp, line);
        }
      pclose (fp);
    }

  return;
}


void
validate_subscribers (webs_t wp, char *value, struct variable *v)
{

  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable subscriber_variables[] = {
  {argv:ARGV ("30")},
  {argv:ARGV ("30")},
    {NULL},
  }, *which;
  buf = nvram_safe_get ("milkfish_ddsubscribersnum");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 128) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  buf[0] = 0;

  for (i = 0; i < count; i++)
    {

      char subscriber_user[] = "userXXX";
      char subscriber_pass[] = "passXXX";
      char *user = "", new_user[200] = "", *pass = "", new_pass[200] =
        "";

      snprintf (subscriber_user, sizeof (subscriber_user), "user%d", i);
      snprintf (subscriber_pass, sizeof (subscriber_pass), "pass%d", i);

      user = websGetVar (wp, subscriber_user, "");
      pass = websGetVar (wp, subscriber_pass, "");

      which = &subscriber_variables[0];
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
      cur += snprintf (cur, buf + sof - cur, "%s%s:%s",
                  cur == buf ? "" : " ", new_user, new_pass);

    }
  if (!error)
    nvram_set (v->name, buf);
  free (buf);

}


void
show_subscriber_table (webs_t wp, char *type, int which)
{

  static char word[256];
  char *next, *wordlist;
  char *user, *pass;
  static char new_user[200], new_pass[200];
  int temp;
  wordlist = nvram_safe_get ("milkfish_ddsubscribers");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
        pass = word;
        user = strsep (&pass, ":");
        if (!user || !pass)
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
        return;
      }
   }
}




void
ej_show_subscribers (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *count;
  int c = 0;

  count = nvram_safe_get ("milkfish_ddsubscribersnum");
  if (count == NULL || strlen (count) == 0 || (c=atoi(count))<=0)
  {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
  }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      websWrite (wp, "<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_subscriber_table (wp, "user", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp, "<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_subscriber_table (wp, "pass", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

void
validate_aliases (webs_t wp, char *value, struct variable *v)
{

  int i, error = 0;
  char *buf, *cur;
  int count, sof;
  struct variable alias_variables[] = {
  {argv:ARGV ("30")},
  {argv:ARGV ("30")},
    {NULL},
  }, *which;
  buf = nvram_safe_get ("milkfish_ddaliasesnum");
  if (buf == NULL || strlen (buf) == 0)
    return;
  count = atoi (buf);
  sof = (count * 128) + 1;
  buf = (char *) malloc (sof);
  cur = buf;
  buf[0] = 0;

  for (i = 0; i < count; i++)
    {

      char alias_user[] = "userXXX";
      char alias_pass[] = "passXXX";
      char *user = "", new_user[200] = "", *pass = "", new_pass[200] =
        "";

      snprintf (alias_user, sizeof (alias_user), "user%d", i);
      snprintf (alias_pass, sizeof (alias_pass), "pass%d", i);

      user = websGetVar (wp, alias_user, "");
      pass = websGetVar (wp, alias_pass, "");

      which = &alias_variables[0];
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
      cur += snprintf (cur, buf + sof - cur, "%s%s:%s",
                  cur == buf ? "" : " ", new_user, new_pass);

    }
  if (!error)
    nvram_set (v->name, buf);
  free (buf);

}


void
show_aliases_table (webs_t wp, char *type, int which)
{

  static char word[256];
  char *next, *wordlist;
  char *user, *pass;
  static char new_user[200], new_pass[200];
  int temp;
  wordlist = nvram_safe_get ("milkfish_ddaliases");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
        pass = word;
        user = strsep (&pass, ":");
        if (!user || !pass)
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
        return;
      }
   }
}


void
ej_show_aliases (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *count;
  int c = 0;
  count = nvram_safe_get ("milkfish_ddaliasesnum");
  if (count == NULL || strlen (count) == 0 || (c=atoi(count))<=0)
  {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
                 "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
  }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      websWrite (wp, "<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_aliases_table (wp, "user", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp, "<input maxlength=\"50\" size=\"50\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_aliases_table (wp, "pass", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

void
milkfish_sip_message (webs_t wp)
{
  char *message = websGetVar (wp, "sip_message", NULL);
  char *dest = websGetVar (wp, "sip_message_dest", NULL);
int i;
FILE *fp=fopen("/tmp/sipmessage","wb");
if (fp==NULL)
    return;
      char *host_key = message;
      i = 0;
      do
	{
	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
	}
      while (host_key[++i]);
putc(0xa,fp);
fclose(fp);
  eval("milkfish_services","simpledd",dest);
  return;
}

void
show_registrations_table (webs_t wp, char *type, int which)
{

  static char word[256];
  char *next, *wordlist;
  char *user, *contact, *agent;
  static char new_user[200], new_contact[200], new_agent[200];
  int temp;
  wordlist = nvram_safe_get ("milkfish_ddactive");
  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {
        contact = word;
        user = strsep (&contact, ":");
        if (!user || !contact)
          continue;

        agent = contact;
        contact = strsep (&agent, ":");
        if (!contact || !agent)
          continue;
       

        if (!strcmp (type, "user"))
          {
            httpd_filter_name (user, new_user, sizeof (new_user), GET);
            websWrite (wp, "%s", new_user);
          }
        else if (!strcmp (type, "contact"))
          {
            httpd_filter_name (contact, new_contact, sizeof (new_contact), GET);
            websWrite (wp, "%s", new_contact);
          }
        else if (!strcmp (type, "agent"))
          {
            httpd_filter_name (agent, new_agent, sizeof (new_agent), GET);
            websWrite (wp, "%s", new_agent);
          }

        return;
      }
   }
}


void
ej_show_registrations (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *count;
  int c = 0;
  count = nvram_safe_get ("milkfish_ddactivenum");
  if (count == NULL || strlen (count) == 0 || (c=atoi(count))<=0)
  {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
                 "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
  }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      websWrite (wp, "<input maxlength=\"20\" size=\"20\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_registrations_table (wp, "user", i);
      websWrite (wp, "\" readonly=\"readonly\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp, "<input maxlength=\"50\" size=\"50\" name=\"contact%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_registrations_table (wp, "contact", i);
      websWrite (wp, "\" readonly=\"readonly\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp, "<input maxlength=\"50\" size=\"50\" name=\"agent%d\" onblur=\"valid_name(this,'Name')\" value=\"",i);
      show_registrations_table (wp, "agent", i);
      websWrite (wp, "\" readonly=\"readonly\" /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}


