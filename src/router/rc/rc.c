/**********************************************************
 *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

/*
 * Router rc control script
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: rc.c,v 1.12 2005/11/30 11:54:21 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <dirent.h>

#include <epivers.h>
#include <bcmnvram.h>
#include <mtd.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmdevs.h>

#include <wlutils.h>
#include <utils.h>
#include <cyutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <support.h>
#include <mkfiles.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <sveasoft.h>
#include <cy_conf.h>
#include <ledcontrol.h>

#define WL_IOCTL(name, cmd, buf, len) (ret = wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28

static void restore_defaults (void);
static void sysinit (void);
static void rc_signal (int sig);

static int check_cfe_nv (void);
static int check_pmon_nv (void);
static void unset_nvram (void);
static int init_nvram (void);
static int check_image (void);

extern struct nvram_tuple router_defaults[];

#if 0
int
endswith (char *str, char *cmp)
{
  int cmp_len, str_len, i;
  cmp_len = strlen (cmp);
  str_len = strlen (str);
  if (cmp_len > str_len)
    return (0);
  for (i = 0; i < cmp_len; i++)
    {
      if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
	return (0);
    }
  return (1);
}
#endif



static int
alreadyInHost (char *host)
{
  FILE *in = fopen ("/tmp/hosts", "rb");
  if (in == NULL)
    return 0;
  char buf[100];
  while (1)
    {
      fscanf (in, "%s", buf);
      if (!strcmp (buf, host))
	{
	  fclose (in);
	  return 1;
	}
      if (feof (in))
	{
	  fclose (in);
	  return 0;
	}
    }
}

void
addHost (char *host, char *ip)
{
  char buf[100];
  char newhost[100];
  if (host == NULL)
    return;
  if (ip == NULL)
    return;
  strcpy (newhost, host);
  char *domain = nvram_safe_get ("lan_domain");
  if (domain != NULL && strlen (domain) > 0 && strcmp (host, "localhost"))
    {
      sprintf (newhost, "%s.%s", host, domain);
    }
  else
    sprintf (newhost, "%s", host);

  if (alreadyInHost (newhost))
    return;
  sprintf (buf, "echo \"%s\t%s\">>/tmp/hosts", ip, newhost);
  system (buf);
}

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif
void
runStartup (char *folder, char *extension)
{
  struct dirent *entry;
  DIR *directory;
  unsigned char *buf;
  buf = malloc (1024);
  directory = opendir (folder);
  if (directory == NULL)
    return;
//list all files in this directory 
  while ((entry = readdir (directory)) != NULL)
    {
      if (endswith (entry->d_name, extension))
	{
	  sprintf (buf, "%s/%s&\n", folder, entry->d_name);
	  //execute script     
	  system (buf);
	}
    }
  free (buf);
  closedir (directory);
}

/* SeG dd-wrt addition for module startup scripts */
int
start_modules ()
{
  runStartup ("/etc/config", ".startup");
  runStartup ("/jffs/etc/config", ".startup");	//if available
  runStartup ("/mmc/etc/config", ".startup");	//if available
  return 0;
}


static void
enableAfterBurner (void)
{

  int boardflags;
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  fprintf (stderr, "boardflags are 0x0%X\n", boardflags);
  if (!(boardflags & BFL_AFTERBURNER))
    {
      boardflags |= BFL_AFTERBURNER;
      char ab[100];
      fprintf (stderr, "enable Afterburner....\n");
      sprintf (ab, "0x0%X", boardflags);
      nvram_set ("boardflags", ab);
      nvram_commit ();
    }
}

/*
int start_rflow()
{
char buffer[256]={0};
system("killall -q rflow");
if (nvram_invmatch("rflow_enable","0"))
{
sprintf(buffer,"rflow -i br0 -F %s:%s&\n",nvram_safe_get("rflow_ip"),nvram_safe_get("rflow_port"));
system(buffer);
}
return 0;
}
int start_macupd()
{
char buffer[256]={0};
system("killall -q macupd");
if (nvram_invmatch("macupd_enable","0"))
{
sprintf(buffer,"macupd %s %s %s&\n",nvram_safe_get("macupd_ip"),nvram_safe_get("macupd_port"),nvram_safe_get("macupd_interval"));
system(buffer);
}
return 0;
}




int start_radius()
{
char buffer[256]={0};
system("killall -q wrt-radauth");
if (strcmp(nvram_safe_get("wl_mode"),"ap")==0)
{
if (nvram_invmatch("wl_radauth","0"))
{
sprintf(buffer,"wrt-radauth -n %s&\n",nvram_safe_get("wl0_ifname"));
system(buffer);
}
}
return 0;
}

//SeG additions
int start_sambafs()
{
char buffer[256]={0};
char *script;
system("umount /tmp/smbshare");
if (nvram_invmatch("samba_mount","0"))
{
mkdir("/tmp/smbshare",0777);
sprintf(buffer,"smbmount %s /tmp/smbshare/ -o username=%s,password=%s",nvram_safe_get("samba_share"),nvram_safe_get("samba_user"),nvram_safe_get("samba_password")); 
system(buffer);
script=nvram_safe_get("samba_script");
if (script!=NULL && strlen(script)>0)
{
sprintf(buffer,"/tmp/smbshare/%s",script);
system(buffer);
}
}
return 0;
} 
*/
// begin Sveasoft addition
int
create_rc_file (char *name)
{
  FILE *fp;
  char *p = nvram_safe_get (name);
  char tmp_file[100] = { 0 };

  if ((void *) 0 == name || 0 == p[0])
    return -1;

  snprintf (tmp_file, 100, "/tmp/.%s", name);
  unlink (tmp_file);

  fp = fopen (tmp_file, "w");
  if (fp)
    {
      // filter Windows <cr>ud
      while (*p)
	{
	  if (*p != 0x0d)
	    fprintf (fp, "%c", *p);
	  p++;
	}
    }
  fclose (fp);
  chmod (tmp_file, 0700);

  return 0;
}

// end Sveasoft addition
static void
ses_cleanup (void)
{
  /* well known event to cleanly initialize state machine */
  nvram_set ("ses_event", "2");

  /* Delete lethal dynamically generated variables */
  nvram_unset ("ses_bridge_disable");
}

static void
ses_restore_defaults (void)
{
  char tmp[100], prefix[] = "wlXXXXXXXXXX_ses_";
  int i;

  /* Delete dynamically generated variables */
  for (i = 0; i < MAX_NVPARSE; i++)
    {
      sprintf (prefix, "wl%d_ses_", i);
      nvram_unset (strcat_r (prefix, "ssid", tmp));
      nvram_unset (strcat_r (prefix, "closed", tmp));
      nvram_unset (strcat_r (prefix, "wpa_psk", tmp));
      nvram_unset (strcat_r (prefix, "auth", tmp));
      nvram_unset (strcat_r (prefix, "wep", tmp));
      nvram_unset (strcat_r (prefix, "auth_mode", tmp));
      nvram_unset (strcat_r (prefix, "crypto", tmp));
      nvram_unset (strcat_r (prefix, "akm", tmp));
    }
}

