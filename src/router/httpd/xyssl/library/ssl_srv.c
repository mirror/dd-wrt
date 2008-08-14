/*
 *  SSLv3/TLSv1 server-side functions
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
#include <stdio.h>
#include <time.h>

#include "net.h"
#include "ssl.h"
#include "rsa.h"
#include "dhm.h"
#include "md5.h"
#include "sha1.h"

static int ssl_get_session( ssl_context *ssl )
{
    time_t t;
    int offset;
    unsigned char buf[128];
    unsigned char hash[20];

    if( ssl->sidtable == NULL || ssl->sidlen != 32 )
        return( ERR_SSL_NO_SESSION_FOUND );

    offset = ( (int) ssl->sessid[0] << 8 )
           | ( (int) ssl->sessid[1]      );
    offset = ( offset % SSL_SESSION_TBL_LEN ) & ~127;

    memcpy( buf, ssl->sidtable + offset, 128 );

    /*
     * Accept the session if:
     *
     *   - the session ID matches
     *   - it has not expired yet
     *   - the checksum is correct
     */
    if( memcmp( buf, ssl->sessid, 32 ) != 0 )
        return( ERR_SSL_NO_SESSION_FOUND );

    t = ( (time_t) buf[80] << 24 )
      | ( (time_t) buf[81] << 16 )
      | ( (time_t) buf[82] <<  8 )
      | ( (time_t) buf[83]       );

    if( time( NULL ) - t > SSL_EXPIRATION_TIME )
        return( ERR_SSL_NO_SESSION_FOUND );

    sha1_csum(  buf,  86, hash );
    if( memcmp( buf + 86, hash, 20 ) != 0 )
        return( ERR_SSL_NO_SESSION_FOUND );

    memcpy(  ssl->master, buf + 32, 48 );
    ssl->cipher = ( (int) buf[84] << 8 )
                | ( (int) buf[85]      );

    return( 0 );
}

static void ssl_set_session( ssl_context *ssl )
{
    time_t t;
    int offset;
    unsigned char buf[128];

    if( ssl->sidtable == NULL || ssl->sidlen != 32 )
        return;

    /*
     *   0  .  31   session ID
     *  32  .  79   master key
     *  80  .  83   start time
     *  84  .  85   chosen cipher
     *  86  . 105   SHA1 checksum
     */
    t = time( NULL );

    memcpy( buf     , ssl->sessid, 32 );
    memcpy( buf + 32, ssl->master, 48 );
    
    buf[80] = (unsigned char)( t >> 24 );
    buf[81] = (unsigned char)( t >> 16 );
    buf[82] = (unsigned char)( t >>  8 );
    buf[83] = (unsigned char)( t       );

    buf[84] = (unsigned char)( ssl->cipher >> 8 );
    buf[85] = (unsigned char)( ssl->cipher      );

    sha1_csum( buf, 86, buf + 86 );

    offset = ( (int) ssl->sessid[0] << 8 )
           | ( (int) ssl->sessid[1]      );
    offset = ( offset % SSL_SESSION_TBL_LEN ) & ~127;

    memcpy( ssl->sidtable + offset, buf, 128 );
}

