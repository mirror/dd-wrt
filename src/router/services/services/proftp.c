/*
 * proftp.c
 *
 * Copyright (C) 2008 dd-wrt
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
#ifdef HAVE_FTP
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>

void start_ftpsrv( void )
{
    if( !nvram_match( "proftpd_enable", "1" ) )
	return;


    FILE *fp;

	mkdir( "/tmp/proftpd", 0700 );
	mkdir( "/tmp/proftpd/etc", 0700 );
	mkdir( "/tmp/proftpd/var", 0700 );	
	fp = fopen( "/tmp/proftpd/etc/proftpd.conf", "wb" );
	fprintf( fp, 
		 "ServerName      DD-WRT\n"
		 "DefaultAddress  %s\n"
		 "ServerType      standalone\n"
		 "DefaultServer   on\n"
		 "AuthUserFile    /tmp/etc/passwd\n"
		 "ScoreboardFile  /tmp/proftpd/etc/proftpd.scoreboard\n"
		 "Port            %s\n"
		 "Umask           022\n"
		 "MaxInstances    10\n"
		 "User            root\n"
		 "Group           root\n"
		 "UseReverseDNS   off\n"
		 "IdentLookups    off\n"
		 "RootLogin       on\n"
		 "AllowOverwrite  off\n"
		 "<Limit SITE_CHMOD>\n"
		 "  DenyAll\n"
		 "</Limit>\n"
		 "DelayEngine     off\n"
		 "DefaultChdir    /%s\n"
		 "<Directory      /%s/*>\n"
		 "AllowOverwrite  %s\n"
		 "</Directory>\n",
		 nvram_safe_get( "lan_ipaddr" ),
		 nvram_safe_get( "proftpd_port" ),
		 nvram_safe_get( "proftpd_dir" ),
		 nvram_safe_get( "proftpd_dir" ),
		 nvram_safe_get( "proftpd_writeen" ) );

		 
	fclose( fp );
		
	eval( "proftpd");
	syslog( LOG_INFO,
		"Proftpd : proftpd server successfully started\n" );

    return;
}

void stop_ftpsrv( void )
{

    if( pidof( "proftpd" ) > 0 )
    {
	syslog( LOG_INFO,
		"Proftpd : proftpd server successfully stopped\n" );
	killall( "proftpd", SIGTERM );
	}
}
#endif
