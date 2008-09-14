
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: upgrade.c,v 1.4 2005/11/30 11:53:42 seg Exp $
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096
#define CODE_PATTERN_ERROR 9999
static int upgrade_ret;

void
// do_upgrade_cgi(char *url, FILE *stream)
do_upgrade_cgi(struct mime_handler *handler, char *url, webs_t streamm, char *query )	// jimmy,
								// https,
								// 8/6/2003
{
#ifndef ANTI_FLASH
    fprintf( stderr, "do post\n" );
    if( upgrade_ret )
	do_ej(handler, "Fail_u_s.asp", stream, NULL );
    else
	do_ej(handler, "Success_u_s.asp", stream, NULL );
    fprintf( stderr, "websdone\n" );

    websDone( stream, 200 );
    fprintf( stderr, "reboot\n" );

    /*
     * Reboot if successful 
     */
    if( upgrade_ret == 0 )
    {
	sleep( 5 );
	sys_reboot(  );
    }
#else
    do_ej(handler, "Fail_u_s.asp", stream, NULL );
    websDone( stream, 200 );
#endif
}

int
// sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade( char *url, webs_t stream, int *total, int type )	// jimmy,
								// https,
								// 8/6/2003
{

#ifndef ANTI_FLASH
    char upload_fifo[] = "/tmp/uploadXXXXXX";
    FILE *fifo = NULL;
    char *write_argv[4];
    pid_t pid;
    char *buf = NULL;
    int count, ret = 0;
    long flags = -1;
    int size = BUFSIZ;
    int i = 0;

    {
	write_argv[0] = "write";
	write_argv[1] = upload_fifo;
	write_argv[2] = "linux";
	write_argv[3] = NULL;
    }

    // diag_led(DIAG, START_LED); // blink the diag led
    C_led( 1 );
#ifdef HAVE_HTTPS
    if( do_ssl )
	ACTION( "ACT_WEBS_UPGRADE" );
    else
#endif
	ACTION( "ACT_WEB_UPGRADE" );

    /*
     * Feed write from a temporary FIFO 
     */
    if( !mktemp( upload_fifo ) || !( fifo = fopen( upload_fifo, "w" ) ) )
    {
	if( !ret )
	    ret = errno;
	goto err;
    }

    /*
     * Set nonblock on the socket so we can timeout 
     */

    /*
     ** The buffer must be at least as big as what the stream file is
     ** using so that it can read all the data that has been buffered
     ** in the stream file. Otherwise it would be out of sync with fn
     ** select specially at the end of the data stream in which case
     ** the select tells there is no more data available but there in
     ** fact is data buffered in the stream file's buffer. Since no
     ** one has changed the default stream file's buffer size, let's
     ** use the constant BUFSIZ until someone changes it.
     **/

    if( size < MIN_BUF_SIZE )
	size = MIN_BUF_SIZE;
    if( ( buf = malloc( size ) ) == NULL )
    {
	ret = ENOMEM;
	goto err;
    }

    /*
     * Pipe the rest to the FIFO 
     */
    cprintf( "Upgrading\n" );
    // while (total && *total)
    {
	wfread( &buf[0], 1, 5, stream );
	*total -= 5;
	if( buf[0] != 'R' || buf[1] != 'B' || buf[2] != '5' || buf[3] != '0'
	    || buf[4] != '0' )
	{
	    ret = -1;
	    goto err;
	}
	int linuxsize;
	int fssize;

	wfread( &linuxsize, 1, 4, stream );
	wfread( &fssize, 1, 4, stream );
	*total -= 8;
	safe_fwrite( &linuxsize, 1, 4, fifo );
	safe_fwrite( &fssize, 1, 4, fifo );
	linuxsize += fssize;
	for( i = 0; i < linuxsize / MIN_BUF_SIZE; i++ )
	{
	    wfread( &buf[0], 1, MIN_BUF_SIZE, stream );
	    fwrite( &buf[0], 1, MIN_BUF_SIZE, fifo );
	}

	wfread( &buf[0], 1, linuxsize % MIN_BUF_SIZE, stream );
	fwrite( &buf[0], 1, linuxsize % MIN_BUF_SIZE, fifo );
	*total -= linuxsize;

    }
    fclose( fifo );
    fifo = NULL;
    fifo = fopen( upload_fifo, "rb" );
    unsigned long linuxsize;
    unsigned long fssize;

    linuxsize = 0;
    linuxsize += getc( fifo );
    linuxsize += getc( fifo ) * 256;
    linuxsize += getc( fifo ) * 256 * 256;
    linuxsize += getc( fifo ) * 256 * 256 * 256;
    fssize = 0;
    fssize += getc( fifo );
    fssize += getc( fifo ) * 256;
    fssize += getc( fifo ) * 256 * 256;
    fssize += getc( fifo ) * 256 * 256 * 256;
    fprintf( stderr, "Write Linux %d\n", linuxsize );
    FILE *out = fopen( "/dev/cf/card0/part1", "wb" );

    for( i = 0; i < linuxsize; i++ )
	putc( getc( fifo ), out );
    fclose( out );
    fprintf( stderr, "Write FileSys %d\n", fssize );
    out = fopen( "/dev/cf/card0/part2", "wb" );
    for( i = 0; i < fssize; i++ )
	putc( getc( fifo ), out );
    fclose( out );

    /*
     * Wait for write to terminate 
     */
    // waitpid (pid, &ret, 0);
    cprintf( "done\n" );
    ret = 0;
  err:
    if( buf )
	free( buf );
    if( fifo )
	fclose( fifo );
    unlink( upload_fifo );

    // diag_led(DIAG, STOP_LED);
    // C_led (0);
    fprintf( stderr, "Idle\n" );
    ACTION( "ACT_IDLE" );

    return ret;
#else
    return 0;
#endif
}

