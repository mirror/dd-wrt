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
#include <termios.h>

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
#include <mkfiles.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>
#include <ledcontrol.h>


#define WL_IOCTL(name, cmd, buf, len) (ret = wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28

void start_restore_defaults (void);
int start_sysinit (void);
static void rc_signal (int sig);
static void overclock (void);
static int check_cfe_nv (void);
static int check_pmon_nv (void);
static void unset_nvram (void);
int start_nvram (void);

extern struct nvram_tuple srouter_defaults[];



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
start_modules (void)
{
  runStartup ("/etc/config", ".startup");
#ifdef HAVE_RB500
  runStartup ("/usr/local/etc/config", ".startup");	//if available
#else
  runStartup ("/jffs/etc/config", ".startup");	//if available
  runStartup ("/mmc/etc/config", ".startup");	//if available
#endif
  return 0;
}

int
start_wanup (void)
{
  runStartup ("/etc/config", ".wanup");
#ifdef HAVE_RB500
  runStartup ("/usr/local/etc/config", ".wanup");	//if available
#else
  runStartup ("/jffs/etc/config", ".wanup");	//if available
  runStartup ("/mmc/etc/config", ".wanup");	//if available
#endif
  return 0;
}


static void
enableAfterBurner (void)
{

  int boardflags;
  if (getRouterBrand () == ROUTER_LINKSYS_WRT55AG
      || getRouterBrand () == ROUTER_MOTOROLA_V1)
    return;
  if (nvram_get ("boardflags") == NULL)
    return;
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  fprintf (stderr, "boardflags are 0x0%X\n", boardflags);
  if (!(boardflags & BFL_AFTERBURNER))
    {
      boardflags |= BFL_AFTERBURNER;
      char ab[100];
      fprintf (stderr, "enable Afterburner....\n");
      sprintf (ab, "0x0%X", boardflags);
      nvram_set ("boardflags", ab);
      nvram_set ("need_commit", "1");
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
start_create_rc_startup (void)
{
  create_rc_file (RC_STARTUP);
  return 0;
}

int
start_create_rc_shutdown (void)
{
  create_rc_file (RC_SHUTDOWN);
  return 0;
}


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

void
start_restore_defaults (void)
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
    {"lan_ifnames", "eth0 eth2", 0},
    {"wan_ifname", "eth1", 0},
    {"wan_ifnames", "eth1", 0},
    {0, 0, 0}
  };
  struct nvram_tuple vlan[] = {
    {"lan_ifname", "br0", 0},
    {"lan_ifnames", "vlan0 eth1", 0},
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
    case ROUTER_BUFFALO_WLA2G54L:
      break;
    default:

      if (nvram_invmatch ("sv_restore_defaults", "0"))	// || nvram_invmatch("os_name", "linux"))
        {
//	nvram_unset("sv_restore_defaults");
	restore_defaults = 1;
	}
      if (nvram_match ("product_name", "INSPECTION"))
	{
	  nvram_unset ("product_name");
	  restore_defaults = 1;
	}
      if (nvram_get ("router_name") == NULL)
	restore_defaults = 1;

      if (restore_defaults)
	cprintf ("Restoring defaults...");

    }

/* Delete dynamically generated variables */
  /* Choose default lan/wan i/f list. */
  char *ds;
  switch (brand)
    {
    case ROUTER_WRTSL54GS:
    case ROUTER_BUFFALO_WZRRSG54:
      linux_overrides = generic;
      break;
    case ROUTER_ASUS:
      linux_overrides = vlan;
      break;
    case ROUTER_SIEMENS:
    case ROUTER_RT210W:
    case ROUTER_BELKIN_F5D7230:
      ds = nvram_safe_get ("dhcp_start");
      if (ds != NULL && strlen (ds) > 3)
	{
	  fprintf (stderr, "cleaning nvram variables\n");
	  for (t = srouter_defaults; t->name; t++)
	    {
	      nvram_unset (t->name);
	    }
	  restore_defaults = 1;
	}

/*      ds = nvram_safe_get ("http_passwd");
      if (ds == NULL || strlen (ds) == 0)	//fix for empty default password
	{
	  nvram_set ("http_passwd", "admin");
	}
      ds = nvram_safe_get ("language");
      if (ds != NULL && strlen (ds) < 3)
	{
	  nvram_set ("language", "english");
	}*/
      // fall through  
    default:
      if (check_vlan_support ())
	linux_overrides = vlan;
      else
	linux_overrides = generic;
      break;
    }
#endif
/*  int i;
 *  for (i=0;i<4;i++)
 * 		nvram_set(linux_overrides[i].name,linux_overrides[i].value);
 */

  /* Restore defaults */
#ifdef HAVE_FON
  int reset = 0;
  char *rev = nvram_safe_get ("fon_revision");
  if (rev == NULL || strlen (rev) == 0)
    reset = 1;
  if (strlen (rev) > 0)
    {
      int n = atoi (rev);
      if (atoi (srouter_defaults[0].value) != n)
	reset = 1;
    }
  if (reset)
    {
      for (t = srouter_defaults; t->name; t++)
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
  int nvcnt = 0;
//  if (!nvram_match("default_init","1"))
  {
    for (t = srouter_defaults; t->name; t++)
      {
	if (restore_defaults || !nvram_get (t->name))
	  {
	    for (u = linux_overrides; u && u->name; u++)
	      {
		if (!strcmp (t->name, u->name))
		  {
		    nvcnt++;
		    nvram_set (u->name, u->value);
		    break;
		  }
	      }
	    if (!u || !u->name)
	      {
		nvcnt++;
		nvram_set (t->name, t->value);
	      }
	  }
      }
  }
#ifndef HAVE_FON
  if (restore_defaults)		//fix for belkin std ip
    {
      if (nvram_match ("boardnum", "WAP54GV3_8M_0614"))
	{
	  nvram_set ("vlan0ports", "3 2 1 0 5*");
	  nvram_set ("vlan1ports", "4 5");
	}
      nvram_set ("lan_ipaddr", "192.168.1.1");
    }
#else
  if (restore_defaults)		//fix for belkin std ip
    {
      nvram_set ("lan_ipaddr", "192.168.10.1");
    }
#endif
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
	case ROUTER_ASUS_WL500G_PRE:
	  nvram_set ("vlan0ports", "1 2 3 4 5*");
	  break;
	case ROUTER_LINKSYS_WRT55AG:
	case ROUTER_MOTOROLA_V1:
	case ROUTER_SIEMENS:
	case ROUTER_BELKIN_F5D7230:
	  nvram_set ("vlan0ports", "0 1 2 3 5*");
	  break;
	default:
	  if (nvram_match ("bootnv_ver", "4")
	      || nvram_match ("boardnum", "WAP54GV3_8M_0614"))
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
	    case ROUTER_LINKSYS_WRT55AG:
	    case ROUTER_MOTOROLA_V1:
	    case ROUTER_SIEMENS:
	    case ROUTER_BELKIN_F5D7230:
	      nvram_set ("vlan1ports", "4 5");
	      break;
	    default:
	      if (nvram_match ("bootnv_ver", "4")
		  || nvram_match ("boardnum", "WAP54GV3_8M_0614"))
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

  if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G1X
      || brand == ROUTER_LINKSYS_WRT55AG || brand == ROUTER_MOTOROLA_V1)
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

#ifdef HAVE_DDLAN
  nvram_unset ("cur_rssi");
  nvram_unset ("cur_noise");
  nvram_unset ("cur_bssid");
  nvram_unset ("cur_snr");
  nvram_set ("cur_state",
	     "<span style=\"background-color: rgb(255, 0, 0);\">Nicht Verbunden</span>");
#endif
#ifdef HAVE_SPUTNIK_APD
  /* Added for Sputnik Agent */
  nvram_unset ("sputnik_mjid");
  nvram_unset ("sputnik_rereg");
#endif
  if (nvram_get ("overclocking") == NULL)
    nvram_set ("overclocking", nvram_safe_get ("clkfreq"));
  cprintf ("start overclocking\n");
  overclock ();
  cprintf ("done()");
  if (nvram_get ("http_username") != NULL)
    {
      if (nvram_match ("http_username", ""))
	{
#ifdef HAVE_POWERNOC
	  nvram_set ("http_username", "admin");
#else
	  nvram_set ("http_username", "root");
#endif
	}
    }
#ifdef DIST
  nvram_set ("dist_type", DIST);
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
//              nvram_set ("need_commit","1");
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
//nvram_set ("need_commit","1");



}


