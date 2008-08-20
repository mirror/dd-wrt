/*
 * sysinit-x86.c
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
 * 
 * System Initialisation for Standard PC and compatible Routers
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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>


static int
detect (char *devicename)
{
  FILE *tmp = fopen ("/tmp/devices", "rb");
  if (tmp == NULL)
    {
      system ("/sbin/lspci>/tmp/devices");
    }
  else
    fclose (tmp);
  char devcall[128];
  int res;
  sprintf (devcall, "cat /tmp/devices|/bin/grep \"%s\"|/bin/wc -l",
	   devicename);
//system(devcall);
  FILE *in = popen (devcall, "rb");
  fscanf (in, "%d", &res);
  pclose (in);
  return res > 0 ? 1 : 0;
}
static int
getdiscindex (void)		//works only for squashfs 
{
  int i;
  for (i = 0; i < 10; i++)
    {
      char dev[64];
      sprintf (dev, "/dev/discs/disc%d/part2", i);
      FILE *in = fopen (dev, "rb");
      if (in == NULL)
	continue;		//no second partition or disc does not exist, skipping
      char buf[4];
      fread (buf, 4, 1, in);
      if (buf[0] == 'h' && buf[1] == 's' && buf[2] == 'q' && buf[3] == 't')
	{
	  fclose (in);
	  //filesystem detected
	  return i;
	}
      fclose (in);
    }
  return -1;
}

int
start_sysinit (void)
{
  struct utsname name;
  time_t tm = 0;
  unlink ("/etc/nvram/.lock");
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
//  system ("/etc/convert");
  mount ("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
  mount ("debugfs", "/sys/kernel/debug", "debugfs", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
  eval ("mknod", "/dev/ppp", "c", "108", "0");
  char dev[64];
  int index = getdiscindex ();
  if (index == -1)
    {
      fprintf (stderr, "no valid dd-wrt partition found, calling shell");
      eval ("/bin/sh");
      exit (0);
    }
//  sprintf (dev, "/dev/discs/disc%d/part1", index);
//  mount (dev, "/boot", "ext2", MS_MGC_VAL, NULL);

  sprintf (dev, "/dev/discs/disc%d/part3", index);
//  eval("insmod","block2mtd",dev);
//  sprintf (dev, "/dev/mtdblock/0");
  //eval ("fsck", dev);         //checking nvram partition and correcting errors
  //detect jffs
/*  FILE *fp=fopen(dev,"rb");
  int nojffs=0;
  if (getc(fp)!=0x85)nojffs=1;
  if (getc(fp)!=0x19)nojffs=1;
  fclose(fp);*/
