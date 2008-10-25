/*
 * sysinit-broadcom.c
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
#include <bcmdevs.h>
#include <cymac.h>

#define sys_restart() eval("event","3","1","1")
#define sys_reboot() eval("sync"); eval("event","3","1","15")

static void check_brcm_cpu_type( void )
{
    FILE *fcpu;
    char cpu_type[20];
    char type2[30];

    fcpu = fopen( "/proc/cpuinfo", "r" );

    if( fcpu == NULL )
	cprintf( "Open /proc/cpuinfo fail...0\n" );
    else
    {
	char buf[500];

	fgets( buf, 500, fcpu );
	fscanf( fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2 );
	if( !strcmp( type2, "BCM4704" ) )
	{
	    nvram_set( "cpu_type", cpu_type );
	    fclose( fcpu );
	    return;
	}
	if( !strcmp( type2, "BCM4712" ) )
	{
	    nvram_set( "cpu_type", cpu_type );
	    fclose( fcpu );
	    return;
	}
	if( !strcmp( type2, "BCM4702" ) )
	{
	    nvram_set( "cpu_type", cpu_type );
	    fclose( fcpu );
	    return;
	}
	if( !strcmp( type2, "BCM3302" ) )
	{
	    nvram_set( "cpu_type", cpu_type );
	    fclose( fcpu );
	    return;
	}
	fgets( buf, 500, fcpu );
	fscanf( fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2 );
	// fprintf(stderr, "cpu_type : %s\n", cpu_type); 
	fclose( fcpu );
	if( !strcmp( cpu_type, "BCM4710" ) || !strcmp( cpu_type, "BCM4702" ) )
	{
	    cprintf( "We got BCM4702 board...\n" );
	    nvram_set( "cpu_type", cpu_type );
	}
	else if( !strcmp( cpu_type, "BCM3302" )
		 || !strcmp( cpu_type, "BCM4712" ) )
	{
	    cprintf( "We got BCM4712 board...\n" );
	    nvram_set( "cpu_type", cpu_type );
	}
	else
	{
	    cprintf( "We got unknown board...\n" );
	    nvram_set( "cpu_type", cpu_type );
	}
    }

}

static void loadWlModule( void )	// set wled params, get boardflags,
					// set afterburner bit, load wl,
					// unset afterburner bit
{

    int brand = getRouterBrand(  );
    char macbuf[32];
    char eaddr[32];

#ifdef HAVE_MSSID		// v24

#ifndef HAVE_BUFFALO
    nvram_set( "pa0maxpwr", "251" );	// force pa0maxpwr to be 251
#endif

    if( check_hw_type(  ) == BCM4702_CHIP )
	nvram_unset( "wl0_abenable" );
    else
	nvram_set( "wl0_abenable", "1" );

    switch ( brand )
    {
	case ROUTER_LINKSYS_WRH54G:
	    nvram_set( "wl0gpio0", "135" );
	    break;
	case ROUTER_BUFFALO_WZRRSG54:
	    nvram_unset( "wl0_abenable" );
	    break;
	case ROUTER_ASUS_WL550GE:
	    nvram_set( "wl0gpio1", "0" );
	    nvram_set( "wl0gpio2", "0" );
	    break;
	case ROUTER_ASUS_WL500W:
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_MOTOROLA:
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_RT480W:
	    nvram_set( "wl0gpio0", "136" );
	    break;
	case ROUTER_WAP54G_V3:
	    nvram_set( "wl0gpio0", "0" );
	    nvram_set( "wl0gpio2", "255" );
	    nvram_set( "wl0gpio3", "255" );
	    nvram_set( "wl0gpio5", "136" );
	    break;
	case ROUTER_ASUS_WL520GUGC:
	    nvram_set( "wl0gpio0", "0" );
	    nvram_set( "wl0gpio1", "136" );
	    nvram_set( "wl0gpio2", "0" );
	    nvram_set( "wl0gpio3", "0" );
	    break;
	case ROUTER_LINKSYS_WTR54GS:
	case ROUTER_WAP54G_V1:
	    nvram_set( "wl0gpio0", "136" );
	    nvram_set( "wl0gpio1", "0" );
	    nvram_set( "wl0gpio2", "0" );
	    nvram_set( "wl0gpio3", "0" );
	    break;
	case ROUTER_BUFFALO_WBR54G:
	case ROUTER_BUFFALO_WBR2G54S:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_WRT600N:
	case ROUTER_WRT610N:
	case ROUTER_WRT350N:
	case ROUTER_WRT310N:
	case ROUTER_NETGEAR_WG602_V4:
	case ROUTER_USR_5461:
	    nvram_set( "wl0gpio0", "8" );
	    break;
	case ROUTER_WRT300NV11:
	    nvram_set( "wl0gpio0", "8" );
	    break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
	    nvram_set( "wl0gpio2", "136" );
	    break;
	case ROUTER_BUFFALO_WLA2G54C:
	    nvram_set( "wl0gpio0", "0" );
	    nvram_set( "wl0gpio5", "136" );
	    break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL500G_PRE:
	    nvram_unset( "wl0gpio0" );
	    break;
	case ROUTER_BELKIN_F5D7230_V2000:
	    // case ROUTER_BELKIN_F5D7230_V3000:
	case ROUTER_BELKIN_F5D7231:
	    nvram_set( "wl0gpio3", "136" );
	    break;
	case ROUTER_NETGEAR_WGR614L:
	    nvram_set( "wl0gpio5", "8" );
	    break;
    }
#else // v23

    switch ( brand )
    {
	case ROUTER_ASUS_WL550GE:
	    nvram_set( "wl0gpio1", "0" );
	    nvram_set( "wl0gpio2", "0" );
	    break;
	case ROUTER_BUFFALO_WZRRSG54:
	case ROUTER_BUFFALO_WBR54G:
	case ROUTER_BUFFALO_WBR2G54S:
	    nvram_set( "wl0gpio0", "130" );
	    break;
	case ROUTER_MOTOROLA:
	case ROUTER_WAP54G_V1:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_RT480W:
	    nvram_set( "wl0gpio0", "2" );
	    break;
	case ROUTER_BUFFALO_WLA2G54C:
	    nvram_set( "wl0gpio0", "0" );
	    nvram_set( "wl0gpio5", "2" );
	    break;
	case ROUTER_WAP54G_V3:
	    nvram_set( "wl0gpio0", "0" );
	    nvram_set( "wl0gpio2", "255" );
	    nvram_set( "wl0gpio3", "255" );
	    nvram_set( "wl0gpio5", "2" );
	    break;
	case ROUTER_ASUS_WL500GD:
	    nvram_unset( "wl0gpio0" );
	    break;
	case ROUTER_BELKIN_F5D7230_V2000:
	    // case ROUTER_BELKIN_F5D7230_V3000:
	    nvram_set( "wl0gpio3", "2" );
	    break;
    }
#endif

    int boardflags;

    switch ( brand )
    {
	case ROUTER_WRT150N:
	case ROUTER_WRT150NV11:
	case ROUTER_WRT160N:
	case ROUTER_WRT310N:
	case ROUTER_WRT300N:
	case ROUTER_WRT300NV11:
	case ROUTER_WRT350N:
	case ROUTER_BUFFALO_WZRG144NH:
	case ROUTER_BUFFALO_WZRG300N:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_NETGEAR_WNR834BV2:
	case ROUTER_NETGEAR_WNDR3300:
	case ROUTER_ASUS_WL500W:
	    insmod( "wl" );	// load module
	    break;
	case ROUTER_WRT600N:
	case ROUTER_WRT610N:
	    insmod( "wl" );	// load module
	    wl_hwaddr( "eth0", macbuf );
	    ether_etoa( ( uchar * ) macbuf, eaddr );
	    nvram_set( "wl0_hwaddr", eaddr );
	    MAC_SUB( eaddr );
	    if( !nvram_match( "et0macaddr", eaddr ) )
	    {
		nvram_set( "et0macaddr", eaddr );
		nvram_commit(  );
		kill( 1, SIGTERM );
		exit( 0 );
	    }
	    wl_hwaddr( "eth1", macbuf );
	    ether_etoa( ( uchar * ) macbuf, eaddr );
	    nvram_set( "wl1_hwaddr", eaddr );
	    break;
	default:
	    boardflags = strtoul( nvram_safe_get( "boardflags" ), NULL, 0 );
	    fprintf( stderr, "boardflags are 0x%04X\n", boardflags );
	    if( boardflags == 0 )	// we can try anyway
	    {
		nvram_set( "boardflags", "0x0200" );
		insmod( "wl" );	// load module
		nvram_unset( "boardflags" );
	    }
	    else if( boardflags & BFL_AFTERBURNER )	// ab flag already
		// set
	    {
		insmod( "wl" );	// load module
	    }
	    else		// ab flag not set
	    {
		char bf[16];

		sprintf( bf, "0x%04X", boardflags );
		boardflags |= BFL_AFTERBURNER;
		fprintf( stderr,
			 "enable Afterburner, boardflags are 0x%04X\n",
			 boardflags );
		char ab[16];

		sprintf( ab, "0x%04X", boardflags );
		char *oldvalue = nvram_get( "boardflags" );	// use the

		// string for 
		// restoring
		// since the
		// Buffalo
		// WZR-RS-G54 
		// does await 
		// a 0x10 in
		// the
		// bootloader, 
		// otherwise
		// the nvram
		// gets
		// deleted
		nvram_set( "boardflags", ab );	// set boardflags with
		// AfterBurner bit on
		insmod( "wl" );	// load module
		nvram_set( "boardflags", oldvalue );	// set back to
		// original
	    }

    }
#ifdef HAVE_MADWIFI
    insmod( "ath_hal" );
    insmod( "ath_pci" );
#endif
    return;
}

char wanifname[8], wlifname[8];

#define BCM4712_CPUTYPE "0x4712"

static void setup_4712( void )
{
    uint boardflags = strtoul( nvram_safe_get( "boardflags" ), NULL, 0 );

    if( nvram_match( "cpu_type", BCM4712_CPUTYPE ) ||
	nvram_match( "cpu_type", "BCM3302" ) ||
	nvram_match( "cpu_type", "BCM4712" ) )
    {
	if( boardflags & BFL_ENETVLAN )
	{
	    cprintf( "setup_4712(): Enable VLAN\n" );
	    // nvram_set("setup_4712","1");
	    strcpy( wanifname, "vlan1" );
	    strcpy( wlifname, "eth1" );
	    if( !strcmp( nvram_safe_get( "wan_ifname" ), "eth1" ) )
	    {
		// nvram_set("setup_4712","1-1");
		nvram_set( "wan_ifname", "vlan1" );
		nvram_set( "wan_ifnames", "vlan1" );
		nvram_set( "wan_default", "vlan1" );
	    }
	    if( !strstr( nvram_safe_get( "lan_ifnames" ), "vlan0" ) )
	    {
		// nvram_set("setup_4712","1-2");
		nvram_set( "lan_ifnames", "vlan0 eth1" );
		nvram_set( "vlan0hwname", "et0" );
		nvram_set( "vlan1hwname", "et0" );
		nvram_set( "wl0_ifname", "eth1" );
		// nvram_set ("need_commit","1");
	    }
	}			// VLAN enabled
	else
	{
	    // nvram_set("setup_4712","2");
	    cprintf
		( "setup_4712(): Disable VLAN, it must be in bridge mode\n" );
	    nvram_set( "lan_ifnames", "eth0 eth1" );
	    strcpy( wlifname, "eth1" );
	    nvram_set( "wl0_ifname", "eth1" );
	}
    }
    else
    {				// 4702, 4704
	cprintf
	    ( "setup_4712(): It's a 4702 or 4704 hardware, VLAN can't be used in these 2 boards\n" );
	strcpy( wanifname, "eth1" );
	strcpy( wlifname, "eth2" );
	nvram_set( "wl0_ifname", "eth2" );
	if( !strcmp( nvram_safe_get( "wan_ifname" ), "" ) )
	    nvram_set( "lan_ifnames", "eth0 eth1 eth2 wlanb0 wlana0" );
	else
	    nvram_set( "lan_ifnames", "eth0 eth2" );
    }
    // nvram_set ("need_commit","1");

}

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
    cprintf( "sysinit() tmp\n" );

    /*
     * /tmp 
     */
    mount( "ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL );
    eval( "mkdir", "/tmp/www" );

    cprintf( "sysinit() var\n" );

    /*
     * /var 
     */
    mkdir( "/tmp/var", 0777 );
    mkdir( "/var/lock", 0777 );
    mkdir( "/var/lock/subsys", 0777 );
    mkdir( "/var/log", 0777 );
    mkdir( "/var/run", 0777 );
    mkdir( "/var/tmp", 0777 );
    cprintf( "sysinit() setup console\n" );
#ifndef HAVE_MICRO
    eval( "watchdog" );		// system watchdog
#endif
    /*
     * Setup console 
     */

    cprintf( "sysinit() klogctl\n" );
    klogctl( 8, NULL, atoi( nvram_safe_get( "console_loglevel" ) ) );
    cprintf( "sysinit() get router\n" );

    int brand = getRouterBrand(  );

    led_control( LED_DIAG, LED_ON );
    char *rname = getRouter(  );

    fprintf( stderr, "Booting device: %s\n", rname );

    nvram_unset( "port_swap" );

    int need_reboot = 0;

    struct nvram_tuple *basic_params = NULL;
    struct nvram_tuple *extra_params = NULL;

    struct nvram_tuple generic1[] = {
	{"lan_ifnames", "eth0 eth2", 0},
	{"wan_ifname", "eth1", 0},
	{"wl0_ifname", "eth2", 0},
	{0, 0, 0}
    };

    struct nvram_tuple generic1_wantolan[] = {
	{"lan_ifnames", "eth2", 0},
	{"wan_ifname", "eth0", 0},
	{"wl0_ifname", "eth2", 0},
	{0, 0, 0}
    };

    switch ( brand )
    {
	case ROUTER_BUFFALO_WZRRSG54:
	    check_brcm_cpu_type(  );
	    setup_4712(  );
	    basic_params = generic1;
	    eval( "gpio", "init", "0" );	// AOSS button
	    eval( "gpio", "init", "4" );	// reset button
	    break;

	case ROUTER_MOTOROLA:
	    nvram_set( "cpu_type", "BCM4712" );
	    setup_4712(  );
	    break;

	case ROUTER_RT480W:
	    setup_4712(  );
	    break;

	case ROUTER_BELKIN_F5D7231:
	case ROUTER_USR_5461:
	    nvram_set( "lan_ifnames", "vlan0 eth1" );
	    nvram_set( "wan_ifname", "vlan1" );
	    nvram_set( "wl0_ifname", "eth1" );
	    if( nvram_match( "vlan1ports", "0 5u" ) )
		nvram_set( "vlan1ports", "0 5" );
	    break;

	case ROUTER_RT210W:
	case ROUTER_ASKEY_RT220XD:
	    basic_params = generic1;

	    if( nvram_get( "et0macaddr" ) == NULL
		|| nvram_get( "et0macaddr" ) == "" )
	    {
		nvram_set( "et0macaddr", "00:16:E3:00:00:10" );	// fix for
		// missing
		// cfe
		// default =
		// dead LAN
		// ports.
		need_reboot = 1;
	    }
	    if( nvram_get( "et1macaddr" ) == NULL
		|| nvram_get( "et1macaddr" ) == "" )
	    {
		nvram_set( "et1macaddr", "00:16:E3:00:00:11" );
		need_reboot = 1;
	    }
	    break;

	case ROUTER_BRCM4702_GENERIC:
	    basic_params = generic1;

	    if( nvram_get( "et0macaddr" ) == NULL
		|| nvram_get( "et0macaddr" ) == "" )
	    {
		nvram_set( "et0macaddr", "00:11:22:00:00:10" );	// fix for
		// missing
		// cfe
		// default =
		// dead LAN
		// ports.
		need_reboot = 1;
	    }
	    if( nvram_get( "et1macaddr" ) == NULL
		|| nvram_get( "et1macaddr" ) == "" )
	    {
		nvram_set( "et1macaddr", "00:11:22:00:00:11" );
		need_reboot = 1;
	    }
	    break;

	case ROUTER_ASUS_WL500G:
	    basic_params = generic1;

	    if( nvram_get( "et0macaddr" ) == NULL
		|| nvram_get( "et0macaddr" ) == "" )
	    {
		nvram_set( "et0macaddr", "00:0C:6E:00:00:10" );	// fix for
		// missing
		// cfe
		// default =
		// dead LAN
		// ports.
		need_reboot = 1;
	    }
	    if( nvram_get( "et1macaddr" ) == NULL
		|| nvram_get( "et1macaddr" ) == "" )
	    {
		nvram_set( "et1macaddr", "00:0C:6E:00:00:10" );
		need_reboot = 1;
	    }
	    break;

	case ROUTER_DELL_TRUEMOBILE_2300:
	    setup_4712(  );
	    nvram_set( "wan_ifname", "eth1" );	// fix for WAN problem.
	    break;

	case ROUTER_BUFFALO_WBR54G:	// for WLA-G54
	    basic_params = generic1;
	    if( nvram_match( "wan_to_lan", "yes" ) && nvram_invmatch( "wan_proto", "disabled" ) )	// = 
		// 
		// no 
		// lan
	    {
		basic_params = generic1_wantolan;
	    }
	    break;

	case ROUTER_BUFFALO_WLI2_TX1_G54:
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_WAP54G_V1:
	case ROUTER_SITECOM_WL105B:
	    nvram_set( "lan_ifnames", "eth1 eth2" );
	    nvram_set( "wl0_ifname", "eth2" );
	    nvram_set( "wan_ifname", "eth0" );	// WAN to nonexist. iface.
	    nvram_set( "port_swap", "1" );
	    if( nvram_match( "wan_to_lan", "yes" ) && nvram_invmatch( "wan_proto", "disabled" ) )	// = 
		// 
		// no 
		// lan
	    {
		nvram_set( "lan_ifnames", "eth2" );
		nvram_set( "wan_ifname", "eth1" );
	    }
	    break;

	case ROUTER_SITECOM_WL111:
	    basic_params = generic1;
	    break;

	case ROUTER_NETGEAR_WNR834BV2:
	    if( nvram_match( "force_vlan_supp", "enabled" ) )
	    {
		nvram_set( "lan_ifnames", "vlan0 eth2" );
		nvram_set( "wan_ifname", "eth1" );
		nvram_set( "vlan0ports", "3 2 1 0 5*" );
		nvram_set( "vlan1ports", "4 5" );	//dummy
		nvram_set( "vlan0hwname", "et0" );
	    }
	    else
	    {
		basic_params = generic1;
	    }

	    if( nvram_get( "pci/1/1/macaddr" ) == NULL )
		need_reboot = 1;

	    nvram_set( "pci/1/1/macaddr", nvram_safe_get( "et0macaddr" ) );

	    struct nvram_tuple wnr834bv2_pci_1_1_params[] = {
		{"stbcpo", "0", 0},
		{"pa2gw1a0", "0", 0},
		{"pa2gw1a1", "0", 0},
		{"ag0", "2", 0},
		{"ag1", "2", 0},
		{"ag2", "2", 0},
		{"ccdpo", "0", 0},
		{"txpid2ga0", "71", 0},
		{"txpid2ga1", "79", 0},
		{"txpt2g", "0x38", 0},
		{"pa2gw0a0", "0", 0},
		{"pa2gw0a1", "0", 0},
		{"boardflags", "0x200", 0},
		{"boardvendor", "0x14e4", 0},
		{"bw40po", "0", 0},
		{"sromrev", "4", 0},
		{"venid", "0x14e4", 0},
		{"boardrev", "0x4b", 0},
		{"itt2ga0", "0", 0},
		{"itt2ga1", "0", 0},
		{"pa2gw3a0", "0", 0},
		{"pa2gw3a1", "0", 0},
		{"maxp2ga0", "0", 0},
		{"maxp2ga1", "0", 0},
		{"boardtype", "0x46d", 0},
		{"boardflags2", "0x0013", 0},
		{"ofdm2gpo", "0", 0},
		{"ledbh0", "0x8", 0},
		{"ledbh1", "-1", 0},
		{"ledbh2", "-1", 0},
		{"ledbh3", "-1", 0},
		{"mcs2gpo0", "0", 0},
		{"mcs2gpo1", "0", 0},
		{"mcs2gpo2", "0", 0},
		{"mcs2gpo3", "0", 0},
		{"mcs2gpo4", "0", 0},
		{"mcs2gpo5", "0", 0},
		{"mcs2gpo6", "0", 0},
		{"mcs2gpo7", "0", 0},
		{"bwduppo", "0", 0},
		{"aa2g", "7", 0},
		{"pa2gw2a0", "0", 0},
		{"pa2gw2a1", "0", 0},
		{"ccode", "all", 0},
		{"regrev", "0", 0},
		{"devid", "0x4329", 0},
		{"cck2gpo", "0", 0},
		{0, 0, 0}
	    };
	    /*
	     * set router's extra parameters 
	     */
	    extra_params = wnr834bv2_pci_1_1_params;
	    for( extra_params; extra_params && extra_params->name;
		 extra_params++ )
	    {
		nvram_nset( extra_params->value, "pci/1/1/%s",
			    extra_params->name );
	    }
	    break;

	case ROUTER_NETGEAR_WNDR3300:
	    nvram_set( "lan_ifnames", "eth0 eth2 eth3" );	// dual radio
	    nvram_set( "wan_ifname", "eth1" );
	    nvram_set( "wl0_ifname", "eth2" );
	    nvram_set( "wl1_ifname", "eth3" );

	    if( nvram_get( "pci/1/1/macaddr" ) == NULL
		|| nvram_get( "pci/1/3/macaddr" ) == NULL )
		need_reboot = 1;

	    {
		unsigned char mac[20];

		strcpy( mac, nvram_safe_get( "et0macaddr" ) );
		nvram_set( "pci/1/3/macaddr", mac );
		MAC_SUB( mac );
		nvram_set( "pci/1/1/macaddr", mac );
	    }

	    struct nvram_tuple wndr3300_pci_1_1_params[] = {
		{"stbcpo", "0", 0},
		{"mcs5gpo0", "0x4200", 0},
		{"pa2gw1a0", "0x13EA", 0},
		{"mcs5gpo1", "0x6664", 0},
		{"pa2gw1a1", "0x14DA", 0},
		{"mcs5gpo2", "0x4200", 0},
		{"maxp5gha0", "0x4A", 0},
		{"mcs5gpo3", "0x6664", 0},
		{"maxp5gha1", "0x4A", 0},
		{"mcs5gpo4", "0", 0},
		{"mcs5gpo5", "0", 0},
		{"mcs5gpo6", "0", 0},
		{"aa5g", "7", 0},
		{"mcs5gpo7", "0", 0},
		{"pa5glw2a0", "0xFB81", 0},
		{"pa5glw2a1", "0xFBC7", 0},
		{"ag0", "2", 0},
		{"ag1", "2", 0},
		{"ag2", "2", 0},
		{"pa5gw2a0", "0xFBD2", 0},
		{"pa5gw2a1", "0xFC11", 0},
		{"pa5ghw2a0", "0xFB92", 0},
		{"pa5ghw2a1", "0xFBC0", 0},
		{"ccdpo", "0", 0},
		{"txpid2ga0", "59", 0},
		{"itt5ga0", "0x3C", 0},
		{"rxchain", "3", 0},
		{"txpid2ga1", "51", 0},
		{"itt5ga1", "0x3C", 0},
		{"maxp5ga0", "0x4A", 0},
		{"maxp5ga1", "0x4A", 0},
		{"txpt2g", "0x48", 0},
		{"pa2gw0a0", "0xFEFC", 0},
		{"pa2gw0a1", "0xFF03", 0},
		{"boardflags", "0x0A00", 0},
		{"mcs5glpo0", "0x4200", 0},
		{"pa5glw1a0", "0x130E", 0},
		{"mcs5glpo1", "0x6664", 0},
		{"ofdm5gpo", "0x88888888", 0},
		{"pa5glw1a1", "0x13BE", 0},
		{"mcs5glpo2", "0x4200", 0},
		{"mcs5glpo3", "0x6664", 0},
		{"mcs5glpo4", "0", 0},
		{"mcs5glpo5", "0", 0},
		{"mcs5glpo6", "0", 0},
		{"mcs5glpo7", "0", 0},
		{"boardvendor", "0x14e4", 0},
		{"bw40po", "0", 0},
		{"sromrev", "4", 0},
		{"venid", "0x14e4", 0},
		{"pa5gw1a0", "0x1238", 0},
		{"pa5gw1a1", "0x14A4", 0},
		{"pa5ghw1a0", "0x12C3", 0},
		{"pa5ghw1a1", "0x1375", 0},
		{"boardrev", "0x13", 0},
		{"itt2ga0", "0x3E", 0},
		{"itt2ga1", "0x3E", 0},
		{"pa2gw3a0", "0", 0},
		{"pa2gw3a1", "0", 0},
		{"maxp2ga0", "0x4A", 0},
		{"maxp2ga1", "0x4A", 0},
		{"boardtype", "0x49C", 0},
		{"boardflags2", "0x0014", 0},
		{"ofdm2gpo", "0x66666666", 0},
		{"ledbh0", "11", 0},
		{"ledbh1", "11", 0},
		{"pa5glw0a0", "0xFEFB", 0},
		{"ledbh2", "11", 0},
		{"pa5glw0a1", "0xFF5B", 0},
		{"ledbh3", "11", 0},
		{"ledbh4", "11", 0},
		{"ledbh5", "5", 0},
		{"ledbh6", "0x82", 0},
		{"ledbh7", "11", 0},
		{"mcs2gpo0", "0x6666", 0},
		{"mcs2gpo1", "0x6666", 0},
		{"mcs2gpo2", "0x6666", 0},
		{"mcs2gpo3", "0x6666", 0},
		{"txpid5gla0", "22", 0},
		{"mcs2gpo4", "0", 0},
		{"txpid5gla1", "26", 0},
		{"mcs2gpo5", "0", 0},
		{"txpt5g", "0x3C", 0},
		{"mcs2gpo6", "0", 0},
		{"mcs2gpo7", "0", 0},
		{"mcs5ghpo0", "0x4200", 0},
		{"mcs5ghpo1", "0x6664", 0},
		{"bwduppo", "0", 0},
		{"mcs5ghpo2", "0x4200", 0},
		{"mcs5ghpo3", "0x6664", 0},
		{"txchain", "3", 0},
		{"mcs5ghpo4", "0", 0},
		{"mcs5ghpo5", "0", 0},
		{"txpid5gha0", "38", 0},
		{"mcs5ghpo6", "0", 0},
		{"ofdm5glpo", "0x88888888", 0},
		{"txpid5gha1", "34", 0},
		{"mcs5ghpo7", "0", 0},
		{"antswitch", "2", 0},
		{"aa2g", "7", 0},
		{"pa5gw0a0", "0xFF3C", 0},
		{"pa5gw0a1", "0xFFEC", 0},
		{"ofdm5ghpo", "0x88888888", 0},
		{"pa5ghw0a0", "0xFEE8", 0},
		{"pa5ghw0a1", "0xFF72", 0},
		{"leddc", "0xFFFF", 0},
		{"pa2gw2a0", "0xFB44", 0},
		{"pa2gw2a1", "0xFB28", 0},
		{"pa5glw3a0", "0", 0},
		{"pa5glw3a1", "0", 0},
		{"ccode", "0", 0},
		{"pa5gw3a0", "0", 0},
		{"regrev", "0", 0},
		{"pa5gw3a1", "0", 0},
		{"devid", "0x4328", 0},
		{"pa5ghw3a0", "0", 0},
		{"pa5ghw3a1", "0", 0},
		{"txpt5gh", "0x3C", 0},
		{"cck2gpo", "0x0000", 0},
		{"txpt5gl", "0x30", 0},
		{"maxp5gla0", "0x4A", 0},
		{"txpid5ga0", "39", 0},
		{"maxp5gla1", "0x4A", 0},
		{"txpid5ga1", "39", 0},
		{0, 0, 0}
	    };
	    /*
	     * set router's extra parameters 
	     */
	    extra_params = wndr3300_pci_1_1_params;
	    for( extra_params; extra_params && extra_params->name;
		 extra_params++ )
	    {
		nvram_nset( extra_params->value, "pci/1/1/%s",
			    extra_params->name );
	    }

	    struct nvram_tuple wndr3300_pci_1_3_params[] = {
		{"ag0", "0x02", 0},
		{"boardflags", "0xAA48", 0},
		{"ccode", "0", 0},
		{"aa0", "0x03", 0},
		{"devid", "0x4318", 0},
		{"pa0b0", "0x14ed", 0},
		{"pa0b1", "0xfac7", 0},
		{"pa0b2", "0xfe8a", 0},
		{"pa0itssit", "62", 0},
		{"pa0maxpwr", "0x0042", 0},
		{"opo", "0", 0},
		{"wl0gpio0", "11", 0},
		{"wl0gpio1", "11", 0},
		{"wl0gpio2", "11", 0},
		{"wl0gpio3", "0x82", 0},
		{"sromrev", "2", 0},
		{0, 0, 0}
	    };
	    /*
	     * set router's extra parameters 
	     */
	    extra_params = wndr3300_pci_1_3_params;
	    for( extra_params; extra_params && extra_params->name;
		 extra_params++ )
	    {
		nvram_nset( extra_params->value, "pci/1/3/%s",
			    extra_params->name );
	    }
	    break;

	case ROUTER_MOTOROLA_WE800G:
	    nvram_set( "lan_ifnames", "eth1 eth2" );
	    nvram_set( "wl0_ifname", "eth2" );
	    nvram_set( "wan_ifname", "eth0" );	// WAN to nonexist. iface.
	    nvram_set( "port_swap", "1" );
	    eval( "gpio", "disable", "7" );
	    if( nvram_match( "wan_to_lan", "yes" ) && nvram_invmatch( "wan_proto", "disabled" ) )	// = 
		// 
		// no 
		// lan
	    {
		nvram_set( "lan_ifnames", "eth2" );
		nvram_set( "wan_ifname", "eth1" );
	    }
	    break;

	case ROUTER_MOTOROLA_V1:
	    nvram_set( "wan_ifname", "eth1" );
	    eval( "gpio", "disable", "7" );
	    break;

	case ROUTER_BUFFALO_WZRG300N:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_ASUS_WL500W:
	case ROUTER_BUFFALO_WLAH_G54:
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
	case ROUTER_MICROSOFT_MN700:
	    nvram_set( "wan_ifname", "eth1" );
	    break;

	case ROUTER_WRTSL54GS:
	    nvram_set( "wan_ifname", "eth1" );
	    if( nvram_match( "force_vlan_supp", "enabled" ) )
	    {
		nvram_set( "lan_ifnames", "vlan0 eth2" );
		nvram_set( "vlan0ports", "0 1 2 3 5*" );
		nvram_set( "vlan1ports", "4 5" );	//dummy
		nvram_set( "vlan0hwname", "et0" );
	    }
	    else
	    {
		nvram_set( "lan_ifnames", "eth0 eth2" );
	    }
	    break;

	case ROUTER_WRT54G1X:
	    if( check_vlan_support(  ) )
	    {
		nvram_set( "lan_ifnames", "vlan0 eth2" );
		nvram_set( "wan_ifname", "vlan1" );
	    }
	    break;

	case ROUTER_WRT350N:
	case ROUTER_WRT310N:
	case ROUTER_WRT600N:
	case ROUTER_WRT610N:
	    nvram_set( "wan_ifname", "vlan2" );
	    break;

	case ROUTER_WRT300NV11:
	case ROUTER_BUFFALO_WZRG144NH:
	    nvram_set( "wan_ifname", "vlan1" );
	    break;

	case ROUTER_ASUS_WL500G_PRE:
	    nvram_set( "sdram_init", "0x0009" );
	    // nvram_set ("sdram_ncdl", "0x208");
	    nvram_set( "lan_ifnames", "vlan0 eth2" );
	    nvram_set( "wl0_ifname", "eth2" );
	    nvram_set( "wan_ifname", "vlan1" );	// fix for Asus WL500gPremium 
	    // 
	    // WAN problem.
	    if( nvram_match( "vlan1ports", "0 5u" ) )
		nvram_set( "vlan1ports", "0 5" );
	    break;

	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL550GE:
	    nvram_set( "wl0_ifname", "eth1" );
	    break;

	case ROUTER_BUFFALO_WLA2G54C:
	case ROUTER_WAP54G_V2:
	case ROUTER_VIEWSONIC_WAPBR_100:
	case ROUTER_USR_5430:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_NETGEAR_WG602_V4:
	    nvram_set( "lan_ifnames", "eth0 eth1" );
	    nvram_set( "wl0_ifname", "eth1" );
	    nvram_set( "wan_ifname", "eth2" );	// map WAN port to
	    // nonexistant interface
	    if( nvram_match( "wan_to_lan", "yes" ) && nvram_invmatch( "wan_proto", "disabled" ) )	// = 
		// 
		// no 
		// lan
	    {
		nvram_set( "lan_ifnames", "eth1" );
		nvram_set( "wan_ifname", "eth0" );
	    }
	    break;

	case ROUTER_BELKIN_F5D7230_V3000:
	    if( nvram_match( "vlan1ports", "4 5u" ) )
		nvram_set( "vlan1ports", "4 5" );
	    break;

	case ROUTER_DELL_TRUEMOBILE_2300_V2:	// we must fix cfe defaults
	    // with CR added
	    nvram_set( "vlan0hwname", "et0" );
	    nvram_set( "vlan1hwname", "et0" );
	    nvram_set( "et0mdcport", "0" );
	    nvram_set( "et0phyaddr", "30" );
	    nvram_set( "gpio2", "adm_eecs" );
	    nvram_set( "gpio3", "adm_eesk" );
	    nvram_set( "gpio4", "adm_eedi" );
	    nvram_set( "gpio5", "adm_rc" );
	    nvram_unset( "gpio6" );
	    break;

	case ROUTER_WRT54G_V8:
	    nvram_set( "reset_gpio", "7" );
	    break;

	case ROUTER_ASUS_WL520G:
	case ROUTER_ASUS_WL500G_PRE_V2:
	case ROUTER_WRT54G_V81:
	    if( nvram_match( "vlan1ports", "4 5u" ) )
		nvram_set( "vlan1ports", "4 5" );
	    break;

	case ROUTER_ASUS_WL520GUGC:
	    if( nvram_match( "vlan1ports", "0 5u" ) )
		nvram_set( "vlan1ports", "0 5" );
	    break;

	case ROUTER_NETGEAR_WGR614L:
	    if( nvram_match( "vlan1ports", "0 5u" ) )
		nvram_set( "vlan1ports", "0 5" );
	    if( nvram_match( "sromrev", "2" )
		&& nvram_match( "boardrev", "0x10" )
		&& nvram_match( "boardtype", "0x48E" ) )
	    {
		nvram_set( "sromrev", "3" );	// This is a fix for WGR614L NA - which has a wrong sromrev
		need_reboot = 1;
	    }
	    break;

	case ROUTER_ALLNET01:
	    nvram_set( "wl0_ifname", "eth1" );
	    if( nvram_match( "vlan1ports", "5u" ) )	//correct bad parameters
	    {
		nvram_set( "vlan1ports", "4 5u" );
		nvram_set( "vlan0ports", "0 1 2 3 5*" );
	    }
	    if( !nvram_match( "no_sercom", "1" ) )
	    {
		//fix mac
		unsigned char mac[6];
		FILE *in = fopen( "/dev/mtdblock/0", "rb" );

		if( in != NULL )	//special sercom mac address handling
		{
		    fseek( in, 0x1ffa0, SEEK_SET );
		    fread( mac, 6, 1, in );
		    fclose( in );
		    char macstr[32];

		    sprintf( macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			     ( int )mac[0] & 0xff, ( int )mac[1] & 0xff,
			     ( int )mac[2] & 0xff, ( int )mac[3] & 0xff,
			     ( int )mac[4] & 0xff, ( int )mac[5] & 0xff );
		    nvram_set( "et0macaddr", macstr );
		    eval( "ifconfig", "eth0", "hw", "ether", macstr );
		}
	    }
	    break;
	case ROUTER_LINKSYS_WTR54GS:
	    eval( "gpio", "enable", "3" );	// prevent reboot loop on
	    // reset
	    break;

	case ROUTER_WAP54G_V3:
	    eval( "gpio", "enable", "0" );	// reset gpio 0 for reset
	    // button 
	    // nvram_set ("vlan0ports", "1 5*");
	    // nvram_set ("vlan1ports", "4 5");
	    // if (nvram_match ("wan_to_lan", "yes") && nvram_invmatch
	    // ("wan_proto", "disabled")) // = no lan
	    // {
	    // nvram_set ("vlan0ports", "4 5*");
	    // nvram_set ("vlan1ports", "1 5");
	    // }
	    break;

    }
#if 0
    /*
     * fix il0macaddr to be lanmac+2 
     */
    if( nvram_get( "il0macaddr" ) == NULL )
	need_reboot = 1;

    unsigned char mac[20];

    if( nvram_match( "port_swap", "1" ) )
	strcpy( mac, nvram_safe_get( "et1macaddr" ) );
    else
	strcpy( mac, nvram_safe_get( "et0macaddr" ) );
    MAC_ADD( mac );
    MAC_ADD( mac );
    nvram_set( "il0macaddr", mac );
#endif

    /*
     * set router's basic parameters 
     */
    for( basic_params; basic_params && basic_params->name; basic_params++ )
    {
	nvram_set( basic_params->name, basic_params->value );
    }

    /*
     * ifnames 
     */
    strcpy( wanifname, nvram_safe_get( "wan_ifname" ) );
    strcpy( wlifname, nvram_safe_get( "wl0_ifname" ) );

    /*
     * set wan_ifnames, pppoe_wan_ifname and pppoe_ifname 
     */
    nvram_set( "wan_ifname", wanifname );
    nvram_set( "wan_ifnames", wanifname );
    nvram_set( "wan_default", wanifname );
    nvram_set( "pppoe_wan_ifname", wanifname );
    nvram_set( "pppoe_ifname", wanifname );

    /*
     * additional boardflags adjustment 
     */
    switch ( brand )
    {
	case ROUTER_BELKIN_F5D7231:
	    if( nvram_match( "boardflags", "0x388" )
		|| nvram_match( "boardflags", "0x0388" ) )
		nvram_set( "boardflags", "0x0f58" );
	    break;

	case ROUTER_BUFFALO_WLI_TX4_G54HP:
	    if( !nvram_match( "buffalo_hp", "1" )
		&& ( nvram_match( "boardflags", "0x1658" )
		     || nvram_match( "boardflags", "0x2658" ) ) )
	    {
		nvram_set( "buffalo_hp", "1" );
#ifndef HAVE_BUFFALO		// if HAVE_BUFFALO not used to be FCC/CE
		// valid
		nvram_set( "boardflags", "0x3658" );	// enable high gain
		// PA
		need_reboot = 1;
#endif
	    }
	    break;

	case ROUTER_BUFFALO_WHRG54S:	// for HP only
	    if( !nvram_match( "buffalo_hp", "1" )
		&& nvram_match( "boardflags", "0x1758" ) )
	    {
		nvram_set( "buffalo_hp", "1" );
#ifndef HAVE_BUFFALO		// if HAVE_BUFFALO not used to be FCC/CE
		// valid
		nvram_set( "boardflags", "0x3758" );	// enable high gain
		// PA
		need_reboot = 1;
#endif
	    }
	    break;

	case ROUTER_WRTSL54GS:
	    if( nvram_match( "force_vlan_supp", "enabled" )
		&& nvram_match( "boardflags", "0x0018" ) )
	    {
		nvram_set( "boardflags", "0x0118" );	//enable lan vlans
		need_reboot = 1;
	    }
	    else if( !nvram_match( "force_vlan_supp", "enabled" )
		     && nvram_match( "boardflags", "0x0118" ) )
	    {
		nvram_set( "boardflags", "0x0018" );	//disable vlans
		need_reboot = 1;
	    }
	    break;

	case ROUTER_NETGEAR_WNR834BV2:
	    if( nvram_match( "force_vlan_supp", "enabled" )
		&& nvram_match( "boardflags", "0x10" ) )
	    {
		nvram_set( "boardflags", "0x110" );	//enable lan vlans
		need_reboot = 1;
	    }
	    else if( !nvram_match( "force_vlan_supp", "enabled" )
		     && nvram_match( "boardflags", "0x110" ) )
	    {
		nvram_set( "boardflags", "0x10" );	//disable vlans
		need_reboot = 1;
	    }
	    break;

    }

    if( need_reboot )
    {
	nvram_commit(  );
	cprintf( "Need reboot now .....\n" );
	sys_reboot(  );
    }

    /*
     * Modules 
     */
    uname( &name );

    snprintf( buf, sizeof( buf ), "/lib/modules/%s", name.release );
    if( stat( "/proc/modules", &tmp_stat ) == 0
	&& stat( buf, &tmp_stat ) == 0 )
    {
	char module[80], *modules, *next;

	// modules="wl switch-core";
	nvram_set( "portprio_support", "1" );

	if( check_vlan_support(  ) && check_hw_type(  ) != BCM5325E_CHIP )
	{
	    switch ( brand )
	    {
		case ROUTER_WRT310N:
		case ROUTER_WRT350N:
		case ROUTER_WRT600N:
		case ROUTER_WRT610N:
		case ROUTER_WRT300NV11:
		case ROUTER_BUFFALO_WZRG144NH:
		    nvram_set( "portprio_support", "0" );
		    modules = "bcm57xxlsys";
		    break;
		case ROUTER_LINKSYS_WRT55AG:
		case ROUTER_MOTOROLA:
		case ROUTER_BUFFALO_WBR2G54S:
		case ROUTER_DELL_TRUEMOBILE_2300_V2:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "switch-core switch-adm";
		    break;

		case ROUTER_WRT54G_V8:
		case ROUTER_WRT54G_V81:
		case ROUTER_LINKSYS_WRH54G:
		case ROUTER_ASUS_WL520G:
		case ROUTER_ASUS_WL520GUGC:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "switch-core switch-robo";
		    break;

		case ROUTER_WRT54G1X:
		case ROUTER_WRT54G:
		    insmod( "switch-core" );
		    if( insmod( "switch-robo" ) )
			insmod( "switch-adm" );
		    break;

		case ROUTER_RT480W:
		case ROUTER_BUFFALO_WLI2_TX1_G54:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "";
		    insmod( "switch-core" );
		    if( insmod( "switch-robo" ) )
			insmod( "switch-adm" );
		    break;

		case ROUTER_WRT54G3G:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			:
			"switch-core switch-robo pcmcia_core yenta_socket ds serial_cs usbcore usb-ohci usbserial sierra";
		    break;

		default:

		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "switch-core switch-robo";
		    break;
	    }
	}
	else
	{
	    switch ( brand )
	    {
		case ROUTER_WRT310N:
		case ROUTER_WRT350N:
		case ROUTER_WRT600N:
		case ROUTER_WRT610N:
		case ROUTER_BUFFALO_WZRG144NH:
		    nvram_set( "portprio_support", "0" );
		    modules = "bcm57xxlsys";
		    break;
		case ROUTER_LINKSYS_WRT55AG:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "switch-core switch-adm";

		    break;
		case ROUTER_ASUS_WL500GD:
		case ROUTER_ASUS_WL550GE:
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "switch-core switch-robo";
		    break;
		case ROUTER_BUFFALO_WZRRSG54:
		    nvram_set( "portprio_support", "0" );
		    modules =
			nvram_invmatch( "ct_modules",
					"" ) ? nvram_safe_get( "ct_modules" )
			: "";
		    break;
		case ROUTER_WRT54G3G:
		    if( check_vlan_support(  ) )
			modules =
			    nvram_invmatch( "ct_modules",
					    "" ) ?
			    nvram_safe_get( "ct_modules" ) :
			    "switch-core switch-robo pcmcia_core yenta_socket ds";
		    else
		    {
			nvram_set( "portprio_support", "0" );

			modules =
			    nvram_invmatch( "ct_modules",
					    "" ) ?
			    nvram_safe_get( "ct_modules" ) :
			    "pcmcia_core yenta_socket ds";
		    }
		    break;

		default:
		    if( check_vlan_support(  ) )
			modules =
			    nvram_invmatch( "ct_modules",
					    "" ) ?
			    nvram_safe_get( "ct_modules" ) :
			    "switch-core switch-robo";
		    else
		    {
			nvram_set( "portprio_support", "0" );
			modules =
			    nvram_invmatch( "ct_modules",
					    "" ) ?
			    nvram_safe_get( "ct_modules" ) : "";
		    }
		    break;
	    }
	}
	cprintf( "insmod %s\n", modules );

	foreach( module, modules, next )
	{
#ifdef HAVE_MACBIND
	    if( nvram_match( "et0macaddr", MACBRAND ) )
		insmod( module );
#else

	    cprintf( "loading %s\n", module );
	    insmod( module );
	    cprintf( "done\n" );
#endif
	}

	if( check_hw_type(  ) == BCM4702_CHIP )
	    insmod( "diag" );

	loadWlModule(  );
	/*
	 * #ifdef HAVE_USB //load usb driver. we will add samba server, ftp
	 * server and ctorrent support in future modules = "usbcore usb-ohci
	 * usb-uhci ehci-hcd scsi_mod usb-storage ide-core ide-detect
	 * ide-disk ide-scsi cdrom ide-cd printer sd_mod sr_mod" foreach
	 * (module, modules, next) { cprintf ("loading %s\n", module);
	 * insmod(module); } #endif 
	 */

#ifdef HAVE_USB

	if( nvram_match( "usb_enable", "1" ) )
	{
	    led_control( LED_USB, LED_ON );

	    cprintf( "loading usbcore\n" );
	    insmod( "usbcore" );

	    if( nvram_match( "usb_uhci", "1" ) )
	    {
		cprintf( "loading usb-uhci\n" );
		insmod( "usb-uhci" );
	    }

	    if( nvram_match( "usb_ohci", "1" ) )
	    {
		cprintf( "loading usb-ohci\n" );
		insmod( "usb-ohci" );
	    }

	    if( nvram_match( "usb_usb2", "1" ) )
	    {
		cprintf( "loading usb2 module\n" );
		insmod( "ehci-hcd" );
	    }

	    if( nvram_match( "usb_storage", "1" ) )
	    {
		cprintf( "loading scsi_mod\n" );
		insmod( "scsi_mod" );
		cprintf( "loading sd_mod\n" );
		insmod( "sd_mod" );
		cprintf( "loading usb-storage\n" );
		insmod( "usb-storage" );

		if( nvram_match( "usb_fs_ext3", "1" ) )
		{
		    cprintf( "loading ext2\n" );
		    insmod( "ext2" );
#ifdef HAVE_USB_ADVANCED
		    cprintf( "loading jbd\n" );
		    insmod( "jbd" );
		    cprintf( "loading ext3\n" );
		    insmod( "ext3" );
#endif
		}

		if( nvram_match( "usb_fs_fat", "1" ) )
		{
		    cprintf( "loading usb_fs_fat\n" );
		    insmod( "fat" );
		    cprintf( "loading usb_fs_vfat\n" );
		    insmod( "vfat" );
		}

		// if (nvram_match ("usb_fs_xfs", "1"))
		// {
		// cprintf ("loading usb_fs_xfs\n");
		// insmod("xfs");
		// } 
	    }

	    if( nvram_match( "usb_printer", "1" ) )
	    {
		cprintf( "loading printer\n" );
		insmod( "printer" );
	    }
	}
	else
	{
	    led_control( LED_USB, LED_OFF );
	}
#endif

    }
    /*
     * Set a sane date 
     */
    stime( &tm );

    led_control( LED_POWER, LED_ON );
    led_control( LED_SES, LED_OFF );
    led_control( LED_BRIDGE, LED_OFF );
    led_control( LED_WLAN, LED_OFF );
    led_control( LED_CONNECTED, LED_OFF );

    if( brand == ROUTER_WRT54G3G )
    {
	eval( "cardmgr" );
    }

    cprintf( "done\n" );
    return 0;

}
static int check_nv( char *name, char *value )
{
    int ret = 0;

    if( nvram_match( "manual_boot_nv", "1" ) )
	return 0;

    if( !nvram_get( name ) )
    {
	cprintf( "ERR: Cann't find %s !.......................\n", name );
	nvram_set( name, value );
	ret++;
    }
    else if( nvram_invmatch( name, value ) )
    {
	cprintf( "ERR: The %s is %s, not %s !.................\n", name,
		 nvram_safe_get( name ), value );
	nvram_set( name, value );
	ret++;
    }

    return ret;
}