static void
restore_defaults (void)
{
#ifdef HAVE_RB500
  struct nvram_tuple generic[] = {
    {"lan_ifname", "br0", 0},
    {"lan_ifnames",
     "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5",
     0},
    {"wan_ifname", "eth0", 0},
    {"wan_ifnames", "eth0", 0},
    {0, 0, 0}
  };
#else
  struct nvram_tuple generic[] = {
    {"lan_ifname", "br0", 0},
    {"lan_ifnames", "eth0 eth2 eth3 eth4", 0},
    {"wan_ifname", "eth1", 0},
    {"wan_ifnames", "eth1", 0},
    {0, 0, 0}
  };
  struct nvram_tuple vlan[] = {
    {"lan_ifname", "br0", 0},
    {"lan_ifnames", "vlan0 eth1 eth2 eth3", 0},
    {"wan_ifname", "vlan1", 0},
    {"wan_ifnames", "vlan1", 0},
    {0, 0, 0}
  };
#endif

  struct nvram_tuple *linux_overrides;
  struct nvram_tuple *t, *u;
  int restore_defaults = 0;
//      uint boardflags;

  /* Restore defaults if told to.

     Note: an intentional side effect is that when
     upgrading from a firmware without the
     sv_restore_defaults var, defaults will
     also be restored.
   */

#ifdef HAVE_RB500
  linux_overrides = generic;
  int brand = getRouterBrand ();
#else
  int brand = getRouterBrand ();
  switch (brand)
    {
    case ROUTER_WRT54G:
    case ROUTER_WRT54G1X:
    case ROUTER_SIEMENS:
    case ROUTER_ASUS:
    case ROUTER_MOTOROLA:
    case ROUTER_BUFFALO_WHRG54S:
      if (nvram_invmatch ("sv_restore_defaults", "0"))	// || nvram_invmatch("os_name", "linux"))
	restore_defaults = 1;

      if (restore_defaults)
	cprintf ("Restoring defaults...");
      break;

    }
/* Delete dynamically generated variables */
  /* Choose default lan/wan i/f list. */
  char *ds;
  switch (brand)
    {
    case ROUTER_ASUS:
      linux_overrides = vlan;
      break;
    case ROUTER_SIEMENS:
    case ROUTER_BUFFALO_WZRRSG54:
      ds = nvram_safe_get ("dhcp_start");
      if (ds != NULL && strlen (ds) > 3)
	{
	  nvram_set ("dhcp_start", "100");	//siemens se505 to dd-wrt default mapping
	}

      ds = nvram_safe_get ("http_passwd");
      if (ds == NULL || strlen (ds) == 0)	//fix for empty default password
	{
	  nvram_set ("http_passwd", "admin");
	}
      ds = nvram_safe_get ("language");
      if (ds != NULL && strlen (ds) < 3)
	{
	  nvram_set ("language", "english");
	}
      // fall through  
    default:
      if (check_vlan_support ())
	linux_overrides = vlan;
      else
	linux_overrides = generic;
      break;
    }
#endif
  int i;
//  for (i=0;i<4;i++)
//      nvram_set(linux_overrides[i].name,linux_overrides[i].value);


  /* Restore defaults */
#ifdef HAVE_FON
  int reset = 0;
  char *rev = nvram_safe_get ("fon_revision");
  if (rev == NULL || strlen (rev) == 0)
    reset = 1;
  if (strlen (rev) > 0)
    {
      int n = atoi (rev);
      if (atoi (router_defaults[0].value) != n)
	reset = 1;
    }
  if (reset)
    {
      for (t = router_defaults; t->name; t++)
	{
	  for (u = linux_overrides; u && u->name; u++)
	    {
	      if (!strcmp (t->name, u->name))
		{
		  nvram_set (u->name, u->value);
		  break;
		}
	    }
	  if (!u || !u->name)
	    nvram_set (t->name, t->value);
	}
    }
#endif
  for (t = router_defaults; t->name; t++)
    {
      if (restore_defaults || !nvram_get (t->name))
	{
	  for (u = linux_overrides; u && u->name; u++)
	    {
	      if (!strcmp (t->name, u->name))
		{
		  nvram_set (u->name, u->value);
		  break;
		}
	    }
	  if (!u || !u->name)
	    nvram_set (t->name, t->value);
	}
    }
  if (restore_defaults && ((brand == ROUTER_BELKIN) || (brand == ROUTER_BUFFALO_WBR2G54S)))	//fix for belkin std ip
    {
      nvram_set ("lan_ipaddr", "192.168.11.1");
    }
#ifdef HAVE_SKYTRON
  if (restore_defaults)
    {
      nvram_set ("lan_ipaddr", "192.168.0.1");
    }
#endif
  if (!nvram_get ("vlan0hwname") || nvram_match ("vlan0hwname", ""))
    nvram_set ("vlan0hwname", "et0");
  if (!nvram_get ("vlan1hwname") || nvram_match ("vlan1hwname", ""))
    nvram_set ("vlan1hwname", "et0");

  if (!nvram_get ("vlan0ports") || nvram_match ("vlan0ports", ""))
    {
      switch (brand)
	{
	case ROUTER_MOTOROLA:
	  nvram_set ("vlan0ports", "1 2 3 4 5*");
	  break;
	case ROUTER_SIEMENS:
	  nvram_set ("vlan0ports", "0 1 2 3 5*");
	  break;
	default:
	  if (nvram_match ("bootnv_ver", "4"))
	    nvram_set ("vlan0ports", "3 2 1 0 5*");
	  else
	    nvram_set ("vlan0ports", "1 2 3 4 5*");
	  break;
	}
    }
  if (nvram_invmatch ("fullswitch", "1"))
    {
      if (!nvram_get ("vlan1ports") || nvram_match ("vlan1ports", ""))
	{
	  switch (brand)
	    {
	    case ROUTER_MOTOROLA:
	      nvram_set ("vlan1ports", "0 5u");
	      break;
	    case ROUTER_SIEMENS:
	      nvram_set ("vlan1ports", "4 5");
	      break;
	    default:
	      if (nvram_match ("bootnv_ver", "4"))
		nvram_set ("vlan1ports", "4 5");
	      else
		nvram_set ("vlan1ports", "0 5");
	      break;
	    }
	}
    }
/*	if (brand==ROUTER_ASUS)
	{
	if(!nvram_get("boardflags")||nvram_match("boardflags", ""))
		nvram_set("boardflags", "0x0388");

	}*/
  if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G1X)
    {
      if (!nvram_get ("aa0"))
	nvram_set ("aa0", "3");
      if (!nvram_get ("ag0"))
	nvram_set ("ag0", "255");
      if (!nvram_get ("gpio2"))
	nvram_set ("gpio2", "adm_eecs");
      if (!nvram_get ("gpio3"))
	nvram_set ("gpio3", "adm_eesk");
      if (!nvram_get ("gpio5"))
	nvram_set ("gpio5", "adm_eedi");
      if (!nvram_get ("gpio6"))
	nvram_set ("gpio6", "adm_rc");
      if (!nvram_get ("boardrev") || nvram_match ("boardrev", ""))
	nvram_set ("boardrev", "0x10");
      if (!nvram_get ("boardflags") || nvram_match ("boardflags", ""))
	nvram_set ("boardflags", "0x0388");
      if (!nvram_get ("boardflags2"))
	nvram_set ("boardflags2", "0");
    }
  /* Always set OS defaults */
  nvram_set ("os_name", "linux");
  nvram_set ("os_version", EPI_VERSION_STR);
