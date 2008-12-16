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

    FILE *fp, *tmp;
    char buf[256];
    char user[256];
    char pass[128];

	mkdir( "/tmp/proftpd", 0700 );
	mkdir( "/tmp/proftpd/etc", 0700 );
	mkdir( "/tmp/proftpd/var", 0700 );	
	
	if( nvram_invmatch( "proftpd_passw", "" ) )
		{
		nvram2file( "proftpd_passw", "/tmp/proftpd/etc/passwd.tmp" );
		tmp = fopen( "/tmp/proftpd/etc/passwd.tmp", "rb" );
		fp = fopen( "/tmp/proftpd/etc/passwd", "wb" );
		
        while (fgets (buf, sizeof( buf ), tmp) != NULL)
            {
            if (sscanf (buf, "%s %s", user, pass) == 2)
                {
	            if( strlen( pass ) == 34 ) //we assume pass is alredy encrypted
	                fprintf( fp, "%s:%s:0:0:Ftp User,,,:/tmp/root:/bin/sh\n", user, pass );
	            else
                    fprintf( fp, "%s:%s:0:0:Ftp User,,,:/tmp/root:/bin/sh\n", user, zencrypt( pass ) );
                }
            }
		fclose( fp );
		fclose( tmp );
		unlink( "/tmp/proftpd/etc/passwd.tmp" );		
  		}
  	else
  		eval( "cp", "/tmp/etc/passwd", "/tmp/proftpd/etc/passwd" );
  		
 // add ftp user (for anonymous access)	
	if( nvram_match( "proftpd_anon", "1" ) )
		{
		fp = fopen( "/tmp/proftpd/etc/passwd", "ab" );
		fprintf( fp,
		"ftp:x:0:0:Ftp Anon,,,:/tmp/root:/bin/sh\n" );
		fclose( fp );
		}

  		
	
	fp = fopen( "/tmp/proftpd/etc/proftpd.conf", "wb" );
	fprintf( fp, 
		 "ServerName      DD-WRT\n"
		 "DefaultAddress  %s\n"
		 "ServerType      standalone\n"
		 "DefaultServer   on\n"
		 "AuthUserFile    /tmp/proftpd/etc/passwd\n"
		 "ScoreboardFile  /tmp/proftpd/etc/proftpd.scoreboard\n"
		 "Port            %s\n"
		 "Umask           022\n"
		 "MaxInstances    10\n"
		 "User            root\n"
		 "Group           root\n"
		 "UseReverseDNS   off\n"
		 "IdentLookups    off\n"
		 "RootLogin       on\n"
		 "<Limit SITE_CHMOD>\n"
		 "  DenyAll\n"
		 "</Limit>\n"
		 "DelayEngine     off\n"
		 "WtmpLog         off\n"
		 "DefaultRoot     /%s\n"
		 "<Directory      /%s/*>\n"
		 "  AllowOverwrite  on\n"
		 "   <Limit WRITE>\n"
		 "%s"
		 "   </Limit>\n"
		 "</Directory>\n",
		 nvram_safe_get( "lan_ipaddr" ),
		 nvram_safe_get( "proftpd_port" ),
		 nvram_safe_get( "proftpd_dir" ),
		 nvram_safe_get( "proftpd_dir" ),
		 nvram_match( "proftpd_writeen", "on" ) ? "" : "    DenyAll\n" );
		 
// Anonymous ftp - read only
	if( nvram_match( "proftpd_anon", "1" ) )
	{
	fprintf( fp, 
		 "<Anonymous      /%s%s>\n"
		 "User           ftp\n"
		 "Group          root\n"
		 "UserAlias      anonymous ftp\n"
		 "<Directory *>\n"
		 "  <Limit WRITE>\n"
		 "    DenyAll\n"
		 "  </Limit>\n"
		 "</Directory>\n"
		 "</Anonymous>\n",
		 nvram_safe_get( "proftpd_dir" ),
		 nvram_safe_get( "proftpd_anon_subdir" ) );
	 }

	fclose( fp );
	
		
	eval( "proftpd");
	syslog( LOG_INFO,
		"Proftpd : proftpd server successfully started\n" );

    return;
}

void stop_ftpsrv( void )
{

	unlink( "/tmp/proftpd/etc/passwd" );
	unlink( "/tmp/proftpd/etc/proftpd.conf" );
	unlink( "/tmp/proftpd/etc/proftpd.scoreboard" );
	
    if( pidof( "proftpd" ) > 0 )
    {
	syslog( LOG_INFO,
		"Proftpd : proftpd server successfully stopped\n" );
	killall( "proftpd", SIGTERM );
	}
}
#endif
