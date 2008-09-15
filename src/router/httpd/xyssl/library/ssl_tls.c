/*
 *  SSLv3/TLSv1 shared functions
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
 *
 *  2008-5-15 Cybetan modify 
 *             https restore config fails
 */
/*
 *  The SSL 3.0 specification was drafted by Netscape in 1996,
 *  and became an IETF standard in 1999.
 *
 *  http://wp.netscape.com/eng/ssl3/
 *  http://www.ietf.org/rfc/rfc2246.txt
 *  http://www.ietf.org/rfc/rfc4346.txt
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "x509.h"
#include "net.h"
#include "ssl.h"
#include "md5.h"
#include "sha1.h"
#include "arc4.h"
#include "des.h"
#include "aes.h"

/*
 * Key material generation
 */
static void tls1_prf( unsigned char *secret, int slen, char *label,
                      unsigned char *random, int rlen,
                      unsigned char *dstbuf, int dlen )
{
    int nb, hs;
    int i, j, k;
    unsigned char *S1, *S2;
    unsigned char tmp[128];
    unsigned char h_i[20];

    if( sizeof( tmp ) < 20 + strlen( label ) + rlen )
        return;

    hs = ( slen + 1 ) / 2;
    S1 = secret;
    S2 = secret + slen - hs;

    nb = strlen( label );
    memcpy( tmp + 20, label, nb );
    memcpy( tmp + 20 + nb, random, rlen );
    nb += rlen;

    /*
     * First compute P_md5(secret,label+random)[0..dlen]
     */
    md5_hmac( S1, hs, tmp + 20, nb, 4 + tmp );

    for( i = 0; i < dlen; i += 16 )
    {
        md5_hmac( S1, hs, 4 + tmp, 16 + nb, h_i );
        md5_hmac( S1, hs, 4 + tmp, 16,  4 + tmp );

        k = ( i + 16 > dlen ) ? dlen % 16 : 16;

        for( j = 0; j < k; j++ )
            dstbuf[i + j]  = h_i[j];
    }

    /*
     * XOR out with P_sha1(secret,label+random)[0..dlen]
     */
    sha1_hmac( S2, hs, tmp + 20, nb, tmp );

    for( i = 0; i < dlen; i += 20 )
    {
        sha1_hmac( S2, hs, tmp, 20 + nb, h_i );
        sha1_hmac( S2, hs, tmp, 20,      tmp );

        k = ( i + 20 > dlen ) ? dlen % 20 : 20;

        for( j = 0; j < k; j++ )
            dstbuf[i + j] ^= h_i[j];
    }
}

int ssl_derive_keys( ssl_context *ssl )
{
    int i;
    md5_context md5;
    sha1_context sha1;
    unsigned char padding[16];
    unsigned char sha1sum[20];
    unsigned char keyblk[256];
    unsigned char *key1, *key2;
    void *ctx1, *ctx2;

    /*
     * SSLv3:
     *   master =
     *     MD5( premaster + SHA1( 'A'   + premaster + randbytes ) ) +
     *     MD5( premaster + SHA1( 'BB'  + premaster + randbytes ) ) +
     *     MD5( premaster + SHA1( 'CCC' + premaster + randbytes ) )
     *
     * TLSv1:
     *   master = PRF( premaster, "master secret", randbytes )[0..47]
     */
    if( ssl->resumed == 0 )
    {
        int len = ssl->pmslen;

        if( ssl->minor_ver == SSLV3_MINOR_VERSION )
        {
            for( i = 0; i < 3; i++ )
            {
                memset( padding, 'A' + i, i + 1 );

                sha1_starts( &sha1 );
                sha1_update( &sha1, padding, i + 1 );
                sha1_update( &sha1, ssl->premaster, len );
                sha1_update( &sha1, ssl->randbytes,  64 );
                sha1_finish( &sha1, sha1sum );

                md5_starts( &md5 );
                md5_update( &md5, ssl->premaster, len );
                md5_update( &md5, sha1sum, 20 );
                md5_finish( &md5, ssl->master + i * 16 );
            }
        }
        else
            tls1_prf( ssl->premaster, len, "master secret",
                      ssl->randbytes, 64, ssl->master, 48 );

        memset( ssl->premaster, 0, sizeof( ssl->premaster ) );
    }

    /*
     *  SSLv3:
     *    key block =
     *      MD5( master + SHA1( 'A'    + master + randbytes ) ) +
     *      MD5( master + SHA1( 'BB'   + master + randbytes ) ) +
     *      MD5( master + SHA1( 'CCC'  + master + randbytes ) ) +
     *      MD5( master + SHA1( 'DDDD' + master + randbytes ) ) +
     *      ...
     *
     *  TLSv1:
     *    key block = PRF( master, "key expansion", randbytes )
     */
    {
        /*
         * Swap the client and server random values.
         */
        unsigned char tmp[64];

        memcpy( tmp, ssl->randbytes, 64 );
        memcpy( ssl->randbytes, tmp + 32, 32 );
        memcpy( ssl->randbytes + 32, tmp, 32 );
        memset( tmp, 0, sizeof( tmp ) );
    }

    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
    {
        for( i = 0; i < 16; i++ )
        {
            memset( padding, 'A' + i, i + 1 );

            sha1_starts( &sha1 );
            sha1_update( &sha1, padding, i + 1 );
            sha1_update( &sha1, ssl->master, 48 );
            sha1_update( &sha1, ssl->randbytes, 64 );
            sha1_finish( &sha1, sha1sum );

            md5_starts( &md5 );
            md5_update( &md5, ssl->master, 48 );
            md5_update( &md5, sha1sum, 20 );
            md5_finish( &md5, keyblk + i * 16 );
        }

        memset( &md5,    0, sizeof( md5     ) );
        memset( &sha1,   0, sizeof( sha1    ) );
        memset( padding, 0, sizeof( padding ) );
        memset( sha1sum, 0, sizeof( sha1sum ) );
    }
    else
        tls1_prf( ssl->master, 48, "key expansion",
                  ssl->randbytes, 64, keyblk, 256 );

    memset( ssl->randbytes, 0, sizeof( ssl->randbytes ) );

    /*
     * Determine the appropriate key, IV and MAC length.
     */
    switch( ssl->cipher )
    {
        case SSL3_RSA_RC4_128_MD5:
            ssl->keylen = 16;
            ssl->ivlen  =  0;
            ssl->maclen = 16;
            ssl->minlen = 16;
            ssl->ctxlen = sizeof( arc4_context );
            break;

        case SSL3_RSA_RC4_128_SHA:
            ssl->keylen = 16;
            ssl->ivlen  =  0;
            ssl->maclen = 20;
            ssl->minlen = 20;
            ssl->ctxlen = sizeof( arc4_context );
            break;

        case SSL3_RSA_DES_168_SHA:
        case SSL3_EDH_RSA_DES_168_SHA:
            ssl->keylen = 24;
            ssl->ivlen  =  8;
            ssl->maclen = 20;
            ssl->minlen = 24;
            ssl->ctxlen = sizeof( des3_context );
            break;

        case TLS1_RSA_AES_256_SHA:
        case TLS1_EDH_RSA_AES_256_SHA:
            ssl->keylen = 32;
            ssl->ivlen  = 16;
            ssl->maclen = 20;
            ssl->minlen = 32;
            ssl->ctxlen = sizeof( aes_context );
            break;

        default:
            return( ERR_SSL_UNKNOWN_CIPHER );
    }

    /*
     * Finally setup the cipher contexts, IVs and MAC secrets.
     */
    key1 = keyblk + ssl->maclen * 2;
    key2 = keyblk + ssl->maclen * 2 + ssl->keylen;

    if( ( ctx1 = (void *) malloc( ssl->ctxlen ) ) == NULL ||
        ( ctx2 = (void *) malloc( ssl->ctxlen ) ) == NULL )
        return( 1 );

    switch( ssl->cipher )
    {
        case SSL3_RSA_RC4_128_MD5:
        case SSL3_RSA_RC4_128_SHA:
            arc4_setup( (arc4_context *) ctx1, key1, ssl->keylen );
            arc4_setup( (arc4_context *) ctx2, key2, ssl->keylen );
            break;

        case SSL3_RSA_DES_168_SHA:
        case SSL3_EDH_RSA_DES_168_SHA:
            des3_set_3keys( (des3_context *) ctx1, key1 );
            des3_set_3keys( (des3_context *) ctx2, key2 );
            break;

        case TLS1_RSA_AES_256_SHA:
        case TLS1_EDH_RSA_AES_256_SHA:
            aes_set_key( (aes_context *) ctx1, key1, 256 );
            aes_set_key( (aes_context *) ctx2, key2, 256 );
            break;

        default:
            return( ERR_SSL_UNKNOWN_CIPHER );
    }

    if( ssl->endpoint == SSL_IS_CLIENT )
    {
        memcpy( ssl->mac_enc, keyblk,  ssl->maclen );
        memcpy( ssl->mac_dec, keyblk + ssl->maclen, ssl->maclen );

        ssl->ctx_enc = ctx1;
        ssl->ctx_dec = ctx2;

        memcpy( ssl->iv_enc, key2 + ssl->keylen,  ssl->ivlen );
        memcpy( ssl->iv_dec, key2 + ssl->keylen + ssl->ivlen,
                ssl->ivlen );
    }
    else
    {
        memcpy( ssl->mac_dec, keyblk,  ssl->maclen );
        memcpy( ssl->mac_enc, keyblk + ssl->maclen, ssl->maclen );

        ssl->ctx_dec = ctx1;
        ssl->ctx_enc = ctx2;

        memcpy( ssl->iv_dec, key2 + ssl->keylen,  ssl->ivlen );
        memcpy( ssl->iv_enc, key2 + ssl->keylen + ssl->ivlen,
                ssl->ivlen );
    }

    memset( keyblk, 0, sizeof( keyblk ) );

    return( 0 );
}

