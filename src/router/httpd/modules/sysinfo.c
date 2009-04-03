
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <broadcom.h>
#include <cyutils.h>
#include <sys/sysinfo.h>

// for test
void show_default_info( webs_t wp )
{
    // int ret=0;

    websDone( wp, 200 );	// Let header in first packet, and bellow
				// information in second packet.

    websWrite( wp, "Vendor:%s\n", VENDOR );
    websWrite( wp, "ModelName:%s\n", MODEL_NAME );

    websWrite( wp, "Firmware Version:%s%s , %s\n", CYBERTAN_VERSION,
	       MINOR_VERSION, __DATE__ );
    websWrite( wp, "#:%s\n", SERIAL_NUMBER );
    websWrite( wp, "Boot Version:%s\n", nvram_safe_get( "boot_ver" ) );
    /*
     * begin Svbeasoft modifications 
     */
    // ret = websWrite(wp, "CodePattern:%s\n", CODE_PATTERN);
    // #if LOCALE == EUROPE
    // ret = websWrite(wp, "Country:Europe\n");
    // #elif LOCALE == JAPAN
    // ret = websWrite(wp, "Country:Japan\n");
    // #else
    // ret = websWrite(wp, "Country:US\n");
    // #endif
    // ret = websWrite(wp, "\n");

    websWrite( wp, "RF Status:%s\n",
	       ( nvram_match( "wl0_hwaddr", "" )
		 || nvram_match( "wl_gmode",
				 "-1" ) ) ? "disabled" : "enabled" );

    websWrite( wp, "RF Firmware Version:%s%s\n", CYBERTAN_VERSION,
	       MINOR_VERSION );
    // #if LOCALE == EUROPE
    // ret = websWrite(wp, "RF Domain:ETSI (channel 1~%s)\n",
    // WL_MAX_CHANNEL);
    // #elif LOCALE == JAPAN
    // ret = websWrite(wp, "RF Domain:JPN (channel 1~%s)\n", WL_MAX_CHANNEL);
    // #else
    // ret = websWrite(wp, "RF Domain:US (channel 1~%s)\n", WL_MAX_CHANNEL);
    // #endif

    websWrite( wp, "RF Domain:Worldwide (channel 1~%s)\n", WL_MAX_CHANNEL );
#ifdef HAVE_MSSID
    websWrite( wp, "RF Channel:%s\n", nvram_safe_get( "wl0_channel" ) );
#else
    websWrite( wp, "RF Channel:%s\n", nvram_safe_get( "wl_channel" ) );
#endif
#ifndef HAVE_MSSID
    websWrite( wp, "RF SSID:%s\n", nvram_safe_get( "wl_ssid" ) );
#else
    websWrite( wp, "RF SSID:%s\n", nvram_safe_get( "wl0_ssid" ) );
#endif

    websWrite( wp, "\n-----Dynamic Information\n" );

    websWrite( wp, "RF Mac Address:%s\n", nvram_safe_get( "wl0_hwaddr" ) );
    websWrite( wp, "LAN Mac Address:%s\n", nvram_safe_get( "lan_hwaddr" ) );
    websWrite( wp, "WAN Mac Address:%s\n", nvram_safe_get( "wan_hwaddr" ) );
    if( check_hw_type(  ) == BCM4702_CHIP )
	websWrite( wp, "Hardware Version:1.x\n" );
    else
	websWrite( wp, "Hardware Version:2.0\n" );

    websWrite( wp, "Device Serial No.:%s\n", nvram_safe_get( "get_sn" ) );

    websWrite( wp, "\n" );	// The last char must be '\n'

    return;
}

static char *exec_cmd( char *cmd )
{
    FILE *fp;
    static char line[254];

    bzero( line, sizeof( line ) );

    if( ( fp = popen( cmd, "r" ) ) )
    {
	fgets( line, sizeof( line ), fp );
	pclose( fp );
    }

    chmod( line );

    return line;
}

void show_other_info( webs_t wp )
{
    int ret = 0;
    struct sysinfo info;

    websDone( wp, 200 );	// Let header in first packet, and bellow
				// information in second packet.

    websWrite( wp, "language = %s\n", nvram_safe_get( "language" ) );
    websWrite( wp, "Flash Type = %s\n", nvram_safe_get( "flash_type" ) );

    websWrite( wp, "Write Mac Address = %s\n",
	       nvram_safe_get( "et0macaddr" ) );
    websWrite( wp, "\n" );
    websWrite( wp, "get wl_gmode = %s\n", nvram_safe_get( "wl_gmode" ) );
    websWrite( wp, "wl_gmode = %s\n", exec_cmd( "wl gmode" ) );

    websWrite( wp, "get wl_afterburner = %s\n",
	       nvram_safe_get( "wl_afterburner" ) );
    websWrite( wp, "wl afterburner = %s\n", exec_cmd( "wl afterburner" ) );

    websWrite( wp, "wl afterburner_override = %s\n",
	       exec_cmd( "wl afterburner_override" ) );
    websWrite( wp, "\n" );

    sysinfo( &info );

    websWrite( wp, "totalram = %ld, freeram = %ld, bufferram = %ld\n",
	       info.totalram, info.freeram, info.bufferram );
    websWrite( wp, "uptime = %ld\n", info.uptime );
    websWrite( wp, "\n" );

    websWrite( wp, "eou_configured = %s\n",
	       nvram_safe_get( "eou_configured" ) );

    websWrite( wp, "get_eou_index = %s\n",
	       nvram_safe_get( "get_eou_index" ) );

    websWrite( wp, "get_sn_index = %s\n", nvram_safe_get( "get_sn_index" ) );
    websWrite( wp, "get_sn = %s\n", nvram_safe_get( "get_sn" ) );
    websWrite( wp, "\n" );

    websWrite( wp, "get_mac_index = %s\n",
	       nvram_safe_get( "get_mac_index" ) );
    websWrite( wp, "get_mac = %s\n", nvram_safe_get( "get_mac" ) );

    return ret;
}

