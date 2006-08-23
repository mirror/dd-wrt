
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>

/* inadyn scripts by lawnmowerguy1 */

char service[32];
char _username[] = "ddns_username_X";
char _passwd[] = "ddns_passwd_X";
char _hostname[] = "ddns_hostname_X";
char _dyndnstype[] = "ddns_dyndnstype_X";
char _wildcard[] = "ddns_wildcard_X";
char _conf[] = "ddns_conf_X";

int
init_ddns (void)
{
  int flag = 0;

  if (nvram_match ("ddns_enable", "0"))
    return -1;

  else if (nvram_match ("ddns_enable", "1"))
    {
      if (nvram_match ("ddns_dyndnstype", "2"))
	strcpy (service, "statdns@dyndns.org");
      else if (nvram_match ("ddns_dyndnstype", "3"))
	strcpy (service, "custom@dyndns.org");
      else
	strcpy (service, "dyndns@dyndns.org");
      flag = 1;
    }
  else if (nvram_match ("ddns_enable", "2"))
    {
      strcpy (service, "default@freedns.afraid.org");
      flag = 2;
    }
  else if (nvram_match ("ddns_enable", "3"))
    {
      strcpy (service, "default@zoneedit.com");
      flag = 3;
    }
  else if (nvram_match ("ddns_enable", "4"))
    {
      strcpy (service, "default@no-ip.com");
      flag = 4;
    }
  else if (nvram_match ("ddns_enable", "5"))
    {
      strcpy (service, "custom@http_svr_basic_auth");
      flag = 5;
    }
    /* botho 30/07/06 : add www.3322.org */
  else if (nvram_match ("ddns_enable", "6"))
    {
      strcpy (service, "dyndns@3322.org");
      flag = 6;
    }
  else if (nvram_match ("ddns_enable", "7"))
    {
      strcpy (service, "default@easydns.com");
      flag = 7;
    }

/* botho 30/07/06 : add www.3322.org */
  if (flag == 1)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname");
      snprintf (_dyndnstype, sizeof (_dyndnstype), "%s", "ddns_dyndnstype");
      snprintf (_wildcard, sizeof (_wildcard), "%s", "ddns_wildcard");
    }
  else if (flag == 2)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_2");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_2");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_2");
    }
  else if (flag == 3)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_3");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_3");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_3");
    }
  else if (flag == 4)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_4");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_4");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_4");
    }
  else if (flag == 5)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_5");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_5");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_5");
      snprintf (_conf, sizeof (_conf), "%s", "ddns_conf");
    }
  else if (flag == 6)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_6");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_6");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_6");
      snprintf (_dyndnstype, sizeof (_dyndnstype), "%s", "ddns_dyndnstype_6");
      snprintf (_wildcard, sizeof (_wildcard), "%s", "ddns_wildcard_6");
    }
  else if (flag == 7)
    {
      snprintf (_username, sizeof (_username), "%s", "ddns_username_7");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd_7");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname_7");
    }

  return 0;
}