/*
 * Compute the client CertificateVerify MAC
 */
int ssl_calc_verify( ssl_context *ssl, unsigned char hash[36] )
{
    md5_context md5;
    sha1_context sha1;
    unsigned char pad_1[48];
    unsigned char pad_2[48];

    memcpy( &md5,  &ssl->hs_md5,  sizeof(  md5_context ) );
    memcpy( &sha1, &ssl->hs_sha1, sizeof( sha1_context ) );

    if( ssl->minor_ver != SSLV3_MINOR_VERSION )
    {
         md5_finish( &md5,  hash );
        sha1_finish( &sha1, hash + 16 );
        return( 0 );
    }

    memset( pad_1, 0x36, 48 );
    memset( pad_2, 0x5C, 48 );

    md5_update( &md5, ssl->master, 48 );
    md5_update( &md5, pad_1, 48 );
    md5_finish( &md5, hash );

    md5_starts( &md5 );
    md5_update( &md5, ssl->master, 48 );
    md5_update( &md5, pad_2, 48 );
    md5_update( &md5, hash,  16 );
    md5_finish( &md5, hash );
    
    sha1_update( &sha1, ssl->master, 48 );
    sha1_update( &sha1, pad_1, 40 );
    sha1_finish( &sha1, hash + 16 );

    sha1_starts( &sha1 );
    sha1_update( &sha1, ssl->master, 48 );
    sha1_update( &sha1, pad_2, 40 );
    sha1_update( &sha1, hash + 16, 20 );
    sha1_finish( &sha1, hash + 16 );

    return( 0 );
}

/*
 * SSLv3 MAC functions
 */
static void ssl_mac_md5( unsigned char *secret,
                         unsigned char *buf, int len,
                         unsigned char *ctr, int type )
{
    unsigned char header[11];
    unsigned char padding[48];
    md5_context md5;

    memcpy( header, ctr, 8 );
    header[ 8] = type;
    header[ 9] = ( len >> 8 );
    header[10] = ( len      );

    memset( padding, 0x36, 48 );
    md5_starts( &md5 );
    md5_update( &md5, secret,  16 );
    md5_update( &md5, padding, 48 );
    md5_update( &md5, header,  11 );
    md5_update( &md5, buf,  len );
    md5_finish( &md5, buf + len );

    memset( padding, 0x5C, 48 );
    md5_starts( &md5 );
    md5_update( &md5, secret,  16 );
    md5_update( &md5, padding, 48 );
    md5_update( &md5, buf + len, 16 );
    md5_finish( &md5, buf + len );
}

static void ssl_mac_sha1( unsigned char *secret,
                          unsigned char *buf, int len,
                          unsigned char *ctr, int type )
{
    unsigned char header[11];
    unsigned char padding[40];
    sha1_context sha1;

    memcpy( header, ctr, 8 );
    header[ 8] = type;
    header[ 9] = ( len >> 8 );
    header[10] = ( len      );

    memset( padding, 0x36, 40 );
    sha1_starts( &sha1 );
    sha1_update( &sha1, secret,  20 );
    sha1_update( &sha1, padding, 40 );
    sha1_update( &sha1, header, 11 );
    sha1_update( &sha1, buf,  len );
    sha1_finish( &sha1, buf + len );

    memset( padding, 0x5C, 40 );
    sha1_starts( &sha1 );
    sha1_update( &sha1, secret,  20 );
    sha1_update( &sha1, padding, 40 );
    sha1_update( &sha1, buf + len, 20 );
    sha1_finish( &sha1, buf + len );
}