//      eval("mtd","erase","mtd0");

  if (mount (dev, "/usr/local", "ext2", MS_MGC_VAL, NULL))
    {
      eval ("/sbin/mke2fs", "-F", "-b", "1024", dev);
      mount (dev, "/usr/local", "ext2", MS_MGC_VAL, NULL);
      eval ("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
    }
  eval ("mkdir", "-p", "/usr/local/nvram");
  eval ("mkdir", "/tmp/www");

  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
//  eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
  eval ("mount", "/usr/local", "-o", "remount,ro");


  cprintf ("sysinit() var\n");

  /* /var */
  mkdir ("/tmp/var", 0777);
  mkdir ("/var/lock", 0777);
  mkdir ("/var/log", 0777);
  mkdir ("/var/run", 0777);
  mkdir ("/var/tmp", 0777);

  eval ("watchdog");	// system watchdog

  cprintf ("sysinit() setup console\n");

  /* Setup console */

  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");


  /* Modules */
  uname (&name);

/*eval("insmod","md5");
eval("insmod","aes");
eval("insmod","blowfish");
eval("insmod","deflate");
eval("insmod","des");
eval("insmod","michael_mic");
eval("insmod","cast5");
eval("insmod","crypto_null");
*/

//system("/etc/kendin");
  if (detect ("Rhine-"))	// VIA Rhine-I, Rhine-II, Rhine-III
    insmod("via-rhine");
  if (detect ("DP8381"))
    insmod("natsemi");
  if (detect ("PCnet32"))	//vmware?
    insmod("pcnet32");
  if (detect ("Tigon3"))	//Broadcom 
    insmod("tg3");
  else if (detect ("NetXtreme"))	//Broadcom 
    insmod("tg3");
  if (detect ("NetXtreme II"))	//Broadcom 
    insmod("bnx2");
  if (detect ("BCM44"))		//Broadcom 
    insmod("b44");


  if (detect ("EtherExpress PRO/100"))	//intel 100 mbit 
    insmod("e100");
  else if (detect ("PRO/100"))	//intel 100 mbit
    insmod("e100");
  else if (detect ("8280"))	//intel 100 mbit 
    insmod("e100");
  else if (detect ("Ethernet Pro 100"))	//intel 100 mbit 
    insmod("e100");
  else if (detect ("8255"))	//intel 100 mbit 
    insmod("eepro100");


  if (detect ("PRO/1000"))	//Intel Gigabit 
    insmod("e1000");
  else if (detect ("82541"))	// Intel Gigabit
    insmod("e1000");
  else if (detect ("82547"))	// Intel Gigabit
    insmod("e1000");
  else if (detect ("82546"))	// Intel Gigabit
    insmod("e1000");
  else if (detect ("82545"))	// Intel Gigabit / VMWare 64 bit mode (nice trick to get gigabit out of it)
    insmod("e1000");
  else if (detect ("82543"))	// Intel Gigabit / VMWare 64 bit mode (nice trick to get gigabit out of it)
    insmod("e1000");
  else if (detect ("82572"))	// Intel Gigabit 
    insmod("e1000");


  if (detect ("RTL-8110"))	// Realtek 8169 Adapter (various notebooks) 
    insmod("r8169");
  else if (detect ("RTL-8169"))	// Realtek 8169 Adapter (various notebooks) 
    insmod("r8169");
  else if (detect ("RTL8101"))	// Realtek 8169 Adapter (various notebooks) 
    insmod("r8169");

  if (detect ("8139"))		// Realtek 8139 Adapter (various notebooks) 
    insmod("8139too");
  if (detect ("DFE-690TXD"))	// Realtek 8139 Adapter (various notebooks) 
    insmod("8139too");
  else if (detect ("SMC2-1211TX"))	// Realtek 8139 Adapter (various notebooks) 
    insmod("8139too");
  else if (detect ("Robotics"))	// Realtek 8139 Adapter (various notebooks) 
    insmod("8139too");


  if (detect ("nForce2 Ethernet"))	// nForce2 
    insmod("forcedeth");
  else if (detect ("nForce3 Ethernet"))	// nForce3 
    insmod("forcedeth");
  else if (detect ("nForce Ethernet"))	// nForce 
    insmod("forcedeth");
  else if (detect ("CK804 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("CK8S Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP04 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP2A Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP51 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP55 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP61 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP65 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP67 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP67 Gigabit"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP73 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP77 Ethernet"))	// nForce
    insmod("forcedeth");
  else if (detect ("MCP79 Ethernet"))	// nForce
    insmod("forcedeth");

  if (detect ("Sundance"))	//Dlink fibre
    insmod("sundance");
  else if (detect ("DL10050"))
    insmod("sundance");


  if (detect ("88E8001"))	//Marvell Yukon
    insmod("sk98lin");
  else if (detect ("RDK-"))
    insmod("sk98lin");
  else if (detect ("SK-98"))
    insmod("sk98lin");
  else if (detect ("3c940"))
    insmod("sk98lin");
  else if (detect ("Marvell"))
    insmod("sk98lin");


  if (detect ("RTL-8029"))	// Old Realtek PCI NE2000 clone (10M only)
    {
      insmod("8390");
      insmod("ne2k-pci");
    }


  if (detect ("3c905"))		// 3Com
    insmod("3c59x");
  else if (detect ("3c555"))	// 3Com
    insmod("3c59x");
  else if (detect ("3c556"))	// 3Com
    insmod("3c59x");
  else if (detect ("ScSOHO100"))	// 3Com
    insmod("3c59x");
  else if (detect ("Hurricane"))	// 3Com
    insmod("3c59x");

  if (detect ("LNE100TX"))	// liteon / linksys
    insmod("tulip");
  else if (detect ("FasterNet"))
    insmod("tulip");
  else if (detect ("ADMtek NC100"))
    insmod("tulip");
  else if (detect ("910-A1"))
    insmod("tulip");
  else if (detect ("tulip"))
    insmod("tulip");
  else if (detect ("DECchip 21142"))
    insmod("tulip");
  else if (detect ("MX987x5"))
    insmod("tulip");

  if (detect ("DGE-530T"))
    insmod("skge");
  else if (detect ("D-Link Gigabit"))
    insmod("skge");

  if (detect ("SiS900"))	// Sis 900
    insmod("sis900");

  eval ("ifconfig", "eth0", "0.0.0.0", "up");
  eval ("ifconfig", "eth1", "0.0.0.0", "up");
  eval ("ifconfig", "eth2", "0.0.0.0", "up");
  eval ("ifconfig", "eth3", "0.0.0.0", "up");

  struct ifreq ifr;
  int s;
  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)))
    {
      char eabuf[32];
      strncpy (ifr.ifr_name, "eth0", IFNAMSIZ);
      ioctl (s, SIOCGIFHWADDR, &ifr);
      nvram_set ("et0macaddr_safe",
		 ether_etoa ((unsigned char *) ifr.ifr_hwaddr.sa_data,
			     eabuf));
      close (s);
    }
#ifndef HAVE_NOWIFI
  insmod("ath_hal");
  insmod("ath_pci");
#ifdef HAVE_MADWIFI_MIMO
  insmod("ath_mimo_pci");
#endif

/*  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");*/
#endif

  insmod("ipv6");
  eval ("mknod", "/dev/rtc", "c", "253", "0");
#ifdef HAVE_CPUTEMP
//  insmod("nsc_gpio");
//  insmod("scx200_gpio");
//  insmod("scx200_i2c");
//  insmod("scx200_acb");
//  insmod("lm77");
#endif
  if (detect ("SafeXcel-1141"))
    {
      insmod("ocf");
      insmod("cryptodev");
      insmod("safe");
      nvram_set ("use_crypto", "1");
    }
  else
    nvram_set ("use_crypto", "0");


  nvram_set ("wl0_ifname", "ath0");
  eval ("mknod", "/dev/crypto", "c", "10", "70");
  /* Set a sane date */
  stime (&tm);
  cprintf ("done\n");
  return 0;
}

int
check_cfe_nv (void)
{
  nvram_set ("portprio_support", "0");
  return 0;
}

int
check_pmon_nv (void)
{
  return 0;
}

void
start_overclocking (void)
{
}
void
enable_dtag_vlan (int enable)
{

}
