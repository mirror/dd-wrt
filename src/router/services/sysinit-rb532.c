/*
 * sysinit-rb532.c
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

int
start_sysinit (void)
{
  char buf[PATH_MAX];
  struct utsname name;
  struct stat tmp_stat;
  time_t tm = 0;
  unlink ("/etc/nvram/.lock");
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
  // fix for linux kernel 2.6
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
  eval ("mkdir", "/tmp/www");

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


  /* Modules */
  uname (&name);

  eval ("insmod", "md5");
  eval ("insmod", "aes");
  eval ("insmod", "blowfish");
  eval ("insmod", "deflate");
  eval ("insmod", "des");
  eval ("insmod", "michael_mic");
  eval ("insmod", "cast5");
  eval ("insmod", "crypto_null");

  system ("/etc/kendin");
  eval ("insmod", "ixp400th");
  eval ("insmod", "ixp400");
  system ("cat /usr/lib/firmware/IxNpeMicrocode.dat > /dev/IxNpe");
  eval ("insmod", "ixp400_eth");
  eval ("insmod", "ocf");
  eval ("insmod", "cryptodev");
  eval ("insmod", "ixp4xx", "init_crypto=0");
  eval ("ifconfig", "ixp0", "0.0.0.0", "up");
  eval ("vconfig", "add", "ixp0", "1");
  eval ("vconfig", "add", "ixp0", "2");

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


//  eval ("insmod", "mii");
//  eval ("insmod", "korina");
//  eval ("insmod", "via-rhine");
  eval ("insmod", "ipv6");
  /* Set a sane date */
  stime (&tm);
  if (brand == ROUTER_SIEMENS)
    {
      eval ("insmod", "led.o");	// Jerry Lee
      powerled_ctrl (0);
      led_ctrl (0);		// turn LED2 off
    }

  return 0;
  cprintf ("done\n");
}