/*
 * Message encryption/decryption
 */ 
static int ssl_encrypt_buf( ssl_context *ssl )
{
    int i, padlen;

    /*
     * Add MAC then encrypt
     */
    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
    {
        if( ssl->maclen == 16 )
             ssl_mac_md5( ssl->mac_enc, ssl->out_msg, ssl->out_msglen,
                          ssl->out_ctr, ssl->out_msgtype );

        if( ssl->maclen == 20 )
            ssl_mac_sha1( ssl->mac_enc, ssl->out_msg, ssl->out_msglen,
                          ssl->out_ctr, ssl->out_msgtype );
    }
    else
    {
        if( ssl->maclen == 16 )
             md5_hmac( ssl->mac_enc, 16, ssl->out_ctr,
                       ssl->out_msglen + 13,
                       ssl->out_msg + ssl->out_msglen );

        if( ssl->maclen == 20 )
            sha1_hmac( ssl->mac_enc, 20, ssl->out_ctr,
                       ssl->out_msglen + 13,
                       ssl->out_msg + ssl->out_msglen );               
    }

    ssl->out_msglen += ssl->maclen;

    for( i = 7; i >= 0; i-- )
        if( ++ssl->out_ctr[i] != 0 )
            break;

    if( ssl->ivlen == 0 )
    {
        padlen = 0;
        arc4_crypt( (arc4_context *) ssl->ctx_enc,
                    ssl->out_msg, ssl->out_msglen );
    }
    else
    {
        padlen = ssl->ivlen - ( ssl->out_msglen + 1 ) % ssl->ivlen;
        if( padlen == ssl->ivlen )
            padlen = 0;

        for( i = 0; i <= padlen; i++ )
            ssl->out_msg[ssl->out_msglen + i] = padlen;

        ssl->out_msglen += padlen + 1;

        if( ssl->ivlen ==  8 )
            des3_cbc_encrypt( (des3_context *) ssl->ctx_enc,
                              ssl->iv_enc,  ssl->out_msg,
                              ssl->out_msg, ssl->out_msglen );

        if( ssl->ivlen == 16 )
            aes_cbc_encrypt( (aes_context *) ssl->ctx_enc,
                             ssl->iv_enc,  ssl->out_msg,
                             ssl->out_msg, ssl->out_msglen );
    }

    return( 0 );
}

static int ssl_decrypt_buf( ssl_context *ssl )
{
    int i, padlen;
    unsigned char tmp[20];

    if( ssl->in_msglen < ssl->minlen )
        return( ERR_SSL_INVALID_MAC );

    if( ssl->ivlen == 0 )
    {
        padlen = 0;
        arc4_crypt( (arc4_context *) ssl->ctx_dec,
                    ssl->in_msg, ssl->in_msglen );
    }
    else
    {
        /*
         * Decrypt and check the padding
         */
        if( ssl->in_msglen % ssl->ivlen != 0 )
            return( ERR_SSL_INVALID_MAC );

        if( ssl->ivlen ==  8 )
            des3_cbc_decrypt( (des3_context *) ssl->ctx_dec,
                              ssl->iv_dec, ssl->in_msg,
                              ssl->in_msg, ssl->in_msglen );

        if( ssl->ivlen == 16 )
             aes_cbc_decrypt( (aes_context *) ssl->ctx_dec,
                              ssl->iv_dec, ssl->in_msg,
                              ssl->in_msg, ssl->in_msglen );

        padlen = 1 + ssl->in_msg[ssl->in_msglen - 1];

        if( ssl->minor_ver == SSLV3_MINOR_VERSION )
        {
            if( padlen > ssl->ivlen )
                padlen = 0;
        }
        else
        {
            for( i = 1; i <= (int) padlen; i++ )
                if( ssl->in_msg[ssl->in_msglen - i] != padlen - 1 )
                    padlen = 0;
        }
    }

    /*
     * Always compute the MAC (RFC4346, CBCTIME).
     */
    ssl->in_msglen -= ( ssl->maclen + padlen );

    ssl->in_hdr[3] = ( ssl->in_msglen >> 8 );
    ssl->in_hdr[4] = ( ssl->in_msglen      );

    memcpy( tmp, ssl->in_msg + ssl->in_msglen, 20 );

    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
    {
        if( ssl->maclen == 16 )
             ssl_mac_md5( ssl->mac_dec,
                          ssl->in_msg, ssl->in_msglen,
                          ssl->in_ctr, ssl->in_msgtype );
        else
            ssl_mac_sha1( ssl->mac_dec,
                          ssl->in_msg, ssl->in_msglen,
                          ssl->in_ctr, ssl->in_msgtype );
    }
    else
    {
        if( ssl->maclen == 16 )
             md5_hmac( ssl->mac_dec, 16,
                       ssl->in_ctr,  ssl->in_msglen + 13,
                       ssl->in_msg + ssl->in_msglen );
        else
            sha1_hmac( ssl->mac_dec, 20,
                       ssl->in_ctr,  ssl->in_msglen + 13,
                       ssl->in_msg + ssl->in_msglen );
    }

    if( memcmp( tmp, ssl->in_msg + ssl->in_msglen,
                     ssl->maclen ) != 0 )
        return( ERR_SSL_INVALID_MAC );

    /*
     * Finally verify the padding length; bad padding
     * will produce the same error as an invalid MAC.
     */
    if( ssl->ivlen != 0 && padlen == 0 )
        return( ERR_SSL_INVALID_MAC );

    if( ssl->in_msglen == 0 )
    {
        ssl->nb_zero++;

        /*
         * Three or more empty messages may be a DoS attack.
         */
        if( ssl->nb_zero > 2 )
            return( ERR_SSL_INVALID_MAC );
    }
    else
        ssl->nb_zero = 0;
            
    for( i = 7; i >= 0; i-- )
        if( ++ssl->in_ctr[i] != 0 )
            break;

    return( 0 );
}

/*
 * Record layer functions
 */
