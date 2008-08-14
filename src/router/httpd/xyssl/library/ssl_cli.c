/*
 *  SSLv3/TLSv1 client-side functions
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
#include <stdlib.h>
#include <time.h>

#include "ssl.h"
#include "rsa.h"
#include "dhm.h"
#include "md5.h"
#include "sha1.h"

static int ssl_write_client_hello( ssl_context *ssl )
{
    int i, n;
    time_t t;
    unsigned char *buf, *p;

     md5_starts( &ssl->hs_md5  );
    sha1_starts( &ssl->hs_sha1 );

    ssl->major_ver = SSLV3_MAJOR_VERSION;
    ssl->minor_ver = SSLV3_MINOR_VERSION;

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   highest version supported
     *     6  .   9   current UNIX time
     *    10  .  37   random bytes
     */
    buf = ssl->out_msg;
    p = buf + 4;

    *p++ = ssl->max_ver[0] = SSLV3_MAJOR_VERSION;
    *p++ = ssl->max_ver[1] = TLS10_MINOR_VERSION;

    t = time( NULL );
    *p++ = (unsigned char)( t >> 24 );
    *p++ = (unsigned char)( t >> 16 );
    *p++ = (unsigned char)( t >>  8 );
    *p++ = (unsigned char)( t       );

    for( i = 28; i > 0; i-- )
        *p++ = ssl->rng_f( ssl->rng_d );

    memcpy( ssl->randbytes, buf + 6, 32 );

    /*
     *    38   .  38    session id length
     *    39   .  39+n  session id
     *    40+n .  41+n  cipherlist length
     *    42+n .  ..    cipherlist
     *    ..   .  ..    compression alg. (0)
     */
    n = ( ssl->sidlen > 0 && ssl->sidlen <= 32 )
        ? ssl->sidlen : 0;

    *p++ = n;
    for( i = 0; i < n; i++ )
        *p++ = ssl->sessid[i];
    
    for( n = 0; ssl->cipherlist[n] != 0; n++ );
    *p++ = ( n >> 7 );
    *p++ = ( n << 1 );

    for( i = 0; i < n; i++ )
    {
        *p++ = ( ssl->cipherlist[i] >> 8 );
        *p++ = ( ssl->cipherlist[i]      );
    }

    *p++ = 1;
    *p++ = SSL_COMPRESS_NULL;

    ssl->out_msglen  = p - buf;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CLIENT_HELLO;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_parse_server_hello( ssl_context *ssl )
{
    int ret, i, n;
    unsigned char *buf;

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   protocol version
     *     6  .   9   UNIX time()
     *    10  .  37   random bytes
     */
    buf = ssl->in_msg;

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    if( ssl->in_hslen < 42 ||
        buf[0] != SSL_HS_SERVER_HELLO ||
        buf[4] != SSLV3_MAJOR_VERSION )
        return( ERR_SSL_BAD_HS_SERVER_HELLO );

    if( buf[5] != SSLV3_MINOR_VERSION &&
        buf[5] != TLS10_MINOR_VERSION )
        return( ERR_SSL_BAD_HS_SERVER_HELLO );

    ssl->minor_ver = buf[5];

    memcpy( ssl->randbytes + 32, buf + 6, 32 );

    /*
     *    38  .  38   session id length
     *    39  . 38+n  session id
     *   39+n . 40+n  chosen cipher
     *   41+n . 41+n  chosen compression alg.
     */
    n = buf[38];
    if( n < 0 || n > 32 || ssl->in_hslen != 42 + n )
        return( ERR_SSL_BAD_HS_SERVER_HELLO );

    ssl->cipher = ( (int) buf[39 + n] << 8 )
                | ( (int) buf[40 + n]      );

    if( n > 0 && memcmp( ssl->sessid, buf + 39, n ) == 0 )
    {
        /*
         * Server accepted to resume the session
         */
        ssl->state = SSL_SERVER_CHANGE_CIPHER_SPEC;
        ssl->resumed = 1;
        ssl_derive_keys( ssl );
    }
    else
    {
        ssl->state++;
        ssl->resumed = 0;
        ssl->sidlen = n;
        memcpy( ssl->sessid, buf + 39, n );
    }

    i = 0;
    while( 1 )
    {
        if( ssl->cipherlist[i] == 0 )
            return( ERR_SSL_NO_CIPHER_CHOSEN );

        if( ssl->cipherlist[i++] == ssl->cipher )
            break;
    }

    if( buf[41 + n] != SSL_COMPRESS_NULL )
        return( ERR_SSL_BAD_HS_SERVER_HELLO );

    return( 0 );
}