int check_cfe_nv( void )
{
    int ret = 0;

    switch ( getRouterBrand(  ) )
    {
	case ROUTER_BUFFALO_WZRRSG54:
	    ret += check_nv( "lan_hwnames", "et0 wl0" );
	    ret += check_nv( "wan_hwname", "et1" );
	    ret += check_nv( "vlans", "0" );
	    break;
	case ROUTER_BUFFALO_WBR2G54S:
	    ret += check_nv( "aa0", "3" );

	    ret += check_nv( "pa0itssit", "62" );
	    ret += check_nv( "pa0b0", "0x1136" );
	    ret += check_nv( "pa0b1", "0xfb93" );
	    ret += check_nv( "pa0b2", "0xfea5" );
#ifndef HAVE_MSSID
	    ret += check_nv( "pa0maxpwr", "60" );
#endif
	    ret += check_nv( "wl0gpio2", "0" );
	    ret += check_nv( "wl0gpio3", "0" );
	    ret += check_nv( "cctl", "0" );
	    ret += check_nv( "ccode", "0" );
	    break;
#ifndef HAVE_BUFFALO

	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_WRT54G_V81:
	    ret += check_nv( "aa0", "3" );
	    /*
	     * if (check_hw_type () == BCM5352E_CHIP || check_hw_type () ==
	     * BCM5354G_CHIP) ret += check_nv ("ag0", "0x02"); else ret +=
	     * check_nv ("ag0", "255");
	     */
	    if( check_hw_type(  ) == BCM5325E_CHIP )
	    {
		/*
		 * Lower the DDR ram drive strength , the value will be
		 * stable for all boards Latency 3 is more stable for all ddr 
		 * 20050420 by honor 
		 */

		ret += check_nv( "sdram_init", "0x010b" );
		ret += check_nv( "sdram_config", "0x0062" );

		if( nvram_match( "clkfreq", "200" )
		    && nvram_match( "overclocking", "200" ) )
		{
		    ret += check_nv( "clkfreq", "216" );
		    nvram_set( "overclocking", "216" );
		}

		if( ret )
		{
		    nvram_set( "sdram_ncdl", "0x0" );

		}
		ret += check_nv( "pa0itssit", "62" );
		ret += check_nv( "pa0b0", "0x15eb" );
		ret += check_nv( "pa0b1", "0xfa82" );
		ret += check_nv( "pa0b2", "0xfe66" );
#ifndef HAVE_MSSID
		ret += check_nv( "pa0maxpwr", "0x4e" );
#endif
	    }
	    else if( check_hw_type(  ) == BCM5354G_CHIP )
	    {
		ret += check_nv( "pa0itssit", "62" );
		ret += check_nv( "pa0b0", "0x1326" );
		ret += check_nv( "pa0b1", "0xFB51" );
		ret += check_nv( "pa0b2", "0xFE87" );
		ret += check_nv( "reset_gpio", "7" );
	    }
	    else if( check_hw_type(  ) == BCM4705_BCM5397_EWC_CHIP )
	    {
		// nothing to do
	    }
	    else if( check_hw_type(  ) == BCM4704_BCM5325F_CHIP )
	    {
		// nothing to do
	    }
	    else
	    {
		ret += check_nv( "pa0itssit", "62" );
		ret += check_nv( "pa0b0", "0x170c" );
		ret += check_nv( "pa0b1", "0xfa24" );
		ret += check_nv( "pa0b2", "0xfe70" );
#ifndef HAVE_MSSID
		ret += check_nv( "pa0maxpwr", "0x48" );
#endif
	    }

	    // ret += check_nv("gpio2", "adm_eecs");
	    // ret += check_nv("gpio3", "adm_eesk");
	    // ret += check_nv("gpio5", "adm_eedi");
	    // ret += check_nv("gpio6", "adm_rc");

	    ret += check_nv( "wl0gpio2", "0" );
	    ret += check_nv( "wl0gpio3", "0" );

	    ret += check_nv( "cctl", "0" );
	    ret += check_nv( "ccode", "0" );
	    break;
#endif
    }
    if( ret )
    {
	cprintf
	    ( "Some error found, we want to reboot!.....................\n" );
	nvram_commit(  );
	sys_reboot(  );
    }

    return ret;
}

