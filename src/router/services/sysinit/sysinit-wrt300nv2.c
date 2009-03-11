/*
 * sysinit-wrt300nv2.c
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
#include <cymac.h>

void start_sysinit( void )
{
    struct utsname name;
    struct stat tmp_stat;
    time_t tm = 0;

    unlink( "/etc/nvram/.lock" );
    cprintf( "sysinit() proc\n" );
    /*
     * /proc 
     */
    mount( "proc", "/proc", "proc", MS_MGC_VAL, NULL );
    // system2 ("/etc/convert");
    mount( "sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL );
    cprintf( "sysinit() tmp\n" );

    /*
     * /tmp 
     */
    mount( "ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL );
    mount( "devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL );
    /*
     * eval("mount","/etc/www.fs","/www","-t","squashfs","-o","loop");
     * eval("mount","/etc/modules.fs","/lib/modules","-t","squashfs","-o","loop");
     * eval("mount","/etc/usr.fs","/usr","-t","squashfs","-o","loop"); 
     */
    eval( "mkdir", "/tmp/www" );
    eval( "mknod", "/dev/gpio", "c", "127", "0" );
    eval( "mknod", "/dev/nvram", "c", "229", "0" );
    eval( "mknod", "/dev/ppp", "c", "108", "0" );
    eval( "mknod", "/dev/rtc", "c", "254", "0" );
    eval( "mknod", "/dev/crypto", "c", "10", "70" );
    eval( "mount", "-o", "remount,rw", "/" );

    unlink( "/tmp/nvram/.lock" );
    eval( "mkdir", "/tmp/nvram" );
    // #ifdef HAVE_REGISTER
    // #else
    // eval ("/bin/tar", "-xzf", "/dev/mtdblock/3", "-C", "/");
    // #endif
    // mkdir ("/usr/local/nvram", 0777);
    // unlink ("/tmp/nvram/.lock");
    // eval ("mkdir", "/tmp/nvram");
    // eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
    // eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
    cprintf( "sysinit() var\n" );

    /*
     * /var 
     */
    mkdir( "/tmp/var", 0777 );
    mkdir( "/var/lock", 0777 );
    mkdir( "/var/log", 0777 );
    mkdir( "/var/run", 0777 );
    mkdir( "/var/tmp", 0777 );

    if (!nvram_match("disable_watchdog","1"))
    eval( "watchdog" );		// system watchdog

    cprintf( "sysinit() setup console\n" );

    /*
     * Setup console 
     */

    cprintf( "sysinit() klogctl\n" );
    klogctl( 8, NULL, atoi( nvram_safe_get( "console_loglevel" ) ) );
    cprintf( "sysinit() get router\n" );

    /*
     * Modules 
     */
    uname( &name );
    nvram_set( "intel_eth", "0" );

#if 1
    insmod( "ixp400th" );
    insmod( "ixp400" );
    system2( "cat /usr/lib/firmware/IxNpeMicrocode.dat > /dev/IxNpe" );
    insmod( "ixp400_eth" );
    eval( "ifconfig", "ixp0", "0.0.0.0", "up" );
    eval( "ifconfig", "ixp1", "0.0.0.0", "up" );
    insmod( "ocf" );
    insmod( "cryptodev" );
    // insmod("ixp4xx", "init_crypto=0");
#else
    // eval ("mknod", "/dev/IxNpe","c","10","184");
    system2( "cat /usr/lib/firmware/NPE-B > /dev/misc/ixp4xx_ucode" );
    system2( "cat /usr/lib/firmware/NPE-C > /dev/misc/ixp4xx_ucode" );
#endif

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

#ifdef HAVE_MADWIFI_MIMO
    insmod( "ath_mimo_pci" );
#endif
    // insmod("ath_pci", "rfkill=0", "autocreate=none");

    /*
     * if (ifexists ("wifi0")) eval ("ifconfig", "wifi0", "up"); if (ifexists 
     * ("wifi1")) eval ("ifconfig", "wifi1", "up"); if (ifexists ("wifi2"))
     * eval ("ifconfig", "wifi2", "up"); if (ifexists ("wifi3")) eval
     * ("ifconfig", "wifi3", "up"); if (ifexists ("wifi4")) eval ("ifconfig", 
     * "wifi4", "up"); if (ifexists ("wifi5")) eval ("ifconfig", "wifi5",
     * "up"); 
     */

    /*
     * Configure mac addresses by reading data from eeprom 
     */
    // char *filename =
    // "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom"; /*
    // bank2=0x100 */
    char *filename = "/dev/mtdblock/0";	/* bank2=0x100 */
    FILE *file = fopen( filename, "r" );

    if( file )
    {
	unsigned char buf[16];

	fseek( file, 0x5ffa0, SEEK_SET );	// point of mac address
	fread( &buf[0], 6, 1, file );
	char mac[16];

	sprintf( mac, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2],
		 buf[3], buf[4], buf[5] );
	fprintf( stderr, "configure primary mac %s\n", mac );
	eval( "ifconfig", "ixp0", "hw", "ether", mac );
	eval( "ifconfig", "wifi0", "hw", "ether", mac );
	nvram_set( "et0macaddr", mac );
	MAC_ADD( mac );
	fprintf( stderr, "configure secondary mac %s\n", mac );
	eval( "ifconfig", "ixp1", "hw", "ether", mac );

	fclose( file );
    }
    eval( "ifconfig", "ixp0", "0.0.0.0", "up" );
    eval( "ifconfig", "ixp1", "0.0.0.0", "up" );

    /*
     * Set a sane date 
     */
    stime( &tm );
    nvram_set( "wl0_ifname", "ath0" );

    nvram_set( "use_crypto", "0" );
    cprintf( "done\n" );
    return;
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
void enable_dtag_vlan( int enable )
{

}
