/*
 * rt2880.c
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

#ifdef HAVE_RT2880
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>

extern int br_add_interface( const char *br, const char *dev );

// returns the number of installed atheros devices/cards

static char iflist[1024];

char *getiflist( void )
{
    return iflist;
}

static int need_commit = 0;

static int getMaxPower( char *ifname )
{
}

/*
 * MADWIFI Encryption Setup 
 */
void setupSupplicant( char *prefix, char *ssidoverride )
{

}
void supplicant_main( int argc, char *argv[] )
{
    setupSupplicant( argv[1], argv[2] );
}

void setupHostAP( char *prefix, int iswan )
{

}
void start_hostapdwan( void )
{

}

void setMacFilter( char *iface )
{
    char *next;
    char var[32];

    sysprintf( "ifconfig %s down", iface );
    sysprintf( "iwpriv %s maccmd 3", iface );

    char nvvar[32];

    sprintf( nvvar, "%s_macmode", iface );
    if( nvram_match( nvvar, "deny" ) )
    {
	sysprintf( "iwpriv %s maccmd 2", iface );
	sysprintf( "ifconfig %s up", iface );
	char nvlist[32];

	sprintf( nvlist, "%s_maclist", iface );

	foreach( var, nvram_safe_get( nvlist ), next )
	{
	    sysprintf( "iwpriv %s addmac %s", iface, var );
	}
    }
    if( nvram_match( nvvar, "allow" ) )
    {
	sysprintf( "iwpriv %s maccmd 1", iface );
	sysprintf( "ifconfig %s up", iface );

	char nvlist[32];

	sprintf( nvlist, "%s_maclist", iface );

	foreach( var, nvram_safe_get( nvlist ), next )
	{
	    sysprintf( "iwpriv %s addmac %s", iface, var );
	}
    }

}

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void start_vifs( void )
{

}

void stop_vifs( void )
{

}
extern void adjust_regulatory( int count );

char *getWDSDev( char *wdsdev )
{
    char *newdev = "";

    if( !strcmp( wdsdev, "wds0.1" ) )
	newdev = "wds0";
    if( !strcmp( wdsdev, "wds0.2" ) )
	newdev = "wds1";
    if( !strcmp( wdsdev, "wds0.3" ) )
	newdev = "wds2";
    if( !strcmp( wdsdev, "wds0.4" ) )
	newdev = "wds3";
    if( !strcmp( wdsdev, "wds0.5" ) )
	newdev = "wds4";
    if( !strcmp( wdsdev, "wds0.6" ) )
	newdev = "wds5";
    if( !strcmp( wdsdev, "wds0.7" ) )
	newdev = "wds6";
    if( !strcmp( wdsdev, "wds0.8" ) )
	newdev = "wds7";
    if( !strcmp( wdsdev, "wds0.9" ) )
	newdev = "wds8";
    if( !strcmp( wdsdev, "wds0.10" ) )
	newdev = "wds9";
    return newdev;
}

void deconfigure_wifi( void )
{

}

void start_radius( void )
{
    char psk[64];

    // wrt-radauth $IFNAME $server $port $share $override $mackey $maxun &
    char ifname[32];

    char *prefix = "wl0";

    strcpy( ifname, "ra0" );

    if( nvram_nmatch( "1", "%s_radauth", prefix )
	&& nvram_nmatch( "ap", "%s_mode", prefix ) )
    {
	char *server = nvram_nget( "%s_radius_ipaddr", prefix );
	char *port = nvram_nget( "%s_radius_port", prefix );
	char *share = nvram_nget( "%s_radius_key", prefix );
	char type[32];

	sprintf( type, "%s_radmactype", prefix );
	char *pragma = "";

	if( nvram_default_match( type, "0", "0" ) )
	    pragma = "-n1 ";
	if( nvram_match( type, "1" ) )
	    pragma = "-n2 ";
	if( nvram_match( type, "2" ) )
	    pragma = "-n3 ";
	if( nvram_match( type, "3" ) )
	    pragma = "";
	sleep( 1 );		// some delay is usefull
	sysprintf( "wrt-radauth %s %s %s %s %s %s %s %s &", pragma, ifname,
		   server, port, share, nvram_nget( "%s_radius_override",
						    prefix ),
		   nvram_nget( "%s_radmacpassword", prefix ),
		   nvram_nget( "%s_max_unauth_users", prefix ) );
    }

}

