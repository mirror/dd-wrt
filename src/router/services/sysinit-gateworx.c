/*
 * sysinit-gateworx.c
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
  system ("/etc/convert");
  mount ("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
/*  eval("mount","/etc/www.fs","/www","-t","squashfs","-o","loop");
  eval("mount","/etc/modules.fs","/lib/modules","-t","squashfs","-o","loop");
  eval("mount","/etc/usr.fs","/usr","-t","squashfs","-o","loop");
 */
  eval ("mkdir", "/tmp/www");

  eval ("mount", "-o", "remount,rw", "/");

  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
//#ifdef HAVE_REGISTER
  eval ("/bin/tar", "-xzf", "/dev/mtdblock/4", "-C", "/");
//#else
//  eval ("/bin/tar", "-xzf", "/dev/mtdblock/3", "-C", "/");
//#endif
//mkdir ("/usr/local/nvram", 0777);
//unlink ("/tmp/nvram/.lock");
//eval ("mkdir", "/tmp/nvram");
//eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
//eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
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
  eval ("insmod", "ixp400th");
  eval ("insmod", "ixp400");
  system ("cat /usr/lib/firmware/IxNpeMicrocode.dat > /dev/IxNpe");
  eval ("insmod", "ixp400_eth");
  eval ("insmod", "ocf");
  eval ("insmod", "cryptodev");
  eval ("insmod", "ixp4xx", "init_crypto=0");
  eval ("ifconfig", "ixp0", "0.0.0.0", "up");

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

  eval ("insmod", "ad7418");	// temp / voltage sensor
/*
Configure mac addresses by reading data from eeprom
*/
  char *filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom";	/* bank2=0x100 */
  FILE *file = fopen (filename, "r");
  unsigned char buf[16];
  fread (&buf[0], 16, 1, file);
  char mac[16];
  sprintf (mac, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2],
	   buf[3], buf[4], buf[5]);
  eval ("ifconfig", "ixp0", "hw", "ether", mac);
  sprintf (mac, "%02x:%02x:%02x:%02x:%02x:%02x", buf[6], buf[7], buf[8],
	   buf[9], buf[10], buf[11]);
  eval ("ifconfig", "ixp1", "hw", "ether", mac);


  /* Set a sane date */
  stime (&tm);

  eval ("mknod", "/dev/gpio", "c", "127", "0");
  eval ("mknod", "/dev/rtc", "c", "253", "0");
  eval ("hwclock", "-s");
  return 0;
  cprintf ("done\n");
}