#ifdef HAVE_SPUTNIK_APD
  /* Added for Sputnik Agent */
  nvram_unset("sputnik_mjid");
  nvram_unset("sputnik_rereg");
#endif
  if (check_now_boot () == CFE_BOOT)
    check_cfe_nv ();
  else if (check_now_boot () == PMON_BOOT)
    check_pmon_nv ();

  /* Commit values */
  if (restore_defaults)
    {
      int i;
      unset_nvram ();
      nvram_commit ();
      cprintf ("done\n");
      for (i = 0; i < MAX_NVPARSE; i++)
	del_wds_wsec (0, i);
    }
}

void
check_brcm_cpu_type (void)
{
  FILE *fcpu;
  char cpu_type[20];
  char type2[30];

  fcpu = fopen ("/proc/cpuinfo", "r");

  if (fcpu == NULL)
    fprintf (stderr, "Open /proc/cpuinfo fail...0\n");
  else
    {
      char buf[500];
      fgets (buf, 500, fcpu);
      fscanf (fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2);
      if (!strcmp (type2, "BCM4704"))
	{
	  nvram_set ("cpu_type", cpu_type);
	  fclose (fcpu);
	  return;
	}
      if (!strcmp (type2, "BCM4712"))
	{
	  nvram_set ("cpu_type", cpu_type);
	  fclose (fcpu);
	  return;
	}
      if (!strcmp (type2, "BCM4702"))
	{
	  nvram_set ("cpu_type", cpu_type);
	  fclose (fcpu);
	  return;
	}
      if (!strcmp (type2, "BCM3302"))
	{
	  nvram_set ("cpu_type", cpu_type);
	  fclose (fcpu);
	  return;
	}
      fgets (buf, 500, fcpu);
      fscanf (fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2);
      //fprintf(stderr, "cpu_type : %s\n", cpu_type); 
      fclose (fcpu);
      if (!strcmp (cpu_type, "BCM4710") || !strcmp (cpu_type, "BCM4702"))
	{
	  fprintf (stderr, "We got BCM4702 board...\n");
	  nvram_set ("cpu_type", cpu_type);
	}
      else if (!strcmp (cpu_type, "BCM3302") || !strcmp (cpu_type, "BCM4712"))
	{
	  fprintf (stderr, "We got BCM4712 board...\n");
	  nvram_set ("cpu_type", cpu_type);
	}
      else
	{
	  fprintf (stderr, "We got unknown board...\n");
	  nvram_set ("cpu_type", cpu_type);
	}
    }



}
char wanifname[8], wlifname[8];
#define BCM4712_CPUTYPE "0x4712"

static void
setup_4712 (void)
{
  uint boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  if (nvram_match ("cpu_type", BCM4712_CPUTYPE) ||
      nvram_match ("cpu_type", "BCM3302") ||
      nvram_match ("cpu_type", "BCM4712"))
    {
      if (boardflags & BFL_ENETVLAN)
	{
	  cprintf ("setup_4712(): Enable VLAN\n");
	  //nvram_set("setup_4712","1");
	  strcpy (wanifname, "vlan1");
	  strcpy (wlifname, "eth1");
	  if (!strcmp (nvram_safe_get ("wan_ifname"), "eth1"))
	    {
	      //nvram_set("setup_4712","1-1");
	      nvram_set ("wan_ifname", "vlan1");
	      nvram_set ("wan_ifnames", "vlan1");
	    }
	  if (!strstr (nvram_safe_get ("lan_ifnames"), "vlan0"))
	    {
	      //nvram_set("setup_4712","1-2");
	      nvram_set ("lan_ifnames", "vlan0 eth1");
	      nvram_set ("vlan0hwname", "et0");
	      nvram_set ("vlan1hwname", "et0");
	      nvram_set ("pppoe_ifname", "vlan1");
	      nvram_set ("wl0_ifname", "eth1");
	      nvram_commit ();
	    }
	}			// VLAN enabled
      else
	{
	  //nvram_set("setup_4712","2");
	  cprintf ("setup_4712(): Disable VLAN, it must be in bridge mode\n");
	  nvram_set ("lan_ifnames", "eth0 eth1");
	  strcpy (wlifname, "eth1");
	  nvram_set ("wl0_ifname", "eth1");
	}
    }
  else
    {				// 4702, 4704
      cprintf
	("setup_4712(): It's a 4702 or 4704 hardware, VLAN can't be used in these 2 boards\n");
      strcpy (wanifname, "eth1");
      strcpy (wlifname, "eth2");
      nvram_set ("wl0_ifname", "eth2");
      nvram_set ("pppoe_ifname", "eth1");
      if (!strcmp (nvram_safe_get ("wan_ifname"), ""))
	nvram_set ("lan_ifnames", "eth0 eth1 eth2 wlanb0 wlana0");
      else
	nvram_set ("lan_ifnames", "eth0 eth2");
    }
  nvram_commit ();


}

static int noconsole = 0;

static void
sysinit (void)
{
  char buf[PATH_MAX];
  struct utsname name;
  struct stat tmp_stat;
  time_t tm = 0;
#ifdef HAVE_RB500
  unlink ("/etc/nvram/.lock");
#endif
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);


