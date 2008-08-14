/*
 *  A simple SSL client demonstration program
 *
 *  Copyright (C) 2006  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdio.h>

#include "xyssl/net.h"
#include "xyssl/ssl.h"
#include "xyssl/havege.h"

/*
#define SERVER_NAME "localhost"
#define SERVER_PORT 4433
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"
*/
#define SERVER_NAME "xyssl.org"
#define SERVER_PORT 443
#define GET_REQUEST \
    "GET /hello/ HTTP/1.0\r\n" \
    "Host: xyssl.org\r\n\r\n"

int main( void )
{
    int ret, len;
    int server_fd;
    unsigned char buf[1024];
    havege_state hs;
    ssl_context ssl;

    /*
     * 1. Initiate the connection
     */
    printf( "\n  . Connecting to tcp/%s/%4d...", SERVER_NAME,
                                                 SERVER_PORT );
    fflush( stdout );

/* resume_test: */

    if( ( ret = net_connect( &server_fd, SERVER_NAME,
                                         SERVER_PORT ) ) != 0 )
    {
        printf( " failed\n  ! net_connect returned %08x\n\n", ret );
        return( 1 );
    }

    printf( " ok\n" );

    /*
     * 2. Setup stuff
     */
    printf( "  . Setting up the RNG and SSL state..." );
    fflush( stdout );

    havege_init( &hs );

    if( ( ret = ssl_init( &ssl, 1 ) ) != 0 )
    {
        printf( " failed\n  ! ssl_init returned %08x\n\n", ret );
        return( 1 );
    }

    printf( " ok\n" );

    ssl_set_endpoint( &ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &ssl, SSL_VERIFY_NONE );

    ssl_set_rng_func( &ssl, havege_rand, &hs );
    ssl_set_io_files( &ssl, server_fd, server_fd );
    ssl_set_ciphlist( &ssl, ssl_default_ciphers );

    /*
     * 3. Write the GET request
     */
    printf( "  > Write to server:" );

    len = sprintf( (char *) buf, GET_REQUEST );

    if( ( ret = ssl_write( &ssl, buf, len ) ) != 0 )
    {
        printf( " failed\n  ! ssl_write returned %08x\n\n", ret );
        goto exit;
    }

    printf( "\n\n%s", buf );

    /*
     * 7. Read the HTTP response
     */
    printf( "  < Read from server:\n\n" );

    do
    {
        len = sizeof( buf ) - 1;
        ret = ssl_read( &ssl, buf, &len );

        if( ret == ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret != 0 )
        {
            printf( "  ! ssl_read returned %08x\n\n", ret );
            break;
        }

        buf[len] = '\0';
        printf( "%s", buf );
    }
    while( 1 );

    ssl_close_notify( &ssl );

exit:

    net_close( server_fd );
    ssl_free( &ssl );
    /* goto resume_test */

    memset( &ssl, 0, sizeof( ssl ) );

#ifdef WIN32
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
