/*
 *  A more complex SSL client program
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
#include "xyssl/certs.h"
#include "xyssl/x509.h"

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
    x509_cert cacert;
    x509_cert clicert;
    rsa_context rsa;

    /*
     * 1.1. Load the trusted CA
     */
    printf( "\n  . Loading the CA root certificate ..." );
    fflush( stdout );

    /*
     * Alternatively, you may load the CA certificate from a pem or
     * crt file by calling x509_read_crtfile( &cacert, "myca.crt" ).
     */
    memset( &cacert, 0, sizeof( x509_cert ) );

    ret = x509_add_certs( &cacert, (unsigned char *) xyssl_ca_crt,
                          strlen( xyssl_ca_crt ) );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_add_certs returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 1.2. Load own certificate and private key
     *
     * (can be skipped if client authentication is not required)
     */
    printf( "  . Loading the client cert. and key..." );
    fflush( stdout );

    memset( &clicert, 0, sizeof( x509_cert ) );

    ret = x509_add_certs( &clicert, (unsigned char *) test_cli_crt,
                          strlen( test_cli_crt ) );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_add_certs returned %08x\n\n", ret );
        goto exit;
    }

    ret = x509_parse_key( &rsa, (unsigned char *) test_cli_key,
                          strlen( test_cli_key ), NULL, 0 );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_parse_key returned %08x\n\n", ret );
        return( ret );
    }

    printf( " ok\n" );

    /*
     * 2. Initiate the connection
     */
    printf( "  . Connecting to tcp/%s/%-4d...", SERVER_NAME,
                                                SERVER_PORT );
    fflush( stdout );

    if( ( ret = net_connect( &server_fd, SERVER_NAME,
                                         SERVER_PORT ) ) != 0 )
    {
        printf( " failed\n  ! net_connect returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 3. Setup stuff
     */
    printf( "  . Setting up the RNG and SSL state..." );
    fflush( stdout );

    havege_init( &hs );

    if( ( ret = ssl_init( &ssl, 1 ) ) != 0 )
    {
        printf( " failed\n  ! ssl_init returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    ssl_set_endpoint( &ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &ssl, SSL_VERIFY_OPTIONAL );

    ssl_set_rng_func( &ssl, havege_rand, &hs );
    ssl_set_ciphlist( &ssl, ssl_default_ciphers );
    ssl_set_io_files( &ssl, server_fd, server_fd );

    ssl_set_ca_chain( &ssl, &cacert, SERVER_NAME );
    ssl_set_rsa_cert( &ssl, &clicert, &rsa );

    /*
     * 4. Handshake
     */
    printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    ret = ssl_handshake( &ssl );
    if( ret != 0 )
    {
        printf( " failed\n  ! ssl_handshake returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n    [ Cipher is %s ]\n",
            ssl_get_cipher_name( &ssl ) );

    /*
     * 5. Verify the server certificate
     */
    printf( "  . Verifying peer X.509 certificate..." );

    if( ( ret = ssl_get_verify_result( &ssl ) ) != 0 )
    {
        printf( " failed\n" );

        if( ( ret & BADCERT_HAS_EXPIRED ) != 0 )
            printf( "  ! server certificate has expired\n" );

        if( ( ret & BADCERT_CN_MISMATCH ) != 0 )
            printf( "  ! CN mismatch (expected CN=%s)\n", SERVER_NAME );

        if( ( ret & BADCERT_NOT_TRUSTED ) != 0 )
            printf( "  ! self-signed or not signed by a trusted CA\n" );

        printf( "\n" );
    }
    else
        printf( " ok\n" );

    /*
     * 6. Write the GET request
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
            /* peer terminated the session */
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
    rsa_free( &rsa );
    x509_free_cert( &clicert );
    x509_free_cert( &cacert );

    memset( &ssl, 0, sizeof( ssl ) );

#ifdef WIN32
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
