
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
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>

char service[10];
char disable_ip[20];
char _username[] = "ddns_username_X";
char _passwd[] = "ddns_passwd_X";
char _hostname[] = "ddns_hostname_X";
char _dyndnstype[] = "ddns_dyndnstype_X";
char _wildcard[] = "ddns_wildcard_X";

int
init_ddns (void)
{
  int flag = 0;

  if (nvram_match ("ddns_enable", "0"))
    {				// disable from ui or default
      if (nvram_match ("ddns_enable_buf", "1"))
	{			// before disable is dyndns, so we want to disable dyndns
	  strcpy (service, "dyndns");
	  strcpy (disable_ip, "192.168.1.1");	// send this address to disable dyndns

	  flag = 1;
	}
      else if (nvram_match ("ddns_enable_buf", "2"))
	{			// before disable is tzo, so we want to disable tz
	  strcpy (service, "tzo");
	  strcpy (disable_ip, "0.0.0.0");
	  flag = 2;
	}
      else if (nvram_match ("ddns_enable_buf", "3"))
	{			// before disable is zoneedit, so we want to disable zoneedit
	  strcpy (service, "zoneedit");
	  strcpy (disable_ip, "0.0.0.0");
	  flag = 3;
	}
      else
	return -1;		// default 
    }
  else if (nvram_match ("ddns_enable", "1"))
    {
      if (nvram_match ("ddns_dyndnstype", "2"))
	strcpy (service, "dyndns-static");
      else if (nvram_match ("ddns_dyndnstype", "3"))
	strcpy (service, "dyndns-custom");
      else
	strcpy (service, "dyndns");
      flag = 1;
    }
  else if (nvram_match ("ddns_enable", "2"))
    {
      strcpy (service, "tzo");
      flag = 2;
    }
  else if (nvram_match ("ddns_enable", "3"))
    {
      strcpy (service, "zoneedit");
      flag = 3;
    }
  else if (nvram_match ("ddns_enable", "4"))
    {
      strcpy (service, "easydns");
      flag = 4;
    }

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

  return 0;
}

