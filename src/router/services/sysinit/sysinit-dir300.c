/*
 * sysinit-dir300.c
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
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>

extern void vlan_init( int num );

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
    cprintf( "sysinit() var\n" );

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

#ifndef HAVE_DIR400
    int mtd = getMTD( "board_config" );
    char mtdpath[64];

    sprintf( mtdpath, "/dev/mtdblock/%d", mtd );
    FILE *fp = fopen( mtdpath, "rb" );

    if( fp )
    {
	fseek( fp, 0x1000, SEEK_SET );
	unsigned int test;

	fread( &test, 4, 1, fp );
	fprintf( stderr, "test pattern is %X\n", test );
	if( test != 0xffffffff )
	{
	    fprintf( stderr,
		     "radio config fixup is required to clean bad stuff out of memory, otherwise the radio config cannot be detected\n" );
	    fseek( fp, 0, SEEK_SET );
	    char *block = ( char * )malloc( 65536 );

	    fread( block, 65536, 1, fp );
	    fclose( fp );
	    int i;

	    for( i = 0x1000; i < 65536; i++ )
		block[i] = 0xff;
	    fp = fopen( "/tmp/radio", "wb" );
	    fwrite( block, 65536, 1, fp );
	    eval( "mtd", "-f", "write", "/tmp/radio", "board_config" );	// writes 
	    // 
	    // back 
	    // new 
	    // config 
	    // and 
	    // reboots
	    eval( "event", "5", "1", "15" );
	}
	fclose( fp );
    }
#else
    if( !nvram_match( "dir400preconfig", "1" ) )
    {
	nvram_set( "dir400preconfig", "1" );
	nvram_commit(  );
	int mtd = getMTD( "fullflash" );
	char mtdpath[64];
	char mac[18];

	sprintf( mtdpath, "/dev/mtdblock/%d", mtd );
	FILE *fp = fopen( mtdpath, "rb" );
	int s = searchfor( fp, "lan_mac=", 512 );

	if( s == -1 )
	{
	    fprintf( stderr, "no mac found in config\n" );
	    fclose( fp );
	}
	else
	{
	    mac[17] = 0;
	    fread( mac, 17, 1, fp );
	    fclose( fp );
	    mtd = getMTD( "board_config" );
	    sprintf( mtdpath, "/dev/mtdblock/%d", mtd );
	    fp = fopen( mtdpath, "rb" );
	    fseek( fp, 0, SEEK_SET );
	    char *block = ( char * )malloc( 65536 );

	    fread( block, 65536, 1, fp );
	    fclose( fp );
	    unsigned char in_addr[6];
	    int changed = 0;

	    ether_atoe( mac, &in_addr[0] );
	    if( memcmp( block + 96, &in_addr[0], 6 ) )
	    {
		changed++;
		memcpy( block + 96, &in_addr[0], 6 );	// wlan mac
	    }
	    in_addr[5]++;
	    if( memcmp( block + 102, &in_addr[0], 6 ) )
	    {
		changed++;
		memcpy( block + 102, &in_addr[0], 6 );	// eth0 mac
	    }
	    in_addr[5]++;
	    if( memcmp( block + 108, &in_addr[0], 6 ) )
	    {
		changed++;
		memcpy( block + 108, &in_addr[0], 6 );	// eth1 mac
	    }
	    in_addr[5]++;
	    if( memcmp( block + 118, &in_addr[0], 6 ) )
	    {
		changed++;
		memcpy( block + 118, &in_addr[0], 6 );	// wlan1 mac
	    }
	    if( changed )
	    {
		fprintf( stderr,
			 "radio config needs to be adjusted, system will reboot after flashing\n" );
		fp = fopen( "/tmp/radio", "wb" );
		fwrite( block, 65536, 1, fp );
		fclose( fp );
		eval( "mtd", "-f", "write", "/tmp/radio", "board_config" );	// writes 
		// 
		// back 
		// new 
		// config 
		// and 
		// reboots
		eval( "event", "5", "1", "15" );
	    }
	    else
	    {
		fprintf( stderr,
			 "no change required, radio config remains unchanged\n" );
	    }
	    free( block );
	}
    }
#endif

    /*
     * Modules 
     */
    uname( &name );
    /*
     * network drivers 
     */
    insmod( "ar2313" );
    insmod( "ath_hal" );
    if( nvram_get( "rate_control" ) != NULL )
    {
	char rate[64];

	sprintf( rate, "ratectl=%s", nvram_safe_get( "rate_control" ) );
	eval( "insmod", "ath_ahb", rate );
    }
    else
    {
	insmod( "ath_ahb" );
    }
    // eval ("ifconfig", "wifi0", "up");
    eval( "ifconfig", "eth0", "up" );	// wan
    system2( "echo 2 >/proc/sys/dev/wifi0/ledpin" );
    system2( "echo 1 >/proc/sys/dev/wifi0/softled" );
    if( getRouterBrand(  ) == ROUTER_BOARD_FONERA2200 )
    {
	eval( "ifconfig", "eth0", "up","promisc" );	// required for vlan config
	eval( "/sbin/vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD" );
	eval( "/sbin/vconfig", "add", "eth0", "0" );
	eval( "/sbin/vconfig", "add", "eth0", "1" );
	struct ifreq ifr;
	int s;

	if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) )
	{
	    char eabuf[32];

	    strncpy( ifr.ifr_name, "eth0", IFNAMSIZ );
	    ioctl( s, SIOCGIFHWADDR, &ifr );
	    char macaddr[32];

	    strcpy( macaddr,
		    ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.sa_data,
				eabuf ) );
	    nvram_set( "et0macaddr", macaddr );
//	    MAC_ADD( macaddr );
	    ether_atoe( macaddr, ( unsigned char * )ifr.ifr_hwaddr.sa_data );
	    strncpy( ifr.ifr_name, "vlan1", IFNAMSIZ );
	    ioctl( s, SIOCSIFHWADDR, &ifr );
	    close( s );
	}
    }
    else
    {
	vlan_init( 0xff );	// 4 lan + 1 wan
    }
    // insmod("ipv6");

    /*
     * Set a sane date 
     */
    stime( &tm );
    nvram_set( "wl0_ifname", "ath0" );

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

void start_fixboard( void )
{
    int mtd = getMTD( "board_config" );
    char mtdpath[64];

    sprintf( mtdpath, "/dev/mtdblock/%d", mtd );
    fprintf( stderr, "board config path = %s\n", mtdpath );
    FILE *fp = fopen( mtdpath, "rb" );

    if( fp )
    {
	fseek( fp, 0x1000, SEEK_SET );
	unsigned int test;

	fread( &test, 4, 1, fp );
	fprintf( stderr, "test pattern is %X\n", test );
	if( test != 0xffffffff )
	{
	    fprintf( stderr,
		     "radio config fixup is required to clean bad stuff out of memory, otherwise the radio config cannot be detected\n" );
	    fseek( fp, 0, SEEK_SET );
	    char *block = ( char * )malloc( 65536 );

	    fread( block, 65536, 1, fp );
	    fclose( fp );
	    int i;

	    for( i = 0x1000; i < 65536; i++ )
		block[i] = 0xff;
	    fp = fopen( "/tmp/radio", "wb" );
	    fwrite( block, 65536, 1, fp );
	    eval( "mtd", "-f", "write", "/tmp/radio", "board_config" );	// writes 
	    // 
	    // back 
	    // new 
	    // config 
	    // and 
	    // reboots
	    eval( "event", "5", "1", "15" );
	}
	fclose( fp );
    }

}