int check_pmon_nv( void )
{
    return 0;
}

void start_overclocking( void )
{
#ifdef HAVE_OVERCLOCKING
    cprintf( "Overclocking started\n" );

    int rev = cpu_plltype(  );

    if( rev == 0 )
	return;			// unsupported

    char *ov = nvram_get( "overclocking" );

    if( ov == NULL )
	return;
    int clk = atoi( ov );

    if( nvram_get( "clkfreq" ) == NULL )
	return;			// unsupported

    char *pclk = nvram_safe_get( "clkfreq" );
    char dup[64];

    strcpy( dup, pclk );
    int i;

    for( i = 0; i < strlen( dup ); i++ )
	if( dup[i] == ',' )
	    dup[i] = 0;
    int cclk = atoi( dup );

    if( ( cclk < 150 && rev == 3 ) || ( cclk < 192 && rev == 4 )
	|| ( cclk < 183 && rev == 7 ) )
    {
	cprintf( "clkfreq is %d (%s), this is unsupported\n", cclk, dup );
	return;			// unsupported
    }

    if( clk == cclk )
    {
	cprintf( "clkfreq identical with new setting\n" );
	return;			// clock already set
    }

    int set = 1;
    int clk2 = 0;
    char clkfr[16];

    switch ( clk )
    {
	case 150:
	    clk2 = 75;
	    // nvram_set ("clkfreq", "150,75");
	    break;
	case 183:
	    clk2 = 92;
	    // nvram_set ("clkfreq", "183,92");
	    break;
	case 187:
	    clk2 = 94;
	    // nvram_set ("clkfreq", "187,94");
	    break;
	case 192:
	    clk2 = 96;
	    // nvram_set ("clkfreq", "192,96");
	    break;
	case 198:
	    clk2 = 98;
	    // nvram_set ("clkfreq", "198,98");
	    break;
	case 200:
	    clk2 = 100;
	    // nvram_set ("clkfreq", "200,100");
	    break;
	case 216:
	    clk2 = 108;
	    // nvram_set ("clkfreq", "216,108");
	    break;
	case 225:
	    clk2 = 113;
	    // nvram_set ("clkfreq", "225,113");
	    break;
	case 228:
	    clk2 = 114;
	    // nvram_set ("clkfreq", "228,114");
	    break;
	case 233:
	    clk2 = 116;
	    // nvram_set ("clkfreq", "233,116");
	    break;
	case 237:
	    clk2 = 119;
	    // nvram_set ("clkfreq", "237,119");
	    break;
	case 240:
	    clk2 = 120;
	    // nvram_set ("clkfreq", "240,120");
	    break;
	case 250:
	    clk2 = 125;
	    // nvram_set ("clkfreq", "250,125");
	    break;
	case 252:
	    clk2 = 126;
	    // nvram_set ("clkfreq", "252,126");
	    break;
	case 264:
	    clk2 = 132;
	    // nvram_set ("clkfreq", "264,132");
	    break;
	case 280:
	    clk2 = 120;
	    // nvram_set ("clkfreq", "280,120");
	    break;
	case 300:
	    clk2 = 120;
	    // nvram_set ("clkfreq", "300,120");
	    break;
	default:
	    set = 0;
	    break;
    }

    if( set )
    {
	cprintf( "clock frequency adjusted from %d to %d, reboot needed\n",
		 cclk, clk );
	sprintf( clkfr, "%d,%d", clk, clk2 );
	nvram_set( "clkfreq", clkfr );
	nvram_commit(  );
	cprintf( "Overclocking done, rebooting...\n" );
	sys_reboot(  );
    }
#endif
}