int
start_ddns (void)
{
  int ret;
  FILE *fp;
  pid_t pid;
  char string[80] = "";

  /* Get correct username, password and hostname */
  if (init_ddns () < 0)
    return -1;

  /* We don't want to update, if user don't input below field */
  if (nvram_match (_username, "") ||
      nvram_match (_passwd, "") || nvram_match (_hostname, ""))
    return -1;


  /* We want to re-update if user change some value from UI */
  if (strcmp (nvram_safe_get ("ddns_enable_buf"), nvram_safe_get ("ddns_enable")) ||	// ddns mode change
      strcmp (nvram_safe_get ("ddns_username_buf"), nvram_safe_get (_username)) ||	// ddns username chane
      strcmp (nvram_safe_get ("ddns_passwd_buf"), nvram_safe_get (_passwd)) ||	// ddns password change
      strcmp (nvram_safe_get ("ddns_hostname_buf"), nvram_safe_get (_hostname)) ||	// ddns hostname change
      strcmp (nvram_safe_get ("ddns_dyndnstype_buf"), nvram_safe_get (_dyndnstype)) ||	// ddns dyndnstype change
      strcmp (nvram_safe_get ("ddns_wildcard_buf"),
	      nvram_safe_get (_wildcard)))
    {				// ddns wildcard change
      cprintf ("Some value had been changed , need to update\n");

      if (nvram_match ("action_service", "ddns")
	  || !file_to_buf ("/tmp/ddns_msg", string, sizeof (string)))
	{
	  cprintf ("Upgrade from UI or first time\n");
	  nvram_unset ("ddns_cache");	// The will let program to re-update
	  unlink ("/tmp/ddns_msg");	// We want to get new message
	}
    }

  /* Some message we want to stop to update */
  if (file_to_buf ("/tmp/ddns_msg", string, sizeof (string)))
    {
      cprintf ("string=[%s]\n", string);
      if (strcmp (string, "") &&
	  !strstr (string, "_good") &&
	  !strstr (string, "noupdate") &&
	  !strstr (string, "nochg") && !strstr (string, "all_"))
	{
	  cprintf ("Last update have error message : %s, don't re-update\n",
		   string);
	  return -1;
	}
    }

  if (nvram_match ("ddns_enable", "0")
      && nvram_invmatch ("action_service", "ddns"))
    return -1;

  /* Generate ddns configuration file */
  if ((fp = fopen ("/tmp/ddns.conf", "w")))
    {
      fprintf (fp, "service-type=%s\n", service);
      fprintf (fp, "user=%s:%s\n", nvram_safe_get (_username),
	       nvram_safe_get (_passwd));
      fprintf (fp, "host=%s\n", nvram_safe_get (_hostname));

      if (nvram_match ("ddns_enable", "0"))
	{
	  fprintf (fp, "address=%s\n", disable_ip);	// send error ip address
	}
      else
	{
#ifdef HAVE_NEWMEDIA
	  if (nvram_match ("pptpd_client_enable", "1"))
	    {
	      fprintf (fp, "address=%s\n",
		       nvram_safe_get ("pptpd_client_info_localip"));
	    }
	  else
	    {
#endif
	      if (nvram_match ("wan_proto", "pptp"))
		fprintf (fp, "address=%s\n", nvram_safe_get ("pptp_get_ip"));
	      else if (nvram_match ("wan_proto", "l2tp"))
		fprintf (fp, "address=%s\n", nvram_safe_get ("l2tp_get_ip"));
	      else
		fprintf (fp, "address=%s\n", nvram_safe_get ("wan_ipaddr"));
#ifdef HAVE_NEWMEDIA
	    }
#endif
	}
      if (nvram_match ("ddns_wildcard", "1")
	  && nvram_match ("ddns_enable", "1"))
	fprintf (fp, "wildcard=yes\n");
      if (nvram_match ("ddns_enable", "1"))
	fprintf (fp, "max-interval=604800\n");
      fclose (fp);
    }
  else
    {
      perror ("/tmp/ddns.conf");
      return -1;
    }

  /* Restore cache data to file */
  if (nvram_invmatch ("ddns_enable", ""))
    nvram2file ("ddns_cache", "/tmp/ddns.cache");

  char *argv[] = { "ez-ipupdate",
    "-i", get_wan_face (),
    "-D",
    "-P", "3600",
    "-e", "ddns_success",
    "-c", "/tmp/ddns.conf",
    "-b", "/tmp/ddns.cache",
    NULL
  };

  ret = _eval (argv, ">/dev/console", 0, &pid);

  cprintf ("done\n");

  return ret;
}

int
stop_ddns (void)
{
  int ret;

  ret = eval ("killall", "-9", "ez-ipupdate");

  cprintf ("done\n");

  return ret;
}



int
ddns_success_main (int argc, char *argv[])
{
  char buf[80];

  init_ddns ();

  snprintf (buf, sizeof (buf), "%ld,%s", time (NULL), argv[1]);

  nvram_set ("ddns_cache", buf);
  nvram_set ("ddns_status", "1");
  nvram_set ("ddns_enable_buf", nvram_safe_get ("ddns_enable"));
  nvram_set ("ddns_username_buf", nvram_safe_get (_username));
  nvram_set ("ddns_passwd_buf", nvram_safe_get (_passwd));
  nvram_set ("ddns_hostname_buf", nvram_safe_get (_hostname));
  nvram_set ("ddns_dyndnstype_buf", nvram_safe_get (_dyndnstype));
  nvram_set ("ddns_wildcard_buf", nvram_safe_get (_wildcard));
  nvram_set ("ddns_change", "");

  nvram_commit ();

  cprintf ("done\n");

  return 0;
}
