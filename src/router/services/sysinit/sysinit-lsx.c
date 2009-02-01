/*
 * sysinit-pb42.c
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
#include <sys/ioctl.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>

void start_sysinit( void )
{
    char buf[PATH_MAX];
    struct utsname name;
    struct stat tmp_stat;
    time_t tm = 0;

    unlink( "/etc/nvram/.lock" );
    cprintf( "sysinit() proc\n" );
    /*
     * /proc 
     */
    mount( "proc", "/proc", "proc", MS_MGC_VAL, NULL );
    mount( "sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL );
    cprintf( "sysinit() tmp\n" );

    /*
     * /tmp 
     */
    mount( "ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL );
    // fix for linux kernel 2.6
    mount( "devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL );
    eval( "mkdir", "/tmp/www" );
    eval( "mknod", "/dev/nvram", "c", "229", "0" );
    eval( "mknod", "/dev/ppp", "c", "108", "0" );

    unlink( "/tmp/nvram/.lock" );
    eval( "mkdir", "/tmp/nvram" );

    /*
     * /var 
     */
    mkdir( "/tmp/var", 0777 );
    mkdir( "/var/lock", 0777 );
    mkdir( "/var/log", 0777 );
    mkdir( "/var/run", 0777 );
    mkdir( "/var/tmp", 0777 );
    cprintf( "sysinit() setup console\n" );
    eval( "watchdog" );
    /*
     * Setup console 
     */

    cprintf( "sysinit() klogctl\n" );
    klogctl( 8, NULL, atoi( nvram_safe_get( "console_loglevel" ) ) );
    cprintf( "sysinit() get router\n" );

    int brand = getRouterBrand(  );

    /*
     * Modules 
     */
    uname( &name );

    /*
     * network drivers 
     */
    insmod( "ag7100_mod" );
    // sleep(1);
    eval( "ifconfig", "eth0", "up" );
    struct ifreq ifr;
    int s;
#ifdef HAVE_RS
    FILE *fp=fopen("/dev/mtdblock/7","rb");
    char buf[256];
    char mac[32];
    fseek(fp,0xfff000,SEEK_SET);
    fread(mac,256,1,fp);
    fclose(fp);
    int i;
    for (i=0;i<256-10;i++)
	{
	if (!strncmp(&mac[i],"ar7100_esa",10))
	    {
	    break;
	    }
	}
    if (i<256)
    {
    i+=11;
    sprintf( mac, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0+i], buf[1+i],
		     buf[2+i], buf[3+i], buf[4+i], buf[5+i] );
    fprintf( stderr, "configure eth0 to %s\n", mac );
    eval("ifconfig","eth0","hw","ether",mac);
    fprintf( stderr, "configure eth1 to %s\n", mac );
    eval("ifconfig","eth1","hw","ether",mac);
    }
#endif
    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) )
    {
	char eabuf[32];
	strncpy( ifr.ifr_name, "eth0", IFNAMSIZ );
	ioctl( s, SIOCGIFHWADDR, &ifr );
	nvram_set( "et0macaddr",
		   ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.sa_data,
			       eabuf ) );
	nvram_set( "et0macaddr_safe",
		   ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.sa_data,
			       eabuf ) );
	close( s );
    }

    insmod( "ath_hal" );
    if( nvram_get( "rate_control" ) != NULL )
    {
	char rate[64];

	sprintf( rate, "ratectl=%s", nvram_safe_get( "rate_control" ) );
	eval( "insmod", "ath_pci", rate );
    }
    else
    {
	insmod( "ath_pci" );
    }
    // insmod("ath_mimo_pci");

#ifdef HAVE_RS
    system2( "echo 2 >/proc/sys/dev/wifi0/ledpin" );
    system2( "echo 1 >/proc/sys/dev/wifi0/softled" );
    system2( "echo 2 >/proc/sys/dev/wifi1/ledpin" );
    system2( "echo 1 >/proc/sys/dev/wifi1/softled" );
    system2( "echo 2 >/proc/sys/dev/wifi2/ledpin" );
    system2( "echo 1 >/proc/sys/dev/wifi2/softled" );
#else
    system2( "echo 2 >/proc/sys/dev/wifi0/ledpin" );
    system2( "echo 1 >/proc/sys/dev/wifi0/softled" );
#endif
    insmod( "ipv6" );

    /*
     * Set a sane date 
     */
    stime( &tm );
    nvram_set( "wl0_ifname", "ath0" );

    return;
    cprintf( "done\n" );
}

int check_cfe_nv( void )
{
    nvram_set( "portprio_support", "0" );
    return 0;
}

int check_pmon_nv( void )
{
    return 0;
}

void start_overclocking( void )
{
}
char *enable_dtag_vlan( int enable )
{
    return "eth0";
}
