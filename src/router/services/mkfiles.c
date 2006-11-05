/*
 * mkfiles.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utils.h>
#include <bcmnvram.h>

#define HOME_DIR	"/tmp/root"
#define PASSWD_DIR	"/tmp/etc"
#define SSH_DIR		"/tmp/root/.ssh"
#define PASSWD_FILE	"/tmp/etc/passwd"
#define GROUP_FILE	"/tmp/etc/group"

#define NOCAT_CONF      "/tmp/etc/nocat.conf"

/* BPsmythe: Return the local network for the NOCAT conf file */
static char *
_get_network (char *ipaddr, char *snmask)
{
  u_long ipaddr2long (char *ipstr)
  {
    int ip[4];
    char *tmp = malloc (4 * sizeof (char));

    ip[0] = atoi (strncpy (tmp, ipstr, strcspn (ipstr, ".")));
    ipstr = strstr (ipstr, ".");
    ipstr++;
    strcpy (tmp, "    ");
    ip[1] = atoi (strncpy (tmp, ipstr, strcspn (ipstr, ".")));
    ipstr = strstr (ipstr, ".");
    ipstr++;
    strcpy (tmp, "    ");
    ip[2] = atoi (strncpy (tmp, ipstr, strcspn (ipstr, ".")));
    ipstr = strstr (ipstr, ".");
    ipstr++;
    strcpy (tmp, "    ");
    ip[3] = atoi (ipstr);

    free (tmp);
    return ((ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3]);
  }

  char *long2ipaddr (u_long addr)
  {
    static char buff[32];

    sprintf (buff, "%ld.%ld.%ld.%ld",
	     (addr >> 24 & 0xff),
	     (addr >> 16 & 0xff), (addr >> 8 & 0xff), (addr & 0xff));

    return buff;
  }


  static char network[32];

  strcpy (network, long2ipaddr (ipaddr2long (ipaddr) & ipaddr2long (snmask)));

  return network;
}

/* end BPsmythe */




int
start_mkfiles (void)
{
  FILE *fp;
  struct stat buf;
#ifdef HAVE_SKYTRON
  char *http_passwd = nvram_safe_get ("skyhttp_passwd");
#elif HAVE_NEWMEDIA
  char *http_passwd = nvram_safe_get ("newhttp_passwd");
#elif HAVE_34TELECOM
  char *http_passwd = nvram_safe_get ("newhttp_passwd");
#else
  char *http_passwd = nvram_safe_get ("http_passwd");
#endif
  char *cp;

  if (stat (HOME_DIR, &buf) != 0)
    {
      mkdir (HOME_DIR, 0700);
    }

#ifdef HAVE_SSHD
  if (stat (SSH_DIR, &buf) != 0)
    {
      mkdir (SSH_DIR, 0700);
    }
#endif

  /* Create password's and group's database directory */
  if (stat (PASSWD_DIR, &buf) != 0)
    {
      mkdir (PASSWD_DIR, 0700);
    }

//#ifdef HAVE_FREEBIRD
//      cp = "bJEt.IiWoP9G2";
//#else
//  cp = (char *) zencrypt (http_passwd);       /* encrypt password */
//#endif
  /* Write password file with username root and password */
  if (!(fp = fopen (PASSWD_FILE, "w")))
    {
      perror (PASSWD_FILE);
      return errno;
    }
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    {
      fprintf (fp, "root:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n",
	       http_passwd);
      fprintf (fp, "reboot:%s:0:0:Root User,,,:/tmp/root:/sbin/reboot\n",
	       http_passwd);
      fclose (fp);
    }
  /* Write group file with group 'root' */
  if (!(fp = fopen (GROUP_FILE, "w")))
    {
      perror (GROUP_FILE);
      return errno;
    }
  fprintf (fp, "root:x:0:\n");
  fclose (fp);

  system2 ("/bin/mkdir -p /var/spool");
  system2 ("/bin/mkdir -p /var/spool/cron");
  system2 ("/bin/mkdir -p /var/spool/cron/crontabs");
  system2 ("/bin/touch /var/spool/cron/crontabs/root");
  system2 ("/bin/mkdir -p /var/lib");
  system2 ("/bin/mkdir -p /var/lib/misc");
  system2 ("/bin/mkdir -p /var/tmp");

  system2 ("/bin/mkdir -p /var/log");
  system2 ("/bin/touch -p /var/log/messages");

#ifdef HAVE_SNMP
  system2 ("/bin/mkdir -p /var/snmp");
#endif
  system2 ("/bin/chmod 0777 /tmp");

  dns_to_resolv ();

  return 0;
}