void
// do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
do_upgrade_post( char *url, webs_t stream, int len, char *boundary )	// jimmy, 
									// https, 
									// 8/6/2003
{
    killall( "udhcpc", SIGKILL );
#ifndef ANTI_FLASH
    char buf[1024];
    int type = 0;

    upgrade_ret = EINVAL;

    // Let below files loaded to memory
    // To avoid the successful screen is blank after web upgrade.
    // system2 ("cat /www/Success_u_s.asp > /dev/null");
    // system2 ("cat /www/Fail_u_s.asp > /dev/null");

    /*
     * Look for our part 
     */
    while( len > 0 )
    {
	if( !wfgets( buf, MIN( len + 1, sizeof( buf ) ), stream ) )
	    return;
	len -= strlen( buf );
	if( !strncasecmp( buf, "Content-Disposition:", 20 ) )
	{
	    if( strstr( buf, "name=\"erase\"" ) )
	    {
		while( len > 0 && strcmp( buf, "\n" )
		       && strcmp( buf, "\r\n" ) )
		{
		    if( !wfgets
			( buf, MIN( len + 1, sizeof( buf ) ), stream ) )
			return;
		    len -= strlen( buf );
		}
		if( !wfgets( buf, MIN( len + 1, sizeof( buf ) ), stream ) )
		    return;
		len -= strlen( buf );
		buf[1] = '\0';	// we only want the 1st digit
		nvram_set( "sv_restore_defaults", buf );
		nvram_commit(  );
	    }
	    else if( strstr( buf, "name=\"file\"" ) )	// upgrade image
	    {
		type = 0;
		break;
	    }
	}
    }

    /*
     * Skip boundary and headers 
     */
    while( len > 0 )
    {
	if( !wfgets( buf, MIN( len + 1, sizeof( buf ) ), stream ) )
	    return;
	len -= strlen( buf );
	if( !strcmp( buf, "\n" ) || !strcmp( buf, "\r\n" ) )
	    break;
    }
    upgrade_ret = sys_upgrade( NULL, stream, &len, type );
    fprintf( stderr, "core upgrade done() %d\n", len );
    /*
     * Restore factory original settings if told to. This will also cause a
     * restore defaults on reboot of a Sveasoft firmware. 
     */
    if( nvram_match( "sv_restore_defaults", "1" ) )
    {
	system2( "rm -f /usr/local/nvram/nvram.db" );
    }
    /*
     * Slurp anything remaining in the request 
     */
    while( len-- )
    {
#ifdef HAVE_HTTPS
	if( do_ssl )
	{
	    wfgets( buf, 1, stream );
	}
	else
	{
	    ( void )fgetc( stream );
	}
#else
	( void )fgetc( stream );
#endif

    }
#endif
    fprintf( stderr, "upgrade done()\n" );

}