#ifdef HAVE_RB500
  //load ext2 
  // eval("insmod","jbd");
  eval ("insmod", "ext2");
  if (mount
      ("/dev/discs/disc0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL))
    {
      //not created yet, create ext2 partition
      eval ("/sbin/mke2fs", "-F", "-b", "1024", "/dev/discs/disc0/part3");
      //mount ext2 
      mount ("/dev/discs/disc0/part3", "/usr/local", "ext2", MS_MGC_VAL,
	     NULL);
      eval ("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
      mkdir ("/usr/local/nvram", 0777);
//    eval("ln","-s","/etc/nvram","/usr/local/nvram");
    }

  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
  eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
#endif

  cprintf ("sysinit() var\n");

  /* /var */
  mkdir ("/tmp/var", 0777);
  mkdir ("/var/lock", 0777);
  mkdir ("/var/log", 0777);
  mkdir ("/var/run", 0777);
  mkdir ("/var/tmp", 0777);
  cprintf ("sysinit() setup console\n");

  /* Setup console */
  if (console_init ())
    noconsole = 1;
  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");

  int brand = getRouterBrand ();
  if (brand == ROUTER_BUFFALO_WZRRSG54)
    {
      check_brcm_cpu_type ();
    }
  if (brand == ROUTER_MOTOROLA)
    nvram_set ("cpu_type", "BCM4712");

  if (brand == ROUTER_SIEMENS || brand == ROUTER_MOTOROLA
      || brand == ROUTER_BUFFALO_WZRRSG54)
    {
      setup_4712 ();
    }

  /* Modules */
  uname (&name);

  enableAfterBurner ();
#ifndef HAVE_RB500
  snprintf (buf, sizeof (buf), "/lib/modules/%s", name.release);
  if (stat ("/proc/modules", &tmp_stat) == 0 && stat (buf, &tmp_stat) == 0)
    {
      char module[80], *modules, *next;
      //modules="wl switch-core";

      if (check_vlan_support () && check_hw_type () != BCM5325E_CHIP)
	{
	  switch (brand)
	    {
	    case ROUTER_WRT54G1X:
	    case ROUTER_WRT54G:
	    case ROUTER_SIEMENS:
	    case ROUTER_BUFFALO_WBR54G:
	    case ROUTER_MOTOROLA:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag et wl switch-core switch-robo switch-adm";
	      break;
	    default:
//          case ROUTER_MOTOROLA:
//          case ROUTER_BELKIN:
//          case ROUTER_BUFFALO_WBR2G54S:
//          case ROUTER_ASUS:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag et wl switch-core switch-robo";
	      break;
	    }
	}
      else
	{
	  switch (brand)
	    {
	    case ROUTER_ASUS:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag et wl switch-core switch-robo";
	      break;
	    case ROUTER_BUFFALO_WZRRSG54:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag b44 wl";
	      break;
	    default:
	      if (check_vlan_support ())
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag et wl switch-core switch-robo";
	      else
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag et wl";
	      break;
	    }
	}
      cprintf ("insmod %s\n", modules);

      foreach (module, modules, next)
      {
#ifdef HAVE_MACBIND
	if (nvram_match ("et0macaddr", MACBRAND))
	  eval ("insmod", module);
#else
	cprintf ("loading %s\n", module);
	eval ("insmod", module);
	cprintf ("done\n");
#endif
      }
    }
#else
  eval ("insmod", "mii");
  eval ("insmod", "korina");
  eval ("insmod", "via-rhine");
  eval ("insmod", "ipv6");

#endif
  /* Set a sane date */
  stime (&tm);
#ifndef HAVE_RB500
  if (brand == ROUTER_SIEMENS)
    {
      eval ("insmod", "led.o");	// Jerry Lee
      powerled_ctrl (0);
      led_ctrl (0);		// turn LED2 off
    }
#endif


  cprintf ("done\n");
}

/* States */
enum
{
  RESTART,
  STOP,
  START,
  TIMER,
  USER,
  IDLE,
};
static int state = START;
static int signalled = -1;

/* Signal handling */
static void
rc_signal (int sig)
{
  if (state == IDLE)
    {
      if (sig == SIGHUP)
	{
	  printf ("signalling RESTART\n");
	  signalled = RESTART;
	}
      else if (sig == SIGUSR2)
	{
	  printf ("signalling START\n");
	  signalled = START;
	}
      else if (sig == SIGINT)
	{
	  printf ("signalling STOP\n");
	  signalled = STOP;
	}
      else if (sig == SIGALRM)
	{
	  printf ("signalling TIMER\n");
	  signalled = TIMER;
	}
      else if (sig == SIGUSR1)
	{			// Receive from WEB
	  printf ("signalling USER1\n");
	  signalled = USER;
	}

    }
}

/* Timer procedure */
int
do_timer (void)
{
  //do_ntp();
  return 0;
}

/* Main loop */
void
main_loop (void)
{
  int ret;
  sigset_t sigset;
  pid_t shell_pid = 0;
  uint boardflags;
  int val;
  char tmp[200];
  //setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin", 1);
  //system("/etc/nvram/nvram");
  /* Basic initialization */
  sysinit ();

  /* Setup signal handlers */
  signal_init ();
  signal (SIGHUP, rc_signal);
  signal (SIGUSR1, rc_signal);	// Start single service from WEB, by honor
  signal (SIGUSR2, rc_signal);
  signal (SIGINT, rc_signal);
  signal (SIGALRM, rc_signal);
  sigemptyset (&sigset);

  /* Give user a chance to run a shell before bringing up the rest of the system */
  if (!noconsole)
    ddrun_shell (1, 0);

  /* init nvram , by honor */
  init_nvram ();

  /* Restore defaults if necessary */
#ifdef HAVE_SKYTEL
  nvram_set ("vlan0ports", "0 1 2 3 4 5*");
  nvram_set ("vlan1ports", "");
#else

  if (nvram_match ("fullswitch", "1") && nvram_invmatch ("wl_mode", "ap"))
    {
      nvram_set ("vlan0ports", "0 1 2 3 4 5*");
      nvram_set ("vlan1ports", "");
    }
  else
    {
      if (nvram_match ("vlan0ports", "0 1 2 3 4 5*"))
	{
	  nvram_set ("vlan0ports", "");
	  nvram_set ("vlan1ports", "");
	}
    }
#endif
  restore_defaults ();

//#ifdef HAVE_MSSID
//create vlans for b44 driver
//robo
#ifndef HAVE_RB500
  
  system("echo 1 > /proc/switch/eth0/reset");
  sprintf (tmp, "echo %s > /proc/switch/eth0/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/eth0/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);

//  system("echo 1 > /proc/switch/eth0/reset");

/*  sprintf (tmp, "echo %s > /proc/switch/bcm53xx/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/bcm53xx/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);
//  system("echo 1 > /proc/switch/adm6996/reset");
  sprintf (tmp, "echo %s > /proc/switch/adm6996/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/adm6996/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);*/
#endif
//#endif
  /* Add vlan */
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  nvram_set ("wanup", "0");
  /* begin Sveasoft add */
  /* force afterburner even for older boards */
//  boardflags |= BFL_AFTERBURNER;
  /* end Sveasoft add */
//  char ab[100];
//  sprintf(ab,"0x%X",boardflags);
//  nvram_set("boardflags",ab);


//      if (boardflags & BFL_ENETVLAN || check_hw_type() == BCM4712_CHIP)
  int brand = getRouterBrand ();
#ifndef HAVE_RB500
  switch (brand)
    {
    case ROUTER_ASUS:
    case ROUTER_MOTOROLA:
    case ROUTER_SIEMENS:
//    case ROUTER_WRT54G:
//    case ROUTER_WRT54G1X:
      config_vlan ();
      break;
    default:
      if (check_vlan_support ())
	{
	  config_vlan ();
	}
      break;

    }
#endif
  /* Update boot from embeded boot.bin. If we find that boot have serious bug, we need do this. by honor */
  check_image ();

  // Sveasoft add 2004-07-23 - set ip_forwarding, adjust ip_contrack limits, timeouts, enable Westwood+ congestion handling
  set_ip_forward ('1');
  system ("/etc/preinit");	//sets default values for ip_conntrack

#ifndef HAVE_RB500
  char *rwpart = "mtd4";
  int itworked = 0;
  if (nvram_match ("sys_enable_jffs2", "1"))
    {
      if (nvram_match ("sys_clean_jffs2", "1"))
	{
	  nvram_set ("sys_clean_jffs2", "0");
	  nvram_commit ();
	  itworked = mtd_erase (rwpart);
//        eval ("insmod", "crc32");
//        eval ("insmod", "jffs2");

	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }



	}
      else
	{
	  itworked = mtd_unlock ("mtd4");
//        eval ("insmod", "crc32");
//        eval ("insmod", "jffs2");
	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }


	}
    }
#endif
  //sprintf(tmp,"%s%s",CYBERTAN_VERSION, MINOR_VERSION);
  //setenv("FIRMWARE_VERSION",tmp,1);

//      system("/bin/echo 4096 > /proc/sys/net/ipv4/ip_conntrack_max");
//      system("/bin/echo 3600 > /proc/sys/net/ipv4/ip_conntrack_tcp_timeouts");
//      system("/bin/echo 3600 > /proc/sys/net/ipv4/ip_conntrack_udp_timeouts");
//      system("/bin/echo 1 > /proc/sys/net/ipv4/tcp_westwood");

  // Sveasoft add 2004-01-04 create passwd, groups, misc files
  mkfiles ();
  char *hostname;

  /* set hostname to wan_hostname or router_name */
  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
    hostname = nvram_safe_get ("wan_hostname");
  else if (strlen (nvram_safe_get ("router_name")) > 0)
    hostname = nvram_safe_get ("router_name");
  else
    hostname = "dd-wrt";

  sethostname (hostname, strlen (hostname));

  stop_httpd ();
  if (brand == ROUTER_SIEMENS)
    {
      powerled_ctrl (1);
    }

  /* Loop forever */
  for (;;)
    {
      switch (state)
	{
	case USER:		// Restart single service from WEB of tftpd, by honor
	  cprintf ("USER1\n");
	  start_single_service ();
#ifdef HAVE_CHILLI
	  start_chilli ();
#endif

	  state = IDLE;
	  break;
	case RESTART:
	  cprintf("RESET NVRAM VARS\n");
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
	  nvram_set ("wl0_akm", nvram_safe_get ("wl_akm"));
	  if (nvram_match ("wl_wep", "tkip"))
	    {
	      nvram_set ("wl_crypto", "tkip");
	    }
	  else if (nvram_match ("wl_wep", "aes"))
	    {
	      nvram_set ("wl_crypto", "aes");
	    }
	  else if (nvram_match ("wl_wep", "tkip+aes"))
	    {
	      nvram_set ("wl_crypto", "tkip+aes");
	    }

	  if (nvram_match ("wl_wep", "restricted"))
	    nvram_set ("wl_wep", "enabled");	// the nas need this value, the "restricted" is no longer need. (20040624 by honor)

	  cprintf ("RESTART\n");
	  if (nvram_match ("wl_akm", "wpa") ||
	      nvram_match ("wl_akm", "psk") ||
	      nvram_match ("wl_akm", "radius") ||
	      nvram_match ("wl_akm", "psk2") ||
	      nvram_match ("wl_akm", "wpa2") ||
	      nvram_match ("wl_akm", "wpa wpa2") ||
	      nvram_match ("wl_akm", "psk psk2"))
	    {
	      eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
	      sleep (4);
	      wlconf_up (nvram_safe_get ("wl0_ifname"));

	    }
	  /* Fall through */
	case STOP:
	  cprintf ("STOP\n");
	  system ("killall -9 udhcpc");
	  setenv ("PATH",
		  "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin",
		  1);
	  setenv ("LD_LIBRARY_PATH",
		  "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:",
		  1);
	  cprintf("STOP WAN\n");
	  stop_wan ();
	  cprintf("STOP SERVICES\n");
	  stop_services ();
	  cprintf("STOP LAN\n");
	  stop_lan ();
#ifndef HAVE_RB500
	  cprintf("STOP RESETBUTTON\n");
	  if ((brand != ROUTER_BELKIN) && (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_SIEMENS) && (brand != ROUTER_BUFFALO_WZRRSG54))	//belkin doesnt like that
	    {
	      stop_resetbutton ();
	    }
#endif
	  create_rc_file (RC_SHUTDOWN);
	  system ("/tmp/.rc_shutdown");
	  if (state == STOP)
	    {
	      state = IDLE;
	      break;
	    }
	  /* Fall through */
	case START:
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
	  nvram_set ("wl0_akm", nvram_safe_get ("wl_akm"));
	  cprintf ("START\n");
	  setenv ("PATH",
		  "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin",
		  1);
	  setenv ("LD_LIBRARY_PATH",
		  "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:",
		  1);

	  start_ipv6 ();
#ifndef HAVE_RB500
	  if ((brand != ROUTER_BELKIN)&& (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_SIEMENS) && (brand != ROUTER_BUFFALO_WZRRSG54))	//belkin doesnt like that
	    {
	      start_resetbutton ();
	    }
#endif
	  setup_vlans ();
	  start_lan ();
	  eval ("rm", "/tmp/hosts");
	  addHost ("localhost", "127.0.0.1");
	  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
	    addHost (nvram_safe_get ("wan_hostname"),
		     nvram_safe_get ("lan_ipaddr"));
	  else if (strlen (nvram_safe_get ("router_name")) > 0)
	    addHost (nvram_safe_get ("router_name"),
		     nvram_safe_get ("lan_ipaddr"));
	  cprintf ("start services\n");
	  start_services ();
	  cprintf ("start wan boot\n");
	  start_wan (BOOT);
	  cprintf ("diaG STOP LED\n");
	  diag_led (DIAG, STOP_LED);
	  cprintf ("set led release wan control\n");
	  SET_LED (RELEASE_WAN_CONTROL);
	  cprintf ("ifconfig wl up\n");
	  if (nvram_match ("wl_mode", "sta")
	      || nvram_match ("wl_mode", "wet"))
	    {
	      //fix for client mode
	      if (wl_probe ("eth2"))
		eval ("/sbin/ifconfig", "eth1", "up");
	      else
		eval ("/sbin/ifconfig", "eth2", "up");
	    }
	  cprintf ("start nas\n");
	  start_nas ("wan");
	  cprintf ("create rc file\n");
	  create_rc_file (RC_STARTUP);
	  chmod ("/tmp/.rc_startup", 0700);
	  system ("/tmp/.rc_startup");
	  //#ifdef HAVE_SER
	  system ("/etc/init.d/rcS");	// start openwrt startup script (siPath impl)
	  //#endif
	  //start_sambafs();
	  //start_macupd();
	  //start_rflow();
	  //start_radius();
	  cprintf ("start modules\n");
	  start_modules ();
#ifdef HAVE_CHILLI
	  start_chilli ();
#endif
	  cprintf ("start syslog\n");
	  stop_syslog ();
	  start_syslog ();

	  system ("/etc/postinit");
//#ifdef HAVE_SKYTRON   
//eval("ifconfig","vlan1","172.16.1.1","255.255.255.0");
//#endif

/*			if (nvram_match("wl_mode", "bridge"))
				{
				if (wl_probe("eth2"))
				    eval("brctl", "addif", "br0", "eth1"); //create bridge
				else
				    eval("brctl", "addif", "br0", "eth2");
				}*/
	  diag_led (DIAG, STOP_LED);
	  /* Fall through */
	case TIMER:
	  cprintf ("TIMER\n");
	  do_timer ();
	  /* Fall through */
	case IDLE:
	  cprintf ("IDLE\n");
	  state = IDLE;
	  /* Wait for user input or state change */
	  while (signalled == -1)
	    {
	      if (!noconsole && (!shell_pid || kill (shell_pid, 0) != 0))
		shell_pid = ddrun_shell (0, 1);
	      else
		sigsuspend (&sigset);
	    }
	  state = signalled;
	  signalled = -1;
	  break;
	default:
	  cprintf ("UNKNOWN\n");
	  return;
	}
    }

}




#define CONVERT_NV(old, new) \
	if(nvram_get(old)) \
		nvram_set(new, nvram_safe_get(old));


static int
init_nvram (void)
{
  int i = 0;

  /* broadcom 3.11.48.7 change some nvram name */
  CONVERT_NV ("d11g_channel", "wl_channel");
  CONVERT_NV ("d11g_rateset", "wl_rateset");
  CONVERT_NV ("d11g_rts", "wl_rts");
  CONVERT_NV ("d11g_bcn", "wl_bcn");
  CONVERT_NV ("d11g_mode", "wl_gmode");
  CONVERT_NV ("d11g_rate", "wl_rate");
  CONVERT_NV ("d11g_frag", "wl_frag");
  CONVERT_NV ("d11g_dtim", "wl_dtim");

  nvram_set ("wl0_hwaddr", "");	// When disbale wireless, we must get null wireless mac */

  nvram_set ("wan_get_dns", "");
  nvram_set ("filter_id", "1");
  nvram_set ("wl_active_add_mac", "0");
  nvram_set ("ddns_change", "");
  nvram_set ("action_service", "");
  nvram_set ("ddns_interval", "60");
  nvram_set ("wan_get_domain", "");

  //if(!nvram_get("wl_macmode1")){
  //      if(nvram_match("wl_macmode","disabled"))
  //              nvram_set("wl_macmode1","disabled");
  //      else
  //              nvram_set("wl_macmode1","other");
  //}
  if (nvram_match ("wl_gmode", "5"))	// Mixed mode had been changed to 5
    nvram_set ("wl_gmode", "1");

  if (nvram_match ("wl_gmode", "4"))	// G-ONLY mode had been changed to 2, after 1.40.1 for WiFi G certication
    nvram_set ("wl_gmode", "2");

//      nvram_set("wl_country","Worldwide");    // The country always Worldwide

#ifndef AOL_SUPPORT
  nvram_set ("aol_block_traffic", "0");
  nvram_set ("aol_block_traffic1", "0");
  nvram_set ("aol_block_traffic2", "0");
#endif
  nvram_set ("ping_ip", "");
  nvram_set ("ping_times", "");
  nvram_set ("traceroute_ip", "");

  nvram_set ("filter_port", "");	// The name have been disbaled from 1.41.3

  if (nvram_invmatch ("sv_restore_defaults", "1"))
    {
      for (i = 0; i < MAX_NVPARSE; i++)
	{
	  char name[] = "forward_portXXXXXXXXXX";
	  snprintf (name, sizeof (name), "forward_port%d", i);
	  if (nvram_get (name) && strstr (nvram_safe_get (name), "msmsgs"))
	    {
	      cprintf ("unset MSN value %d..........\n", i);
	      nvram_unset (name);
	    }
	}
    }

  /* The tkip and aes already are changed to wl_crypto from v3.63.3.0 */
  if (nvram_match ("wl_wep", "tkip"))
    {
      nvram_set ("wl_crypto", "tkip");
    }
  else if (nvram_match ("wl_wep", "aes"))
    {
      nvram_set ("wl_crypto", "aes");
    }
  else if (nvram_match ("wl_wep", "tkip+aes"))
    {
      nvram_set ("wl_crypto", "tkip+aes");
    }

  if (nvram_match ("wl_wep", "restricted"))
    nvram_set ("wl_wep", "enabled");	// the nas need this value, the "restricted" is no longer need. (20040624 by honor)


// begin Sveasoft additions
#ifdef HAVE_SET_BOOT
  if (!nvram_match ("boot_wait_web", "0"))
    nvram_set ("boot_wait_web", "1");
#endif

#ifdef HAVE_SSHD
  if (!nvram_match ("sshd_web", "0"))
    nvram_set ("sshd_web", "1");
#endif

  /* added 2003-12-24 Sveasoft - force board to 14 channels by setting locale to Japan */
  nvram_set ("wl0_country_code", "JP");
  nvram_set ("wl0_country", "Japan");
  if (check_hw_type () == BCM5352E_CHIP)
    {
      nvram_set ("opo", "0x0008");
      nvram_set ("ag0", "0x02");
    }
// end Sveasoft addition

  /* Let HW1.x users can communicate with WAP54G without setting to factory default */
//      nvram_safe_set("wl_lazywds", "1");
//  eval ("misc", "-t", "get_mac", "-w", "3");
//  eval ("misc", "-t", "get_eou", "-w", "2");
//  eval ("misc", "-t", "get_sn", "-w", "3");

//  eval ("misc", "-t", "get_flash_type", "-w", "1");

  return 0;
}

static void
unset_nvram (void)
{
#ifndef MPPPOE_SUPPORT
  nvram_safe_unset ("ppp_username_1");
  nvram_safe_unset ("ppp_passwd_1");
  nvram_safe_unset ("ppp_idletime_1");
  nvram_safe_unset ("ppp_demand_1");
  nvram_safe_unset ("ppp_redialperiod_1");
  nvram_safe_unset ("ppp_service_1");
  nvram_safe_unset ("mpppoe_enable");
  nvram_safe_unset ("mpppoe_dname");
#endif
#ifndef HAVE_HTTPS
  nvram_safe_unset ("remote_mgt_https");
#endif
#ifndef HSIAB_SUPPORT
  nvram_safe_unset ("hsiab_mode");
  nvram_safe_unset ("hsiab_provider");
  nvram_safe_unset ("hsiab_device_id");
  nvram_safe_unset ("hsiab_device_password");
  nvram_safe_unset ("hsiab_admin_url");
  nvram_safe_unset ("hsiab_registered");
  nvram_safe_unset ("hsiab_configured");
  nvram_safe_unset ("hsiab_register_ops");
  nvram_safe_unset ("hsiab_session_ops");
  nvram_safe_unset ("hsiab_config_ops");
  nvram_safe_unset ("hsiab_manual_reg_ops");
  nvram_safe_unset ("hsiab_proxy_host");
  nvram_safe_unset ("hsiab_proxy_port");
  nvram_safe_unset ("hsiab_conf_time");
  nvram_safe_unset ("hsiab_stats_time");
  nvram_safe_unset ("hsiab_session_time");
  nvram_safe_unset ("hsiab_sync");
  nvram_safe_unset ("hsiab_config");
#endif

#ifndef HEARTBEAT_SUPPORT
  nvram_safe_unset ("hb_server_ip");
  nvram_safe_unset ("hb_server_domain");
#endif

#ifndef PARENTAL_CONTROL_SUPPORT
  nvram_safe_unset ("artemis_enable");
  nvram_safe_unset ("artemis_SVCGLOB");
  nvram_safe_unset ("artemis_HB_DB");
  nvram_safe_unset ("artemis_provisioned");
#endif

#ifndef WL_STA_SUPPORT
//        nvram_safe_unset("wl_ap_ssid");
//        nvram_safe_unset("wl_ap_ip");
#endif

#ifndef UPNP_FORWARD_SUPPORT
  {
    int i;
    for (i = 0; i < MAX_NVPARSE; i++)
      {
	del_forward_port (i);
      }
  }
#endif


//#ifndef SYSLOG_SUPPORT
  {
    struct support_list *s;
    char buf[80];
    for (s = supports; s < &supports[SUPPORT_COUNT]; s++)
      {
	snprintf (buf, sizeof (buf), "LOG_%s", s->name);
	if (nvram_get (buf))
	  {
	    nvram_safe_unset (buf);
	  }
      }
  }
  nvram_safe_unset ("log_show_all");
  nvram_safe_unset ("log_show_type");
//#endif

}

static int
check_nv (char *name, char *value)
{
  int ret = 0;
  if (nvram_match ("manual_boot_nv", "1"))
    return 0;

  if (!nvram_get (name))
    {
      cprintf ("ERR: Cann't find %s !.......................\n", name);
      nvram_set (name, value);
      ret++;
    }
  else if (nvram_invmatch (name, value))
    {
      cprintf ("ERR: The %s is %s, not %s !.................\n", name,
	       nvram_safe_get (name), value);
      nvram_set (name, value);
      ret++;
    }

  return ret;
}

static int
check_cfe_nv (void)
{
  int ret = 0;
  ;
//      ret += check_nv("boardtype", "0x0101");
//      ret += check_nv("boardflags", "0x0188");
//      ret += check_nv("boardflags", "0x0388");

//      ret += check_nv("boardrev", "0x10");
//      ret += check_nv("boardflags2", "0");
//      ret += check_nv("sromrev", "2");

  switch (getRouterBrand ())
    {
    case ROUTER_ASUS:
      //check_nv ("wl0_hwaddr", nvram_safe_get ("et0macaddr")); //fixing missing wireless mac
      //nvram_commit ();
      return 0;
      break;
    case ROUTER_SIEMENS:
//      check_nv("wl0_hwaddr",nvram_safe_get("wan_hwaddr"));
//      ret += check_nv("lan_ifname","br0");
//      ret += check_nv("lan_hwnames","et0 il0 wl0 wl1");
//      ret += check_nv("lan_ifnames","vlan0 eth1");
//      ret += check_nv("wan_hwname","et1");
//      ret += check_nv("wan_ifname","vlan1");
//      ret += check_nv("pppoe_ifname","vlan1");
      break;
    case ROUTER_MOTOROLA:
//nothign
      break;
    case ROUTER_BUFFALO_WBR54G:
    case ROUTER_BUFFALO_WZRRSG54:

      ret += check_nv ("lan_hwnames", "et0 wl0");
      ret += check_nv ("lan_ifnames", "eth0 eth2");
      ret += check_nv ("wan_hwname", "et1");
      ret += check_nv ("wan_ifname", "eth1");
      ret += check_nv ("wan_ifnames", "eth1");
      ret += check_nv ("pppoe_ifname", "eth1");
      ret += check_nv ("wl0_ifname", "eth2");
      ret += check_nv ("vlans", "0");
      break;
    case ROUTER_BUFFALO_WBR2G54S:
      ret += check_nv ("aa0", "3");

      ret += check_nv ("pa0itssit", "62");
      ret += check_nv ("pa0b0", "0x1136");
      ret += check_nv ("pa0b1", "0xfb93");
      ret += check_nv ("pa0b2", "0xfea5");
      ret += check_nv ("pa0maxpwr", "60");

      ret += check_nv ("wl0gpio2", "0");
      ret += check_nv ("wl0gpio3", "0");
      ret += check_nv ("cctl", "0");
      ret += check_nv ("ccode", "0");
      break;
    case ROUTER_WRT54G1X:
      break;

    case ROUTER_WRT54G:
      ret += check_nv ("aa0", "3");
      if (check_hw_type () == BCM5352E_CHIP)
	ret += check_nv ("ag0", "0x02");
      else
	ret += check_nv ("ag0", "255");

      if (check_hw_type () == BCM5325E_CHIP)
	{
	  /* Lower the DDR ram drive strength , the value will be stable for all boards
	     Latency 3 is more stable for all ddr 20050420 by honor */
//      if (!nvram_match("manual_boot_nv", "1")) {

	  ret += check_nv ("sdram_init", "0x010b");
	  ret += check_nv ("sdram_config", "0x0062");
	  ret += check_nv ("clkfreq", "216");
	  if (ret)
	    {
	      nvram_set ("sdram_ncdl", "0x0");
	      nvram_commit ();
//                  kill(1, SIGTERM);
//                  exit(0);
	    }
//              }
	  ret += check_nv ("pa0itssit", "62");
	  ret += check_nv ("pa0b0", "0x15eb");
	  ret += check_nv ("pa0b1", "0xfa82");
	  ret += check_nv ("pa0b2", "0xfe66");
	  ret += check_nv ("pa0maxpwr", "0x4e");
	}
      else if (check_hw_type () == BCM4704_BCM5325F_CHIP)
	{
	  //nothing to do
	}
      else
	{
	  ret += check_nv ("pa0itssit", "62");
	  ret += check_nv ("pa0b0", "0x170c");
	  ret += check_nv ("pa0b1", "0xfa24");
	  ret += check_nv ("pa0b2", "0xfe70");
	  ret += check_nv ("pa0maxpwr", "0x48");
	}

      //ret += check_nv("gpio2", "adm_eecs");
      //ret += check_nv("gpio3", "adm_eesk");
      //ret += check_nv("gpio5", "adm_eedi");
      //ret += check_nv("gpio6", "adm_rc");

      ret += check_nv ("wl0gpio2", "0");
      ret += check_nv ("wl0gpio3", "0");

      ret += check_nv ("cctl", "0");
      ret += check_nv ("ccode", "0");
      break;
    case ROUTER_BELKIN:
      ret += check_nv ("aa0", "3");
      if (check_hw_type () == BCM5352E_CHIP)
	ret += check_nv ("ag0", "0x02");
      else
	ret += check_nv ("ag0", "255");

      if (check_hw_type () == BCM5325E_CHIP)
	{

	  ret += check_nv ("sdram_init", "0x010b");
	  ret += check_nv ("sdram_config", "0x0062");
	  ret += check_nv ("clkfreq", "216");
	  if (ret)
	    {
	      nvram_set ("sdram_ncdl", "0x0");
	      nvram_commit ();
	    }
	  ret += check_nv ("pa0itssit", "62");
	  ret += check_nv ("pa0b0", "0x15eb");
	  ret += check_nv ("pa0b1", "0xfa82");
	  ret += check_nv ("pa0b2", "0xfe66");
	  ret += check_nv ("pa0maxpwr", "0x4e");
	}
      else if (check_hw_type () == BCM4704_BCM5325F_CHIP)
	{
	  //nothing to do
	}
      else
	{
	  ret += check_nv ("pa0itssit", "62");
	  ret += check_nv ("pa0b0", "0x1136");
	  ret += check_nv ("pa0b1", "0xfb93");
	  ret += check_nv ("pa0b2", "0xfea5");
	  ret += check_nv ("pa0maxpwr", "60");
	}
      ret += check_nv ("wl0gpio2", "0");
      ret += check_nv ("wl0gpio3", "0");

      ret += check_nv ("cctl", "0");
      ret += check_nv ("ccode", "0");

      break;

    }
  if (ret)
    {
      cprintf ("Some error found, we want to reboot!.....................\n");
      nvram_commit ();
      kill (1, SIGTERM);
      exit (0);
    }


  return ret;
}

static int
check_pmon_nv (void)
{
  return 0;
}

static int
check_image (void)
{
  int ret = 0;
/*  eval ("insmod", "writemac", "flag=get_flash");	//Barry adds for set flash_type 2003 09 08
  nvram_set ("firmware_version", CYBERTAN_VERSION);
  nvram_set ("Intel_firmware_version", INTEL_FLASH_SUPPORT_VERSION_FROM);
  nvram_set ("bcm4712_firmware_version", BCM4712_CHIP_SUPPORT_VERSION_FROM);
  eval ("rmmod", "writemac");
*/
  return ret;
}

#if 0

int
ddwrt_main (int argc, char **argv)
{
  char *base = strrchr (argv[0], '/');

  base = base ? base + 1 : argv[0];

  /* init */
  if (strstr (base, "init"))
    {
      main_loop ();
      return 0;
    }

  /* Set TZ for all rc programs */
  setenv ("TZ", nvram_safe_get ("time_zone"), 1);

  /* ppp */
  if (strstr (base, "ip-up"))
    return ipup_main (argc, argv);
  else if (strstr (base, "ip-down"))
    return ipdown_main (argc, argv);

  /* udhcpc [ deconfig bound renew ] */
  else if (strstr (base, "udhcpc"))
    return udhcpc_main (argc, argv);
#ifdef HAVE_PPTPD
  /* poptop [ stop start restart ]  */
  else if (strstr (base, "poptop"))
    return pptpd_main (argc, argv);
#endif
#ifndef HAVE_RB500
  /* erase [device] */
  else if (strstr (base, "erase"))
    {
      int brand = getRouterBrand ();
      if (brand == ROUTER_MOTOROLA)
	{
	  if (argv[1] && strcmp (argv[1], "nvram"))
	    {
	      fprintf (stderr,
		       "Sorry, erasing nvram will get the motorola unit unuseable\n");
	      return 0;
	    }
	}
      else
	{
	  if (argv[1])
	    return mtd_erase (argv[1]);
	  else
	    {
	      fprintf (stderr, "usage: erase [device]\n");
	      return EINVAL;
	    }
	}
    }

  /* write [path] [device] */
  else if (strstr (base, "write"))
    {
      if (argc >= 3)
	return mtd_write (argv[1], argv[2]);
      else
	{
	  fprintf (stderr, "usage: write [path] [device]\n");
	  return EINVAL;
	}
    }
#endif
  /* hotplug [event] */
  else if (strstr (base, "hotplug"))
    {
      if (argc >= 2)
	{
	  if (!strcmp (argv[1], "net"))
	    return hotplug_net ();
	}
      else
	{
	  fprintf (stderr, "usage: hotplug [event]\n");
	  return EINVAL;
	}
    }
  /* rc [stop|start|restart ] */
  else if (strstr (base, "rc"))
    {
      if (argv[1])
	{
	  if (strncmp (argv[1], "start", 5) == 0)
	    return kill (1, SIGUSR2);
	  else if (strncmp (argv[1], "stop", 4) == 0)
	    return kill (1, SIGINT);
	  else if (strncmp (argv[1], "restart", 7) == 0)
	    return kill (1, SIGHUP);
	}
      else
	{
	  fprintf (stderr, "usage: rc [start|stop|restart]\n");
	  return EINVAL;
	}
    }

  //////////////////////////////////////////////////////
  //
  else if (strstr (base, "filtersync"))
    return filtersync_main ();
  /* filter [add|del] number */
  else if (strstr (base, "filter"))
    {
      if (argv[1] && argv[2])
	{
	  int num = 0;
	  if ((num = atoi (argv[2])) > 0)
	    {
	      if (strcmp (argv[1], "add") == 0)
		return filter_add (num);
	      else if (strcmp (argv[1], "del") == 0)
		return filter_del (num);
	    }
	}
      else
	{
	  fprintf (stderr, "usage: filter [add|del] number\n");
	  return EINVAL;
	}
    }
  else if (strstr (base, "redial"))
    return redial_main (argc, argv);

  else if (strstr (base, "resetbutton"))
    {
#ifndef HAVE_RB500
      int brand = getRouterBrand ();
      if ((brand != ROUTER_BELKIN)  && (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_BUFFALO_WZRRSG54) && (brand != ROUTER_SIEMENS))	//belkin doesnt like that
	{
	  return resetbutton_main (argc, argv);
	}
      else
	{
	  fprintf (stderr,
		   "Belkin,Buffalo,Siemens S505 doesnt support the resetbutton!");
	  return 0;
	}
#endif
    }
#ifndef HAVE_MADWIFI
  else if (strstr (base, "wland"))
    return wland_main (argc, argv);
#endif
//  else if (strstr (base, "write_boot"))
//    return write_boot ("/tmp/boot.bin", "pmon");

#ifdef DEBUG_IPTABLE
  else if (strstr (base, "iptable_range"))
    return range_main (argc, argv);
  else if (strstr (base, "iptable_rule"))
    return rule_main (argc, argv);
#endif



  else if (strstr (base, "hb_connect"))
    return hb_connect_main (argc, argv);
  else if (strstr (base, "hb_disconnect"))
    return hb_disconnect_main (argc, argv);

  else if (strstr (base, "gpio"))
    return gpio_main (argc, argv);
  else if (strstr (base, "listen"))
    return listen_main (argc, argv);
  else if (strstr (base, "check_ps"))
    return check_ps_main (argc, argv);
  else if (strstr (base, "ddns_success"))
    return ddns_success_main (argc, argv);
//      else if (strstr(base, "eou_status"))
//                return eou_status_main();
  else if (strstr (base, "process_monitor"))
    return process_monitor_main ();
  else if (strstr (base, "restart_dns"))
    {
      stop_dns ();
      stop_dhcpd ();
      start_dhcpd ();
      start_dns ();
    }
  else if (strstr (base, "site_survey"))
    return site_survey_main (argc, argv);
  else if (strstr (base, "setpasswd"))
    mkfiles ();
#ifdef HAVE_WOL
  else if (strstr (base, "wol"))
    wol_main ();
#endif
  else if (strstr (base, "sendudp"))
    return sendudp_main (argc, argv);
  else if (strstr (base, "check_ses_led"))
    return check_ses_led_main (argc, argv);
  else if (strstr (base, "httpd"))
    return httpd_main (argc, argv);
  else if (strstr (base, "bird"))
    return bird_main (argc, argv);
  else if (strstr (base, "dnsmasq"))
    return dnsmasq_main (argc, argv);
#ifdef HAVE_SSHD
  else if (strstr (base, "dropbearkey"))
    return dropbearkey_main (argc, argv);
  else if (strstr (base, "dropbearkonvert"))
    return dropbearconvert_main (argc, argv);
  else if (strstr (base, "dropbear"))
    return dropbear_main (argc, argv);
  else if (strstr (base, "dbclient"))
    return cli_main (argc, argv);
  else if (strstr (base, "ssh"))
    return cli_main (argc, argv);
  else if (strstr (base, "scp"))
    return scp_main (argc, argv);
#endif
#ifdef HAVE_IPROUTE2
  else if (strstr (base, "tc"))
    return tc_main (argc, argv);
  else if (strstr (base, "ip"))
    return ip_main (argc, argv);
#endif
#ifdef HAVE_DHCPFWD
  else if (strstr (base, "dhcp-fwd"))
    return dhcpforward_main (argc, argv);
#endif
#ifdef HAVE_PPPD
  else if (strstr (base, "pppd"))
    return pppd_main (argc, argv);
#endif
//  else if (strstr (base, "reboot"))
//    shutdown_system();

  return EINVAL;
}
#endif