#ifdef HAVE_NOCAT
int
mk_nocat_conf (void)
{
  FILE *fp;
  /* BPsmythe: Write out a nocat.conf file */
  if (!(fp = fopen (NOCAT_CONF, "w")))
    {
      perror (NOCAT_CONF);
      return errno;
    }

  fprintf (fp, "#\n");

  /* settings that need to be set based on router configurations */
  /* These are now autodetected on WRT54G via: lan_ifname and wan_ifname */
  /*fprintf(fp, "InternalDevice\t%s\n", nvram_safe_get("lan_ifname")); */
  /*fprintf(fp, "ExternalDevice\t%s\n", nvram_safe_get("wan_ifname")); */
  /*
   * fprintf(fp, "InternalDevice\t%s\n", nvram_safe_get("NC_InternalDevice") ); 
   * fprintf(fp, "ExternalDevice\t%s\n", nvram_safe_get("NC_ExternalDevice") ); 
   * // InsideIP is now depreciated, use GatewayAddr
   * fprintf(fp, "InsideIP\t%s\n", nvram_safe_get("lan_ipaddr")); 
   * fprintf(fp, "LocalNetwork\t%s/%s\n", 
   get_network(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask")),
   nvram_safe_get("lan_netmask") );
   */
  /* These are now hardcoded as the defaults */
  //fprintf(fp, "SplashForm\t%s\n", "splash.html");
  //fprintf(fp, "StatusForm\t%s\n", "status.html");

  fprintf (fp, "RouteOnly\t%s\n", nvram_safe_get ("NC_RouteOnly"));

  /* These are user defined, eventually via the web page */
  fprintf (fp, "Verbosity\t%s\n", nvram_safe_get ("NC_Verbosity"));
  fprintf (fp, "GatewayName\t%s\n", nvram_safe_get ("NC_GatewayName"));
  fprintf (fp, "GatewayAddr\t%s\n", nvram_safe_get ("lan_ipaddr"));
  fprintf (fp, "GatewayPort\t%s\n", nvram_safe_get ("NC_GatewayPort"));
  fprintf (fp, "GatewayMAC\t%s\n", nvram_safe_get ("et0macaddr"));
  fprintf (fp, "GatewayPassword\t%s\n", nvram_safe_get ("NC_Password"));
  fprintf (fp, "GatewayMode\t%s\n", nvram_safe_get ("NC_GatewayMode"));
  fprintf (fp, "DocumentRoot\t%s\n", nvram_safe_get ("NC_DocumentRoot"));
  if (nvram_invmatch ("NC_SplashURL", ""))
    {
      fprintf (fp, "SplashURL\t%s\n", nvram_safe_get ("NC_SplashURL"));
      fprintf (fp, "SplashURLTimeout\t%s\n",
	       nvram_safe_get ("NC_SplashURLTimeout"));
    }
  fprintf (fp, "LeaseFile\t%s\n", nvram_safe_get ("NC_LeaseFile"));

  /* Open-mode and common options */
  fprintf (fp, "FirewallPath\t%s\n", "/usr/libexec/nocat/");
  fprintf (fp, "ExcludePorts\t%s\n", nvram_safe_get ("NC_ExcludePorts"));
  fprintf (fp, "IncludePorts\t%s\n", nvram_safe_get ("NC_IncludePorts"));
  fprintf (fp, "AllowedWebHosts\t%s %s\n", nvram_safe_get ("lan_ipaddr"),
	   nvram_safe_get ("NC_AllowedWebHosts"));
  /* TJaqua: Added MACWhiteList to ignore given machines or routers on the local net (e.g. routers with an alternate Auth). */
  fprintf (fp, "MACWhiteList\t%s\n", nvram_safe_get ("NC_MACWhiteList"));
  /* TJaqua: Added AnyDNS to pass through any client-defined servers. */
  if (!strcmp (nvram_safe_get ("NC_AnyDNS"), "1"))
    {
      fprintf (fp, "AnyDNS\t%s\n", nvram_safe_get ("NC_AnyDNS"));
    }
  else
    {
      /* Irving - Rework getting DNS */
      struct dns_lists *dns_list = NULL;
      dns_list = get_dns_list ();
      if (!dns_list || dns_list->num_servers == 0)
	{
	  fprintf (fp, "DNSAddr \t%s\n", nvram_safe_get ("lan_ipaddr"));
	}
      else
	{
	  fprintf (fp, "DNSAddr \t%s %s %s\n", dns_list->dns_server[0],
		   dns_list->dns_server[1], dns_list->dns_server[2]);
	}
    }
  fprintf (fp, "HomePage\t%s\n", nvram_safe_get ("NC_HomePage"));
  fprintf (fp, "ForcedRedirect\t%s\n", nvram_safe_get ("NC_ForcedRedirect"));
  fprintf (fp, "PeerCheckTimeout\t%s\n",
	   nvram_safe_get ("NC_PeerChecktimeout"));
  fprintf (fp, "IdleTimeout\t%s\n", nvram_safe_get ("NC_IdleTimeout"));
  fprintf (fp, "MaxMissedARP\t%s\n", nvram_safe_get ("NC_MaxMissedARP"));
  fprintf (fp, "LoginTimeout\t%s\n", nvram_safe_get ("NC_LoginTimeout"));
  fprintf (fp, "RenewTimeout\t%s\n", nvram_safe_get ("NC_RenewTimeout"));

  /* defined for RADIUS 
     fprintf(fp, "AuthServiceAddr\t%s\n", nvram_safe_get("NC_AuthServiceAddr") );
     fprintf(fp, "LoginPage\t%s\n", nvram_safe_get("NC_LoginPage") );
     fprintf(fp, "ConfirmPage\t%s\n", nvram_safe_get("NC_ConfirmPage") );
     fprintf(fp, "LogoutPage\t%s\n", nvram_safe_get("NC_LogoutPage") );
     fprintf(fp, "RADIUSAuthServer\t%s\n", nvram_safe_get("NC_RADIUSAuthServer") );
     fprintf(fp, "RADIUSAuthPort\t%s\n", nvram_safe_get("NC_RADIUSAuthPort") );
     fprintf(fp, "RADIUSAuthSecret\t%s\n", nvram_safe_get("NC_RADIUSAuthSecret") );
     fprintf(fp, "RADIUSAuthNASIdentifier\t%s\n", nvram_safe_get("NC_RADIUSAuthNASIdentifier") );
     fprintf(fp, "RADIUSAuthWait\t%s\n", nvram_safe_get("NC_RADIUSAuthWait") );
     fprintf(fp, "RADIUSAuthRetries\t%s\n", nvram_safe_get("NC_RADIUSAuthRetries") );
     fprintf(fp, "RADIUSAcctServer\t%s\n", nvram_safe_get("NC_RADIUSAcctServer") );
     fprintf(fp, "RADIUSAcctPort\t%s\n", nvram_safe_get("NC_RADIUSAcctPort") );
     fprintf(fp, "RADIUSAcctSecret\t%s\n", nvram_safe_get("NC_RADIUSAcctSecret") );
     fprintf(fp, "RADIUSAcctNASIdentifier\t%s\n", nvram_safe_get("NC_RADIUSAcctNASIdentifier") );
     fprintf(fp, "RADIUSAcctWait\t%s\n", nvram_safe_get("NC_RADIUSAcctWait") );
     fprintf(fp, "RADIUSAcctRetries\t%s\n", nvram_safe_get("NC_RADIUSAcctRetries") );
   */

  /* defined for second radius server
     fprintf(fp, "RADIUSAuth1Server\t%s\n", nvram_safe_get("NC_RADIUSAuth1Server") );
     fprintf(fp, "RADIUSAuth1Port\t%s\n", nvram_safe_get("NC_RADIUSAuth1Port") );
     fprintf(fp, "RADIUSAuth1Secret\t%s\n", nvram_safe_get("NC_RADIUSAuth1Secret") );
     fprintf(fp, "RADIUSAuth1NASIdentifier\t%s\n", nvram_safe_get("NC_RADIUSAuth1NASIdentifier") );
     fprintf(fp, "RADIUSAcct1Server\t%s\n", nvram_safe_get("NC_RADIUSAcct1Server") );
     fprintf(fp, "RADIUSAcct1Port\t%s\n", nvram_safe_get("NC_RADIUSAcct1Port") );
     fprintf(fp, "RADIUSAcct1Secret\t%s\n", nvram_safe_get("NC_RADIUSAcct1Secret") );
     fprintf(fp, "RADIUSAcct1NASIdentifier\t%s\n", nvram_safe_get("NC_RADIUSAcct1NASIdentifier") );
   */

  fclose (fp);
  /* end BPsmythe */
  fprintf (stderr, "Wrote: %s\n", NOCAT_CONF);

  return 0;
}
#endif
