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
ej_dummy (webs_t wp, int argc, char_t ** argv)
{
  FILE *fp;
  char *request;
  char *mfs = "milkfish_services ";
  char line[254];
  char *param;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s", &param);
#else
  if (ejArgs (argc, argv, "%s", &param) != 1)
    return;
#endif

  request = strcat (mfs, param);

  websWrite (wp, "1 = %s\n", param);
  websWrite (wp, "2 = %s\n", request);

  if ((fp = popen (request, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  websWrite (wp, line);
	}
      pclose (fp);
    }

  // if (ejArgs (argc, argv, "%s", &key) < 1)
  //   {
  //     websError (wp, 400, "Insufficient args\n");
  //   }


  return;
}



void
ej_exec_milkfish_service (webs_t wp, int argc, char_t ** argv)
{

  FILE *fp;
  char line[254];
  char *request;

  if (ejArgs (argc, argv, "%s", &request) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
    }

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
ej_show_phonebook (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "phonebook: = %s\n",
	     exec_cmd ("milkfish_services phonebook"));
  return;
}

void
ej_show_ppptime (webs_t wp, int argc, char_t ** argv)
{
  char *ppptime;

  ppptime = nvram_safe_get ("milkfish_ppptime");
  websWrite (wp, "%s", ppptime);
  websWrite (wp, "\n");
  websWrite (wp, "language = %s\n", nvram_safe_get ("language"));
  return;
}
