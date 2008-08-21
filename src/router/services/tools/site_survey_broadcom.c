/*
 * site_survey_broadcom.c
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
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <utils.h>

#define sys_restart() kill(1, SIGHUP)
#define SITE_SURVEY_DB	"/tmp/site_survey"
#define SITE_SURVEY_NUM	50

int write_site_survey( void );
static int open_site_survey( void );
int write_site_survey( void );

struct site_survey_list
{
    uint8 SSID[33];
    unsigned char BSSID[18];
    uint8 channel;		/* Channel no. */
    int16 RSSI;			/* receive signal strength (in dBm) */
    int16 phy_noise;		/* noise (in dBm) */
    uint16 beacon_period;	/* units are Kusec */
    uint16 capability;		/* Capability information */
    // unsigned char athcaps;
    unsigned char ENCINFO[32];	/* encryption info */
    uint rate_count;		/* # rates in this set */
    uint8 dtim_period;		/* DTIM period */
} site_survey_lists[SITE_SURVEY_NUM];

static bool wpa_ie( uint8 ** wpaie, uint8 ** tlvs, uint * tlvs_len )
{
    uint8 *ie = *wpaie;

    if( ( ie[1] >= 6 ) && !memcmp( &ie[2], WPA_OUI "\x01", 4 ) )
    {
	return TRUE;
    }
    ie += ie[1] + 2;
    *tlvs_len -= ( int )( ie - *tlvs );
    *tlvs = ie;

    return FALSE;
}

static uint8 *parse_tlvs( uint8 * tlv_buf, int buflen, uint key )
{
    uint8 *cp;
    int totlen;

    cp = tlv_buf;
    totlen = buflen;

    /*
     * find tagged parameter 
     */
    while( totlen >= 2 )
    {
	uint tag;
	int len;

	tag = *cp;
	len = *( cp + 1 );

	/*
	 * validate remaining totlen 
	 */
	if( ( tag == key ) && ( totlen >= ( len + 2 ) ) )
	    return ( cp );

	cp += ( len + 2 );
	totlen -= ( len + 2 );
    }

    return NULL;
}

