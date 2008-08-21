/*
 * chillispot.c
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <nvparse.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <syslog.h>
#include <wlutils.h>
#include <errno.h>

#ifdef HAVE_CHILLI

int start_chilli( void )
{
    int ret = 0;
    FILE *fp;
    int i;

    if( !nvram_match( "chilli_enable", "1" ) )
	return 0;

#ifdef HAVE_CHILLILOCAL

    if( !( fp = fopen( "/tmp/fonusers.local", "w" ) ) )
    {
	perror( "/tmp/fonusers.local" );
	return errno;
    }
    char *users = nvram_safe_get( "fon_userlist" );
    char *u = ( char * )malloc( strlen( users ) + 1 );
    char *o = u;

    strcpy( u, users );
    char *sep = strsep( &u, "=" );

    while( sep != NULL )
    {
	fprintf( fp, "%s ", sep );
	char *pass = strsep( &u, " " );

	fprintf( fp, "%s \n", pass != NULL ? pass : "" );
	sep = strsep( &u, "=" );
    }
    free( o );
    fclose( fp );
#endif

    if( !( fp = fopen( "/tmp/chilli.conf", "w" ) ) )
    {
	perror( "/tmp/chilli.conf" );
	return errno;
    }

    fprintf( fp, "radiusserver1 %s\n", nvram_get( "chilli_radius" ) );
    fprintf( fp, "radiusserver2 %s\n", nvram_get( "chilli_backup" ) );
    fprintf( fp, "radiussecret %s\n", nvram_get( "chilli_pass" ) );
    fprintf( fp, "dhcpif %s\n", nvram_safe_get( "chilli_interface" ) );

    fprintf( fp, "uamserver %s\n", nvram_get( "chilli_url" ) );
    if( nvram_invmatch( "chilli_dns1", "0.0.0.0" )
	&& nvram_invmatch( "chilli_dns1", "" ) )
    {
	fprintf( fp, "dns1 %s\n", nvram_get( "chilli_dns1" ) );
	if( nvram_invmatch( "sv_localdns", "0.0.0.0" )
	    && nvram_invmatch( "sv_localdns", "" ) )
	    fprintf( fp, "dns2 %s\n", nvram_get( "sv_localdns" ) );
    }
    else if( nvram_invmatch( "sv_localdns", "0.0.0.0" )
	     && nvram_invmatch( "sv_localdns", "" ) )
	fprintf( fp, "dns1 %s\n", nvram_get( "sv_localdns" ) );

    if( nvram_invmatch( "chilli_uamsecret", "" ) )
	fprintf( fp, "uamsecret %s\n", nvram_get( "chilli_uamsecret" ) );
    if( nvram_invmatch( "chilli_uamanydns", "0" ) )
	fprintf( fp, "uamanydns\n" );
    if( nvram_invmatch( "chilli_uamallowed", "" ) )
	fprintf( fp, "uamallowed %s\n", nvram_get( "chilli_uamallowed" ) );
    if( nvram_invmatch( "chilli_net", "" ) )
	fprintf( fp, "net %s\n", nvram_get( "chilli_net" ) );
    if( nvram_match( "chilli_macauth", "1" ) )
	fprintf( fp, "macauth\n" );
#ifndef HAVE_FON
    if( nvram_match( "fon_enable", "1" ) )
    {
#endif
	char hyp[32];

	strcpy( hyp, nvram_safe_get( "wl0_hwaddr" ) );
	for( i = 0; i < strlen( hyp ); i++ )
	    if( hyp[i] == ':' )
		hyp[i] = '-';
	if( i > 0 )
	    fprintf( fp, "radiusnasid %s\n", hyp );
	nvram_set( "chilli_radiusnasid", hyp );
	fprintf( fp, "interval 300\n" );
#ifndef HAVE_FON
    }
    else
    {
	if( nvram_invmatch( "chilli_radiusnasid", "" ) )
	    fprintf( fp, "radiusnasid %s\n",
		     nvram_get( "chilli_radiusnasid" ) );
    }
#endif

    if( nvram_invmatch( "chilli_additional", "" ) )
    {
	char *add = nvram_safe_get( "chilli_additional" );

	i = 0;
	do
	{
	    if( add[i] != 0x0D )
		fprintf( fp, "%c", add[i] );
	}
	while( add[++i] );
	i = 0;
	int a = 0;
	char *filter = strdup( add );

	do
	{
	    if( add[i] != 0x0D )
		filter[a++] = add[i];
	}
	while( add[++i] );

	filter[a] = 0;
	if( strcmp( filter, add ) )
	{
	    nvram_set( "chilli_additional", filter );
	    nvram_commit(  );
	}
	free( filter );
    }
    fflush( fp );
    fclose( fp );
    /*
     * if (nvram_match ("ntp_enable", "1")) { if (time(0)<1000) { sleep(10);
     * // wait for ntp connectivity } } 
     */
    ret = killall( "chilli", SIGTERM );
    ret = killall( "chilli", SIGKILL );
    ret = eval( "chilli", "-c", "/tmp/chilli.conf" );
    dd_syslog( LOG_INFO, "chilli : chilli daemon successfully started\n" );

    cprintf( "done\n" );
    return ret;
}

int stop_chilli( void )
{
    int ret = 0;

    if( pidof( "chilli" ) > 0 )
    {
	syslog( LOG_INFO, "chilli : chilli daemon successfully stopped\n" );
	ret = killall( "chilli", SIGKILL );
    }
    cprintf( "done\n" );
    return ret;
}

#endif /* HAVE_CHILLI */
