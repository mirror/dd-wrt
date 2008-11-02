/*
 * sshd.c
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

#ifdef HAVE_SSHD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>

#define RSA_HOST_KEY_FILE	"/tmp/root/.ssh/ssh_host_rsa_key"
#define DSS_HOST_KEY_FILE	"/tmp/root/.ssh/ssh_host_dss_key"
#define TMP_HOST_KEY_FILE	"/tmp/tmp_host_key"
#define AUTHORIZED_KEYS_FILE	"/tmp/root/.ssh/authorized_keys"
#define NVRAM_RSA_KEY_NAME      "sshd_rsa_host_key"
#define NVRAM_DSS_KEY_NAME      "sshd_dss_host_key"

static void empty_dir_check( void );
static int write_key_file( char *keyname, char *keyfile, int chmodval );
static int generate_dropbear_rsa_host_key( void );
static int generate_dropbear_dss_host_key( void );

void start_sshd( void )
{
    int ret = 0;

    // pid_t pid;
    // char buf[255] = { 0 };
    cprintf( "check for ssh\n" );
    if( !nvram_invmatch( "sshd_enable", "0" ) )
	return 0;
    cprintf( "empty dir check\n" );
    empty_dir_check(  );
    cprintf( "write key file\n" );
    int changed = 0;

    if( write_key_file( NVRAM_RSA_KEY_NAME, RSA_HOST_KEY_FILE, 0600 ) == -1 )
    {
	generate_dropbear_rsa_host_key(  );
	write_key_file( NVRAM_RSA_KEY_NAME, RSA_HOST_KEY_FILE, 0600 );
	changed = 1;
    }
    cprintf( "convert key\n" );
    eval( "dropbearconvert", "openssh", "dropbear", RSA_HOST_KEY_FILE,
	  RSA_HOST_KEY_FILE );
    cprintf( "write dss key\n" );
    if( write_key_file( NVRAM_DSS_KEY_NAME, DSS_HOST_KEY_FILE, 0600 ) == -1 )
    {
	generate_dropbear_dss_host_key(  );
	write_key_file( NVRAM_DSS_KEY_NAME, DSS_HOST_KEY_FILE, 0600 );
	changed = 1;
    }
    cprintf( "convert dss key\n" );
    if( changed )
	nvram_commit(  );
    eval( "dropbearconvert", "openssh", "dropbear", DSS_HOST_KEY_FILE,
	  DSS_HOST_KEY_FILE );
    cprintf( "write authorized keys\n" );
    write_key_file( "sshd_authorized_keys", AUTHORIZED_KEYS_FILE, 0600 );
    // cprintf("start sshd %s\n",sshd_argv);
    /*
     * char *sshd_argv[] = { "dropbear", "-r", RSA_HOST_KEY_FILE, "-d",
     * DSS_HOST_KEY_FILE, "-p", port, passwd_ok, NULL }; 
     */
    stop_sshd(  );
    char *port = nvram_safe_get( "sshd_port" );
    char *passwd_ok = nvram_match( "sshd_passwd_auth", "1" ) ? "" : "-s";
    char *forwarding_ok = nvram_match( "sshd_forwarding", "1" ) ? "-a" : "";

#ifdef HAVE_MAKSAT
    ret = eval( "dropbear", "-r", RSA_HOST_KEY_FILE, "-d",
		DSS_HOST_KEY_FILE, "-p", port, passwd_ok );
#else
    ret =
	eval( "dropbear", "-b", "/tmp/loginprompt", "-r", RSA_HOST_KEY_FILE,
	      "-d", DSS_HOST_KEY_FILE, "-p", port, passwd_ok, forwarding_ok );
#endif
    dd_syslog( LOG_INFO, "dropbear : ssh daemon successfully started\n" );
    // ret = _eval (sshd_argv, NULL, 0, &pid);

    return ret;
}

void stop_sshd( void )
{
    int ret = 0;

    if( pidof( "dropbear" ) > 0 )
    {
	killall( "dropbear", SIGTERM );
	sleep( 1 );
	ret = killall( "dropbear", SIGKILL );
	dd_syslog( LOG_INFO, "dropbear : ssh daemon successfully stopped\n" );
    }

    cprintf( "done\n" );

    return ret;
}

static void empty_dir_check( void )
{
    struct stat buf;

    if( stat( "/tmp/root/.ssh", &buf ) != 0 )
	mkdir( "/tmp/root/.ssh", 0700 );

}

/**
 * Write the sshd key file making sure that it contains no LF 0x0D chars
 * and is terminated by a single CR 0x0A char
 *
 * @return zero on sucess, non-zero on error
 */
static int write_key_file( char *keyname, char *keyfile, int chmodval )
{
    FILE *fd = NULL;
    char *host_key = NULL;
    int i = 0;

    if( !keyname || !keyfile )
	return -1;

    if( !( host_key = nvram_get( keyname ) ) || '\0' == host_key[0] )
	return -1;

    /*
     * Update the named key file 
     */
    if( ( fd = fopen( keyfile, "wb" ) ) == NULL )
    {
	cprintf( "Can't open %s\n", keyfile );
	return -1;
    }
    fwritenvram( keyname, fd );
    // add CR at end
    if( host_key[i - 1] != 0x0A )
	fprintf( fd, "%c", 0x0A );

    fclose( fd );

    // set perms
    chmod( keyfile, chmodval );

    return 0;
}

static int generate_dropbear_rsa_host_key( void )
{
    FILE *fp = ( void * )0;
    char buf[4096] = "";
    int ret = -1;

    eval( "dropbearkey", "-t", "rsa", "-f", RSA_HOST_KEY_FILE );

    eval( "dropbearconvert", "dropbear", "openssh", RSA_HOST_KEY_FILE,
	  TMP_HOST_KEY_FILE );

    fp = fopen( TMP_HOST_KEY_FILE, "r" );

    if( fp == ( void * )0 )
	return -1;

    ret = fread( buf, 1, 4095, fp );

    if( ret <= 0 )
	return -1;

    nvram_set( NVRAM_RSA_KEY_NAME, buf );

    fclose( fp );

    unlink( TMP_HOST_KEY_FILE );

    return ret;
}

static int generate_dropbear_dss_host_key( void )
{
    FILE *fp = ( void * )0;
    char buf[4096] = "";
    int ret = -1;

    eval( "dropbearkey", "-t", "dss", "-f", DSS_HOST_KEY_FILE );

    eval( "dropbearconvert", "dropbear", "openssh", DSS_HOST_KEY_FILE,
	  TMP_HOST_KEY_FILE );

    fp = fopen( TMP_HOST_KEY_FILE, "r" );

    if( fp == ( void * )0 )
	return -1;

    ret = fread( buf, 1, 4095, fp );

    if( ret <= 0 )
	return -1;

    nvram_set( NVRAM_DSS_KEY_NAME, buf );

    fclose( fp );

    unlink( TMP_HOST_KEY_FILE );

    return ret;
}

#endif /* HAVE_SSHD */