int
start_ddns (void)
{
  int ret;
  int i;
  FILE *fp;

  /* Get correct username, password and hostname */
  if (init_ddns () < 0)
    return -1;

  /* We don't want to update, if user don't input below field */
  if (nvram_match (_username, "") ||
      nvram_match (_passwd, "") || nvram_match (_hostname, ""))
    return -1;

  mkdir ("/tmp/ddns", 0744);

  if (strcmp (nvram_safe_get ("ddns_enable_buf"), nvram_safe_get ("ddns_enable")) ||	// ddns mode change
      strcmp (nvram_safe_get ("ddns_username_buf"), nvram_safe_get (_username)) ||	// ddns username chane
      strcmp (nvram_safe_get ("ddns_passwd_buf"), nvram_safe_get (_passwd)) ||	// ddns password change
      strcmp (nvram_safe_get ("ddns_hostname_buf"), nvram_safe_get (_hostname)) ||	// ddns hostname change
      strcmp (nvram_safe_get ("ddns_dyndnstype_buf"), nvram_safe_get (_dyndnstype)) ||	// ddns dyndnstype change
      strcmp (nvram_safe_get ("ddns_wildcard_buf"), nvram_safe_get (_wildcard)) || // ddns wildcard change
      strcmp (nvram_safe_get ("ddns_conf_buf"), nvram_safe_get (_conf)) || // ddns conf change
      strcmp (nvram_safe_get ("ddns_custom_5_buf"), nvram_safe_get ("ddns_custom_5")))
    {
      /* If the user changed anything in the GUI, delete all cache and log */
      nvram_unset ("ddns_cache");
      nvram_unset ("ddns_time");
      unlink ("/tmp/ddns/ddns.log");
      unlink ("/tmp/ddns/inadyn_ip.cache");
      unlink ("/tmp/ddns/inadyn_time.cache");
    }

  /* Generate ddns configuration file */
  if ((fp = fopen ("/tmp/ddns/inadyn.conf", "w")))
    {
      fprintf (fp, "--background");
      fprintf (fp, " --dyndns_system %s", service);	//service
      if (nvram_match ("ddns_enable", "5"))
	fprintf (fp, " --dyndns_server_name %s", nvram_safe_get("ddns_custom_5"));
      fprintf (fp, " -u %s", nvram_safe_get (_username));	//username/email
      fprintf (fp, " -p %s", nvram_safe_get (_passwd));	// password
      fprintf (fp, " -a %s", nvram_safe_get (_hostname));	// alias/hostname
      if (nvram_match ("ddns_wildcard", "1") && nvram_match ("ddns_enable", "1"))
	fprintf (fp, ",wildcard=ON");
      if (nvram_match ("ddns_wildcard_6", "1") && nvram_match ("ddns_enable", "6"))
	fprintf (fp, ",wildcard=ON");
      fprintf (fp, " --update_period_sec %s", "360");	// check ip every 6 mins
      fprintf (fp, " --forced_update_period %s", "2419200");	//force update after 28days
      fprintf (fp, " --log_file %s", "/tmp/ddns/ddns.log");	//log to file
      fprintf (fp, " --exec %s", "ddns_success");	//run after update
      if (nvram_invmatch ("ddns_conf", "")
	  && nvram_match ("ddns_enable", "5"))
	{
	  fprintf (fp, " %s", nvram_safe_get (_conf));
	}
      fprintf (fp, "\n"); 
      fclose (fp);
    }
  else
    {
      perror ("/tmp/ddns/inadyn.conf");
      return -1;
    }

  /* Restore cache data to file from NV */
  if (nvram_invmatch ("ddns_cache", "")
      && nvram_invmatch ("ddns_time", ""))
    {
      nvram2file ("ddns_cache", "/tmp/ddns/inadyn_ip.cache");
      nvram2file ("ddns_time", "/tmp/ddns/inadyn_time.cache");
    }

  ret = eval ("inadyn", "--input_file", "/tmp/ddns/inadyn.conf");

  cprintf ("done\n");

  return ret;
}

int
stop_ddns (void)
{
  int ret;

  unlink ("/tmp/ddns/ddns.log");
  ret = eval ("killall", "inadyn");

  cprintf ("done\n");

  return ret;
}



int
ddns_success_main (int argc, char *argv[])
{
  char buf[80];
  char buf2[80];
  FILE *fp;

  init_ddns ();

  if ((fp = fopen ("/tmp/ddns/inadyn_ip.cache", "r")))
    {
      fgets (buf, sizeof (buf),fp);
      fclose(fp);
      nvram_set ("ddns_cache", buf);
    }

  if ((fp = fopen ("/tmp/ddns/inadyn_time.cache", "r")))
    {
      fgets (buf2, sizeof (buf2),fp);
      fclose(fp);
      nvram_set ("ddns_time", buf2);
    }

  nvram_set ("ddns_enable_buf", nvram_safe_get ("ddns_enable"));
  nvram_set ("ddns_username_buf", nvram_safe_get (_username));
  nvram_set ("ddns_passwd_buf", nvram_safe_get (_passwd));
  nvram_set ("ddns_hostname_buf", nvram_safe_get (_hostname));
  nvram_set ("ddns_dyndnstype_buf", nvram_safe_get (_dyndnstype));
  nvram_set ("ddns_wildcard_buf", nvram_safe_get (_wildcard));
  nvram_set ("ddns_conf_buf", nvram_safe_get (_conf));
  nvram_set ("ddns_custom_5_buf", nvram_safe_get ("ddns_custom_5"));

  nvram_commit ();

  cprintf ("done\n");

  return 0;
}
