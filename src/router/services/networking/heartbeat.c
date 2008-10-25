/*
 * heartbeat.c
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
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <shutils.h>
#include <bcmnvram.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>

#define DEBUG_HEARTBEAT

#ifdef DEBUG_HEARTBEAT
#define MY_LOG syslog
#else
#define MY_LOG(level, fmt, args...)
#endif

static int start_heartbeat( int status )
{
    FILE *fp;
    int ret;
    char authserver[20];
    char authdomain[80];
    char buf[254];

    if( nvram_invmatch( "wan_proto", "heartbeat" ) )
	return 0;

    openlog( "heartbeat", LOG_PID | LOG_NDELAY, LOG_DAEMON );

    MY_LOG( LOG_DEBUG, "hb_server_ip[%s] wan_get_domain[%s]\n",
	    nvram_safe_get( "hb_server_ip" ),
	    nvram_safe_get( "wan_get_domain" ) );

    /*
     * We must find out HB auth server from domain that get by dhcp if user
     * don't input HB sever. 
     */
    if( nvram_invmatch( "hb_server_ip", "" )
	&& nvram_invmatch( "hb_server_ip", "0.0.0.0" ) )
    {
	snprintf( authserver, sizeof( authserver ), "%s",
		  nvram_safe_get( "hb_server_ip" ) );
	snprintf( authdomain, sizeof( authdomain ), "%s", "" );
    }
    else if( ( nvram_match( "wan_get_domain", "nsw.bigpond.net.au" ) ) ||	// NSW
	     ( nvram_match( "wan_get_domain", "vic.bigpond.net.au" ) ) ||	// Victoria
	     ( nvram_match( "wan_get_domain", "qld.bigpond.net.au" ) ) ||	// Queensland
	     ( nvram_match( "wan_get_domain", "sa.bigpond.net.au" ) ) ||	// South 
	     // Australia
	     ( nvram_match( "wan_get_domain", "wa.bigpond.net.au" ) ) )
    {				// Western Australia
	snprintf( authserver, sizeof( authserver ), "%s", "sm-server" );
	snprintf( authdomain, sizeof( authdomain ), "%s",
		  nvram_safe_get( "wan_get_domain" ) );
    }
    else
    {
	MY_LOG( LOG_ERR, "Can't find HB server from domain! Use gateway.\n" );
	snprintf( authserver, sizeof( authserver ), "%s",
		  nvram_safe_get( "wan_gateway" ) );
	snprintf( authdomain, sizeof( authdomain ), "%s", "" );
	// return 1;
    }

    snprintf( buf, sizeof( buf ), "%s%s%s", authserver,
	      !strcmp( authdomain, "" ) ? "" : ".", authdomain );

    nvram_set( "hb_server_name", buf );

    MY_LOG( LOG_INFO, "Connecting to HB server [%s]\n", buf );

    if( !( fp = fopen( "/tmp/bpalogin.conf", "w" ) ) )
    {
	MY_LOG( LOG_ERR, "Can't write %s\n", "/tmp/bpalogin.conf" );
	perror( "/tmp/bpalogin.conf" );
	return errno;
    }
    fprintf( fp, "username %s\n", nvram_safe_get( "ppp_username" ) );
    fprintf( fp, "password %s\n", nvram_safe_get( "ppp_passwd" ) );
    fprintf( fp, "authserver %s\n", authserver );
    if( strcmp( authdomain, "" ) )
    {
	fprintf( fp, "authdomain %s\n", authdomain );
    }
    fprintf( fp, "localport 5050\n" );
    fprintf( fp, "logging syslog\n" );
    fprintf( fp, "debuglevel 2\n" );
    fprintf( fp, "minheartbeatinterval 60\n" );
    fprintf( fp, "maxheartbeatinterval 420\n" );
    fprintf( fp, "connectedprog hb_connect\n" );
    fprintf( fp, "disconnectedprog hb_disconnect\n" );

    fclose( fp );

    mkdir( "/tmp/ppp", 0777 );
    if( ( fp = fopen( "/tmp/hb_connect_success", "r" ) ) )
    {
	ret = eval( "bpalogin", "-c", "/tmp/bpalogin.conf", "-t" );
	fclose( fp );
    }
    else
	ret = eval( "bpalogin", "-c", "/tmp/bpalogin.conf" );

    if( nvram_invmatch( "ppp_demand", "1" ) )
    {
	if( status != REDIAL )
	    start_redial(  );
    }

    return ret;
}

int stop_heartbeat( void )
{
    int ret;

    unlink( "/tmp/ppp/link" );
    ret = killall( "bpalogin", SIGTERM );

    cprintf( "done\n" );

    return ret;
}

void start_heartbeat_boot( void )
{
    start_heartbeat( BOOT );

}

void start_heartbeat_redial( void )
{
    start_heartbeat( REDIAL );

}

/*
 *  Called when link comes up
 *  argv[1] : listenport
 *  argv[2] : pid
 *  argv[3] : IP address of heartbeat server
 */
int hb_connect_main( int argc, char **argv )
{
    FILE *fp;

    openlog( "heartbeat", LOG_PID, LOG_DAEMON );

    MY_LOG( LOG_INFO, "The user has been logged in successfully\n" );

    mkdir( "/tmp/ppp", 0777 );

    if( !( fp = fopen( "/tmp/ppp/link", "a" ) ) )
    {
	perror( "/tmp/ppp/link" );
	return errno;
    }
    fprintf( fp, "%s", argv[2] );
    fclose( fp );

    start_wan_done( get_wan_face(  ) );

    sysprintf
	( "iptables -I INPUT -i %s -p udp -s %s -d %s --dport %s -j ACCEPT",
	  get_wan_face(  ), argv[3], nvram_safe_get( "wan_ipaddr" ),
	  argv[1] );

    return TRUE;
}

/*
 *  * Called when link goes down
 *   */
int hb_disconnect_main( int argc, char **argv )
{

    openlog( "heartbeat", LOG_PID, LOG_DAEMON );

    MY_LOG( LOG_INFO, "The user has been logged out\n" );

    if( strcmp( argv[1], "1" ) )
    {
	stop_wan(  );
    }

    return unlink( "/tmp/ppp/link" );
}