int ssl_write_record( ssl_context *ssl, int do_crypt )
{
    int ret, len = ssl->out_msglen;

    ssl->out_hdr[0] = ssl->out_msgtype;
    ssl->out_hdr[1] = ssl->major_ver;
    ssl->out_hdr[2] = ssl->minor_ver;
    ssl->out_hdr[3] = ( len >> 8 );
    ssl->out_hdr[4] = ( len      );

    if( ssl->out_msgtype == SSL_MSG_HANDSHAKE )
    {
        ssl->out_msg[1] = ( (len - 4) >> 16 );
        ssl->out_msg[2] = ( (len - 4) >>  8 );
        ssl->out_msg[3] = ( (len - 4)       );

         md5_update( &ssl->hs_md5 , ssl->out_msg, len );
        sha1_update( &ssl->hs_sha1, ssl->out_msg, len );
    }

    if( do_crypt != 0 )
    {
        if( ( ret = ssl_encrypt_buf( ssl ) ) != 0 )
            return( ret );

        len = ssl->out_msglen;
        ssl->out_hdr[3] = ( len >> 8 );
        ssl->out_hdr[4] = ( len      );
    }

    ssl->out_left = 5 + ssl->out_msglen;

    return( net_send( ssl->write_fd,
                      ssl->out_hdr,
                     &ssl->out_left ) );
}

int ssl_read_record( ssl_context *ssl, int do_crypt )
{
    int ret, len;

    if( ssl->in_msgtype == SSL_MSG_HANDSHAKE &&
        ssl->in_msglen   > ssl->in_hslen )
    {
        /*
         * Get next Handshake message in the current record
         */
        ssl->in_msglen -= ssl->in_hslen;

        memcpy( ssl->in_msg, ssl->in_msg + ssl->in_hslen,
                ssl->in_msglen );

        if( ssl->in_msglen < 4 || ssl->in_msg[1] != 0 )
            return( ERR_SSL_INVALID_RECORD );

        ssl->in_hslen = 4 + ( ( (int) ssl->in_msg[2] << 8 )
                            | ( (int) ssl->in_msg[3]      ) );

        if( ssl->in_msglen < ssl->in_hslen )
            return( ERR_SSL_INVALID_RECORD );

        return( 0 );
    }

    /*
     * Read the record header and validate it
     */
    if( ssl->in_left < 5 )
    {
        len = 5 - ssl->in_left;
        ret = net_recv( ssl->read_fd, ssl->in_hdr
                                    + ssl->in_left, &len );
        ssl->in_left += len;

        if( ret != 0 )
            return( ret );
    }

    ssl->in_msgtype = ssl->in_hdr[0];
    ssl->in_msglen  = ( (int) ssl->in_hdr[3] << 8 )
                    | ( (int) ssl->in_hdr[4]      );

    if( ssl->in_hdr[1] != ssl->major_ver )
        return( ERR_SSL_INVALID_RECORD );

    if( ssl->in_hdr[2] != SSLV3_MINOR_VERSION &&
        ssl->in_hdr[2] != TLS10_MINOR_VERSION )
        return( ERR_SSL_INVALID_RECORD );

    /*
     * Make sure the message length is acceptable
     */
    if( do_crypt == 0 )
    {
        if( ssl->in_msglen < 1 ||
            ssl->in_msglen > SSL_MAX_CONTENT_LEN )
            return( ERR_SSL_INVALID_RECORD );
    }
    else
    {
        if( ssl->in_msglen < ssl->minlen )
            return( ERR_SSL_INVALID_RECORD );

        if( ssl->minor_ver == 0 &&
            ssl->in_msglen > ssl->minlen + SSL_MAX_CONTENT_LEN )
            return( ERR_SSL_INVALID_RECORD );

        if( ssl->minor_ver != 0 &&
            ssl->in_msglen > ssl->minlen + SSL_MAX_CONTENT_LEN + 256 )
            return( ERR_SSL_INVALID_RECORD );
    }

    /*
     * Read and decrypt the message contents
     */
    len = ssl->in_msglen - ( ssl->in_left - 5 );
    ret = net_recv( ssl->read_fd, ssl->in_hdr
                                + ssl->in_left, &len );
    ssl->in_left += len;

    if( ret != 0 )
        return( ret );

    if( do_crypt != 0 )
    {
        if( ( ret = ssl_decrypt_buf( ssl ) ) != 0 )
            return( ret );

        if( ssl->in_msglen > SSL_MAX_CONTENT_LEN )
            return( ERR_SSL_INVALID_RECORD );
    }

    if( ssl->in_hdr[2] != ssl->minor_ver )
    {
        /*
         * Only in ServerHello can the minor version vary
         */
        if( ssl->in_msgtype != SSL_MSG_HANDSHAKE ||
            ssl->in_msg[0]  != SSL_HS_SERVER_HELLO )
            return( ERR_SSL_INVALID_RECORD );
    }

    if( ssl->in_msgtype == SSL_MSG_HANDSHAKE )
    {
        /*
         * Additional checks to validate the handshake header
         */
        if( len < 4 || ssl->in_msg[1] != 0 )
            return( ERR_SSL_INVALID_RECORD );

        ssl->in_hslen = 4 + ( ( (int) ssl->in_msg[2] << 8 )
                            | ( (int) ssl->in_msg[3]      ) );

        if( ssl->in_msglen < ssl->in_hslen )
            return( ERR_SSL_INVALID_RECORD );

          md5_update( &ssl->hs_md5 , ssl->in_msg, ssl->in_msglen );
         sha1_update( &ssl->hs_sha1, ssl->in_msg, ssl->in_msglen );
    }
    else
        ssl->in_hslen = ~0;

    if( ssl->in_msgtype == SSL_MSG_ALERT )
    {
        /*
         * Ignore non-fatal alerts, except close_notify
         */
        if( ssl->in_msg[0] == SSL_ALERT_FATAL )
            return( ERR_SSL_FATAL_ALERT_MESSAGE );

        if( ssl->in_msg[0] == SSL_ALERT_WARNING &&
            ssl->in_msg[1] == SSL_ALERT_CLOSE_NOTIFY )
            return( ERR_SSL_PEER_CLOSE_NOTIFY );
    }

    ssl->in_left = 0;
    return( 0 );
}

/*
 * Flush any data not yet written
 */
int ssl_flush_output( ssl_context *ssl )
{
    int ret = 0;
    unsigned char *buf;

    if( ssl->out_left > 0 )
    {
        buf = ssl->out_hdr + ssl->out_msglen -
            ( ssl->out_left - 5 );

        ret = net_send( ssl->write_fd, buf,
                       &ssl->out_left );
        if( ret <= 0 )
            return( ret );
    }

    return 0;
}

/*
 * Handshake functions
 */
