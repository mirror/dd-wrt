
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


int
diag_ping_start (webs_t wp)
{
  int ret = 0;
  char *ip = websGetVar (wp, "ping_ip", NULL);
  char *times = websGetVar (wp, "ping_times", NULL);

  if (!ip || !times || !strcmp (ip, ""))
    return ret;
  unlink (PING_TMP);
  nvram_set ("ping_ip", ip);
  nvram_set ("ping_times", times);


  // The web will hold, so i move to service.c
  //snprintf(cmd, sizeof(cmd), "ping -c %s %s &", times, ip);
  //cprintf("cmd=[%s]\n",cmd);
  //system(cmd);

  return ret;
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

int
ping_startup (webs_t wp)
{
  char *startup = websGetVar (wp, "ping_ip", NULL);

  // filter Windows <cr>ud
  removeLineBreak (startup);

  nvram_set ("rc_startup", startup);
  nvram2file ("rc_startup", "/tmp/.rc_startup");
  chmod ("/tmp/.rc_startup", 0700);
//  diag_ping_start(wp);

  return 0;

}

int
ping_firewall (webs_t wp)
{
  char *firewall = websGetVar (wp, "ping_ip", NULL);

  // filter Windows <cr>ud
  removeLineBreak (firewall);
  nvram_set ("rc_firewall", firewall);
  nvram2file ("rc_firewall", "/tmp/.rc_firewall");
  chmod ("/tmp/.rc_firewall", 0700);
  //diag_ping_start(wp);

  return 0;
}

int
ping_wol (webs_t wp)
{
  int ret = 0;
  char *wol_type = websGetVar (wp, "wol_type", NULL);

  unlink (PING_TMP);

  if (!wol_type || !strcmp (wol_type, ""))
    return ret;

  if (!strcmp (wol_type, "update"))
    {
      char *wol_hosts = websGetVar (wp, "wol_hosts", NULL);

      if (!wol_hosts || !strcmp (wol_hosts, ""))
	return ret;

      // filter Windows <cr>ud
      // removeLineBreak (wol_hosts);
      nvram_set ("wol_hosts", wol_hosts);
      nvram_set ("wol_cmd", "");
      return ret;
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
  system (cmd);

  return ret;
}

int
diag_ping_stop (webs_t wp)
{
  return killall("ping",SIGKILL);
}

int
diag_ping_clear (webs_t wp)
{
  return unlink (PING_TMP);
}

int
ping_onload (webs_t wp, char *arg)
{
  int ret = 0;
  int pid;
  char *type = websGetVar (wp, "submit_type", "");

  pid = find_pid_by_ps ("ping");

  if (pid > 0 && strncmp (type, "stop", 4))
    {				// pinging
      websWrite (wp, arg);
    }

  return ret;
}
/* OBSOLETE
int
diag_traceroute_start (webs_t wp)
{
  int ret = 0;
  char *ip = websGetVar (wp, "traceroute_ip", NULL);

  if (!ip || !strcmp (ip, ""))
    return ret;

  unlink (TRACEROUTE_TMP);
  nvram_set ("traceroute_ip", ip);

  return ret;
}

int
diag_traceroute_stop (webs_t wp)
{
  return eval ("killall", "-9", "traceroute");
}

int
diag_traceroute_clear (webs_t wp)
{
  return unlink (TRACEROUTE_TMP);
}

int
traceroute_onload (webs_t wp, char *arg)
{
  int ret = 0;
  int pid;
  char *type = websGetVar (wp, "submit_type", "");

  pid = find_pid_by_ps ("traceroute");

  if (pid > 0 && strncmp (type, "stop", 4))
    {				// tracerouting
      websWrite (wp, arg);
    }

  return ret;
}
END OBSOLETE */

void
ej_dump_ping_log (int eid, webs_t wp, int argc, char_t ** argv)
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
		c = fgetc(fp);
			while (c != EOF)
			{
				count1++;
				c = fgetc(fp);
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
	fclose(fp);
    }


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