char *enable_dtag_vlan( int enable )
{
    int donothing = 0;

    nvram_set( "fromvdsl", "1" );
    if( nvram_match( "vdsl_state", "1" ) && enable )
	donothing = 1;
    if( ( nvram_match( "vdsl_state", "0" )
	  || nvram_match( "vdsl_state", "" ) ) && !enable )
	donothing = 1;
    if( enable )
	nvram_set( "vdsl_state", "1" );
    else
	nvram_set( "vdsl_state", "0" );

    char *vlan7ports = NULL;

    vlan7ports = "4t 5";
    if( nvram_match( "vlan1ports", "4 5" ) )
    {
	vlan7ports = "4t 5";
    }
    if( nvram_match( "vlan1ports", "0 5" ) )
    {
	vlan7ports = "0t 5";
    }
    if( nvram_match( "vlan1ports", "1 5" ) )
    {
	vlan7ports = "1t 5";
    }
    if( nvram_match( "vlan2ports", "0 8" ) || nvram_match( "vlan2ports", "0 8*" ) || nvram_match( "vlan2ports", "0 8t" ) || nvram_match( "vlan1ports", "4 8" ) )	// special 
	// 
	// condition 
	// for 
	// Broadcom 
	// Gigabit 
	// Phy 
	// routers 
	// 
    {
#ifdef HAVE_MADWIFI
	char *eth = "eth0";
#else
	char *eth = "eth1";
#endif
	vlan7ports = "0t 8";
	int vlanswap = 0;

	if( nvram_match( "vlan1ports", "4 8" ) )
	{
	    vlanswap = 1;
	    vlan7ports = "4t 8";
	}
	char *save_ports2 = nvram_safe_get( "vlan2ports" );

	if( vlanswap )
	    save_ports2 = nvram_safe_get( "vlan1ports" );
#ifndef HAVE_MADWIFI
	if( getRouterBrand(  ) == ROUTER_WRT600N
	    || getRouterBrand(  ) == ROUTER_WRT610N )
	    eth = "eth2";
#endif
	if( donothing )
	{
	    nvram_set( "fromvdsl", "0" );
	    return eth;
	}
	if( enable )
	{
	    if( vlanswap )
		nvram_set( "vlan1ports", "" );
	    else
		nvram_set( "vlan2ports", "" );
	    nvram_set( "vlan7ports", vlan7ports );
	}
	stop_lan(  );
	eval( "ifconfig", eth, "down" );
	rmmod( "bcm57xxlsys" );
	insmod( "bcm57xxlsys" );
	eval( "ifconfig", eth, "up" );
	start_config_vlan(  );
	start_lan(  );
	if( enable )
	{
	    if( vlanswap )
		nvram_set( "vlan1ports", save_ports2 );
	    else
		nvram_set( "vlan2ports", save_ports2 );
	    nvram_set( "vlan7ports", "" );
	}
	nvram_set( "fromvdsl", "0" );
	return eth;
    }

    if( nvram_match( "switch_type", "BCM5325" ) )	// special condition
	// for Broadcom
	// Gigabit Phy
	// routers 
    {
#ifdef HAVE_MADWIFI
	char *eth = "eth0";
#else
	char *eth = "eth1";
#endif
	vlan7ports = "0t 5";
	int vlanswap = 0;
	char *save_ports2 = nvram_safe_get( "vlan1ports" );

	if( donothing )
	{
	    nvram_set( "fromvdsl", "0" );
	    return eth;
	}
	if( enable )
	{
	    nvram_set( "vlan1ports", "" );
	    nvram_set( "vlan7ports", vlan7ports );
	}
	stop_lan(  );
	eval( "ifconfig", eth, "down" );
	rmmod( "bcm57xxlsys" );
	insmod( "bcm57xxlsys" );
	eval( "ifconfig", eth, "up" );
	start_config_vlan(  );
	start_lan(  );
	if( enable )
	{
	    nvram_set( "vlan1ports", save_ports2 );
	    nvram_set( "vlan7ports", "" );
	}
	nvram_set( "fromvdsl", "0" );
	return eth;
    }

    char *eth = "eth0";

    FILE *in = fopen( "/proc/switch/eth1/reset", "rb" );	// this

    // condition
    // fails
    // almost.
    // just one
    // router
    // (DLINK
    // DIR-330)
    // requires
    // it
    if( in )
    {
	eth = "eth1";
	fclose( in );
    }
    else
    {
	FILE *in = fopen( "/proc/switch/eth2/reset", "rb" );	// this

	// condition
	// fails
	// almost.
	// just one
	// router
	// (DLINK
	// DIR-330)
	// requires
	// it
	if( in )
	{
	    eth = "eth2";
	    fclose( in );
	}
	else
	    eth = "eth0";
    }

    if( !donothing )
    {
	sysprintf( "echo 1 > /proc/switch/%s/reset", eth );
	if( enable )
	{
	    fprintf( stderr, "enable vlan port mapping %s/%s\n",
		     nvram_safe_get( "vlan0ports" ), vlan7ports );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/0/ports",
		       nvram_safe_get( "vlan0ports" ), eth );
	    start_setup_vlans(  );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/1/ports", "",
		       eth );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/7/ports",
		       vlan7ports, eth );
	}
	else
	{
	    fprintf( stderr, "disable vlan port mapping %s/%s\n",
		     nvram_safe_get( "vlan0ports" ),
		     nvram_safe_get( "vlan1ports" ) );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/7/ports", "",
		       eth );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/0/ports",
		       nvram_safe_get( "vlan0ports" ), eth );
	    sysprintf( "echo \"%s\" > /proc/switch/%s/vlan/1/ports",
		       nvram_safe_get( "vlan1ports" ), eth );
	    start_setup_vlans(  );
	}
    }
    nvram_set( "fromvdsl", "0" );
    return eth;
}

start_dtag( void )
{
    enable_dtag_vlan( 1 );
}
