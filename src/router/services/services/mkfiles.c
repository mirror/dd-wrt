/*
 * mkfiles.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>

#define HOME_DIR	"/tmp/root"
#define PASSWD_DIR	"/tmp/etc"
#define SSH_DIR		"/tmp/root/.ssh"
#define PASSWD_FILE	"/tmp/etc/passwd"
#define GROUP_FILE	"/tmp/etc/group"

int isregistered_real( void );

void setPassword( char *passwd )
{
    FILE *fp;
    struct stat buf;

    /*
     * Create password's and group's database directory 
     */
    if( stat( PASSWD_DIR, &buf ) != 0 )
    {
	mkdir( PASSWD_DIR, 0700 );
    }
    if( !( fp = fopen( PASSWD_FILE, "w" ) ) )
    {
	perror( PASSWD_FILE );
	return;
    }

    fprintf( fp, "root:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", passwd );
    fclose( fp );
}

void start_mkfiles( void )
{
    FILE *fp;
    struct stat buf;

    cprintf( "%s:%d", __func__, __LINE__ );
#ifdef HAVE_SKYTRON
    char *http_passwd = nvram_safe_get( "skyhttp_passwd" );
#elif HAVE_34TELECOM
    char *http_passwd = nvram_safe_get( "newhttp_passwd" );
#else
    char *http_passwd = nvram_safe_get( "http_passwd" );
#endif
    cprintf( "%s:%d", __func__, __LINE__ );
    if( stat( HOME_DIR, &buf ) != 0 )
    {
	mkdir( HOME_DIR, 0700 );
    }
    cprintf( "%s:%d", __func__, __LINE__ );

#ifdef HAVE_SSHD
    if( stat( SSH_DIR, &buf ) != 0 )
    {
	mkdir( SSH_DIR, 0700 );
    }
#endif
    cprintf( "%s:%d", __func__, __LINE__ );

    /*
     * Create password's and group's database directory 
     */
    if( stat( PASSWD_DIR, &buf ) != 0 )
    {
	mkdir( PASSWD_DIR, 0700 );
    }
    cprintf( "%s:%d", __func__, __LINE__ );

    /*
     * Write password file with username root and password 
     */
    if( !( fp = fopen( PASSWD_FILE, "w" ) ) )
    {
	perror( PASSWD_FILE );
	return;
    }
    cprintf( "%s:%d", __func__, __LINE__ );
#ifdef HAVE_REGISTER
    if( isregistered_real(  ) )
#endif
    {
	fprintf( fp, "root:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n"
		 "reboot:%s:0:0:Root User,,,:/tmp/root:/sbin/reboot\n",
		 http_passwd, http_passwd );
	fclose( fp );
    }
    cprintf( "%s:%d", __func__, __LINE__ );
    /*
     * Write group file with group 'root' 
     */
    if( !( fp = fopen( GROUP_FILE, "w" ) ) )
    {
	perror( GROUP_FILE );
	return;
    }
    cprintf( "%s:%d", __func__, __LINE__ );
    fprintf( fp, "root:x:0:\n" );
    fclose( fp );

    system2( "/bin/mkdir -p /var/spool" );
    system2( "/bin/mkdir -p /var/spool/cron" );
    system2( "/bin/mkdir -p /var/lock/subsys" );
    system2( "/bin/mkdir -p /var/spool/cron/crontabs" );
    system2( "/bin/touch /var/spool/cron/crontabs/root" );
    system2( "/bin/mkdir -p /var/lib" );
    system2( "/bin/mkdir -p /var/lib/misc" );
    system2( "/bin/mkdir -p /var/tmp" );

    system2( "/bin/mkdir -p /var/log" );
    system2( "/bin/touch /var/log/messages" );
    cprintf( "%s:%d", __func__, __LINE__ );

#ifdef HAVE_SNMP
    system2( "/bin/mkdir -p /var/snmp" );
#endif
    system2( "/bin/chmod 0777 /tmp" );
    cprintf( "%s:%d", __func__, __LINE__ );

    dns_to_resolv(  );
    cprintf( "%s:%d", __func__, __LINE__ );

    return;
}