static int ssl_parse_server_key_exchange( ssl_context *ssl )
{
    int ret, n;
    unsigned char *p, *end;
    unsigned char hash[36];
    md5_context md5;
    sha1_context sha1;

    if( ssl->cipher != SSL3_EDH_RSA_DES_168_SHA &&
        ssl->cipher != TLS1_EDH_RSA_AES_256_SHA )
    {
        ssl->state++;
        return( 0 );
    }

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    if( ssl->in_msg[0] != SSL_HS_SERVER_KEY_EXCHANGE )
        return( ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE );

    /*
     * Ephemeral DH parameters:
     *
     * struct {
     *     opaque dh_p<1..2^16-1>;
     *     opaque dh_g<1..2^16-1>;
     *     opaque dh_Ys<1..2^16-1>;
     * } ServerDHParams;
     */
    p   = ssl->in_msg + 4;
    end = ssl->in_msg + ssl->in_hslen;

    if( ( ret = dhm_ssl_read_params( &ssl->dhm_ctx, &p, end ) ) != 0 )
        return( ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE | ret );

    if( (int)( end - p ) != ssl->peer_cert->rsa.len )
        return( ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE );

    if( ssl->dhm_ctx.len < 64 || ssl->dhm_ctx.len > 256 )
        return( ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE );

    /*
     * digitally-signed struct {
     *     opaque md5_hash[16];
     *     opaque sha_hash[20];
     * };
     *
     * md5_hash
     *     MD5(ClientHello.random + ServerHello.random
     *                            + ServerParams);
     * sha_hash
     *     SHA(ClientHello.random + ServerHello.random
     *                            + ServerParams);
     */
    n = ssl->in_hslen - ( end - p ) - 6;

    md5_starts( &md5 );
    md5_update( &md5, ssl->randbytes, 64 );
    md5_update( &md5, ssl->in_msg + 4, n );
    md5_finish( &md5, hash );

    sha1_starts( &sha1 );
    sha1_update( &sha1, ssl->randbytes, 64 );
    sha1_update( &sha1, ssl->in_msg + 4, n );
    sha1_finish( &sha1, hash + 16 );

    n = ssl->peer_cert->rsa.len;
    if( ( ret = rsa_pkcs1_verify( &ssl->peer_cert->rsa,
                            RSA_NONE, hash, 36, p, n ) ) != 0 )
        return( ret );

    ssl->state++;
    return( 0 );
}

static int ssl_parse_certificate_request( ssl_context *ssl )
{
    int ret;

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   SSL version
     *     6  .   6   cert type count
     *     7  .. n-1  cert types
     *     n  .. n+1  length of all DNs
     *    n+2 .. n+3  length of DN 1
     *    n+4 .. ...  Distinguished Name #1
     *    ... .. ...  length of DN 2, etc.
     */
    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    ssl->state++;
    ssl->client_auth = 0;

    if( ssl->in_msg[0] == SSL_HS_CERTIFICATE_REQUEST )
    {
        ssl->client_auth++;

        if( ssl->own_cert == NULL )
            return( ERR_SSL_CERTIFICATE_REQUIRED );

        if( ssl->own_key  == NULL )
            return( ERR_SSL_PRIVATE_KEY_REQUIRED );
    }

    return( 0 );
}

static int ssl_parse_server_hello_done( ssl_context *ssl )
{
    int ret;

    if( ssl->client_auth != 0 )
    {
        if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
            return( ret );

        if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
            return( ERR_SSL_UNEXPECTED_MESSAGE );
    }

    if( ssl->in_hslen  != 4 ||
        ssl->in_msg[0] != SSL_HS_SERVER_HELLO_DONE )
        return( ERR_SSL_BAD_HS_SERVER_HELLO_DONE );

    ssl->state++;
    return( 0 );
}