int ssl_write_certificate( ssl_context *ssl )
{
    int i, n;
    x509_cert *crt;

    if( ssl->endpoint == SSL_IS_CLIENT )
    {
        if( ssl->client_auth == 0 )
        {
            ssl->state++;
            return( 0 );
        }

        /*
         * If using SSLv3 and got no cert, send an Alert message
         * (otherwise an empty Certificate message will be sent).
         */
        if( ssl->own_cert  == NULL &&
            ssl->minor_ver == SSLV3_MINOR_VERSION )
        {
            ssl->out_msglen  = 2;
            ssl->out_msgtype = SSL_MSG_ALERT;
            ssl->out_msg[0]  = SSL_ALERT_WARNING;
            ssl->out_msg[1]  = SSL_ALERT_NO_CERTIFICATE;

            ssl->state++;
            return( ssl_write_record( ssl, 0 ) );
        }
    }
    else /* SSL_IS_SERVER */
        if( ssl->own_cert == NULL )
            return( ERR_SSL_CERTIFICATE_REQUIRED );

    /*
     *     0  .  0    handshake type
     *     1  .  3    handshake length
     *     4  .  6    length of all certs
     *     7  .  9    length of cert. 1
     *    10  . n-1   peer certificate
     *     n  . n+2   length of cert. 2
     *    n+3 . ...   upper level cert, etc.
     */
    i = 7;
    crt = ssl->own_cert;
    while( crt != NULL && crt->next != NULL )
    {
        n = crt->raw.len;
        if( i + 3 + n > SSL_MAX_CONTENT_LEN )
            return( ERR_SSL_CERTIFICATE_TOO_LARGE );

        ssl->out_msg[i]     = ( n >> 16 );
        ssl->out_msg[i + 1] = ( n >>  8 );
        ssl->out_msg[i + 2] = ( n       );

        i += 3; memcpy( ssl->out_msg + i, crt->raw.p, n );
        i += n; crt = crt->next;
    }

    ssl->out_msg[4] = ( (i - 7) >> 16 );
    ssl->out_msg[5] = ( (i - 7) >>  8 );
    ssl->out_msg[6] = ( (i - 7)       );

    ssl->out_msglen  = i;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CERTIFICATE;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

int ssl_parse_certificate( ssl_context *ssl )
{
    int ret, i, n;

    if( ssl->endpoint == SSL_IS_SERVER &&
        ssl->authmode == SSL_VERIFY_NONE )
    {
        ssl->state++;
        return( 0 );
    }

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    ssl->state++;

    /*
     * Check if the client sent an empty certificate
     */
    if( ssl->endpoint  == SSL_IS_SERVER &&
        ssl->minor_ver == SSLV3_MINOR_VERSION )
    {
        if( ssl->in_msglen  == 2                    &&
            ssl->in_msgtype == SSL_MSG_ALERT        &&
            ssl->in_msg[0]  == SSL_ALERT_WARNING    &&
            ssl->in_msg[1]  == SSL_ALERT_NO_CERTIFICATE )
        {
            if( ssl->authmode == SSL_VERIFY_OPTIONAL )
                return( 0 );
            else
                return( ERR_SSL_NO_CLIENT_CERTIFICATE );
        }
    }

    if( ssl->endpoint  == SSL_IS_SERVER &&
        ssl->minor_ver != SSLV3_MINOR_VERSION )
    {
        if( ssl->in_hslen   == 7                    &&
            ssl->in_msgtype == SSL_MSG_HANDSHAKE    &&
            ssl->in_msg[0]  == SSL_HS_CERTIFICATE   &&
            memcmp( ssl->in_msg + 4, "\0\0\0", 3 ) == 0 )
        {
            if( ssl->authmode == SSL_VERIFY_OPTIONAL )
                return( 0 );
            else
                return( ERR_SSL_NO_CLIENT_CERTIFICATE );
        }
    }

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    if( ssl->in_msg[0] != SSL_HS_CERTIFICATE || ssl->in_hslen < 10 )
        return( ERR_SSL_BAD_HS_CERTIFICATE );

    /*
     * Same message structure as shown above
     */
    n = ( (int) ssl->in_msg[5] << 8 )
      | ( (int) ssl->in_msg[6]      );

    if( ssl->in_msg[4] != 0 || ssl->in_hslen != 7 + n )
        return( ERR_SSL_BAD_HS_CERTIFICATE );

    if( ( ssl->peer_cert = (x509_cert *) malloc(
                    sizeof( x509_cert ) ) ) == NULL )
        return( 1 );

    memset( ssl->peer_cert, 0, sizeof( x509_cert ) );
    i = 7;

    while( i < ssl->in_hslen )
    {
        if( ssl->in_msg[i] != 0 )
            return( ERR_SSL_BAD_HS_CERTIFICATE );

        n = ( (unsigned int) ssl->in_msg[i + 1] << 8 )
          | ( (unsigned int) ssl->in_msg[i + 2]      );
        i += 3;

        if( n < 128 || i + n > ssl->in_hslen )
            return( ERR_SSL_BAD_HS_CERTIFICATE );

        ret = x509_add_certs( ssl->peer_cert, ssl->in_msg + i, n );
        if( ret != 0 )
            return( ret );

        i += n;
    }

    if( ssl->authmode != SSL_VERIFY_NONE )
    {
        if( ssl->ca_chain == NULL )
            return( ERR_SSL_CA_CHAIN_REQUIRED );

        ret = x509_verify_cert( ssl->peer_cert, ssl->ca_chain,
                                ssl->peer_cn,  &ssl->verify_result );

        if( ssl->authmode == SSL_VERIFY_REQUIRED )
            return( ret );
    }

    return( 0 );
}

int ssl_write_change_cipher_spec( ssl_context *ssl )
{
    ssl->out_msgtype = SSL_MSG_CHANGE_CIPHER_SPEC;
    ssl->out_msg[0]  = ssl->out_msglen = 1;

    ssl->state++;
    return( ssl_write_record( ssl, 0 ) );
}

int ssl_parse_change_cipher_spec( ssl_context *ssl )
{
    int ret;

    if( ( ret = ssl_read_record( ssl, 0 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_CHANGE_CIPHER_SPEC )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    if( ssl->in_msglen != 1 || ssl->in_msg[0] != 1 )
        return( ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC );

    ssl->state++;
    return( 0 );
}

static void ssl_calc_finished(
                ssl_context *ssl, unsigned char *buf, int from,
                md5_context *md5, sha1_context *sha1 )
{
    char *sender;
    unsigned char padbuf[48];
    unsigned char md5sum[16];
    unsigned char sha1sum[20];

    /*
     * SSLv3:
     *   hash =
     *      MD5( master + pad2 +
     *          MD5( handshake + sender + master + pad1 ) )
     *   + SHA1( master + pad2 +
     *         SHA1( handshake + sender + master + pad1 ) )
     *
     * TLSv1:
     *   hash = PRF( master, finished_label,
     *               MD5( handshake ) + SHA1( handshake ) )[0..11]
     */
    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
    {
        sender = ( from == SSL_IS_CLIENT ) ? (char *) "CLNT"
                                           : (char *) "SRVR";

        memset( padbuf, 0x36, 48 );

        md5_update( md5, (unsigned char *) sender, 4 );
        md5_update( md5, ssl->master, 48 );
        md5_update( md5, padbuf, 48 );
        md5_finish( md5, md5sum );

        sha1_update( sha1, (unsigned char *) sender, 4 );
        sha1_update( sha1, ssl->master, 48 );
        sha1_update( sha1, padbuf, 40 );
        sha1_finish( sha1, sha1sum );

        memset( padbuf, 0x5C, 48 );

        md5_starts( md5 );
        md5_update( md5, ssl->master, 48 );
        md5_update( md5, padbuf, 48 );
        md5_update( md5, md5sum, 16 );
        md5_finish( md5, buf );

        sha1_starts( sha1 );
        sha1_update( sha1, ssl->master, 48 );
        sha1_update( sha1, padbuf , 40 );
        sha1_update( sha1, sha1sum, 20 );
        sha1_finish( sha1, buf + 16 );
    }
    else
    {
        sender = ( from == SSL_IS_CLIENT )
                 ? (char *) "client finished"
                 : (char *) "server finished";

         md5_finish(  md5, padbuf );
        sha1_finish( sha1, padbuf + 16 );

        tls1_prf( ssl->master, 48, sender, padbuf, 36, buf, 12 );
    }

    memset(  md5, 0, sizeof(  md5_context ) );
    memset( sha1, 0, sizeof( sha1_context ) );

    memset(  padbuf, 0, sizeof(  padbuf ) );
    memset(  md5sum, 0, sizeof(  md5sum ) );
    memset( sha1sum, 0, sizeof( sha1sum ) );
}

int ssl_write_finished( ssl_context *ssl )
{
    int hash_len = 12;
     md5_context  md5;
    sha1_context sha1;

    memcpy( &md5 , &ssl->hs_md5 , sizeof(  md5_context ) );
    memcpy( &sha1, &ssl->hs_sha1, sizeof( sha1_context ) );

    ssl_calc_finished( ssl, ssl->out_msg + 4,
                       ssl->endpoint, &md5, &sha1 );

    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
        hash_len += 24;

    ssl->out_msglen  = 4 + hash_len;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_FINISHED;

    /*
     * In case of session resuming, invert the client and server
     * ChangeCipherSpec messages order.
     */
    if( ssl->resumed != 0 )
    {
        if( ssl->endpoint == SSL_IS_CLIENT )
            ssl->state = SSL_HANDSHAKE_OVER;

        if( ssl->endpoint == SSL_IS_SERVER )
            ssl->state = SSL_CLIENT_CHANGE_CIPHER_SPEC;
    }
    else
        ssl->state++;

    return( ssl_write_record( ssl, 1 ) );
}

int ssl_parse_finished( ssl_context *ssl )
{
    int ret, hash_len = 12;
    unsigned char buf[36];
     md5_context  md5;
    sha1_context sha1;

    memcpy( &md5 , &ssl->hs_md5 , sizeof(  md5_context ) );
    memcpy( &sha1, &ssl->hs_sha1, sizeof( sha1_context ) );

    if( ( ret = ssl_read_record( ssl, 1 ) ) != 0 )
        return( ret );

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        return( ERR_SSL_UNEXPECTED_MESSAGE );

    if( ssl->minor_ver == SSLV3_MINOR_VERSION )
        hash_len += 24;

    if( ssl->in_msg[0] != SSL_HS_FINISHED ||
        ssl->in_hslen  != 4 + hash_len )
        return( ERR_SSL_BAD_HS_FINISHED );

    ssl_calc_finished( ssl, buf, ssl->endpoint ^ 1, &md5, &sha1 );

    if( memcmp( ssl->in_msg + 4, buf, hash_len ) != 0 )
        return( ERR_SSL_BAD_HS_FINISHED );

    if( ssl->resumed != 0 )
    {
        if( ssl->endpoint == SSL_IS_CLIENT )
            ssl->state = SSL_CLIENT_CHANGE_CIPHER_SPEC;

        if( ssl->endpoint == SSL_IS_SERVER )
            ssl->state = SSL_HANDSHAKE_OVER;
    }
    else
        ssl->state++;

    return( 0 );
}

/*
 * SSL context setup functions
 */
int ssl_init( ssl_context *ssl, int client_resume )
{
    int tmp_sidlen = 0;
    unsigned char tmp_sessid[32];
    unsigned char tmp_master[48];

    if( client_resume != 0 )
    {
        /*
         * Backup the session id and master secret
         */
        tmp_sidlen = ssl->sidlen;
        memcpy( tmp_sessid, ssl->sessid, 32 );
        memcpy( tmp_master, ssl->master, 48 );
    }

    memset( ssl, 0, sizeof( ssl_context ) );

    ssl->in_ctr  = (unsigned char *) malloc( SSL_BUFFER_LEN );
    ssl->in_hdr  = ssl->in_ctr +  8;
    ssl->in_msg  = ssl->in_ctr + 13;

    ssl->out_ctr = (unsigned char *) malloc( SSL_BUFFER_LEN );
    ssl->out_hdr = ssl->out_ctr +  8;
    ssl->out_msg = ssl->out_ctr + 13;

    if( ssl->in_ctr == NULL || ssl->out_ctr == NULL )
        return( 1 );

    memset( ssl-> in_ctr, 0, 8 );
    memset( ssl->out_ctr, 0, 8 );

    if( client_resume != 0 )
    {
        ssl->sidlen = tmp_sidlen;
        memcpy( ssl->sessid, tmp_sessid, 32 );
        memcpy( ssl->master, tmp_master, 48 );
    }

    return( 0 );
}

/*
 * SSL set accessors
 */
void ssl_set_endpoint( ssl_context *ssl, int endpoint )
{
    ssl->endpoint   = endpoint;
}

void ssl_set_authmode( ssl_context *ssl, int authmode )
{
    ssl->authmode   = authmode;
}

void ssl_set_rng_func( ssl_context *ssl,
                       int (*rng_f)(void *),
                       void *rng_d )
{
    ssl->rng_f      = rng_f;
    ssl->rng_d      = rng_d;
}

void ssl_set_io_files( ssl_context *ssl, int read_fd, int write_fd )
{
    ssl->read_fd    = read_fd;
    ssl->write_fd   = write_fd;
}

void ssl_set_ciphlist( ssl_context *ssl, int *ciphers )
{
    ssl->cipherlist = ciphers;
}

void ssl_set_ca_chain( ssl_context *ssl, x509_cert *ca, char *cn )
{
    ssl->ca_chain   = ca;
    ssl->peer_cn    = cn;
}

void ssl_set_rsa_cert( ssl_context *ssl, x509_cert *own_cert,
                       rsa_context *own_key )
{
    ssl->own_cert   = own_cert;
    ssl->own_key    = own_key;
}

void ssl_set_sidtable( ssl_context *ssl, unsigned char *sidtable )
{
    ssl->sidtable   = sidtable;
}

void ssl_set_dhm_vals( ssl_context *ssl, char *dhm_P, char *dhm_G )
{
    ssl->dhm_P      = dhm_P;
    ssl->dhm_G      = dhm_G;
}

/*
 * SSL get accessors
 */
int ssl_get_verify_result( ssl_context *ssl )
{
    return( ssl->verify_result );
}

char *ssl_get_cipher_name( ssl_context *ssl )
{
    switch( ssl->cipher )
    {
        case SSL3_RSA_RC4_128_MD5:
            return( "SSL3_RSA_RC4_128_MD5" );

        case SSL3_RSA_RC4_128_SHA:
            return( "SSL3_RSA_RC4_128_SHA" );

        case SSL3_RSA_DES_168_SHA:
            return( "SSL3_RSA_DES_168_SHA" );

        case SSL3_EDH_RSA_DES_168_SHA:
            return( "SSL3_EDH_RSA_DES_168_SHA" );

        case TLS1_RSA_AES_256_SHA:
            return( "TLS1_RSA_AES_256_SHA" );

        case TLS1_EDH_RSA_AES_256_SHA:
            return( "TLS1_EDH_RSA_AES_256_SHA" );

        default:
            break;
    }

    return( "UNKNOWN_CIPHER" );
}

int ssl_default_ciphers[] =
{
    TLS1_EDH_RSA_AES_256_SHA,
    SSL3_EDH_RSA_DES_168_SHA,
    TLS1_RSA_AES_256_SHA,
    SSL3_RSA_DES_168_SHA,
    SSL3_RSA_RC4_128_SHA,
    SSL3_RSA_RC4_128_MD5,
    0
};

/*
 * Perform the SSL handshake
 */
int ssl_handshake( ssl_context *ssl )
{
    return( ( ssl->endpoint == SSL_IS_CLIENT )
            ? ssl_client_start( ssl )
            : ssl_server_start( ssl ) );
}

/*
 * Receive application data decrypted from the SSL layer
 */
#if 0
int ssl_read( ssl_context *ssl, unsigned char *buf, int *len )
{
    int ret, n;

    if( ssl->state != SSL_HANDSHAKE_OVER )
    if( ( ret = ssl_handshake( ssl ) ) != 0 )
        return( ret );

    while( ssl->in_left == 0 )
    {
        if( ( ret = ssl_read_record( ssl, 1 ) ) != 0 )
            return( ret );

        if( ssl->in_msgtype != SSL_MSG_APPLICATION_DATA )
            return( ERR_SSL_UNEXPECTED_MESSAGE );

        ssl->in_left = ssl->in_msglen;
    }

    n = ( *len < ssl->in_left )
        ? *len : ssl->in_left;

    memcpy( buf, ssl->in_msg + ssl->in_msglen
                             - ssl->in_left, n );
    ssl->in_left -= n;
    *len = n;

    return( 0 );
}
#endif

int ssl_read( ssl_context *ssl, unsigned char *buf, int *len )
{
    int ret, n;
                                                                                                                             
    if( ssl->state != SSL_HANDSHAKE_OVER )
    if( ( ret = ssl_handshake( ssl ) ) != 0 )
        return( ret );
                                                                                                                             
    while( ssl->in_left == 0 )
    {
        if( ( ret = ssl_read_record( ssl, 1 ) ) != 0 )
            return( ret );
                                                                                                                             
        if( ssl->in_msgtype != SSL_MSG_APPLICATION_DATA )
            return( ERR_SSL_UNEXPECTED_MESSAGE );
                                                                                                                             
        ssl->in_left = ssl->in_msglen;
    }
                                                                                                                             
    n = ( *len < ssl->in_left )
        ? *len : ssl->in_left;
                                                                                                                             
    memcpy( buf, ssl->in_msg + ssl->in_msglen
                             - ssl->in_left, n );
    ssl->in_left -= n;
    *len = n;
                                                                                                                             
//    return( 0 );
        return( n );
}

int ssl_read_line( ssl_context *ssl, unsigned char *buf, int *len )
{
//add by michael for fix the http/https will die at 20080412
//    int ret, n, bytes;
//    char *cur_ptr;
    int ret,n,bytes=0;
    char *cur_ptr=NULL;
//end by michael
    char cur_para[1024];
    memset(cur_para, 0, sizeof(cur_para));
    memset(buf, 0, strlen(buf));
                                                                                                                             
    if( ssl->state != SSL_HANDSHAKE_OVER )
    if( ( ret = ssl_handshake( ssl ) ) != 0 )
        return( ret );
                                                                                                                             
    while( ssl->in_left == 0 )
    {
        if( ( ret = ssl_read_record( ssl, 1 ) ) != 0 )
            return( ret );
                                                                                                                             
        if( ssl->in_msgtype != SSL_MSG_APPLICATION_DATA )
            return( ERR_SSL_UNEXPECTED_MESSAGE );
                                                                                                                             
        ssl->in_left = ssl->in_msglen;
    }
    n = ( *len < ssl->in_left )
        ? *len : ssl->in_left;

    memcpy( cur_para, ssl->in_msg + ssl->in_msglen
                             - ssl->in_left, n );
    *len = n;
    cur_ptr = strstr(cur_para, "\r\n");
    if(cur_ptr)
    {
        bytes = (int)(cur_ptr - cur_para);
        memcpy( buf, cur_para, bytes + 2);
        n = bytes + 2;
    }
    else
    {
//modified by michael to fix the the httpd/https die at 20080412
//        memcpy( buf, cur_para, bytes);
	memcpy(buf,cur_para,n);
//end by michael
        bytes = n;
    }
        ssl->in_left -= n;
//modified by michael to fix when restore in https will fail at 20080514
//        return bytes;
	return n;
}

/*
 * Send application data to be encrypted by the SSL layer
 */
#if 0
int ssl_write( ssl_context *ssl, unsigned char *buf, int len )
{
    int ret, n;

    ret = ssl_handshake( ssl );

    while( ssl->out_uoff < len && ret == 0 )
    {
        n = ( ( len - ssl->out_uoff ) < SSL_MAX_CONTENT_LEN )
            ? ( len - ssl->out_uoff ) : SSL_MAX_CONTENT_LEN;

        ssl->out_uoff   += n;
        ssl->out_msglen  = n;
        ssl->out_msgtype = SSL_MSG_APPLICATION_DATA;
        memcpy( ssl->out_msg, buf, n ); buf += n;

        ssl_write_record( ssl, 1 );
    }

    if( ssl->out_uoff >= len )
        ssl->out_uoff  = 0;

    return( ret );
}
#endif
int _ssl_write( ssl_context *ssl, unsigned char *buf, int len )
{
    int ret=0, n;
//    fprintf(stderr,"_ssl_write %d\n",len);                                                                                                                             
    if( ssl->state != SSL_HANDSHAKE_OVER )
	ret = ssl_handshake( ssl );
                                                                                                                             
    while( ssl->out_uoff < len && ret == 0 )
    {
        n = ( ( len - ssl->out_uoff ) < SSL_MAX_CONTENT_LEN )
            ? ( len - ssl->out_uoff ) : SSL_MAX_CONTENT_LEN;
                                                                                                                             
        ssl->out_uoff   += n;
        ssl->out_msglen  = n;
        ssl->out_msgtype = SSL_MSG_APPLICATION_DATA;
        memset( ssl->out_msg, 0, n);
        memcpy( ssl->out_msg, buf, n ); buf += n;
                                                                                                                             
        ssl_write_record( ssl, 1 );
    }
                                                                                                                             
    if( ssl->out_uoff >= len )
        ssl->out_uoff  = 0;
                                                                                                                             
//    return( ret );
        return( n );
}

int ssl_write( ssl_context *ssl, unsigned char *buf, int len )
{
    int ret, i, remain;
                                                                                                                             
    if(ssl == NULL || buf == NULL || len == 0)
        return 0;
//    fprintf(stderr, "write %d bytes\n");
    if(ssl->ssl_buflen + len <= SSL_SEND_BUF_SZ)
    {
//    fprintf(stderr, "memcpy\n");
        memcpy(ssl->ssl_buffer + ssl->ssl_buflen, buf, len);
        ssl->ssl_buflen += len;
//    fprintf(stderr, "newlen %d\n",ssl->ssl_buflen);
        ret = len;
    }
    else
    {   remain = len;
        ret = 1;
        i = 0;
        if(remain > SSL_SEND_BUF_SZ)
        {
                for(i = 0; remain >= SSL_SEND_BUF_SZ; i++)
                {
//		    fprintf(stderr, "ssl_write remain %d\n",remain);
                    ret = _ssl_write(ssl, buf + i * SSL_SEND_BUF_SZ, SSL_SEND_BUF_SZ);
//		    fprintf(stderr, "ssl_write remain2 %d\n",remain);
                    if(ret <= 0)
                    break;
                    remain -= SSL_SEND_BUF_SZ;
//		    fprintf(stderr, "ssl_write remain3 %d\n",remain);
                }
        }
        if(ret > 0)
        {
           if(ssl->ssl_buflen + remain > SSL_SEND_BUF_SZ)
           {
//		    fprintf(stderr, "ssl_write2 remain0 %d\n",remain);
                ret = _ssl_write(ssl, ssl->ssl_buffer, ssl->ssl_buflen);
//		    fprintf(stderr, "ssl_write2 remain1 %d\n",remain);
                if(ret > 0)
                ssl->ssl_buflen = 0;
           }
        if(ret > 0)
        {
//		    fprintf(stderr, "memcpy remain ret %d\n",ret);
                memcpy(ssl->ssl_buffer + ssl->ssl_buflen, buf + i * SSL_SEND_BUF_SZ, remain);
//		    fprintf(stderr, "memcpy2 %d\n",ssl->ssl_buflen);
                ssl->ssl_buflen += remain;
                ret = len;
        }
    }
  }
  return(ret);
}

/*
 * Notify the peer that the connection is being closed
 */
int ssl_close_notify( ssl_context *ssl )
{
    int ret = ssl_flush_output( ssl );

    if( ret == 0 && ssl->state == SSL_HANDSHAKE_OVER )
    {
        ssl->out_msgtype = SSL_MSG_ALERT;
        ssl->out_msglen  = 2;
        ssl->out_msg[0]  = SSL_ALERT_WARNING;
        ssl->out_msg[1]  = SSL_ALERT_CLOSE_NOTIFY;

        ssl->state++;
        ret = ssl_write_record( ssl, 1 );
    }

    return( ret );
}

/*
 * Free an SSL context
 */
void ssl_free( ssl_context *ssl )
{
    if( ssl->ctx_dec != NULL )
    {
        memset( ssl->ctx_dec, 0, ssl->ctxlen );
          free( ssl->ctx_dec );
          ssl->ctx_dec = NULL;
    }

    if( ssl->ctx_enc != NULL )
    {
        memset( ssl->ctx_enc, 0, ssl->ctxlen );
          free( ssl->ctx_enc );
          ssl->ctx_enc = NULL;
    }

    dhm_free( &ssl->dhm_ctx );

    if( ssl->peer_cert != NULL )
    {
        x509_free_cert( ssl->peer_cert );
        free( ssl->peer_cert );
        ssl->peer_cert = NULL;
    }

    if( ssl->out_ctr != NULL )
    {
        memset( ssl->out_ctr, 0, SSL_BUFFER_LEN );
          free( ssl->out_ctr );
          ssl->out_ctr = NULL;
    }

    if( ssl->in_ctr != NULL )
    {
        memset( ssl->in_ctr, 0, SSL_BUFFER_LEN );
          free( ssl->in_ctr );
          ssl->in_ctr = NULL;
    }
}

int ssl_printf(ssl_context *ssl, const char *format, ...)
{
        va_list args;
        int ret;
        va_start(args, format);
        ret = ssl_vprintf(ssl, format, args);
        va_end(args);
        return ret;
}
                                                                                                                             
int ssl_vprintf(ssl_context *ssl, const char *format, va_list args)
{
        int ret,rc;
        char buf[1024*2];
        memset(buf, 0, sizeof(buf));
        rc = vsnprintf(buf, sizeof(buf), format, args);
        ret = ssl_write(ssl, buf, rc);
        return ret;
}

void ssl_flush( ssl_context *ssl)
{
        _ssl_write(ssl, ssl->ssl_buffer, ssl->ssl_buflen);
        ssl->ssl_buflen = 0;
}

