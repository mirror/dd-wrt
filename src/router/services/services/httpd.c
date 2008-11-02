/*
 * httpd.c
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
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>

void start_httpd( void )
{
    int ret = 0;

    if( nvram_invmatch( "http_enable", "0" )
	&& !is_exist( "/var/run/httpd.pid" ) )
    {
	chdir( "/www" );
	cprintf( "[HTTPD Starting on /www]\n" );
	if( nvram_invmatch( "http_lanport", "" ) )
	{
	    char *lan_port = nvram_safe_get( "http_lanport" );

	    ret = eval( "httpd", "-p", lan_port );
	}
	else
	{
	    ret = eval( "httpd" );
	    dd_syslog( LOG_INFO,
		       "httpd : http daemon successfully started\n" );
	}
	chdir( "/" );
    }
#ifdef HAVE_HTTPS
    if( nvram_invmatch( "https_enable", "0" )
	&& !is_exist( "/var/run/httpsd.pid" ) )
    {

	// Generate a new certificate
	// if(!is_exist("/tmp/cert.pem") || !is_exist("/tmp/key.pem"))
	// eval("gencert.sh", BUILD_SECS); 

	chdir( "/www" );
	ret = eval( "httpd", "-S" );
	syslog( LOG_INFO, "httpd : https daemon successfully started\n" );
	chdir( "/" );
    }
#endif

    cprintf( "done\n" );
    return;
}

void stop_httpd( void )
{
    int ret = 0;

    if( pidof( "httpd" ) > 0 )
    {
	syslog( LOG_INFO, "httpd : http daemon successfully stopped\n" );
	ret = killall( "httpd", SIGTERM );

	cprintf( "done\n" );
    }
    unlink( "/var/run/httpd.pid" );
#ifdef HAVE_HTTPS
    unlink( "/var/run/httpsd.pid" );
#endif
    return;
}