static int ssl_write_client_key_exchange( ssl_context *ssl )
{
    int ret, i, n;

    if( ssl->cipher == SSL3_EDH_RSA_DES_168_SHA ||
        ssl->cipher == TLS1_EDH_RSA_AES_256_SHA )
    {
        /*
         * DHM key exchange -- send G^X mod P
         */
        n = ssl->dhm_ctx.len;

        ssl->out_msg[4] = ( n >> 8 );
        ssl->out_msg[5] = ( n      );
        i = 6;

        if( ( ret = dhm_make_public( &ssl->dhm_ctx,
                                     &ssl->out_msg[i], n,
                                      ssl->rng_f,
                                      ssl->rng_d  ) ) != 0 )
            return( ret );

        ssl->pmslen = ssl->dhm_ctx.len;

        if( ( ret = dhm_calc_secret( &ssl->dhm_ctx,
                                      ssl->premaster,
                                     &ssl->pmslen ) ) != 0 )
            return( ret );
    }
    else
    {
        /*
         * RSA key exchange -- send rsa_public(premaster)
         */
        memcpy( ssl->premaster, ssl->max_ver, 2 );

        ssl->pmslen = 48;

        for( i = 2; i < ssl->pmslen; i++ )
            ssl->premaster[i] = ssl->rng_f( ssl->rng_d );

        i = 4;
        n = ssl->peer_cert->rsa.len;

        if( ssl->minor_ver != SSLV3_MINOR_VERSION )
        {
            i += 2;
            ssl->out_msg[4] = ( n >> 8 );
            ssl->out_msg[5] = ( n      );
        }

        ret = rsa_pkcs1_encrypt( &ssl->peer_cert->rsa,
                                  ssl->premaster,
                                  ssl->pmslen,
                                  ssl->out_msg + i, n );
        if( ret != 0 )
            return( ret );
    }

    ssl_derive_keys( ssl );

    ssl->out_msglen  = i + n;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CLIENT_KEY_EXCHANGE;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_write_certificate_verify( ssl_context *ssl )
{
    int ret, n;
    unsigned char hash[36];

    if( ssl->client_auth == 0 || ssl->own_key == NULL )
    {
        ssl->state++;
        return( 0 );
    }

    /*
     * Make an RSA signature of the handshake digests
     */
    ssl_calc_verify( ssl, hash );

    n = ssl->own_key->len;
    ssl->out_msg[4] = ( n >> 8 );
    ssl->out_msg[5] = ( n      );

    if( ( ret = rsa_pkcs1_sign( ssl->own_key, RSA_NONE, hash, 36,
                                ssl->out_msg + 6, n ) ) != 0 )
        return( ret );

    ssl->out_msglen  = 6 + n;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CERTIFICATE_VERIFY;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

/*
 * SSL handshake -- client side
 */
int ssl_client_start( ssl_context *ssl )
{
    int ret = ssl_flush_output( ssl );

    while( ret == 0 )
    {
        switch( ssl->state )
        {
        case SSL_HELLO_REQUEST:
            ssl->state = SSL_CLIENT_HELLO;
            break;

        /*
         *  ==>   ClientHello
         */
        case SSL_CLIENT_HELLO:
            ret = ssl_write_client_hello( ssl );
            break;

        /*
         *  <==   ServerHello
         *        Certificate
         *      ( ServerKeyExchange  )
         *      ( CertificateRequest )
         *        ServerHelloDone
         */
        case SSL_SERVER_HELLO:
            ret = ssl_parse_server_hello( ssl );
            break;

        case SSL_SERVER_CERTIFICATE:
            ret = ssl_parse_certificate( ssl );
            break;

        case SSL_SERVER_KEY_EXCHANGE:
            ret = ssl_parse_server_key_exchange( ssl );
            break;

        case SSL_CERTIFICATE_REQUEST:
            ret = ssl_parse_certificate_request( ssl );
            break;

        case SSL_SERVER_HELLO_DONE:
            ret = ssl_parse_server_hello_done( ssl );
            break;

        /*
         *  ==> ( Certificate/Alert  )
         *        ClientKeyExchange
         *      ( CertificateVerify  )
         *        ChangeCipherSpec
         *        Finished
         */
        case SSL_CLIENT_CERTIFICATE:
            ret = ssl_write_certificate( ssl );
            break;

        case SSL_CLIENT_KEY_EXCHANGE:
            ret = ssl_write_client_key_exchange( ssl );
            break;

        case SSL_CERTIFICATE_VERIFY:
            ret = ssl_write_certificate_verify( ssl );
            break;

        case SSL_CLIENT_CHANGE_CIPHER_SPEC:
            ret = ssl_write_change_cipher_spec( ssl );
            break;

        case SSL_CLIENT_FINISHED:
            ret = ssl_write_finished( ssl );
            break;

        /*
         *  <==   ChangeCipherSpec
         *        Finished
         */
        case SSL_SERVER_CHANGE_CIPHER_SPEC:
            ret = ssl_parse_change_cipher_spec( ssl );
            break;

        case SSL_SERVER_FINISHED:
            ret = ssl_parse_finished( ssl );
            break;

        default:
            return( 0 );
        }
    }

    return( ret );
}
