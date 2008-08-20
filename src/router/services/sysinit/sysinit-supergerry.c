/*
 * sysinit-supergerry.c
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
  char buf[PATH_MAX];
  struct utsname name;
  struct stat tmp_stat;
  time_t tm = 0;
  unlink ("/etc/nvram/.lock");
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
  mount ("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
  // fix for linux kernel 2.6
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
  eval ("mkdir", "/tmp/www");
  eval ("mknod", "/dev/ppp", "c", "108", "0");

  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("/bin/tar", "-xzf", "/dev/mtdblock4", "-C", "/");
  // eval ("/bin/tar", "-xzf", "/dev/mtdblock/2", "-C", "/");
  cprintf ("sysinit() var\n");

  /* /var */
  mkdir ("/tmp/var", 0777);
  mkdir ("/var/lock", 0777);
  mkdir ("/var/log", 0777);
  mkdir ("/var/run", 0777);
  mkdir ("/var/tmp", 0777);
  cprintf ("sysinit() setup console\n");
  eval ("watchdog");
  /* Setup console */

  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");

  int brand = getRouterBrand ();
/* test -e /lib/modules/$KERNELVER/extra/profdrvdd.ko && insmod /lib/modules/$KERNELVER/extra/profdrvdd.ko
 test -e /lib/modules/$KERNELVER/extra/atmapi.ko && insmod /lib/modules/$KERNELVER/extra/atmapi.ko
 test -e /lib/modules/$KERNELVER/extra/atmbonding.ko && insmod /lib/modules/$KERNELVER/extra/atmbonding.ko
 test -e /lib/modules/$KERNELVER/extra/atmbondingeth.ko && insmod /lib/modules/$KERNELVER/extra/atmbondingeth.ko
 test -e /lib/modules/$KERNELVER/extra/adsldd.ko && insmod /lib/modules/$KERNELVER/extra/adsldd.ko
 test -e /lib/modules/$KERNELVER/extra/blaa_dd.ko && insmod /lib/modules/$KERNELVER/extra/blaa_dd.ko
 test -e /lib/modules/$KERNELVER/kernel/net/atm/br2684.ko && insmod /lib/modules/$KERNELVER/kernel/net/atm/br2684.ko
 test -e /lib/modules/$KERNELVER/extra/bcmprocfs.ko && insmod /lib/modules/$KERNELVER/extra/bcmprocfs.ko
 test -e /lib/modules/$KERNELVER/extra/bcm_enet.ko && insmod /lib/modules/$KERNELVER/extra/bcm_enet.ko
 test -e /lib/modules/$KERNELVER/extra/bcm_usb.ko && insmod /lib/modules/$KERNELVER/extra/bcm_usb.ko
 test -e /lib/modules/$KERNELVER/extra/wl.ko && insmod /lib/modules/$KERNELVER/extra/wl.ko
 test -e /lib/modules/$KERNELVER/extra/dspdd.ko && insmod /lib/modules/$KERNELVER/extra/dspdd.ko
 test -e /lib/modules/$KERNELVER/extra/endpointdd.ko && insmod /lib/modules/$KERNELVER/extra/endpointdd.ko
*/
  insmod("atmapi");
  insmod("adsldd");
  insmod("blaa_dd");
  insmod("bcmprocfs");
  insmod("bcm_enet");


  /* Modules */
  uname (&name);
/* network drivers */
  insmod("ath_hal");
  insmod("ath_pci");

/*  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");
*/

  // insmod("ipv6");

  /* Set a sane date */
  stime (&tm);

  return 0;
  cprintf ("done\n");
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
