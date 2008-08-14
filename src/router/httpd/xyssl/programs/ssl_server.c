/*
 *  SSL server demonstration program
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

#ifdef WIN32
#include <windows.h>
#endif

#include <string.h>
#include <stdio.h>

#include "xyssl/havege.h"
#include "xyssl/certs.h"
#include "xyssl/x509.h"
#include "xyssl/ssl.h"
#include "xyssl/net.h"

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "<h2><p><center>Successful connection using: %s\r\n"

/*
 * Computing a safe DH-1024 prime takes ages, so it's faster
 * to use a precomputed value (provided below as an example).
 */
char *dhm_P = 
    "E4004C1F94182000103D883A448B3F80" \
    "2CE4B44A83301270002C20D0321CFD00" \
    "11CCEF784C26A400F43DFB901BCA7538" \
    "F2C6B176001CF5A0FD16D2C48B1D0C1C" \
    "F6AC8E1DA6BCC3B4E1F96B0564965300" \
    "FFA1D0B601EB2800F489AA512C4B248C" \
    "01F76949A60BB7F00A40B1EAB64BDD48" \
    "E8A700D60B7F1200FA8E77B0A979DABF";

char *dhm_G = "4";

unsigned char session_table[SSL_SESSION_TBL_LEN];

/*
 * sorted by order of preference
 */
int my_preferred_ciphers[] =
{
    TLS1_EDH_RSA_AES_256_SHA,
    SSL3_EDH_RSA_DES_168_SHA,
    TLS1_RSA_AES_256_SHA,
    SSL3_RSA_DES_168_SHA,
    SSL3_RSA_RC4_128_SHA,
    SSL3_RSA_RC4_128_MD5,
    0
};

int main( void )
{
    int ret, len;
    int listen_fd;
    int client_fd;
    unsigned char buf[1024];
    havege_state hs;
    ssl_context ssl;
    x509_cert srvcert;
    rsa_context rsa;

    /*
     * 1. Load the certificates and private RSA key
     */
    printf( "\n  . Loading the server cert. and key..." );
    fflush( stdout );

    memset( &srvcert, 0, sizeof( x509_cert ) );

    ret = x509_add_certs( &srvcert, (unsigned char *) test_srv_crt,
                          strlen( test_srv_crt ) );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_add_certs returned %08x\n\n", ret );
        goto exit;
    }

    ret = x509_add_certs( &srvcert, (unsigned char *) test_ca_crt,
                          strlen( test_ca_crt ) );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_add_certs returned %08x\n\n", ret );
        goto exit;
    }

    ret = x509_parse_key( &rsa, (unsigned char *) test_srv_key,
                          strlen( test_srv_key ), NULL, 0 );
    if( ret != 0 )
    {
        printf( " failed\n  ! x509_parse_key returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 2. Setup the listening TCP socket
     */
    printf( "  . Bind on https://localhost:4433/ ..." );
    fflush( stdout );

    ret = net_bind( &listen_fd, NULL, 4433 );
    if( ret != 0 )
    {
        printf( " failed\n  ! net_bind returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 3. Wait until a client connects
     */
#ifdef WIN32
    ShellExecute( NULL, "open", "https:localhost:4433/",
                  NULL, NULL, SW_SHOWNORMAL );
#endif

    memset( &ssl, 0, sizeof( ssl ) );
    client_fd = -1;

accept:

    net_close( client_fd );
    ssl_free( &ssl );

    printf( "  . Waiting for a remote connection ..." );
    fflush( stdout );

    ret = net_accept( listen_fd, &client_fd, NULL );
    if( ret != 0 )
    {
        printf( " failed\n  ! net_accept returned %08x\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 4. Setup stuff
     */
    printf( "  . Setting up the RNG and SSL state..." );
    fflush( stdout );

    havege_init( &hs );

    if( ( ret = ssl_init( &ssl, 0 ) ) != 0 )
    {
        printf( " failed\n  ! ssl_init returned %08x\n\n", ret );
        goto accept;
    }

    printf( " ok\n" );

    ssl_set_endpoint( &ssl, SSL_IS_SERVER );
    ssl_set_authmode( &ssl, SSL_VERIFY_NONE );

    ssl_set_rng_func( &ssl, havege_rand, &hs );
    ssl_set_io_files( &ssl, client_fd, client_fd );
    ssl_set_ciphlist( &ssl, my_preferred_ciphers );

    ssl_set_ca_chain( &ssl, srvcert.next, NULL );
    ssl_set_rsa_cert( &ssl, &srvcert, &rsa );

    ssl_set_sidtable( &ssl, session_table );
    ssl_set_dhm_vals( &ssl, dhm_P, dhm_G );

    /*
     * 5. Handshake
     */
    printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    ret = ssl_server_start( &ssl );
    if( ret != 0 )
    {
        printf( " failed\n  ! ssl_server_start returned %08x\n\n", ret );
        goto accept;
    }

    printf( " ok\n" );

    /*
     * 6. Read the HTTP Request
     */
    printf( "  < Read from client:" );

    len = sizeof( buf ) - 1;
    if( ( ret = ssl_read( &ssl, buf, &len ) ) != 0 )
    {
        printf( " failed\n  ! " );
        switch( ret )
        {
            case ERR_SSL_PEER_CLOSE_NOTIFY:
                printf( "Peer closed the connection\n\n" );
                break;

            case ERR_NET_CONN_RESET:
                printf( "Connection was reset by peer\n\n" );
                break;

            default:
                printf( "ssl_read returned %08x\n\n", ret );
                break;
        }

        goto accept;
    }

    buf[len] = '\0';
    printf( "\n\n%s", buf );

    /*
     * 7. Write the 200 Response
     */
    printf( "  > Write to client:" );

    len = sprintf( (char *) buf, HTTP_RESPONSE,
                   ssl_get_cipher_name( &ssl ) );

    if( ( ret = ssl_write( &ssl, buf, len ) ) != 0 )
    {
        printf( " failed\n  ! ssl_write returned %08x\n\n", ret );
        goto accept;
    }

    printf( "\n\n%s\n", buf );
    ssl_close_notify( &ssl );
    goto accept;

exit:

    net_close( client_fd );

    x509_free_cert( &srvcert );
    rsa_free( &rsa );
    ssl_free( &ssl );

    memset( &ssl, 0, sizeof( ssl ) );

#ifdef WIN32
    printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