void configure_wifi( void )	// madwifi implementation for atheros based
				// cards
{
    char var[64];
    char *next;

    deconfigure_wifi(  );
    eval( "ifconfig", "ra0", "down" );
    eval( "ifconfig", "ra1", "down" );
    eval( "ifconfig", "ra2", "down" );
    eval( "ifconfig", "ra3", "down" );
    eval( "ifconfig", "ra4", "down" );
    eval( "ifconfig", "ra5", "down" );
    eval( "ifconfig", "ra6", "down" );
    eval( "ifconfig", "ra7", "down" );
    eval( "ifconfig", "wds0", "down" );
    eval( "ifconfig", "wds1", "down" );
    eval( "ifconfig", "wds2", "down" );
    eval( "ifconfig", "wds3", "down" );
    eval( "ifconfig", "wds4", "down" );
    eval( "ifconfig", "wds5", "down" );
    eval( "ifconfig", "wds6", "down" );
    eval( "ifconfig", "wds7", "down" );
    eval( "ifconfig", "wds8", "down" );
    eval( "ifconfig", "wds9", "down" );
    eval( "ifconfig", "apcli0", "down" );

    rmmod( "rt2860v2_ap" );
    FILE *fp = fopen( "/tmp/RT2860.dat", "wb" );	// config file for driver (don't ask me, its really the worst config thing i have seen)

    fprintf( fp, "Default\n" );
    fprintf( fp, "CountryRegion=1\n" );
    fprintf( fp, "CountryRegionABand=7\n" );
    fprintf( fp, "CountryCode=DE\n" );
    fprintf( fp, "SSID1=%s\n", nvram_safe_get( "wl0_ssid" ) );
    char *vifs = nvram_nget( "wl0_vifs" );
    int count = 2;

    if( vifs != NULL )
	foreach( var, vifs, next )
    {
	fprintf( fp, "SSID%d=%s\n", count, nvram_nget( "%s_ssid", var ) );
	count++;
    }
//    if( nvram_match( "wl0_mode", "sta" ) || nvram_match( "wl0_mode", "wet" ) )
//	fprintf( fp, "BssidNum=0\n" );
//    else
	fprintf( fp, "BssidNum=%d\n", count - 1 );
    if( nvram_match( "wl0_net_mode", "bg-mixed" ) )
	fprintf( fp, "WirelessMode=0\n" );
    if( nvram_match( "wl0_net_mode", "b-only" ) )
	fprintf( fp, "WirelessMode=1\n" );
    if( nvram_match( "wl0_net_mode", "g-only" ) )
	fprintf( fp, "WirelessMode=4\n" );
    if( nvram_match( "wl0_net_mode", "n-only" ) )
	fprintf( fp, "WirelessMode=6\n" );
    if( nvram_match( "wl0_net_mode", "mixed" ) )
	fprintf( fp, "WirelessMode=9\n" );

    char hidestr[64];

    hidestr[0] = 0;

    if( nvram_nmatch( "1", "wl0_closed" ) )
	strcat( hidestr, "1" );
    else
	strcat( hidestr, "0" );

    vifs = nvram_get( "wl0_vifs" );
    if( vifs != NULL )
	foreach( var, vifs, next )
    {
	if( nvram_nmatch( "1", "%s_closed", var ) )
	    strcat( hidestr, ";1" );
	else
	    strcat( hidestr, ";0" );
    }
    fprintf( fp, "HideSSID=%s\n", hidestr );
    fprintf( fp, "ShortSlot=%s\n",
	     nvram_match( "wl0_shortslot", "long" ) ? "0" : "1" );
    if( nvram_match( "wl0_channel", "0" ) )
	fprintf( fp, "AutoChannelSelect=1\n" );
    else
	fprintf( fp, "AutoChannelSelect=0\n" );

//encryption setup
    fprintf( fp, "IEEE8021X=0\n" );
    fprintf( fp, "IEEE80211H=0\n" );
    char keyidstr[64] = { 0 };
    char encryptype[64] = { 0 };
    char authmode[64] = { 0 };

    if( nvram_match( "wl0_key", "" ) )
	strcat( keyidstr, "1" );
    else
	strcat( keyidstr, nvram_safe_get( "wl0_key" ) );
    if( nvram_match( "wl0_akm", "wep" ) )
    {
	strcat( authmode, "OPEN" );
	strcat( encryptype, "WEP" );
	fprintf( fp, "Key1Str1=%s\n", nvram_safe_get( "wl0_key1" ) );
	fprintf( fp, "Key2Str1=%s\n", nvram_safe_get( "wl0_key2" ) );
	fprintf( fp, "Key3Str1=%s\n", nvram_safe_get( "wl0_key3" ) );
	fprintf( fp, "Key4Str1=%s\n", nvram_safe_get( "wl0_key4" ) );
	fprintf( fp, "Key1Type=0\n" );
	fprintf( fp, "Key2Type=0\n" );
	fprintf( fp, "Key3Type=0\n" );
	fprintf( fp, "Key4Type=0\n" );

    }
    if( nvram_match( "wl0_akm", "disabled" ) )
    {
	strcat( authmode, "OPEN" );
	strcat( encryptype, "NONE" );
    }
    if( nvram_match( "wl0_akm", "psk2" ) )
    {
	fprintf( fp, "WPAPSK1=%s\n", nvram_safe_get( "wl0_wpa_psk" ) );
	strcat( authmode, "WPAPSK2" );
	if( nvram_match( "wl0_crypto", "tkip" ) )
	    strcat( encryptype, "TKIP" );
	if( nvram_match( "wl0_crypto", "aes" ) )
	    strcat( encryptype, "AES" );
	if( nvram_match( "wl0_crypto", "tkip+aes" ) )
	    strcat( encryptype, "TKIPAES" );
    }
    if( nvram_match( "wl0_akm", "psk psk2" ) )
    {
	fprintf( fp, "WPAPSK1=%s\n", nvram_safe_get( "wl0_wpa_psk" ) );
	strcat( authmode, "WPAPSKWPAPSK2" );
	if( nvram_match( "wl0_crypto", "tkip" ) )
	    strcat( encryptype, "TKIP" );
	if( nvram_match( "wl0_crypto", "aes" ) )
	    strcat( encryptype, "AES" );
	if( nvram_match( "wl0_crypto", "tkip+aes" ) )
	    strcat( encryptype, "TKIPAES" );
    }

    if( nvram_match( "wl0_akm", "psk" ) )
    {
	fprintf( fp, "WPAPSK1=%s\n", nvram_safe_get( "wl0_wpa_psk" ) );
	strcat( authmode, "WPAPSK" );
	if( nvram_match( "wl0_crypto", "tkip" ) )
	    strcat( encryptype, "TKIP" );
	if( nvram_match( "wl0_crypto", "aes" ) )
	    strcat( encryptype, "AES" );
	if( nvram_match( "wl0_crypto", "tkip+aes" ) )
	    strcat( encryptype, "TKIPAES" );
    }

    count = 2;
    vifs = nvram_get( "wl0_vifs" );
    if( vifs != NULL )
	foreach( var, vifs, next )
    {
	strcat( keyidstr, ";" );
	if( nvram_nmatch( "", "%s_key", var ) )
	    strcat( keyidstr, "1" );
	else
	    strcat( keyidstr, nvram_nget( "%s_key", var ) );
	if( nvram_nmatch( "wep", "%s_akm", var ) )
	{
	    fprintf( fp, "Key1Str%d=%s\n", count,
		     nvram_nget( "%s_key1", var ) );
	    fprintf( fp, "Key2Str%d=%s\n", count,
		     nvram_nget( "%s_key2", var ) );
	    fprintf( fp, "Key3Str%d=%s\n", count,
		     nvram_nget( "%s_key3", var ) );
	    fprintf( fp, "Key4Str%d=%s\n", count,
		     nvram_nget( "%s_key4", var ) );
	}
	if( nvram_nmatch( "psk", "%s_akm", var ) )
	{
	    fprintf( fp, "WPAPSK%d=%s\n", count,
		     nvram_nget( "%s_wpa_psk", var ) );
	    strcat( authmode, ";WPAPSK" );
	    if( nvram_nmatch( "tkip", "%s_crypto", var ) )
		strcat( encryptype, ";TKIP" );
	    if( nvram_nmatch( "aes", "%s_crypto", var ) )
		strcat( encryptype, ";AES" );
	    if( nvram_nmatch( "tkip+aes", "%s_crypto", var ) )
		strcat( encryptype, ";TKIPAES" );
	}
	if( nvram_nmatch( "disabled", "%s_akm", var ) )
	{
	    strcat( authmode, ";OPEN" );
	    strcat( encryptype, ";NONE" );
	}
	if( nvram_nmatch( "psk psk2", "%s_akm", var ) )
	{
	    fprintf( fp, "WPAPSK%d=%s\n", count,
		     nvram_nget( "%s_wpa_psk", var ) );
	    strcat( authmode, ";WPAPSKWPA2PSK" );
	    if( nvram_nmatch( "tkip", "%s_crypto", var ) )
		strcat( encryptype, ";TKIP" );
	    if( nvram_nmatch( "aes", "%s_crypto", var ) )
		strcat( encryptype, ";AES" );
	    if( nvram_nmatch( "tkip+aes", "%s_crypto", var ) )
		strcat( encryptype, ";TKIPAES" );
	}
	if( nvram_nmatch( "psk2", "%s_akm", var ) )
	{
	    fprintf( fp, "WPAPSK%d=%s\n", count,
		     nvram_nget( "%s_wpa_psk", var ) );
	    strcat( authmode, ";WPAPSK2" );
	    if( nvram_nmatch( "tkip", "%s_crypto", var ) )
		strcat( encryptype, ";TKIP" );
	    if( nvram_nmatch( "aes", "%s_crypto", var ) )
		strcat( encryptype, ";AES" );
	    if( nvram_nmatch( "tkip+aes", "%s_crypto", var ) )
		strcat( encryptype, ";TKIPAES" );
	}

	count++;
    }

    fprintf( fp, "DefaultKeyID=%s\n", keyidstr );
    fprintf( fp, "EncrypType=%s\n", encryptype );
    fprintf( fp, "AuthMode=%s\n", authmode );

//wds entries
    char wdsentries[128] = { 0 };
    int wdscount = 0;
    int s;

    for( s = 1; s <= 10; s++ )
    {
	char wdsvarname[32] = { 0 };
	char wdsdevname[32] = { 0 };
	char wdsmacname[32] = { 0 };
	char *wdsdev;
	char *hwaddr;
	char *dev = "wl0";

	sprintf( wdsvarname, "%s_wds%d_enable", dev, s );
	sprintf( wdsdevname, "%s_wds%d_if", dev, s );
	sprintf( wdsmacname, "%s_wds%d_hwaddr", dev, s );
	wdsdev = nvram_safe_get( wdsdevname );
	if( strlen( wdsdev ) == 0 )
	    continue;
	if( nvram_match( wdsvarname, "0" ) )
	    continue;
	hwaddr = nvram_get( wdsmacname );
	if( hwaddr != NULL )
	{
	    sprintf( wdsentries, "%s;%s", wdsentries, hwaddr );
	    wdscount++;
	}
    }

    if( wdscount )
    {
	if( nvram_match( "wl0_lazy_wds", "1" ) )
	    fprintf( fp, "WdsEnable=4\n" );	// 2 is exclusive
	else
	    fprintf( fp, "WdsEnable=3\n" );	// 2 is exclusive
	fprintf( fp, "WdsEncrypType=NONE\n" );	//for now we do not support encryption
	fprintf( fp, "WdsList=%s\n", wdsentries );
	fprintf( fp, "WdsKey=\n" );
    }
    else
    {
	fprintf( fp, "WdsEnable=0\n" );
	fprintf( fp, "WdsEncrypType=NONE\n" );
	fprintf( fp, "WdsList=\n" );
	fprintf( fp, "WdsKey=\n" );

    }

//channel width
    if( nvram_match( "wl0_nbw", "20" ) )
	fprintf( fp, "HT_BW=0\n" );
    else
	fprintf( fp, "HT_BW=1\n" );

    int channel = atoi( nvram_safe_get( "wl0_channel" ) );

    if( channel <= 4 )
	fprintf( fp, "HT_EXTCHA=1\n" );
    else if( channel >= 8 )
	fprintf( fp, "HT_EXTCHA=0\n" );
    else
	fprintf( fp, "HT_EXTCHA=0\n" );

    if( nvram_default_match( "wl0_greenfield", "1", "0" ) )
	fprintf( fp, "HT_OpMode=1\n" );	// green field mode
    else
	fprintf( fp, "HT_OpMode=0\n" );

    int mcs = atoi( nvram_default_get( "wl0_nmcsidx", "-1" ) );

    if( mcs == -1 )
	fprintf( fp, "HT_MCS=33\n" );
    else
	fprintf( fp, "HT_MCS=%d\n", mcs );

//txrate
    if( nvram_match( "wl0_rate", "0" ) )
	fprintf( fp, "TxRate=0\n" );
    else if( nvram_match( "wl0_rate", "1000000" ) )
	fprintf( fp, "TxRate=1\n" );
    else if( nvram_match( "wl0_rate", "2000000" ) )
	fprintf( fp, "TxRate=2\n" );
    else if( nvram_match( "wl0_rate", "5500000" ) )
	fprintf( fp, "TxRate=3\n" );
    else if( nvram_match( "wl0_rate", "6000000" ) )
	fprintf( fp, "TxRate=5\n" );
    else if( nvram_match( "wl0_rate", "9000000" ) )
	fprintf( fp, "TxRate=6\n" );
    else if( nvram_match( "wl0_rate", "1100000" ) )
	fprintf( fp, "TxRate=4\n" );
    else if( nvram_match( "wl0_rate", "1200000" ) )
	fprintf( fp, "TxRate=7\n" );
    else if( nvram_match( "wl0_rate", "1800000" ) )
	fprintf( fp, "TxRate=8\n" );
    else if( nvram_match( "wl0_rate", "2400000" ) )
	fprintf( fp, "TxRate=9\n" );
    else if( nvram_match( "wl0_rate", "3600000" ) )
	fprintf( fp, "TxRate=10\n" );
    else if( nvram_match( "wl0_rate", "4800000" ) )
	fprintf( fp, "TxRate=11\n" );
    else if( nvram_match( "wl0_rate", "5400000" ) )
	fprintf( fp, "TxRate=12\n" );
    else
	fprintf( fp, "TxRate=0\n" );

    fprintf( fp, "Channel=%s\n", nvram_safe_get( "wl0_channel" ) );
    if( nvram_match( "wl0_rateset", "12" ) )
	fprintf( fp, "BasicRate=3\n" );
    if( nvram_match( "wl0_rateset", "default" ) )
	fprintf( fp, "BasicRate=15\n" );
    if( nvram_match( "wl0_rateset", "all" ) )
	fprintf( fp, "BasicRate=351\n" );
    fprintf( fp, "BeaconPeriod=%s\n", nvram_safe_get( "wl0_bcn" ) );
    fprintf( fp, "DtimPeriod=%s\n", nvram_safe_get( "wl0_dtim" ) );
    fprintf( fp, "TxPower=%s\n", nvram_safe_get( "wl0_txpwr" ) );	// warning. percentage this time
    fprintf( fp, "DisableOLBC=0\n" );	//what is this?
    fprintf( fp, "BGProtection=%s\n",
	     nvram_match( "wl0_gmode_protection", "auto" ) ? "0" : "2" );
    fprintf( fp, "TXPreamble=%s\n",
	     nvram_match( "wl0_plcphdr", "long" ) ? "0" : "1" );
    fprintf( fp, "RTSThreshold=%s\n", nvram_safe_get( "wl0_rts" ) );
    fprintf( fp, "FragThreshold=%s\n", nvram_safe_get( "wl0_frag" ) );
    fprintf( fp, "TxBurst=%s\n",
	     nvram_match( "wl0_frameburst", "on" ) ? "0" : "1" );
    fprintf( fp, "PktAggregate=0\n" );
    fprintf( fp, "TurboRate=0\n" );
    fprintf( fp, "wmm=%s\n", nvram_match( "wl0_wme", "on" ) ? "1" : "0" );
    fprintf( fp, "APAifsn=3;7;1;1\n" );
    fprintf( fp, "APCwmin=4;4;3;2\n" );
    fprintf( fp, "APCwmax=6;10;4;3\n" );
    fprintf( fp, "APTxop=0;0;94;47\n" );
    fprintf( fp, "APACM=0;0;0;0\n" );
    fprintf( fp, "BSSAifsn=3;7;2;2\n" );
    fprintf( fp, "BSSCwmin=4;4;3;2\n" );
    fprintf( fp, "BSSCwmax=10;10;4;3\n" );
    fprintf( fp, "BSSTxop=0;0;94;47\n" );
    fprintf( fp, "BSSACM=0;0;0;0\n" );
    fprintf( fp, "AckPolicy=0;0;0;0\n" );
    fprintf( fp, "NoForwarding=%s\n", nvram_safe_get( "wl0_ap_isolate" ) );	//between lan and ap
    fprintf( fp, "NoForwardingBTNBSSID=%s\n", nvram_safe_get( "wl0_ap_isolate" ) );	// between bssid

//station

    if( getSTA(  ) || getWET(  ) )
    {
	fprintf( fp, "ApCliEnable=1\n" );
	fprintf( fp, "ApCliSsid=%s\n", nvram_safe_get( "wl0_ssid" ) );
	if( nvram_match( "wl0_akm", "psk" )
	    || nvram_match( "wl0_akm", "psk2" )
	    || nvram_match( "wl0_akm", "psk psk2" ) )
	{
	    if( nvram_match( "wl0_akm", "psk" ) )
	    {
		if( nvram_match( "wl0_crypto", "tkip" ) )
		    fprintf( fp, "ApCliEncrypType=TKIP\n" );
		if( nvram_match( "wl0_crypto", "aes" ) )
		    fprintf( fp, "ApCliEncrypType=AES\n" );
		if( nvram_match( "wl0_crypto", "tkip+aes" ) )
		    fprintf( fp, "ApCliEncrypType=TKIPAES\n" );
		fprintf( fp, "ApCliAuthMode=WPAPSK\n" );
	    }
	    if( nvram_match( "wl0_akm", "psk2" ) )
	    {
		if( nvram_match( "wl0_crypto", "tkip" ) )
		    fprintf( fp, "ApCliEncrypType=TKIP\n" );
		if( nvram_match( "wl0_crypto", "aes" ) )
		    fprintf( fp, "ApCliEncrypType=AES\n" );
		if( nvram_match( "wl0_crypto", "tkip+aes" ) )
		    fprintf( fp, "ApCliEncrypType=TKIPAES\n" );
		fprintf( fp, "ApCliAuthMode=WPA2PSK\n" );
	    }
	    fprintf( fp, "ApCliWPAPSK=%s\n",
		     nvram_safe_get( "wl0_wpa_psk" ) );
	}
	if( nvram_match( "wl0_akm", "disabled" ) )
	{
	    fprintf( fp, "ApCliEncrypType=NONE\n" );
	    fprintf( fp, "ApCliAuthMode=OPEN\n" );
	}
	if( nvram_match( "wl0_akm", "wep" ) )
	{
	    fprintf( fp, "ApCliEncrypType=WEP\n" );
	    fprintf( fp, "ApCliAuthMode=OPEN\n" );
	    fprintf( fp, "ApCliDefaultKeyID=%s\n",
		     nvram_safe_get( "wl0_key" ) );
	    fprintf( fp, "ApCliKey1Type=0\n" );
	    fprintf( fp, "ApCliKey2Type=0\n" );
	    fprintf( fp, "ApCliKey3Type=0\n" );
	    fprintf( fp, "ApCliKey4Type=0\n" );
	    fprintf( fp, "ApCliKey1Str=%s\n", nvram_safe_get( "wl0_key1" ) );
	    fprintf( fp, "ApCliKey2Str=%s\n", nvram_safe_get( "wl0_key2" ) );
	    fprintf( fp, "ApCliKey3Str=%s\n", nvram_safe_get( "wl0_key3" ) );
	    fprintf( fp, "ApCliKey4Str=%s\n", nvram_safe_get( "wl0_key4" ) );
	}
    }
    else
    {
	fprintf( fp, "ApCliEnable=0\n" );
    }

    fprintf( fp, "CSPeriod=10\n" );
    fprintf( fp, "WirelessEvent=0\n" );
    fprintf( fp, "PreAuth=0\n" );
    fprintf( fp, "RekeyInterval=0\n" );
    fprintf( fp, "RekeyMethod=DISABLE\n" );
    fprintf( fp, "PMKCachePeriod=10\n" );
    fprintf( fp, "HSCounter=0\n" );
    fprintf( fp, "AccessPolicy0=0\n" );
    fprintf( fp, "AccessControlList0=\n" );
    fprintf( fp, "AccessPolicy1=0\n" );
    fprintf( fp, "AccessControlList1=\n" );
    fprintf( fp, "AccessPolicy2=0\n" );
    fprintf( fp, "AccessControlList2=\n" );
    fprintf( fp, "AccessPolicy3=0\n" );
    fprintf( fp, "AccessControlList3=\n" );
    fprintf( fp, "RADIUS_Server=192.168.2.3\n" );
    fprintf( fp, "RADIUS_Port=1812\n" );
    fprintf( fp, "RADIUS_Key=ralink\n" );
    fprintf( fp, "own_ip_addr=192.168.5.234\n" );
    fprintf( fp, "EAPifname=br0\n" );
    fprintf( fp, "PreAuthifname=br0\n" );
    fprintf( fp, "HT_HTC=0\n" );
    fprintf( fp, "HT_RDG=1\n" );
    fprintf( fp, "HT_LinkAdapt=0\n" );
    fprintf( fp, "HT_MpduDensity=5\n" );
    fprintf( fp, "HT_AutoBA=1\n" );
    fprintf( fp, "HT_AMSDU=0\n" );
    fprintf( fp, "HT_BAWinSize=64\n" );
    fprintf( fp, "HT_GI=1\n" );
    fprintf( fp, "HT_STBC=1\n" );

    fclose( fp );
    insmod( "rt2860v2_ap" );

    char *dev = "wl0";
    char bridged[32];



    sprintf( bridged, "%s_bridged", getRADev( dev ) );
    if( nvram_default_match( bridged, "1", "1" ) )
    {
    if( getSTA(  ) || getWET(  ) )
	{
	sysprintf( "ifconfig ra0 up");
	sysprintf( "ifconfig %s 0.0.0.0 up", "apcli0" );	
	}else{
	sysprintf( "ifconfig %s 0.0.0.0 up", "ra0" );
	br_add_interface( getBridge( "ra0" ), "ra0" );
	}
    }
    else
    {
    if( getSTA(  ) || getWET(  ) )
	{
	sysprintf( "ifconfig ra0 up");
	sysprintf( "ifconfig %s mtu 1500", "apcli0" );
	sysprintf( "ifconfig %s %s netmask %s up", "apcli0",
		   nvram_nget( "%s_ipaddr", getRADev( dev ) ),
		   nvram_nget( "%s_netmask", getRADev( dev ) ) );	
	}else
	{
	sysprintf( "ifconfig %s mtu 1500", "ra0" );
	sysprintf( "ifconfig %s %s netmask %s up", "ra0",
		   nvram_nget( "%s_ipaddr", getRADev( dev ) ),
		   nvram_nget( "%s_netmask", getRADev( dev ) ) );
	}
    }
    char vathmac[32];

    sprintf( vathmac, "wl0_hwaddr" );
    char vmacaddr[32];

    getMacAddr( "ra0", vmacaddr );
    nvram_set( vathmac, vmacaddr );

    vifs = nvram_safe_get( "wl0_vifs" );
    if( vifs != NULL && strlen( vifs ) > 0 )
    {
	int count = 1;

	foreach( var, vifs, next )
	{

	    sprintf( bridged, "%s_bridged", getRADev( var ) );
	    if( nvram_default_match( bridged, "1", "1" ) )
	    {
		char ra[32];

		sprintf( ra, "ra%d", count );
		sysprintf( "ifconfig ra%d 0.0.0.0 up", count );
		br_add_interface( getBridge( getRADev( var ) ), ra );
	    }
	    else
	    {
		char ip[32];
		char mask[32];

		sprintf( ip, "%s_ipaddr", getRADev( var ) );
		sprintf( mask, "%s_netmask", getRADev( var ) );
		sysprintf( "ifconfig ra%d mtu 1500", count );
		sysprintf( "ifconfig ra%d %s netmask %s up", count,
			   nvram_safe_get( ip ), nvram_safe_get( mask ) );
	    }

	    sprintf( vathmac, "%s_hwaddr", var );
	    getMacAddr( getRADev( var ), vmacaddr );
	    nvram_set( vathmac, vmacaddr );

	    count++;
	}
    }

    for( s = 1; s <= 10; s++ )
    {
	char wdsvarname[32] = { 0 };
	char wdsdevname[32] = { 0 };
	char wdsmacname[32] = { 0 };
	char *wdsdev;
	char *dev = "wl0";
	char *hwaddr;

	sprintf( wdsvarname, "%s_wds%d_enable", dev, ( 11 - s ) );
	sprintf( wdsdevname, "%s_wds%d_if", dev, ( 11 - s ) );
	sprintf( wdsmacname, "%s_wds%d_hwaddr", dev, ( 11 - s ) );
	wdsdev = nvram_safe_get( wdsdevname );
	if( strlen( wdsdev ) == 0 )
	    continue;
	if( nvram_match( wdsvarname, "0" ) )
	    continue;
	hwaddr = nvram_get( wdsmacname );
	if( hwaddr != NULL )
	{
	    char *newdev = getWDSDev( wdsdev );

	    sysprintf( "ifconfig %s 0.0.0.0 up", newdev );
	}
    }
    start_radius(  );
}

void start_configurewifi( void )
{
    configure_wifi(  );
}

void start_deconfigurewifi( void )
{
    deconfigure_wifi(  );
}
#endif