static int ssl_parse_client_hello( ssl_context *ssl )
{
    int ret, i, j, n;
    int ciph_len, sess_len;
    int chal_len, comp_len;
    unsigned char *buf, *p;

    buf = ssl->in_hdr;

    if( ssl->in_left < 5 )
    {
        /*
         * Handle partial reads, for non-blocking sockets
         */
        n = 5 - ssl->in_left;
        ret = net_recv( ssl->read_fd, buf + ssl->in_left, &n );
        ssl->in_left += n;

        if( ret != 0 )
            return( ret );
    }

    if( ( buf[0] & 0x80 ) != 0 )
    {
        /*
         * SSLv2 Client Hello
         *
         * Record layer:
         *     0  .   1   message length
         *
         * SSL layer:
         *     2  .   2   message type
         *     3  .   4   protocol version
         */
        if( buf[0] != 0x80 || buf[1] < 17 ||
            buf[2] != SSL_HS_CLIENT_HELLO ||
            buf[3] != SSLV3_MAJOR_VERSION )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        memcpy( ssl->max_ver, buf + 3, 2 );

        ssl->major_ver = SSLV3_MAJOR_VERSION;
        ssl->minor_ver = ( buf[4] <= TLS10_MINOR_VERSION )
                         ? buf[4]  : TLS10_MINOR_VERSION;

        n = (int) buf[1] - 3;

        if( ssl->in_left < 5 + n )
        {
            n -= ssl->in_left - 5;
            ret = net_recv( ssl->read_fd, buf + ssl->in_left, &n );
            ssl->in_left += n;

            if( ret != 0 )
                return( ret );
        }

        md5_starts( &ssl->hs_md5 );
        md5_update( &ssl->hs_md5, buf + 2, (int) buf[1] );

        sha1_starts( &ssl->hs_sha1 );
        sha1_update( &ssl->hs_sha1, buf + 2, (int) buf[1] );

        buf = ssl->in_msg;
        n = ssl->in_left - 5;

        /*
         *    0  .   1   cipherlist length
         *    2  .   3   session id length
         *    4  .   5   challenge length
         *    6  .  ..   cipherlist
         *   ..  .  ..   session id
         *   ..  .  ..   challenge
         */
        ciph_len = ( (int) buf[0] << 8 ) | (int) buf[1];
        sess_len = ( (int) buf[2] << 8 ) | (int) buf[3];
        chal_len = ( (int) buf[4] << 8 ) | (int) buf[5];

        if(   ciph_len < 3 || ciph_len > 192 ||
            ( ciph_len % 3 ) != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        if( sess_len < 0 || sess_len > 32 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        if( chal_len < 8 || chal_len > 32 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        if( n != 6 + ciph_len + sess_len + chal_len )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        p = buf + 6 + ciph_len;
        memset( ssl->randbytes, 0, 64 );
        memcpy( ssl->randbytes + 32 - chal_len, p, chal_len );

        p += sess_len;
        ssl->sidlen = sess_len;
        memcpy( ssl->sessid, p, sess_len );

        for( i = 0; ssl->cipherlist[i] != 0; i++ )
        {
            for( j = 0, p = buf + 6; j < ciph_len;
                j += 3, p += 3 )
            {
                if( p[0] == 0 && p[1] == 0 &&
                    p[2] == ssl->cipherlist[i] )
                    goto have_cipher;
            }
        }
    }
    else
    {
        /*
         * SSLv3 Client Hello
         *
         * Record layer:
         *     0  .   0   message type
         *     1  .   2   protocol version
         *     3  .   4   message length
         */
        if( buf[0] != SSL_MSG_HANDSHAKE   ||
            buf[1] != SSLV3_MAJOR_VERSION ||
            buf[3] != 0 || buf[4] < 45 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        n = (int) buf[4];

        if( ssl->in_left < 5 + n )
        {
            n -= ( ssl->in_left - 5 );
            ret = net_recv( ssl->read_fd, buf + ssl->in_left, &n );
            ssl->in_left += n;

            if( ret != 0 )
                return( ret );
        }

        buf = ssl->in_msg;
        n = ssl->in_left - 5;

        md5_starts( &ssl->hs_md5 );
        md5_update( &ssl->hs_md5, buf, n );

        sha1_starts( &ssl->hs_sha1 );
        sha1_update( &ssl->hs_sha1, buf, n );

        /*
         * SSL layer:
         *     0  .   0   handshake type
         *     1  .   3   handshake length
         *     4  .   5   protocol version
         *     6  .   9   UNIX time()
         *    10  .  37   random bytes
         *    38  .  38   session id length
         *    39  . 38+x  session id
         *   39+x . 40+x  cipherlist length
         *   41+x .  ..   cipherlist
         *    ..  .  ..   compression alg.
         */

        /* check the protocol version */
        if( buf[0] != SSL_HS_CLIENT_HELLO ||
            buf[4] != SSLV3_MAJOR_VERSION )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        ssl->major_ver = SSLV3_MAJOR_VERSION;
        ssl->minor_ver = ( buf[5] <= TLS10_MINOR_VERSION )
                         ? buf[5]  : TLS10_MINOR_VERSION;

        memcpy( ssl->max_ver  , buf + 4,  2 );
        memcpy( ssl->randbytes, buf + 6, 32 );

        /* check the message length */
        if( buf[1] != 0 || buf[2] != 0 || (int) buf[3] + 4 != n )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        /* check the session length */
        sess_len = (int) buf[38];

        if( sess_len < 0 || sess_len > 32 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        ssl->sidlen = sess_len;
        memcpy( ssl->sessid, buf + 39, ssl->sidlen );

        /* check the cipherlist length */
        ciph_len = (int) buf[40 + sess_len];

        if(   ciph_len < 2 || ciph_len > 128 ||
            ( ciph_len % 2 ) != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        /* check the compression alg. length */
        comp_len = (int) buf[41 + sess_len + ciph_len];

        if( n != 42 + sess_len + ciph_len + comp_len )
            return( ERR_SSL_BAD_HS_CLIENT_HELLO );

        /* search for a matching cipher */
        for( i = 0; ssl->cipherlist[i] != 0; i++ )
        {
            for( j = 0, p = buf + 41 + sess_len; j < ciph_len;
                j += 2, p += 2 )
            {
                if( p[0] == 0 && p[1] == ssl->cipherlist[i] )
                    goto have_cipher;
            }
        }
    }

    return( ERR_SSL_NO_CIPHER_CHOSEN );

have_cipher:

    ssl->cipher  = ssl->cipherlist[i];
    ssl->in_left = 0;
    ssl->state++;
    return( 0 );
}

static int ssl_write_server_hello( ssl_context *ssl )
{
    int i;
    time_t t;
    unsigned char *buf, *p;

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   protocol version
     *     6  .   9   UNIX time()
     *    10  .  37   random bytes
     */
    buf = ssl->out_msg;
    p = buf + 4;

    *p++ = ssl->major_ver;
    *p++ = ssl->minor_ver;

    t = time( NULL );
    *p++ = (unsigned char)( t >> 24 );
    *p++ = (unsigned char)( t >> 16 );
    *p++ = (unsigned char)( t >>  8 );
    *p++ = (unsigned char)( t       );

    for( i = 28; i > 0; i-- )
        *p++ = ssl->rng_f( ssl->rng_d );

    memcpy( ssl->randbytes + 32, buf + 6, 32 );

    /*
     *    38  .  38   session id length
     *    39  . 38+x  session id
     *   39+x . 40+x  chosen cipher
     *   41+x . 41+x  chosen compression alg.
     */
    *p++ = ssl->sidlen = 32;

    if( ssl_get_session( ssl ) == 0 )
    {
        /*
         * Found a matching session, resume it
         */
        ssl->resumed = 1;
        ssl->state = SSL_SERVER_CHANGE_CIPHER_SPEC;
        ssl_derive_keys( ssl );
    }
    else
    {
        ssl->resumed = 0;
        ssl->state++;

        for( i = 0; i < ssl->sidlen; i++ )
            ssl->sessid[i] = ssl->rng_f( ssl->rng_d );
    }

    memcpy( p, ssl->sessid, ssl->sidlen );
    p += ssl->sidlen;

    *p++ = 0;
    *p++ = ssl->cipher;
    *p++ = SSL_COMPRESS_NULL;

    ssl->out_msglen  = p - buf;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_SERVER_HELLO;

    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_write_certificate_request( ssl_context *ssl )
{
    int n;
    unsigned char *buf, *p;
    x509_cert *crt;

    ssl->state++;

    if( ssl->authmode == SSL_VERIFY_NONE )
        return( 0 );

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   4   cert type count
     *     5  .. n-1  cert types
     *     n  .. n+1  length of all DNs
     *    n+2 .. n+3  length of DN 1
     *    n+4 .. ...  Distinguished Name #1
     *    ... .. ...  length of DN 2, etc.
     */
    buf = ssl->out_msg;
    p = buf + 4;

    *p++ = 1;
    *p++ = 1;   /* RSA sign */

    p += 2;
    crt = ssl->ca_chain;

    while( crt != NULL && crt->next != NULL )
    {
        if( p - buf > 4096 )
            break;

        n = crt->subject_raw.len;
        *p++ = ( n >> 8 );
        *p++ = ( n      );

        memcpy( p, crt->subject_raw.p, n );
        p += n;

        crt = crt->next;
    }

    ssl->out_msglen  = n = p - buf;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CERTIFICATE_REQUEST;
    ssl->out_msg[6]  = ( (n - 8) >> 8 );
    ssl->out_msg[7]  = ( (n - 8)      );

    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_write_server_key_exchange( ssl_context *ssl )
{
    int ret, n;
    unsigned char hash[36];
    md5_context md5;
    sha1_context sha1;

    if( ssl->cipher != SSL3_EDH_RSA_DES_168_SHA &&
        ssl->cipher != TLS1_EDH_RSA_AES_256_SHA )
    {
        ssl->state++;
        return( 0 );
    }

    /*
     * Ephemeral DH parameters:
     *
     * struct {
     *     opaque dh_p<1..2^16-1>;
     *     opaque dh_g<1..2^16-1>;
     *     opaque dh_Ys<1..2^16-1>;
     * } ServerDHParams;
     */
    if( ( ret = dhm_ssl_make_params(  &ssl->dhm_ctx,
                           ssl->dhm_P, ssl->dhm_G,
                           ssl->rng_f, ssl->rng_d,
                           ssl->out_msg + 4, &n ) ) != 0 )
        return( ret );

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
    md5_starts( &md5 );
    md5_update( &md5, ssl->randbytes,  64 );
    md5_update( &md5, ssl->out_msg + 4, n );
    md5_finish( &md5, hash );

    sha1_starts( &sha1 );
    sha1_update( &sha1, ssl->randbytes,  64 );
    sha1_update( &sha1, ssl->out_msg + 4, n );
    sha1_finish( &sha1, hash + 16 );

    ssl->out_msg[4 + n] = ( ssl->own_key->len >> 8 );
    ssl->out_msg[5 + n] = ( ssl->own_key->len      );

    if( ( ret = rsa_pkcs1_sign( ssl->own_key, RSA_NONE,
                                hash, 36, ssl->out_msg + 6 + n,
                                ssl->own_key->len ) ) != 0 )
        return( ret );

    ssl->out_msglen  = 6 + n + ssl->own_key->len;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_SERVER_KEY_EXCHANGE;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_write_server_hello_done( ssl_context *ssl )
{
    ssl->out_msglen  = 4;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_SERVER_HELLO_DONE;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

static int ssl_parse_client_key_exchange( ssl_context *ssl )
{
    int ret, i, n;

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );

    if( ssl->in_msg[0] != SSL_HS_CLIENT_KEY_EXCHANGE )
        return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );

    if( ssl->cipher == SSL3_EDH_RSA_DES_168_SHA ||
        ssl->cipher == TLS1_EDH_RSA_AES_256_SHA )
    {
        /*
         * Receive G^Y mod P, premaster = (G^Y)^X mod P
         */
        n = ( ssl->in_msg[4] << 8 )
           | ( ssl->in_msg[5]      );

        if( n < 1 || n > ssl->dhm_ctx.len ||
            n + 6 != ssl->in_hslen )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );

        if( ( ret = dhm_read_public( &ssl->dhm_ctx,
                                      ssl->in_msg + 6, n ) ) != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE | ret );

        ssl->pmslen = ssl->dhm_ctx.len;

        if( ( ret = dhm_calc_secret( &ssl->dhm_ctx,
                                      ssl->premaster,
                                     &ssl->pmslen ) ) != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE | ret );
    }
    else
    {
        /*
         * Decrypt the premaster using own private RSA key
         */
        i = 4;
        n = ssl->own_key->len;
        ssl->pmslen = 48;

        if( ssl->minor_ver != SSLV3_MINOR_VERSION )
        {
            i += 2;
            if( (int) ssl->in_msg[4] != ( n >> 8 ) ||
                (int) ssl->in_msg[5] != ( n      ) )
                return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );
        }

        if( ssl->in_hslen != i + n )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );

        ret = rsa_pkcs1_decrypt( ssl->own_key,
                                 ssl->in_msg + i, n,
                                 ssl->premaster,
                                &ssl->pmslen );

        if( ret != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE | ret );

        if( ssl->pmslen != 48 ||
            memcmp( ssl->premaster, ssl->max_ver, 2 ) != 0 )
            return( ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE );
    }

    ssl_derive_keys( ssl );
    ssl_set_session( ssl );

    ssl->state++;
    return( 0 );
}

static int ssl_parse_certificate_verify( ssl_context *ssl )
{
    int n1, n2, ret;
    unsigned char hash[36];

    if( ssl->peer_cert == NULL )
    {
        ssl->state++;
        return( 0 );
    }

    ssl_calc_verify( ssl, hash );

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_BAD_HS_CERTIFICATE_VERIFY );

    if( ssl->in_msg[0] != SSL_HS_CERTIFICATE_VERIFY )
        return( ERR_SSL_BAD_HS_CERTIFICATE_VERIFY );

    n1 = ssl->peer_cert->rsa.len;
    n2 = ( (int) ssl->in_msg[4] << 8 )
       | ( (int) ssl->in_msg[5]      );

    if( n1 + 6 != ssl->in_hslen || n1 != n2 )
        return( ERR_SSL_BAD_HS_CERTIFICATE_VERIFY );

    ret = rsa_pkcs1_verify( &ssl->peer_cert->rsa,
                            RSA_NONE, hash, 36,
                            ssl->in_msg + 6, n1 );

    ssl->state++;
    return( ( ssl->authmode == SSL_VERIFY_REQUIRED ) ? ret : 0 );
}

/*
 * SSL handshake -- server side
 */
int ssl_server_start( ssl_context *ssl )
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
         *  <==   ClientHello
         */
        case SSL_CLIENT_HELLO:
            ret = ssl_parse_client_hello( ssl );
            break;

        /*
         *  ==>   ServerHello
         *        Certificate
         *      ( ServerKeyExchange  )
         *      ( CertificateRequest )
         *        ServerHelloDone
         */
        case SSL_SERVER_HELLO:
            ret = ssl_write_server_hello( ssl );
            break;

        case SSL_SERVER_CERTIFICATE:
            ret = ssl_write_certificate( ssl );
            break;

        case SSL_SERVER_KEY_EXCHANGE:
            ret = ssl_write_server_key_exchange( ssl );
            break;

        case SSL_CERTIFICATE_REQUEST:
            ret = ssl_write_certificate_request( ssl );
            break;

        case SSL_SERVER_HELLO_DONE:
            ret = ssl_write_server_hello_done( ssl );
            break;

        /*
         *  <== ( Certificate/Alert  )
         *        ClientKeyExchange
         *      ( CertificateVerify  )
         *        ChangeCipherSpec
         *        Finished
         */
        case SSL_CLIENT_CERTIFICATE:
            ret = ssl_parse_certificate( ssl );
            break;

        case SSL_CLIENT_KEY_EXCHANGE:
            ret = ssl_parse_client_key_exchange( ssl );
            break;

        case SSL_CERTIFICATE_VERIFY:
            ret = ssl_parse_certificate_verify( ssl );
            break;

        case SSL_CLIENT_CHANGE_CIPHER_SPEC:
            ret = ssl_parse_change_cipher_spec( ssl );
            break;

        case SSL_CLIENT_FINISHED:
            ret = ssl_parse_finished( ssl );
            break;

        /*
         *  ==>   ChangeCipherSpec
         *        Finished
         */
        case SSL_SERVER_CHANGE_CIPHER_SPEC:
            ret = ssl_write_change_cipher_spec( ssl );
            break;

        case SSL_SERVER_FINISHED:
            ret = ssl_write_finished( ssl );
            break;

        default:
            return( 0 );
        }
    }

    return( ret );
}
