#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>


static char *
exec_cmd (char *cmd)
{
  FILE *fp;
  static char line[254];

  bzero (line, sizeof (line));

  if ((fp = popen (cmd, "r")))
    {
      fgets (line, sizeof (line), fp);
      pclose (fp);
    }

  chmod (line);

  return line;
}


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

/*

void
show_subscriber_table (webs_t wp, char *type, int which)
{

  static char word[256];
  char *next, *wordlist;
  char *user, *pass;
  static char new_user[200], new_pass[200];
  int temp;
  wordlist = nvram_safe_get ("mf_subscribers");
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
          }
        return;
      }
}

*/

#define CHAPSHOW(a,b) sprintf(buffer,a,b); do_ej_buffer(buffer,wp);

void
ej_show_subscribers (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char buffer[1024], *count;
  int c = 0;
  // count = nvram_safe_get ("mf_subscribernum");
  count = 1;
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
      websWrite (wp, "Horst");
      //show_subscriber_table (wp, "user", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      CHAPSHOW
	("<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
	 i);
      websWrite (wp, "Wanschura");
      //show_subscriber_table (wp, "pass", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

void
ej_mf_test (webs_t wp, int argc, char_t ** argv)
{

  websWrite (wp, "Test\n");
  return;
}