static char *dump_bss_ie( uint8 * cp, uint len )
{
    uint8 *wpaie;
    int i;
    int n;
    int offset;
    uint8 *parse = cp;
    uint parse_len = len;
    uint16 capabilities;
    int unicast_count = 0;
    uint8 oui[3];
    uint8 idx = 0;

    while( ( wpaie = parse_tlvs( parse, parse_len, DOT11_MNG_WPA_ID ) ) )
	if( wpa_ie( &wpaie, &parse, &parse_len ) )
	    break;
    if( wpaie == NULL )
	// return "Unknown";
	return "WEP";		// Eko, testing...

    if( ( wpaie[6] | ( wpaie[7] << 8 ) ) != WPA_VERSION )
	return "WPA-Unsupported";	/* WPA version unsupported */

    // We got WPA
    char sum[64] = { 0 };
    /*
     * Check for multicast suite 
     */
    if( wpaie[1] >= 10 )
    {
	if( !memcmp( &wpaie[8], WPA_OUI, 3 ) )
	{
	    switch ( wpaie[11] )
	    {
		case WPA_CIPHER_NONE:
		    strcat( sum, "MULTINONE " );
		    break;
		case WPA_CIPHER_WEP_40:
		    strcat( sum, "MULTIWEP64 " );
		    break;
		case WPA_CIPHER_WEP_104:

		    strcat( sum, "MULTIWEP128 " );
		    break;
		case WPA_CIPHER_TKIP:
		    strcat( sum, "MULTITKIP " );
		    break;
		case WPA_CIPHER_AES_OCB:
		    strcat( sum, "MULTIAESOCB " );
		    break;
		case WPA_CIPHER_AES_CCM:
		    strcat( sum, "MULTIAESCCMP " );
		    break;
		default:	/* unknown WPA cipher */
		    strcat( sum, "MULTIWPAUNKOWN " );
		    break;
	    }
	}
    }

    /*
     * Check for unicast suite(s) 
     */
    if( wpaie[1] >= 12 )
    {
	unicast_count = ( wpaie[12] | ( wpaie[13] << 8 ) );
	for( i = 0; i < unicast_count; i++ )
	{

	    if( wpaie[1] < ( 12 + ( i * 4 ) + 4 ) )
		break;

	    memcpy( oui, &wpaie[2 + 12 + ( i * 4 )], 3 );
	    idx = wpaie[2 + 12 + ( i * 4 ) + 3];

	    if( !memcmp( oui, WPA_OUI, 3 ) )
	    {
		switch ( idx )
		{
		    case WPA_CIPHER_NONE:
			break;
		    case WPA_CIPHER_WEP_40:
			strcat( sum, "WEP64 " );
			break;
		    case WPA_CIPHER_WEP_104:
			strcat( sum, "WEP128 " );
			break;
		    case WPA_CIPHER_TKIP:
			strcat( sum, "TKIP " );
			break;
		    case WPA_CIPHER_AES_OCB:
			strcat( sum, "AESOCB " );
			break;
		    case WPA_CIPHER_AES_CCM:
			strcat( sum, "AESCCMP " );
			break;
		    default:
			break;
		}
	    }
	    else
	    {
		strcat( sum, "UNICHIPHERWPAUNKNOWN " );
	    }
	}
    }
    /*
     * Authentication Key Management 
     */
    /*
     * Fixed 8, Group 4 , 2 + min one Unicast 4 - 2 bytes of VerID and Len 
     */
    if( wpaie[1] >= 16 )
    {
	offset = 8 + 4 + 2 + ( unicast_count * 4 );
	n = wpaie[offset] + ( wpaie[offset + 1] << 8 );
	if( wpaie[1] < ( offset + ( n * 4 ) ) )
	{
	    return sum;
	}
	for( i = 0; i < n; i++ )
	{

	    memcpy( oui, &wpaie[offset + 2 + ( i * 4 )], 3 );
	    idx = wpaie[offset + 2 + ( i * 4 ) + 3];

	    if( !memcmp( oui, WPA_OUI, 3 ) )
	    {
		switch ( idx )
		{
		    case RSN_AKM_NONE:
			strcat( sum, "WPA-NONE" );
			break;
		    case RSN_AKM_UNSPECIFIED:
			strcat( sum, "WPA" );
			break;
		    case RSN_AKM_PSK:
			strcat( sum, "WPA-PSK" );
			break;
		    default:
			strcat( sum, "WPA-Unknown" );
			break;
		}
	    }
	    else
	    {
		strcat( sum, "WPA-Unknown" );
	    }
	}
    }
    return sum;
}

static char *getEncInfo( wl_bss_info_t * bi )
{
    if( bi->capability & DOT11_CAP_PRIVACY )
    {
	if( bi->ie_length )
#ifdef HAVE_MSSID
	    return
		dump_bss_ie( ( uint8 * ) ( ( ( uint8 * ) bi ) +
					   bi->ie_offset ), bi->ie_length );
#else
	    return
		dump_bss_ie( ( uint8 * ) ( ( ( uint8 * ) bi ) +
					   sizeof( wl_bss_info_t ) ),
			     bi->ie_length );
#endif
	else
	    return "WEP";
    }
    else
	return "Open";
}