void ej_show_sysinfo( webs_t wp, int argc, char_t ** argv )
{
    char *type;

#ifdef FASTWEB
    ejArgs( argc, argv, "%s", &type );
#else
    if( ejArgs( argc, argv, "%s", &type ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }
#endif

    if( type && !strcmp( type, "other" ) )
	show_other_info( wp );
    else
	show_default_info( wp );

    return;
}

// for Setup Wizard and others test
void ej_show_miscinfo( webs_t wp, int argc, char_t ** argv )
{

    websDone( wp, 200 );	// Let header in first packet, and bellow
				// information in second packet.

    websWrite( wp, "Module Name = %s\n", MODEL_NAME );

    websWrite( wp, "Firmware Version = %s%s,%s;\n", CYBERTAN_VERSION,
	       MINOR_VERSION, __DATE__ );
    websWrite( wp, "Firmware Time = %s\n", __TIME__ );
    websWrite( wp, "Flash Type = %s\n", nvram_safe_get( "flash_type" ) );
    websWrite( wp, "CPU Clock = %s\n", nvram_safe_get( "clkfreq" ) );
    websWrite( wp, "sdram_init = %s\n", nvram_safe_get( "sdram_init" ) );

    websWrite( wp, "sdram_config = %s\n", nvram_safe_get( "sdram_config" ) );
    websWrite( wp, "sdram_ncdl = %s\n", nvram_safe_get( "sdram_ncdl" ) );

    websWrite( wp, "Write Mac Address = %s\n",
	       nvram_safe_get( "et0macaddr" ) );

    websWrite( wp, "SWAOLstatus = %s\n",
	       nvram_safe_get( "aol_block_traffic" ) );
    websWrite( wp, "SWAT&Tstatus = 0\n" );

    if( nvram_match( "wan_proto", "dhcp" ) )
	websWrite( wp, "SWWanStatus = 0\n" );
    if( nvram_match( "wan_proto", "static" ) )
	websWrite( wp, "SWWanStatus = 1\n" );
    if( nvram_match( "wan_proto", "pppoe" ) )
	websWrite( wp, "SWWanStatus = 2\n" );
    if( nvram_match( "wan_proto", "pptp" ) )
	websWrite( wp, "SWWanStatus = 3\n" );
    if( nvram_match( "wan_proto", "3g" ) )
	websWrite( wp, "SWWanStatus = 4\n" );

    websWrite( wp, "\n" );

    websWrite( wp, "SWGetRouterIP = %s\n", nvram_safe_get( "lan_ipaddr" ) );

    websWrite( wp, "SWGetRouterDomain = %s\n",
	       nvram_safe_get( "wan_domain" ) );

    websWrite( wp, "SWpppoeUName = %s\n", nvram_safe_get( "ppp_username" ) );

    websWrite( wp, "\n" );
#ifndef HAVE_MSSID
    websWrite( wp, "SWGetRouterSSID = %s\n", nvram_safe_get( "wl_ssid" ) );
#else
    websWrite( wp, "SWGetRouterSSID = %s\n", nvram_safe_get( "wl0_ssid" ) );
#endif

#ifdef HAVE_MSSID
    websWrite( wp, "SWGetRouterChannel = %s\n",
	       nvram_safe_get( "wl0_channel" ) );
#else
    websWrite( wp, "SWGetRouterChannel = %s\n",
	       nvram_safe_get( "wl_channel" ) );
#endif
    websWrite( wp, "SWssidBroadcast = %s\n", nvram_safe_get( "wl_closed" ) );

    websWrite( wp, "\n" );

    if( nvram_match( "security_mode", "disabled" ) )
	websWrite( wp, "SWwirelessStatus = 0\n" );
    if( nvram_match( "security_mode", "wep" ) )
	websWrite( wp, "SWwirelessStatus = 1\n" );
    if( nvram_match( "security_mode", "psk" ) )
	websWrite( wp, "SWwirelessStatus = 2\n" );
    if( nvram_match( "security_mode", "radius" ) )
	websWrite( wp, "SWwirelessStatus = 3\n" );

    if( nvram_match( "wl_wep", "off" ) )
	websWrite( wp, "SWwlEncryption = off\n" );
    if( nvram_match( "wl_wep", "on" )
	|| nvram_match( "wl_wep", "restricted" ) )
    {
	websWrite( wp, "SWwlEncryption = wep\n" );

	websWrite( wp, "SWwepEncryption = %s\n",
		   nvram_safe_get( "wl_wep_bit" ) );
    }
    if( nvram_match( "wl_wep", "tkip" ) )
	websWrite( wp, "SWwlEncryption = tkip\n" );
    if( nvram_match( "wl_wep", "aes" ) )
	websWrite( wp, "SWwlEncryption = aes\n" );

    /*
     * Below for RF test 2003-10-29 
     */

    websWrite( wp, "WL_tssi_result = %s\n",
	       nvram_safe_get( "wl_tssi_result" ) );

    return;
}
