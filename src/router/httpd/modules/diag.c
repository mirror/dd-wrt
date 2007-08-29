
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <broadcom.h>


void
diag_ping_start (webs_t wp)
{
  char *ip = websGetVar (wp, "ping_ip", NULL);

  if (!ip  || !strcmp (ip, ""))
    return;

  unlink (PING_TMP);
  nvram_set ("ping_ip", ip);

  char cmd[256] = { 0 };

  setenv ("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);

  snprintf (cmd, sizeof (cmd),
	    "alias ping=\'ping -c 3\'; eval \"%s\" > %s 2>&1 &", ip,
	    PING_TMP);
  system (cmd);

  return;
}


void
removeLineBreak (char *startup)
{
  int i = 0;
  int c = 0;
  for (i = 0; i < strlen (startup); i++)
    {
      if (startup[i] == '\r')
	continue;
      startup[c++] = startup[i];
    }
  startup[c++] = 0;

}

void
ping_startup (webs_t wp)
{
  char *startup = websGetVar (wp, "ping_ip", NULL);

  // filter Windows <cr>ud
  removeLineBreak (startup);

  nvram_set ("rc_startup", startup);
  nvram_commit ();
  nvram2file ("rc_startup", "/tmp/.rc_startup");
  chmod ("/tmp/.rc_startup", 0700);

  return;

}

void
ping_firewall (webs_t wp)
{
  char *firewall = websGetVar (wp, "ping_ip", NULL);

  // filter Windows <cr>ud
  removeLineBreak (firewall);
  nvram_set ("rc_firewall", firewall);
  nvram_commit ();
  nvram2file ("rc_firewall", "/tmp/.rc_firewall");
  chmod ("/tmp/.rc_firewall", 0700);

  return;
}

void
ping_custom (webs_t wp)
{
  char *custom = websGetVar (wp, "ping_ip", NULL);

  // filter Windows <cr>ud
  unlink ("/tmp/custom.sh");
  removeLineBreak (custom);
  nvram_set ("rc_custom", custom);
  nvram_commit ();
  if (nvram_invmatch ("rc_custom", ""))
    {
      nvram2file ("rc_custom", "/tmp/custom.sh");
      chmod ("/tmp/custom.sh", 0700);
    }

  return;
}

void
ping_wol (webs_t wp)
{
  char *wol_type = websGetVar (wp, "wol_type", NULL);

  unlink (PING_TMP);

  if (!wol_type || !strcmp (wol_type, ""))
    return;

  if (!strcmp (wol_type, "update"))
    {
      char *wol_hosts = websGetVar (wp, "wol_hosts", NULL);
      if (!wol_hosts || !strcmp (wol_hosts, ""))
	return;

      nvram_set ("wol_hosts", wol_hosts);
      nvram_set ("wol_cmd", "");
      return;
    }

  char *manual_wol_mac = websGetVar (wp, "manual_wol_mac", NULL);
  char *manual_wol_network = websGetVar (wp, "manual_wol_network", NULL);
  char *manual_wol_port = websGetVar (wp, "manual_wol_port", NULL);

  if (!strcmp (wol_type, "manual"))
    {
      nvram_set ("manual_wol_mac", manual_wol_mac);
      nvram_set ("manual_wol_network", manual_wol_network);
      nvram_set ("manual_wol_port", manual_wol_port);
    }

  char wol_cmd[256] = { 0 };
  snprintf (wol_cmd, sizeof (wol_cmd), "/usr/sbin/wol -v -i %s -p %s %s",
	    manual_wol_network, manual_wol_port, manual_wol_mac);
  nvram_set ("wol_cmd", wol_cmd);

  // use Wol.asp as a debugging console
  char cmd[256] = { 0 };
  snprintf (cmd, sizeof (cmd), "%s > %s 2>&1 &", wol_cmd, PING_TMP);
  system2 (cmd);

}

void
diag_ping_stop (webs_t wp)
{
  killall ("ping", SIGKILL);
}

void
diag_ping_clear (webs_t wp)
{
  unlink (PING_TMP);
}

void
ping_onload (webs_t wp, char *arg)
{
  int pid;
  char *type = websGetVar (wp, "submit_type", "");

  pid = find_pid_by_ps ("ping");

  if (pid > 0 && strncmp (type, "stop", 4))
    {				// pinging
      websWrite (wp, arg);
    }

}


void
ej_dump_ping_log (webs_t wp, int argc, char_t ** argv)
{
  int count = 0;
  FILE *fp;
  char line[254];
  char newline[300];
  int i;

/* wait as long file size increases, but max. 10 s)*/
  int c, count1 = 0, count2 = -1, timeout = 0;

  while ((count1 > count2) && (timeout < 5))
    {
      count2 = count1;
      count1 = 0;

      if ((fp = fopen (PING_TMP, "r")) != NULL)
	{
	  c = fgetc (fp);
	  while (c != EOF)
	    {
	      count1++;
	      c = fgetc (fp);
	    }
	  fclose (fp);
	  timeout++;
	  sleep (2);
	}
    }
/* end waiting */

  if ((fp = fopen (PING_TMP, "r")) != NULL)
    {				// show result
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  line[strlen (line) - 1] = '\0';
	  if (!strcmp (line, ""))
	    continue;
	  int nc = 0;
	  for (i = 0; i < strlen (line) + 1; i++)
	    {
	      if (line[i] == '"')
		{
		  newline[nc++] = '&';
		  newline[nc++] = 'q';
		  newline[nc++] = 'u';
		  newline[nc++] = 'o';
		  newline[nc++] = 't';
		  newline[nc++] = ';';
		}
	      else
		newline[nc++] = line[i];
	    }
	  newline[nc++] = 0;
	  websWrite (wp, "%c\"%s\"\n", count ? ',' : ' ', newline);
	  count++;
	}
      fclose (fp);
    }

  unlink (PING_TMP);
    
  return;
}

/* OBSOLETE
void
ej_dump_traceroute_log (int eid, webs_t wp, int argc, char_t ** argv)
{
  int count = 0;
  FILE *fp;
  char line[254];

  if ((fp = fopen (TRACEROUTE_TMP, "r")) != NULL)
    {				// show result
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  line[strlen (line) - 1] = '\0';
	  if (!strcmp (line, ""))
	    continue;
	  websWrite (wp, "%c\"%s\"\n", count ? ',' : ' ', line);
	  count++;
	}
      fclose (fp);
    }

  return;
}
END OBSOLETE */
