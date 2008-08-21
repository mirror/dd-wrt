/*
 * olsrd.c
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

#ifdef HAVE_OLSRD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void stop_olsrd( void )
{
    if( pidof( "olsrd" ) > 0 )
    {
	syslog( LOG_INFO, "olsrd : OLSR daemon successfully stopped\n" );
	killall( "olsrd", SIGTERM );
    }
}

void start_olsrd( void )
{
    if( !nvram_match( "wk_mode", "olsr" ) )
	return;
    stop_olsrd(  );
    char net[64];

    strcpy( net, nvram_safe_get( "lan_ipaddr" ) );
    int a, b, c, d;

    sscanf( net, "%d.%d.%d.%d", &a, &b, &c, &d );
    sprintf( net, "%d.%d.%d.0", a, b, c );
    FILE *fp = fopen( "/tmp/olsrd.conf", "wb" );

    fprintf( fp, "DebugLevel\t0\n" );
    fprintf( fp, "IpVersion\t4\n" );
    fprintf( fp, "AllowNoInt\tyes\n" );
    fprintf( fp, "Pollrate\t%s\n", nvram_safe_get( "olsrd_pollsize" ) );
    fprintf( fp, "TcRedundancy\t%s\n", nvram_safe_get( "olsrd_redundancy" ) );
    fprintf( fp, "MprCoverage\t%s\n", nvram_safe_get( "olsrd_coverage" ) );
    fprintf( fp, "LinkQualityFishEye\t%s\n",
	     nvram_safe_get( "olsrd_lqfisheye" ) );
    fprintf( fp, "LinkQualityAging\t%s\n",
	     nvram_safe_get( "olsrd_lqaging" ) );
    fprintf( fp, "LinkQualityAlgorithm    \"etx_fpm\"\n" );
    fprintf( fp, "LinkQualityDijkstraLimit\t%s %s\n",
	     nvram_safe_get( "olsrd_lqdijkstramin" ),
	     nvram_safe_get( "olsrd_lqdijkstramax" ) );
    fprintf( fp, "UseHysteresis\t%s\n",
	     nvram_match( "olsrd_hysteresis", "1" ) ? "yes" : "no" );
    if( nvram_match( "olsrd_hysteresis", "0" ) )
	fprintf( fp, "LinkQualityLevel\t%s\n",
		 nvram_safe_get( "olsrd_lqlevel" ) );
    else
	fprintf( fp, "LinkQualityLevel\t0\n" );
    fprintf( fp, "LoadPlugin \"olsrd_dyn_gw_plain.so\"\n" );
    fprintf( fp, "{\n" );
    fprintf( fp, "}\n" );
#ifndef HAVE_MICRO
    fprintf( fp, "LoadPlugin \"olsrd_httpinfo.so\"\n" );
    fprintf( fp, "{\n" );
    fprintf( fp, "\tPlParam \"port\"\t\"8080\"\n" );
    fprintf( fp, "\tPlParam \"Host\"\t\"127.0.0.1\"\n" );
    fprintf( fp, "\tPlParam \"Net\"\t\"%s 255.255.255.0\"\n", net );
    fprintf( fp, "}\n" );
#endif
    fprintf( fp, "IpcConnect\n" );
    fprintf( fp, "{\n" );
    fprintf( fp, "\tMaxConnections\t1\n" );
    fprintf( fp, "\tHost\t127.0.0.1\n" );
    fprintf( fp, "\tNet\t%s 255.255.255.0\n", net );
    fprintf( fp, "}\n" );

    char *wordlist = nvram_safe_get( "olsrd_interfaces" );
    char *next;
    char word[128];

    foreach( word, wordlist, next )
    {
	char *interface = word;
	char *hellointerval = interface;

	strsep( &hellointerval, ">" );
	char *hellovaliditytime = hellointerval;

	strsep( &hellovaliditytime, ">" );
	char *tcinterval = hellovaliditytime;

	strsep( &tcinterval, ">" );
	char *tcvaliditytime = tcinterval;

	strsep( &tcvaliditytime, ">" );
	char *midinterval = tcvaliditytime;

	strsep( &midinterval, ">" );
	char *midvaliditytime = midinterval;

	strsep( &midvaliditytime, ">" );
	char *hnainterval = midvaliditytime;

	strsep( &hnainterval, ">" );
	char *hnavaliditytime = hnainterval;

	strsep( &hnavaliditytime, ">" );
	fprintf( fp, "Interface \"%s\"\n", interface );
	fprintf( fp, "{\n" );
	fprintf( fp, "\tHelloInterval\t%s\n", hellointerval );
	fprintf( fp, "\tHelloValidityTime\t%s\n", hellovaliditytime );
	fprintf( fp, "\tTcInterval\t%s\n", tcinterval );
	fprintf( fp, "\tTcValidityTime\t%s\n", tcvaliditytime );
	fprintf( fp, "\tMidInterval\t%s\n", midinterval );
	fprintf( fp, "\tMidValidityTime\t%s\n", midvaliditytime );
	fprintf( fp, "\tHnaInterval\t%s\n", hnainterval );
	fprintf( fp, "\tHnaValidityTime\t%s\n", hnavaliditytime );
	fprintf( fp, "}\n" );
    }
    if( strlen( nvram_safe_get( "olsrd_hna" ) ) > 0 )
    {
	fprintf( fp, "Hna4{\n" );
	fprintf( fp, "%s\n", nvram_safe_get( "olsrd_hna" ) );
	fprintf( fp, "}\n" );
    }
    fclose( fp );
    eval( "olsrd" );

    syslog( LOG_INFO, "olsrd : OLSR daemon successfully started\n" );
}

#endif
