/*
 * sysinit-adm5120.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
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

int start_sysinit( void )
{
    char buf[PATH_MAX];
    struct utsname name;
    struct stat tmp_stat;
    time_t tm = 0;

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
    eval( "mknod", "-m", "0660", "/dev/mmc", "b", "126", "0" );
    eval( "mknod", "-m", "0660", "/dev/mmc0", "b", "126", "1" );
    eval( "mknod", "-m", "0660", "/dev/mmc1", "b", "126", "2" );
    eval( "mknod", "-m", "0660", "/dev/mmc2", "b", "126", "3" );
    eval( "mknod", "-m", "0660", "/dev/mmc3", "b", "126", "4" );

    eval( "mkdir", "/dev/mtd" );
    eval( "mknod", "/dev/mtd/0", "c", "90", "0" );
    eval( "mknod", "/dev/mtd/0ro", "c", "90", "1" );
    eval( "mknod", "/dev/mtd/1", "c", "90", "2" );
    eval( "mknod", "/dev/mtd/1ro", "c", "90", "3" );
    eval( "mknod", "/dev/mtd/2", "c", "90", "4" );
    eval( "mknod", "/dev/mtd/2ro", "c", "90", "5" );
    eval( "mknod", "/dev/mtd/3", "c", "90", "6" );
    eval( "mknod", "/dev/mtd/3ro", "c", "90", "7" );
    eval( "mknod", "/dev/mtd/4", "c", "90", "8" );
    eval( "mknod", "/dev/mtd/4ro", "c", "90", "9" );

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
    /*
     * load some netfilter stuff 
     */

    insmod( "nf_conntrack_ftp" );
    insmod( "nf_conntrack_irc" );
    insmod( "nf_conntrack_netbios_ns" );
    insmod( "nf_conntrack_pptp" );
    insmod( "nf_conntrack_proto_gre" );
    insmod( "nf_conntrack_proto_udplite" );
    insmod( "nf_conntrack_tftp" );
    insmod( "xt_CLASSIFY" );
    insmod( "xt_MARK" );
    insmod( "xt_TCPMSS" );
    insmod( "xt_length" );
    insmod( "xt_limit" );
    insmod( "xt_multiport" );
    insmod( "xt_pkttype" );
    insmod( "xt_state" );
    insmod( "xt_tcpmss" );
    insmod( "xt_u32" );

    insmod( "iptable_filter" );
    insmod( "iptable_mangle" );
    insmod( "nf_nat" );
    insmod( "iptable_nat" );
    insmod( "nf_nat_ftp" );
    insmod( "nf_nat_irc" );
    insmod( "nf_nat_pptp" );
    insmod( "nf_nat_proto_gre" );
    insmod( "nf_nat_tftp" );
    insmod( "ipt_LOG" );
    insmod( "ipt_MASQUERADE" );
    insmod( "ipt_REDIRECT" );
    insmod( "ipt_REJECT" );
    insmod( "ipt_ULOG" );
    insmod( "ipt_TRIGGER" );
    insmod( "ipt_iprange" );
    insmod( "ipt_ipp2p" );
    insmod( "ipt_layer7" );
    insmod( "ipt_webstr" );

    // ppp drivers

    insmod( "slhc" );
    insmod( "ppp_generic" );
    insmod( "ppp_async" );
    insmod( "ppp_synctty" );
    insmod( "pppox" );
    insmod( "pppoe" );

    insmod( "adm5120_wdt" );
    insmod( "adm5120sw" );
    unsigned char mac[6];
    char eabuf[32];
    char mtdpath[32];

    memset( mac, 0, 6 );
    FILE *fp;
    int mtd = getMTD( "boot" );

    sprintf( mtdpath, "/dev/mtdblock/%d", mtd );
    fp = fopen( mtdpath, "rb" );
    if( fp != NULL )
    {
	int s = searchfor( fp, "mgmc", 0x20000 - 5 );

	if( s != -1 )
	{
	    // fseek(fp,ftell(fp),SEEK_SET);
	    // fprintf(stderr,"found mac at 0x%08X\n",ftell(fp));
	    fread( mac, 6, 1, fp );
	    struct ifreq ifr;
	    int s;

	    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) )
	    {
		strncpy( ifr.ifr_name, "eth0", IFNAMSIZ );
		ioctl( s, SIOCGIFHWADDR, &ifr );
		// fprintf(stderr,"old mac %s\n",ether_etoa ((unsigned char
		// *) ifr.ifr_hwaddr.sa_data,eabuf));
		memcpy( ( unsigned char * )ifr.ifr_hwaddr.sa_data, mac, 6 );
		// fprintf( stderr, "new mac %s\n",
		// ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.
		// sa_data, eabuf ) );
		ioctl( s, SIOCSIFHWADDR, &ifr );
		close( s );
	    }

	    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) )
	    {
		strncpy( ifr.ifr_name, "eth0", IFNAMSIZ );
		ioctl( s, SIOCGIFHWADDR, &ifr );
		nvram_set( "et0macaddr_safe",
			   ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.
				       sa_data, eabuf ) );
		close( s );
	    }

	}
	else
	{
	    fprintf( stderr, "error: no valid mac address found for eth0\n" );
	}
	fclose( fp );
    }
    /*
     * network drivers 
     */
    insmod( "ath_hal" );
    insmod( "ath_pci" );

    eval( "watchdog" );
    /*
     * Set a sane date 
     */

    stime( &tm );
    nvram_set( "wl0_ifname", "ath0" );

    return 0;
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