int site_survey_main( int argc, char *argv[] )
{
    char *name = nvram_safe_get( "wl0_ifname" );
    unsigned char buf[10000];
    wl_scan_results_t *scan_res = ( wl_scan_results_t * ) buf;
    wl_bss_info_t *bss_info;
    unsigned char mac[20];
    int i;
    char *dev = name;

    unlink( SITE_SURVEY_DB );
    int ap = 0, oldap = 0;
    wl_scan_params_t params;

    memset( &params, 0, sizeof( params ) );

    /*
     * use defaults (same parameters as wl scan) 
     */

    memset( &params.bssid, 0xff, sizeof( params.bssid ) );
    if( argc > 1 )
    {
	params.ssid.SSID_len = strlen( argv[1] );
	strcpy( params.ssid.SSID, argv[1] );
    }
    params.bss_type = DOT11_BSSTYPE_ANY;
    params.scan_type = -1;
    params.nprobes = -1;
    params.active_time = -1;
    params.passive_time = -1;
    params.home_time = -1;

    /*
     * can only scan in STA mode 
     */
#ifndef HAVE_MSSID
    wl_ioctl( dev, WLC_GET_AP, &oldap, sizeof( oldap ) );
    if( oldap > 0 )
	eval( "wl", "ap", "0" );
#endif
    if( wl_ioctl( dev, WLC_SCAN, &params, 64 ) < 0 )
    {
	fprintf( stderr, "scan failed\n" );
	return -1;
    }
    sleep( 1 );
    bzero( buf, sizeof( buf ) );
    scan_res->buflen = sizeof( buf );

    if( wl_ioctl( dev, WLC_SCAN_RESULTS, buf, WLC_IOCTL_MAXLEN ) < 0 )
    {
	fprintf( stderr, "scan results failed\n" );
	return -1;
    }

    fprintf( stderr, "buflen=[%d] version=[%d] count=[%d]\n",
	     scan_res->buflen, scan_res->version, scan_res->count );

    if( scan_res->count == 0 )
    {
	cprintf( "Can't find any wireless device\n" );
	goto endss;
    }

    bss_info = &scan_res->bss_info[0];
    for( i = 0; i < scan_res->count; i++ )
    {
	strcpy( site_survey_lists[i].SSID, bss_info->SSID );
	strcpy( site_survey_lists[i].BSSID,
		ether_etoa( bss_info->BSSID.octet, mac ) );
#ifndef HAVE_RB500
#ifndef HAVE_MSSID
	site_survey_lists[i].channel = bss_info->channel;
#else
	site_survey_lists[i].channel = bss_info->chanspec & 0xff;
#endif
#endif
	site_survey_lists[i].RSSI = bss_info->RSSI;
	site_survey_lists[i].phy_noise = bss_info->phy_noise;
	site_survey_lists[i].beacon_period = bss_info->beacon_period;
	site_survey_lists[i].capability = bss_info->capability;
	site_survey_lists[i].rate_count = bss_info->rateset.count;
	site_survey_lists[i].dtim_period = bss_info->dtim_period;
	strcpy( site_survey_lists[i].ENCINFO, getEncInfo( bss_info ) );

	bss_info =
	    ( wl_bss_info_t * ) ( ( uint32 ) bss_info + bss_info->length );
    }
    write_site_survey(  );
    open_site_survey(  );
    for( i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].SSID[0]; i++ )
    {
	fprintf( stderr,
		 "[%2d] SSID[%20s] BSSID[%s] channel[%2d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d] enc[%s]\n",
		 i, site_survey_lists[i].SSID, site_survey_lists[i].BSSID,
		 site_survey_lists[i].channel, site_survey_lists[i].RSSI,
		 site_survey_lists[i].phy_noise,
		 site_survey_lists[i].beacon_period,
		 site_survey_lists[i].capability,
		 site_survey_lists[i].dtim_period,
		 site_survey_lists[i].rate_count,
		 site_survey_lists[i].ENCINFO );
    }

  endss:
#ifndef HAVE_MSSID
    if( oldap > 0 )
	eval( "wl", "ap", "1" );
#endif

    C_led( 0 );
#ifdef HAVE_MSSID
    eval( "wl", "up" );
#endif
    return 0;
}

int write_site_survey( void )
{
    FILE *fp;

    if( ( fp = fopen( SITE_SURVEY_DB, "w" ) ) )
    {
	fwrite( &site_survey_lists[0], sizeof( site_survey_lists ), 1, fp );
	fclose( fp );
	return FALSE;
    }
    return TRUE;
}

static int open_site_survey( void )
{
    FILE *fp;

    bzero( site_survey_lists, sizeof( site_survey_lists ) );

    if( ( fp = fopen( SITE_SURVEY_DB, "r" ) ) )
    {
	fread( &site_survey_lists[0], sizeof( site_survey_lists ), 1, fp );
	fclose( fp );
	return TRUE;
    }
    return FALSE;
}
