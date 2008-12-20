/*
 * site_survey_madwifi.c
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

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <shutils.h>

static int
copy_essid( char buf[], size_t bufsize, const u_int8_t * essid,
	    size_t essid_len )
{
    const u_int8_t *p;
    int maxlen;
    int i;

    if( essid_len > bufsize )
	maxlen = bufsize;
    else
	maxlen = essid_len;
    /*
     * determine printable or not 
     */
    for( i = 0, p = essid; i < maxlen; i++, p++ )
    {
	if( *p < ' ' || *p > 0x7e )
	    break;
    }
    if( i != maxlen )
    {				/* not printable, print as hex */
	if( bufsize < 3 )
	    return 0;
#if 0
	strlcpy( buf, "0x", bufsize );
#else
	strncpy( buf, "0x", bufsize );
#endif
	bufsize -= 2;
	p = essid;
	for( i = 0; i < maxlen && bufsize >= 2; i++ )
	{
	    sprintf( &buf[2 + 2 * i], "%02x", *p++ );
	    bufsize -= 2;
	}
	maxlen = 2 + 2 * i;
    }
    else
    {				/* printable, truncate as needed */
	memcpy( buf, essid, maxlen );
    }
    if( maxlen != essid_len )
	memcpy( buf + maxlen - 3, "...", 3 );
    return maxlen;
}

#define sys_restart() kill(1, SIGHUP)
#define SITE_SURVEY_DB	"/tmp/site_survey"
#define SITE_SURVEY_NUM	256

int write_site_survey( void );
static int open_site_survey( void );
int write_site_survey( void );

static struct site_survey_list
{
    unsigned char SSID[33];
    unsigned char BSSID[18];
    unsigned char channel;	/* Channel no. */
    short RSSI;			/* receive signal strength (in dBm) */
    short phy_noise;		/* noise (in dBm) */
    unsigned short beacon_period;	/* units are Kusec */
    unsigned short capability;	/* Capability information */
    // unsigned char athcaps;
    unsigned char ENCINFO[128];	/* encryption info */
    int rate_count;		/* # rates in this set */
    unsigned char dtim_period;	/* DTIM period */
} site_survey_lists[SITE_SURVEY_NUM];

int site_survey_main( int argc, char *argv[] )
{
#define DOT11_CAP_ESS				0x0001
#define DOT11_CAP_IBSS				0x0002
#define DOT11_CAP_PRIVACY			0x0010	/* d11 cap. privacy */

    unsigned char b1[32], b2[64], b3[32], b4[32], b5[32], b6[32], b7[32],
	b8[32];
    int i = 0;

    unlink( SITE_SURVEY_DB );
    int ap = 0, oldap = 0;
    int len;

    memset( site_survey_lists, sizeof( site_survey_lists ), 0 );
    if( nvram_match( "wl0_mode", "ap" )
	|| nvram_match( "wl0_mode", "apsta" ) )
    {
	eval( "iwpriv", "ra0", "set", "SiteSurvey=1" );	// only in ap mode
	sleep( 4 );		//wait 4 seconds per spec
    }

    FILE *scan = popen( "iwpriv ra0 get_site_survey", "rb" );

    fscanf( scan, "%s %s", b1, b2 );	// skip first line
    fscanf( scan, "%s %s %s %s %s %s %s", b1, b2, b3, b4, b5, b6, b7 );	//skip second line
    i = 0;
    do
    {
	if( feof( scan ) )
	    break;
	fread( b1, 4, 1, scan );
	b1[4] = 0;
	b1[strlen( b1 )] = 0;
	fread( b2, 33, 1, scan );
	b2[32] = 0;
	b2[strlen( b2 )] = 0;
	int ret = fscanf( scan, "%s %s %s %s %s %s", b3, b4, b5, b6, b7, b8 );	//skip second line

	if( ret < 6 )
	    break;
	site_survey_lists[i].channel = atoi( b1 );	// channel
	strcpy( site_survey_lists[i].SSID, b2 );	//SSID
	strcpy( site_survey_lists[i].BSSID, b3 );	//BSSID
	site_survey_lists[i].phy_noise = -95;	// no way
	strcpy( site_survey_lists[i].ENCINFO, b5 );
	strcat( site_survey_lists[i].ENCINFO, b4 );
	site_survey_lists[i].RSSI = -atoi( b6 );

	if( !strcmp( b7, "11b/g" ) )
	    site_survey_lists[i].rate_count = 12;
	if( !strcmp( b7, "11b" ) )
	    site_survey_lists[i].rate_count = 4;
	if( !strcmp( b7, "11b/g/n" ) )
	    site_survey_lists[i].rate_count = 300;

	if( !strcmp( b8, "In" ) )
	    site_survey_lists[i].capability = DOT11_CAP_ESS;

	if( !strcmp( b8, "Ad" ) )
	    site_survey_lists[i].capability = DOT11_CAP_IBSS;

	if( strcmp( b5, "OPEN" ) )
	    site_survey_lists[i].capability |= DOT11_CAP_PRIVACY;

	i++;
    }
    while( 1 );
    fclose( scan );
    write_site_survey(  );
    open_site_survey(  );
    for( i = 0;
	 i < SITE_SURVEY_NUM && site_survey_lists[i].BSSID[0]
	 && site_survey_lists[i].channel != 0; i++ )
    {

	fprintf( stderr,
		 "[%2d] SSID[%20s] BSSID[%s] channel[%2d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d]\n",
		 i, site_survey_lists[i].SSID, site_survey_lists[i].BSSID,
		 site_survey_lists[i].channel, site_survey_lists[i].RSSI,
		 site_survey_lists[i].phy_noise,
		 site_survey_lists[i].beacon_period,
		 site_survey_lists[i].capability,
		 site_survey_lists[i].dtim_period,
		 site_survey_lists[i].rate_count );
    }

    return 0;
}

int write_site_survey( void )
{
    FILE *fp;

    if( ( fp = fopen( SITE_SURVEY_DB, "w" ) ) )
    {
	fwrite( &site_survey_lists[0], sizeof( site_survey_lists ), 1, fp );
	fclose( fp );
	return 0;
    }
    return 1;
}

static int open_site_survey( void )
{
    FILE *fp;

    bzero( site_survey_lists, sizeof( site_survey_lists ) );

    if( ( fp = fopen( SITE_SURVEY_DB, "r" ) ) )
    {
	fread( &site_survey_lists[0], sizeof( site_survey_lists ), 1, fp );
	fclose( fp );
	return 1;
    }
    return 0;
}