int
start_sysinit (void)
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
  // fix for linux kernel 2.6
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
#endif
  eval ("mkdir", "/tmp/www");

#ifdef HAVE_RB500
  //load ext2 
  // eval("insmod","jbd");
  eval ("insmod", "ext2");
#ifndef KERNEL_24
  if (mount ("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL))
#else
  if (mount
      ("/dev/discs/disc0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL))
#endif
    {
      //not created yet, create ext2 partition
      eval ("/sbin/mke2fs", "-F", "-b", "1024", "/dev/cf/card0/part3");
      //mount ext2 
      mount ("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL);
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

  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");

  int brand = getRouterBrand ();
  if (brand == ROUTER_BUFFALO_WZRRSG54)
    {
      check_brcm_cpu_type ();
    }
  if (brand == ROUTER_MOTOROLA)
    {
      nvram_set ("cpu_type", "BCM4712");
      nvram_set ("wl0gpio0", "2");	//Fix for wireless led, Eko.10.may.06
    }

  if (brand == ROUTER_SIEMENS || brand == ROUTER_MOTOROLA
      || brand == ROUTER_RT210W || brand == ROUTER_BUFFALO_WZRRSG54
      || brand == ROUTER_BELKIN_F5D7230)
    {
      setup_4712 ();
    }

  if (brand == ROUTER_RT210W)
    {
      nvram_set ("wan_ifname", "eth1");	// fix for Belkin f5d7230 v1000 WAN problem.
      nvram_set ("wan_ifnames", "eth1");
    }
  if (brand == ROUTER_WRTSL54GS)
    {
      nvram_set ("wan_ifname", "eth1");
      nvram_set ("wan_ifnames", "eth1");
      nvram_set ("pppoe_wan_ifname", "eth1");
    }
  if (brand == ROUTER_ASUS_WL500G_PRE)
    {
	  nvram_set ("lan_ifnames", "vlan0 eth2");
	  nvram_set ("wl0_ifname", "eth2");
	  strcpy (wlifname, "eth2");
      nvram_set ("wan_ifname", "vlan1");	// fix for Asus WL500gPremium WAN problem.
      nvram_set ("wan_ifnames", "vlan1");
	  strcpy (wanifname, "vlan1");
      nvram_set ("vlan1ports", "0 5");
      eval ("gpio", "disable", "1");	//Asus WL-500gP power led on
    }
  if ((brand == ROUTER_MICROSOFT_MN700) && nvram_match ("boardnum", "mn700"))
    {
      eval ("gpio", "enable", "6");	//MN700 power led on
    }

  if (brand == ROUTER_BUFFALO_WBR2G54S)
    {
      eval ("gpio", "disable", "1");	//WBR2G54 diag led off
    }
    
  if (brand == ROUTER_BUFFALO_WLA2G54C)
    {
      eval ("gpio", "enable", "4");	//WLA2-G54C, WLA3-TX1-G54 diag led off
    }
    
  if ((brand == ROUTER_BUFFALO_WLA2G54C) || (nvram_match ("boardnum", "1024") && nvram_match ("boardtype", "0x0446")))
    {
      nvram_set ("lan_ifnames", "eth0 eth1");	// fix for WLA2G54C & WAP54Gv2 interfaces
      nvram_set ("wl0_ifname", "eth1");
      strcpy (wlifname, "eth1");
      nvram_set ("wan_ifname", "eth2");	// map WAN port to nonexistant interface
      nvram_set ("wan_ifnames", "eth2");
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
	    case ROUTER_LINKSYS_WRT55AG:
	    case ROUTER_MOTOROLA_V1:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-adm";

	      break;
	    case ROUTER_WRT54G1X:
	    case ROUTER_WRT54G:
	    case ROUTER_SIEMENS:
	    case ROUTER_RT210W:
	    case ROUTER_BELKIN_F5D7230:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      eval ("insmod", "switch-core");
	      if (eval ("insmod", "switch-robo"))
		eval ("insmod", "switch-adm");

	      break;
//          case ROUTER_BUFFALO_WBR54G:
	    case ROUTER_MOTOROLA:
	    case ROUTER_BUFFALO_WBR2G54S:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-robo switch-adm";
	      break;
	    default:
//          case ROUTER_MOTOROLA:
//          case ROUTER_BELKIN:
//          case ROUTER_BUFFALO_WBR2G54S:
//          case ROUTER_ASUS:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-robo";
	      break;
	    }
	}
      else
	{
	  switch (brand)
	    {
	    case ROUTER_LINKSYS_WRT55AG:
	    case ROUTER_MOTOROLA_V1:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-adm";

	      break;
	    case ROUTER_ASUS:
	    case ROUTER_BELKIN_F5D7230:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      eval ("insmod", "switch-core");
	      if (eval ("insmod", "switch-robo"))
		eval ("insmod", "switch-adm");
	      break;
	    case ROUTER_BUFFALO_WZRRSG54:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      break;
	    default:
	      if (check_vlan_support ())
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag wl switch-core switch-robo";
	      else
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag wl";
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
/*insmod("diag");
insmod("wl");
if (check_vlan_support())
    {
    insmod("switch-core");
    if (insmod("switch-robo"))
	insmod("switch-adm");
    }
*/
	cprintf ("loading %s\n", module);
	eval ("insmod", module);
	cprintf ("done\n");
#endif
      }
    }
#else
//  eval ("insmod", "mii");
//  eval ("insmod", "korina");
//  eval ("insmod", "via-rhine");
  eval ("insmod", "ipv6");
//  load_drivers(); //load madwifi drivers
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

  return 0;
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


#define CONVERT_NV(old, new) \
	if(nvram_get(old)) \
		nvram_set(new, nvram_safe_get(old));

static void
to64 (char *s, long v, int n)
{

  unsigned char itoa64[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  while (--n >= 0)
    {
      *s++ = itoa64[v & 0x3f];
      v >>= 6;
    }
}

static char *
zencrypt (char *passwd)
{
  char salt[6];
  struct timeval tv;
  char *crypt (const char *, const char *);

  gettimeofday (&tv, 0);

  to64 (&salt[0], random (), 3);
  to64 (&salt[3], tv.tv_usec, 3);
  salt[5] = '\0';

  return crypt (passwd, salt);
}

int
start_nvram (void)
{
  int i = 0;

  /* broadcom 3.11.48.7 change some nvram name */
#ifdef HAVE_MSSID
  CONVERT_NV ("d11g_channel", "wl_channel");
#else
  CONVERT_NV ("d11g_channel", "wl0_channel");
#endif
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

#ifdef HAVE_UPNP
  if ((nvram_match ("restore_defaults", "1"))
      || (nvram_match ("upnpcas", "1")))
    {
      nvram_set ("upnp_clear", "1");
    }
  else
    {
      char s[32];
      char *nv;
      for (i = 0; i < MAX_NVPARSE; ++i)
	{
	  sprintf (s, "forward_port%d", i);
	  if ((nv = nvram_get (s)) != NULL)
	    {
	      if (strstr (nv, "msmsgs"))
		nvram_unset (s);
	    }
	}
    }
  nvram_set ("upnp_wan_proto", "");
#endif

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
//  nvram_set ("wl0_country_code", "JP");
//  nvram_set ("wl0_country", "Japan");
  nvram_set ("wl0_country_code", "ALL");
  nvram_set ("wl0_country", "<unknown>");
  if (check_hw_type () == BCM5352E_CHIP)
    {
      nvram_set ("opo", "0x0008");
      nvram_set ("ag0", "0x02");
    }
// end Sveasoft addition
// Fix for newer stylesheet settings, BrainSlayer, Eko
  char style[32];

//  {"svqos_port1bw", "FULL", 0},
//  {"svqos_port2bw", "FULL", 0},
//  {"svqos_port3bw", "FULL", 0},
//  {"svqos_port4bw", "FULL", 0},

  if (nvram_match ("svqos_port1bw", "full"))
    nvram_set ("svqos_port1bw", "FULL");
  if (nvram_match ("svqos_port2bw", "full"))
    nvram_set ("svqos_port2bw", "FULL");
  if (nvram_match ("svqos_port3bw", "full"))
    nvram_set ("svqos_port3bw", "FULL");
  if (nvram_match ("svqos_port4bw", "full"))
    nvram_set ("svqos_port4bw", "FULL");

  if (nvram_get("nvram_ver")==NULL || !nvram_match ("nvram_ver","2"))
    {
    nvram_set("http_username","root");
    nvram_set("http_passwd","admin");
    nvram_set("http_passwd",zencrypt(nvram_safe_get("http_passwd"))); 
    nvram_set("http_username",zencrypt(nvram_safe_get("http_username")));
    }

  strcpy (style, nvram_safe_get ("router_style"));

  {
    if (endswith (style, ".css"))
      {
	for (i = 0; i < strlen (style); i++)
	  if (style[i] == '.')
	    style[i] = 0;
      }
    nvram_set ("router_style", style);

    if (nvram_match ("router_style", "") || (nvram_get ("router_style") == NULL))	//if still not set, force to cyan
      nvram_set ("router_style", "cyan");

#ifdef DIST
    if (nvram_match ("dist_type", "micro"))	//if dist_type micro, force to cyan
      nvram_set ("router_style", "cyan");
#endif
  }

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

#define ISCLK(a) nvram_match("clkfreq",a);
static void
overclock (void)
{
  int rev = getcpurev ();
  char *ov = nvram_get ("overclocking");
  if (ov == NULL)
    return;
  int clk = atoi (ov);
  if (nvram_get ("clkfreq") == NULL)
    return;			//unsupported
  if (nvram_match ("clkfreq", "125"))
    return;			//unsupported
  if (rev == 0)
    return;			//unsupported

//int cclk = atoi(nvram_safe_get("clkfreq"));
//if (cclk<192)return; //unsupported
  char *pclk = nvram_safe_get ("clkfreq");
  char dup[64];
  strcpy (dup, pclk);
  int i;
  for (i = 0; i < strlen (dup); i++)
    if (dup[i] == ',')
      dup[i] = 0;
  int cclk = atoi (dup);
  if (cclk < 192 && rev == 7)
    {
      cprintf ("clkfreq is %d (%s), this is unsupported\n", cclk, dup);
      return;			//unsupported
    }
  if (cclk < 183 && rev == 8)
    {
      cprintf ("clkfreq is %d (%s), this is unsupported\n", cclk, dup);
      return;			//unsupported
    }

  if (clk == cclk)
    {
      cprintf ("clkfreq identical with new setting\n");
      return;			//clock already set
    }


  int set = 1;

  switch (clk)
    {
    case 183:
      nvram_set ("clkfreq", "183,92");
      break;
    case 187:
      nvram_set ("clkfreq", "187,94");
      break;
    case 192:
      nvram_set ("clkfreq", "192,96");
      break;
    case 198:
      nvram_set ("clkfreq", "198,98");
      break;
    case 200:
      nvram_set ("clkfreq", "200,100");
      break;
    case 216:
      nvram_set ("clkfreq", "216,108");
      break;
    case 225:
      nvram_set ("clkfreq", "225,113");
      break;
    case 228:
      nvram_set ("clkfreq", "228,114");
      break;
    case 233:
      nvram_set ("clkfreq", "233,116");
      break;
    case 237:
      nvram_set ("clkfreq", "237,119");
      break;
    case 240:
      nvram_set ("clkfreq", "240,120");
      break;
    case 250:
      nvram_set ("clkfreq", "250,125");
      break;
    case 252:
      nvram_set ("clkfreq", "252,126");
      break;
    case 264:
      nvram_set ("clkfreq", "264,132");
      break;
    case 280:
      nvram_set ("clkfreq", "280,120");
      break;
    case 300:
      nvram_set ("clkfreq", "300,120");
      break;
    default:
      set = 0;
      break;
    }

  if (set)
    {
      cprintf ("clock frequency adjusted from %d to %d, reboot needed\n",
	       cclk, clk);
      nvram_commit ();
      kill (1, SIGTERM);
      exit (0);
    }
}

void
start_overclocking (void)
{
  overclock ();
}

static int
check_cfe_nv (void)
{
  int ret = 0;
 //  ret += check_nv ("pa0maxpwr", "0xff");
  
//      ret += check_nv("boardtype", "0x0101");
//      ret += check_nv("boardflags", "0x0188");
//      ret += check_nv("boardflags", "0x0388");

//      ret += check_nv("boardrev", "0x10");
//      ret += check_nv("boardflags2", "0");
//      ret += check_nv("sromrev", "2");
  switch (getRouterBrand ())
    {
    case ROUTER_ASUS:
       ret += check_nv ("wl0_ifname", "eth1");
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
    case ROUTER_BELKIN_F5D7230:
// nothing for now
      break;
    case ROUTER_MOTOROLA:
      ret += check_nv ("wl0gpio0", "2");	//fix for wlan led, Eko
      break;
    case ROUTER_BUFFALO_WBR54G:
    case ROUTER_BUFFALO_WLAG54C:
    case ROUTER_BUFFALO_WZRRSG54:
//    case ROUTER_MICROSOFT_MN700:

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
    case ROUTER_LINKSYS_WRT55AG:
    case ROUTER_MOTOROLA_V1:
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

	  ret += check_nv ("sdram_init", "0x010b");
	  ret += check_nv ("sdram_config", "0x0062");
	  if (nvram_match ("clkfreq", "200")
	      && nvram_match ("overclocking", "200"))
	    {
	      ret += check_nv ("clkfreq", "216");
	      nvram_set ("overclocking", "216");
	    }

	  if (ret)
	    {
	      nvram_set ("sdram_ncdl", "0x0");

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

#if 0
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
#endif
