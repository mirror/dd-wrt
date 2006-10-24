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
 * System Initialisation for Avila Gateworks and compatible Routers
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


static int detect(char *devicename)
{
char devcall[128];
int res;
sprintf(devcall,"/sbin/lspci|/bin/grep \"%s\"|/bin/wc -l",devicename);
//system(devcall);
FILE *in=popen(devcall,"rb");
fscanf(in,"%d",&res);
fclose(in);
return res>0?1:0;
} 
static int getdiscindex(void) //works only for squashfs 
{
int i;
for (i=0;i<10;i++)
    {
    char dev[64];
    sprintf(dev,"/dev/discs/disc%d/part2",i);
    FILE *in=fopen(dev,"rb");
    if (in==NULL)
	continue; //no second partition or disc does not exist, skipping
    char buf[4];
    fread(buf,4,1,in);
    if (buf[0]=='h' && buf[1]=='s' && buf[2]=='q' && buf[3]=='t')
	{
	fclose(in); 
	//filesystem detected
	return i;
	}
    fclose(in);
    }
return -1;
}

int
start_sysinit (void)
{
  struct utsname name;
  struct stat tmp_stat;
  time_t tm = 0;
  unlink ("/etc/nvram/.lock");
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
//  system ("/etc/convert");
  mount ("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
  char dev[64];
  int index=getdiscindex();
  if (index==-1)
    {
    fprintf(stderr,"no valid dd-wrt partition found, calling shell");
    eval("/bin/sh");
    exit(0);
    }
  sprintf(dev,"/dev/discs/disc%d/part1",index);
  mount (dev,"/boot","ext2",MS_MGC_VAL,NULL);
  sprintf(dev,"/dev/discs/disc%d/part3",index);
  if (mount(dev, "/usr/local", "ext2", MS_MGC_VAL, NULL))

    {
      //not created yet, create ext2 partition
      eval ("/sbin/mke2fs", "-F", "-b", "1024", dev);
      //mount ext2 
      mount (dev, "/usr/local", "ext2", MS_MGC_VAL, NULL);
      eval ("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
      mkdir ("/usr/local/nvram", 0777);
//    eval("ln","-s","/etc/nvram","/usr/local/nvram");
    }

  eval ("mkdir", "/tmp/www");
  
  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
  eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");


  cprintf ("sysinit() var\n");

  /* /var */
  mkdir ("/tmp/var", 0777);
  mkdir ("/var/lock", 0777);
  mkdir ("/var/log", 0777);
  mkdir ("/var/run", 0777);
  mkdir ("/var/tmp", 0777);

  eval ("/sbin/watchdog");	// system watchdog

  cprintf ("sysinit() setup console\n");

  /* Setup console */

  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");

  int brand = getRouterBrand ();

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
if (detect("DP8381"))  
    eval ("insmod", "natsemi");
if (detect("PCnet32"))  //vmware?
    eval ("insmod", "pcnet32");
if (detect("Tigon3"))  //Broadcom 
    eval ("insmod", "tg3");
else
if (detect("NetXtreme"))  //Broadcom 
    eval ("insmod", "tg3");
if (detect("NetXtreme II"))  //Broadcom 
    eval ("insmod", "bnx2");
if (detect("BCM44"))  //Broadcom 
    eval ("insmod", "b44");
if (detect("EtherExpress PRO/100"))  //intel 100 mbit 
    eval ("insmod", "e100");
else
if (detect("Ethernet Pro 100"))  //intel 100 mbit 
    eval ("insmod", "e100");
else
if (detect("8255"))  //intel 100 mbit 
    eval ("insmod", "e100");
    
if (detect("PRO/1000"))  //Intel Gigabit 
    eval ("insmod", "e1000");
else
if (detect("82541"))  // Intel Gigabit
    eval ("insmod", "e1000");
else
if (detect("82547"))  // Intel Gigabit
    eval ("insmod", "e1000");


if (detect("RTL-8169"))  // Realtek 8169 Adapter (various notebooks) 
    eval ("insmod", "r8169");
if (detect("8139"))  // Realtek 8169 Adapter (various notebooks) 
    eval ("insmod", "8139too");

if (detect("nForce2 Ethernet"))  // nForce2 
    eval ("insmod", "forcedeth");
else
if (detect("nForce3 Ethernet"))  // nForce3 
    eval ("insmod", "forcedeth");
else
if (detect("nForce Ethernet"))  // nForce 
    eval ("insmod", "forcedeth");
else
if (detect("CK804 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("CK8S Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP04 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP2A Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP51 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP55 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP61 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");
else
if (detect("MCP65 Ethernet"))  // nForce
    eval ("insmod", "forcedeth");

    
    
  eval ("ifconfig", "eth0", "0.0.0.0", "up");
  eval ("ifconfig", "eth1", "0.0.0.0", "up");
  eval ("ifconfig", "eth2", "0.0.0.0", "up");
  eval ("ifconfig", "eth3", "0.0.0.0", "up");

  eval ("insmod", "ath_hal");
  eval ("insmod", "wlan");
  eval ("insmod", "ath_rate_sample");
  eval ("insmod", "ath_pci", "rfkill=0", "autocreate=none");


  eval ("insmod", "wlan_acl");
  eval ("insmod", "wlan_ccmp");
  eval ("insmod", "wlan_tkip");
  eval ("insmod", "wlan_wep");
  eval ("insmod", "wlan_xauth");
  eval ("insmod", "wlan_scan_ap");
  eval ("insmod", "wlan_scan_sta");


  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");


  eval ("insmod", "ipv6");
  eval ("mknod", "/dev/rtc", "c", "253", "0");

  /* Set a sane date */
  stime (&tm);
  return 0;
  cprintf ("done\n");
}
