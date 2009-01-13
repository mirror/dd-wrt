/*
 *  SSL client with certificate authentication
 *
 *  Copyright (C) 2006-2007  Christophe Devine
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
#include "xyssl/certs.h"
#include "xyssl/x509.h"

#define SERVER_PORT 443

#define DEBUG_LEVEL 0

void my_debug( void *ctx, int level, char *str )
{
    if( level < DEBUG_LEVEL )
    {
	fprintf( ( FILE * ) ctx, "%s", str );
	fflush( ( FILE * ) ctx );
    }
}

int main( int argc, char *argv[] )
{
    int ret, len, server_fd;
    unsigned char buf[2];
    havege_state hs;
    ssl_context ssl;
    ssl_session ssn;
    x509_cert cacert;
    x509_cert clicert;
    rsa_context rsa;

    char *SERVER_NAME = argv[1];
    char GET_REQUEST[128];

    sprintf( GET_REQUEST, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", argv[2],
	     argv[1] );

    /*
     * 0. Initialize the RNG and the session data
     */
    havege_init( &hs );
    memset( &ssn, 0, sizeof( ssl_session ) );

    memset( &cacert, 0, sizeof( x509_cert ) );

    /*
     * Alternatively, you may load the CA certificates from a .pem or
     * .crt file by calling x509parse_crtfile( &cacert, "myca.crt" ).
     */
    ret = x509parse_crt( &cacert, ( unsigned char * )xyssl_ca_crt,
			 strlen( xyssl_ca_crt ) );
    if( ret != 0 )
    {
	printf( " failed\n  !  x509parse_crt returned %d\n\n", ret );
	goto exit;
    }

    /*
     * 1.2. Load own certificate and private key
     *
     * (can be skipped if client authentication is not required)
     */

    memset( &clicert, 0, sizeof( x509_cert ) );

    ret = x509parse_crt( &clicert, ( unsigned char * )test_cli_crt,
			 strlen( test_cli_crt ) );
    if( ret != 0 )
    {
	printf( " failed\n  !  x509parse_crt returned %d\n\n", ret );
	goto exit;
    }

    ret = x509parse_key( &rsa, ( unsigned char * )test_cli_key,
			 strlen( test_cli_key ), NULL, 0 );
    if( ret != 0 )
    {
	printf( " failed\n  !  x509parse_key returned %d\n\n", ret );
	goto exit;
    }

    if( ( ret = net_connect( &server_fd, SERVER_NAME, SERVER_PORT ) ) != 0 )
    {
	printf( " failed\n  ! net_connect returned %d\n\n", ret );
	goto exit;
    }

    havege_init( &hs );

    if( ( ret = ssl_init( &ssl ) ) != 0 )
    {
	printf( " failed\n  ! ssl_init returned %d\n\n", ret );
	goto exit;
    }

    ssl_set_endpoint( &ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &ssl, SSL_VERIFY_OPTIONAL );

    ssl_set_rng( &ssl, havege_rand, &hs );
    ssl_set_bio( &ssl, net_recv, &server_fd, net_send, &server_fd );

    ssl_set_ciphers( &ssl, ssl_default_ciphers );
    ssl_set_session( &ssl, 1, 600, &ssn );

    ssl_set_ca_chain( &ssl, &cacert, SERVER_NAME );
    ssl_set_own_cert( &ssl, &clicert, &rsa );

    ssl_set_hostname( &ssl, SERVER_NAME );

    while( ( ret = ssl_handshake( &ssl ) ) != 0 )
    {
	if( ret != XYSSL_ERR_NET_TRY_AGAIN )
	{
	    printf( " failed\n  ! ssl_handshake returned %d\n\n", ret );
	    goto exit;
	}
    }

    if( ( ret = ssl_get_verify_result( &ssl ) ) != 0 )
    {
	printf( "certificate check failed\n" );

	if( ( ret & BADCERT_EXPIRED ) != 0 )
	    printf( "  ! server certificate has expired\n" );

	if( ( ret & BADCERT_REVOKED ) != 0 )
	    printf( "  ! server certificate has been revoked\n" );

	if( ( ret & BADCERT_CN_MISMATCH ) != 0 )
	    printf( "  ! CN mismatch (expected CN=%s)\n", SERVER_NAME );

	if( ( ret & BADCERT_NOT_TRUSTED ) != 0 )
	    printf( "  ! self-signed or not signed by a trusted CA\n" );

	printf( "\n" );
    }

    len = sprintf( ( char * )buf, GET_REQUEST );

    while( ( ret = ssl_write( &ssl, buf, len ) ) <= 0 )
    {
	if( ret != XYSSL_ERR_NET_TRY_AGAIN )
	{
	    printf( " failed\n  ! ssl_write returned %d\n\n", ret );
	    goto exit;
	}
    }

    len = ret;

    /*
     * 7. Read the HTTP response
     */
    fflush( stdout );
    FILE *out = fopen( argv[3], "wb" );
    int found = 0;
    int offset = 0;

    do
    {
	offset = 0;
	len = sizeof( buf ) - 1;
	memset( buf, 0, sizeof( buf ) );
	ret = ssl_read( &ssl, buf, len );

	if( ret == XYSSL_ERR_NET_TRY_AGAIN )
	    continue;

	if( ret == XYSSL_ERR_SSL_PEER_CLOSE_NOTIFY )
	    break;

	if( ret <= 0 )
	{
	    printf( "failed\n  ! ssl_read returned %d\n\n", ret );
	    break;
	}
	if( found < 4 )
	{
	    if( found == 0 || found == 2 )
		if( buf[0] == '\r' )
		{
		    found++;
		    continue;
		}
		else
		    found = 0;
	    if( found == 1 || found == 3 )
		if( buf[0] == '\n' )
		{
		    found++;
		    continue;
		}
		else
		    found = 0;
	}
	else
	    putc( buf[0], out );
	len = ret;
    }
    while( 1 );

    ssl_close_notify( &ssl );

  exit:
    fclose( out );

    net_close( server_fd );
    x509_free( &clicert );
    x509_free( &cacert );
    rsa_free( &rsa );
    ssl_free( &ssl );

    memset( &ssl, 0, sizeof( ssl ) );

    return ( ret );
}
