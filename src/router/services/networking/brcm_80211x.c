/*
 * brcm_80211x.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include <bcmnvram.h>
#include <bcmutils.h>
#include <utils.h>
#include <shutils.h>

void setupSupplicant( char *prefix )
{
    char akm[16];

    sprintf( akm, "%s_akm", prefix );
    char wmode[16];

    sprintf( wmode, "%s_mode", prefix );
    if( nvram_match( akm, "wep" ) )
    {
	char key[16];
	int cnt = 1;
	int i;
	char bul[8];

	for( i = 1; i < 5; i++ )
	{
	    sprintf( key, "%s_key%d", prefix, i );
	    char *athkey = nvram_safe_get( key );

	    if( athkey != NULL && strlen( athkey ) > 0 )
	    {
		sprintf( bul, "[%d]", cnt++ );
		eval( "iwconfig", prefix, "key", bul, athkey );	// setup wep
		// encryption 
		// key
	    }
	}
	sprintf( key, "%s_key", prefix );
	sprintf( bul, "[%s]", nvram_safe_get( key ) );
	eval( "iwconfig", prefix, "key", bul );
	// eval ("iwpriv", prefix, "authmode", "2");
    }
    else if( nvram_match( akm, "psk" ) ||
	     nvram_match( akm, "psk2" ) || nvram_match( akm, "psk psk2" ) )
    {
	char fstr[64];
	char psk[16];

	sprintf( fstr, "/tmp/%s_wpa_supplicant.conf", prefix );
	FILE *fp = fopen( fstr, "wb" );

#ifdef HAVE_MAKSAT
	fprintf( fp, "ap_scan=1\n" );
#elif HAVE_NEWMEDIA
	fprintf( fp, "ap_scan=1\n" );
#else
	fprintf( fp, "ap_scan=2\n" );
#endif
	fprintf( fp, "fast_reauth=1\n" );
	fprintf( fp, "eapol_version=1\n" );
	// fprintf (fp, "ctrl_interface_group=0\n");
	// fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");

	fprintf( fp, "network={\n" );
	sprintf( psk, "%s_ssid", prefix );
	fprintf( fp, "\tssid=\"%s\"\n", nvram_safe_get( psk ) );
	// fprintf (fp, "\tmode=0\n");
	fprintf( fp, "\tscan_ssid=1\n" );
	fprintf( fp, "\tkey_mgmt=WPA-PSK\n" );

	sprintf( psk, "%s_crypto", prefix );
	if( nvram_match( psk, "aes" ) )
	{
	    fprintf( fp, "\tpairwise=CCMP\n" );
	    fprintf( fp, "\tgroup=CCMP\n" );
	}
	if( nvram_match( psk, "tkip" ) )
	{
	    fprintf( fp, "\tpairwise=TKIP\n" );
	    fprintf( fp, "\tgroup=TKIP\n" );
	}
	if( nvram_match( psk, "tkip+aes" ) )
	{
	    fprintf( fp, "\tpairwise=CCMP TKIP\n" );
	    fprintf( fp, "\tgroup=CCMP TKIP\n" );
	}
	if( nvram_match( akm, "psk" ) )
	    fprintf( fp, "\tproto=WPA\n" );
	if( nvram_match( akm, "psk2" ) )
	    fprintf( fp, "\tproto=RSN\n" );
	if( nvram_match( akm, "psk psk2" ) )
	    fprintf( fp, "\tproto=WPA RSN\n" );

	sprintf( psk, "%s_wpa_psk", prefix );
	fprintf( fp, "\tpsk=\"%s\"\n", nvram_safe_get( psk ) );
	fprintf( fp, "}\n" );
	fclose( fp );
	if( !strcmp( prefix, "wl0" ) )
	    sprintf( psk, "-i%s", nvram_safe_get( "wl0_ifname" ) );
	else
	    sprintf( psk, "-i%s", prefix );

	if( nvram_match( wmode, "wdssta" ) || nvram_match( wmode, "wet" ) )
	    eval( "wpa_supplicant", "-b", getBridge( prefix ), "-B",
		  "-Dmadwifi", psk, "-c", fstr );
	else
	    eval( "wpa_supplicant", "-B", "-Dwext", psk, "-c", fstr );
    }
    else if( nvram_match( akm, "8021X" ) )
    {
	char fstr[32];
	char psk[64];
	char ath[64];

	sprintf( fstr, "/tmp/%s_wpa_supplicant.conf", prefix );
	FILE *fp = fopen( fstr, "wb" );

	fprintf( fp, "ap_scan=1\n" );
	fprintf( fp, "fast_reauth=1\n" );
	fprintf( fp, "eapol_version=2\n" );
	// fprintf (fp, "ctrl_interface_group=0\n");
	// fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");
	fprintf( fp, "network={\n" );
	sprintf( psk, "%s_ssid", prefix );
	fprintf( fp, "\tssid=\"%s\"\n", nvram_safe_get( psk ) );
	fprintf( fp, "\tscan_ssid=1\n" );
	if( nvram_prefix_match( "8021xtype", prefix, "tls" ) )
	{
	    fprintf( fp, "\tkey_mgmt=IEEE8021X\n" );
	    fprintf( fp, "\teap=TLS\n" );
	    fprintf( fp, "\tidentity=\"%s\"\n",
		     nvram_prefix_get( "tls8021xuser", prefix ) );
	    sprintf( psk, "/tmp/%s", prefix );
	    mkdir( psk );
	    sprintf( psk, "/tmp/%s/ca.pem", prefix );
	    sprintf( ath, "%s_tls8021xca", prefix );
	    write_nvram( psk, ath );
	    sprintf( psk, "/tmp/%s/user.pem", prefix );
	    sprintf( ath, "%s_tls8021xpem", prefix );
	    write_nvram( psk, ath );

	    sprintf( psk, "/tmp/%s/user.prv", prefix );
	    sprintf( ath, "%s_tls8021xprv", prefix );
	    write_nvram( psk, ath );
	    fprintf( fp, "\tca_cert=/tmp/%s/ca.pem\n", prefix );
	    fprintf( fp, "\tclient_cert=/tmp/%s/user.pem\n", prefix );
	    fprintf( fp, "\tprivate_key=/tmp/%s/user.prv\n", prefix );
	    fprintf( fp, "\tprivate_key_passwd=\"%s\"\n",
		     nvram_prefix_get( "tls8021xpasswd", prefix ) );
	    fprintf( fp, "\teapol_flags=3\n" );
	}
	if( nvram_prefix_match( "8021xtype", prefix, "peap" ) )
	{
	    fprintf( fp, "\tkey_mgmt=WPA-EAP\n" );
	    fprintf( fp, "\teap=PEAP\n" );
	    fprintf( fp, "\tpairwise=CCMP TKIP\n" );
	    fprintf( fp, "\tgroup=CCMP TKIP\n" );
	    fprintf( fp, "\tphase1=\"peapver=0\"\n" );
	    fprintf( fp, "\tidentity=\"%s\"\n",
		     nvram_prefix_get( "peap8021xuser", prefix ) );
	    fprintf( fp, "\tpassword=\"%s\"\n",
		     nvram_prefix_get( "peap8021xpasswd", prefix ) );
	    sprintf( psk, "/tmp/%s", prefix );
	    mkdir( psk );
	    sprintf( psk, "/tmp/%s/ca.pem", prefix );
	    sprintf( ath, "%s_peap8021xca", prefix );
	    write_nvram( psk, ath );
	    fprintf( fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix );
	}
	if( nvram_prefix_match( "8021xtype", prefix, "ttls" ) )
	{
	    fprintf( fp, "\tkey_mgmt=WPA-EAP\n" );
	    fprintf( fp, "\teap=TTLS\n" );
	    fprintf( fp, "\tpairwise=CCMP TKIP\n" );
	    fprintf( fp, "\tgroup=CCMP TKIP\n" );
	    fprintf( fp, "\tidentity=\"%s\"\n",
		     nvram_prefix_get( "ttls8021xuser", prefix ) );
	    fprintf( fp, "\tpassword=\"%s\"\n",
		     nvram_prefix_get( "ttls8021xpasswd", prefix ) );
	    if( strlen( nvram_nget( "%s_ttls8021xca", prefix ) ) > 0 )
	    {
		sprintf( psk, "/tmp/%s", prefix );
		mkdir( psk );
		sprintf( psk, "/tmp/%s/ca.pem", prefix );
		sprintf( ath, "%s_ttls8021xca", prefix );
		write_nvram( psk, ath );
		fprintf( fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix );
	    }
	}
	if( nvram_prefix_match( "8021xtype", prefix, "leap" ) )
	{
	    fprintf( fp, "\tkey_mgmt=WPA-EAP\n" );
	    fprintf( fp, "\teap=LEAP\n" );
	    fprintf( fp, "\tauth_alg=LEAP\n" );
	    fprintf( fp, "\tproto=WPA RSN\n" );
	    fprintf( fp, "\tpairwise=CCMP TKIP\n" );
	    fprintf( fp, "\tgroup=CCMP TKIP\n" );
	    fprintf( fp, "\tidentity=\"%s\"\n",
		     nvram_prefix_get( "leap8021xuser", prefix ) );
	    fprintf( fp, "\tpassword=\"%s\"\n",
		     nvram_prefix_get( "leap8021xpasswd", prefix ) );
	}
	fprintf( fp, "}\n" );
	fclose( fp );
	if( !strcmp( prefix, "wl0" ) )
	    sprintf( psk, "-i%s", nvram_safe_get( "wl0_ifname" ) );
	else if( !strcmp( prefix, "wl1" ) )
	    sprintf( psk, "-i%s", nvram_safe_get( "wl1_ifname" ) );
	else
	    sprintf( psk, "-i%s", prefix );

	char bvar[32];

	sprintf( bvar, "%s_bridged", prefix );
	if( nvram_match( bvar, "1" )
	    && ( nvram_match( wmode, "wdssta" )
		 || nvram_match( wmode, "wet" ) ) )
	    eval( "wpa_supplicant", "-b", nvram_safe_get( "lan_ifname" ),
		  "-B", "-Dwext", psk, "-c", fstr );
	else
	    eval( "wpa_supplicant", "-B", "-Dwext", psk, "-c", fstr );
    }
    else
    {
	eval( "iwconfig", prefix, "key", "off" );
	// eval ("iwpriv", prefix, "authmode", "0");
    }

}

void start_supplicant( void )	// for testing only
{
    setupSupplicant( "wl0" );
}
