/*
 * Copyright (C) 1998-2011 Sourcefire, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
*/

/*
 * Adam Keeton
 * ssl.c
 * 10/09/07
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef WIN32
#include <arpa/inet.h>
#endif
#include "sf_snort_packet.h"
#include "ssl.h"

#define THREE_BYTE_LEN(x) (x[2] | x[1] << 8 | x[0] << 16)

static uint32_t SSL_decode_version_v3(uint8_t major, uint8_t minor) 
{
    /* Should only be called internally and by functions which have previously
     * validated their arguments */

    if(major == 3) 
    {
        /* Minor version */
        switch(minor) {
            case 0: 
                    return SSL_VER_SSLV3_FLAG;
                    break;
            case 1:
                    return SSL_VER_TLS10_FLAG;
                    break;
            case 2:
                    return SSL_VER_TLS11_FLAG;
                    break;
            case 3:
                    return SSL_VER_TLS12_FLAG;
                    break;
            default:
                return SSL_BAD_VER_FLAG;
        };
    }
    /* This is a special case. Technically, major == 2 is SSLv2. But if this
     * traffic was SSLv2, this code path would not have been exercised. */
    else if(major == 2)
    {
        return SSL_BAD_VER_FLAG;    
    }

    return SSL_BAD_VER_FLAG;
}

static uint32_t SSL_decode_handshake_v3(const uint8_t *pkt , int size, 
                                         uint32_t cur_flags, uint32_t pkt_flags) 
{
    SSL_handshake_t *handshake;
    SSL_handshake_hello_t *hello;
    uint32_t hs_len;
    uint32_t retval = 0;

    while (size > 0)
    {
        if (size < (int)SSL_HS_PAYLOAD_OFFSET)
        {
            retval |= SSL_TRUNCATED_FLAG;
            break;
        }

        /* Note, handhshake version field is optional depending on type */
        /* Will recast to different type as necessary. */
        handshake = (SSL_handshake_t *)pkt;
        pkt += SSL_HS_PAYLOAD_OFFSET;
        size -= SSL_HS_PAYLOAD_OFFSET;

        /* The code below effectively implements the following:
         *      hs_len = 0;
         *      memcpy(&hs_len, handshake->length, 3);
         *      hs_len = ntohl(hs_len); 
         * It was written this way for performance */
        hs_len = THREE_BYTE_LEN(handshake->length);

        switch(handshake->type)
        {
            case SSL_HS_CHELLO:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                else
                    retval |= SSL_CLIENT_HELLO_FLAG | SSL_CUR_CLIENT_HELLO_FLAG;

                /* This type of record contains a version string. */
                /* Make sure there is room for a version. */
                if (size < (int)sizeof(uint16_t))
                {
                    retval |= SSL_TRUNCATED_FLAG;
                    break;
                }

                hello = (SSL_handshake_hello_t *)handshake;
                retval |= SSL_decode_version_v3(hello->major, hello->minor);

                /* Compare version of record with version of handshake */
                if((cur_flags & SSL_VERFLAGS) != (retval & SSL_VERFLAGS))
                    retval |= SSL_BAD_VER_FLAG;

                break;

            case SSL_HS_SHELLO:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_SERVER_HELLO_FLAG | SSL_CUR_SERVER_HELLO_FLAG;
                else
                    retval |= SSL_BOGUS_HS_DIR_FLAG;

                /* This type of record contains a version string. */
                if (size < (int)sizeof(uint16_t))
                {
                    retval |= SSL_TRUNCATED_FLAG;
                    break;
                }

                hello = (SSL_handshake_hello_t *)handshake;
                retval |= SSL_decode_version_v3(hello->major, hello->minor);

                /* Compare version of record with version of handshake */
                if((cur_flags & SSL_VERFLAGS) != (retval & SSL_VERFLAGS))
                    retval |= SSL_BAD_VER_FLAG;

                break;

            case SSL_HS_SHELLO_DONE:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_HS_SDONE_FLAG;
                else
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                break;

            case SSL_HS_SKEYX:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_SERVER_KEYX_FLAG | SSL_CUR_SERVER_KEYX_FLAG;
                else
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                break;

            case SSL_HS_CKEYX:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                else
                    retval |= SSL_CLIENT_KEYX_FLAG | SSL_CUR_CLIENT_KEYX_FLAG;
                break;

            case SSL_HS_CERT:
                retval |= SSL_CERTIFICATE_FLAG;
                break;

                /* The following types are not presently of interest */
            case SSL_HS_HELLO_REQ: 
            case SSL_HS_CERT_VERIFY:
            case SSL_HS_CERT_REQ:
            case SSL_CERT_URL:  /* RFC 3546 */
            case SSL_CERT_STATUS: /* RFC 3546 */
                break;

                /* Will never see this since it's always encrypted */
            case SSL_HS_FINISHED: 
            default:
                /* Could be either a bad type or an encrypted handshake record */
                /* If the record is encrypted, the type will likely appear bogus. */
                return SSL_POSSIBLE_HS_FLAG | SSL_POSSIBLY_ENC_FLAG;
        }

        size -= hs_len;
        pkt += hs_len;
    }

    if (size < 0)
        retval |= SSL_TRUNCATED_FLAG;

    return retval;
}

static uint32_t SSL_decode_v3(const uint8_t *pkt, int size, uint32_t pkt_flags)
{
    SSL_record_t *record;
    uint32_t retval = 0;
    uint16_t reclen;
    int ccs = 0;   /* Set if we see a Change Cipher Spec and reset after the next record */

    while(size > 0)
    {
        if (size < (int)SSL_REC_PAYLOAD_OFFSET)
        {
            retval |= SSL_TRUNCATED_FLAG;
            break;
        }

        record = (SSL_record_t*)pkt;
        pkt += SSL_REC_PAYLOAD_OFFSET;
        size -= SSL_REC_PAYLOAD_OFFSET;

        retval |= SSL_decode_version_v3(record->major, record->minor);

        reclen = ntohs(record->length);

        switch (record->type)
        {
            case SSL_CHANGE_CIPHER_REC:
                retval |= SSL_CHANGE_CIPHER_FLAG;
                
                /* If there is another record, mark it as possibly encrypted */
                if((size - (int)reclen) > 0)
                    retval |= SSL_POSSIBLY_ENC_FLAG;

                ccs = 1;
                break;

            case SSL_ALERT_REC:
                retval |= SSL_ALERT_FLAG;
                ccs = 0;
                break;

            case SSL_HANDSHAKE_REC:
                /* If the CHANGE_CIPHER_FLAG is set, the following handshake
                 * record should be encrypted */
                if(!(retval & SSL_CHANGE_CIPHER_FLAG)) 
                {
                    int hsize = size < (int)reclen ? size : (int)reclen;
                    retval |= SSL_decode_handshake_v3(pkt, hsize, retval, pkt_flags);
                }
                else if (ccs)
                {
                    /* If we just got a change cipher spec, the next record must
                     * be a finished encrypted, which has no type, so it will fall
                     * into this default case, but it's good and we still need to
                     * see client and server app data */
                    retval |= SSL_HS_SDONE_FLAG;
                }

                ccs = 0;
                break;

            case SSL_APPLICATION_REC:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_SAPP_FLAG;
                else
                    retval |= SSL_CAPP_FLAG;
                ccs = 0;
                break;

            default:
                retval |= SSL_BAD_TYPE_FLAG;
                ccs = 0;
                break;
        };

        size -= reclen;
        pkt += reclen;
    }

    if (size < 0)
        retval |= SSL_TRUNCATED_FLAG; 

    if(!(retval & SSL_VERFLAGS) || (retval & SSL_BAD_VER_FLAG))
        return retval | SSL_UNKNOWN_FLAG;

    return retval;
}

static uint32_t SSL_decode_v2(const uint8_t *pkt, int size, uint32_t pkt_flags)
{
    uint16_t reclen;
    SSLv2_chello_t *chello;
    SSLv2_shello_t *shello;
    uint32_t retval = 0;
    SSLv2_record_t *record = (SSLv2_record_t*)pkt;

    while (size > 0)
    {
        if(size < SSL_V2_MIN_LEN)
        {
            retval |= SSL_TRUNCATED_FLAG | SSL_UNKNOWN_FLAG;
            break;
        }

        /* Note: top bit has special meaning and is not included 
         * with the length */
        reclen = ntohs(record->length) & 0x7fff;

        switch(record->type)
        {
            case SSL_V2_CHELLO:
                if(pkt_flags & FLAG_FROM_SERVER)
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                else
                    retval |= SSL_CLIENT_HELLO_FLAG | SSL_CUR_CLIENT_HELLO_FLAG ;

                if (size < (int)sizeof(SSLv2_chello_t))
                {
                    retval |= SSL_TRUNCATED_FLAG | SSL_UNKNOWN_FLAG;
                    break;
                }

                chello = (SSLv2_chello_t*)pkt;

                if(chello->major != 2)
                {
                    retval |= SSL_BAD_VER_FLAG | SSL_UNKNOWN_FLAG;
                    break;
                }

                break;

            case SSL_V2_SHELLO:
                if(pkt_flags & FLAG_FROM_CLIENT)
                    retval |= SSL_BOGUS_HS_DIR_FLAG;
                else
                    retval |= SSL_SERVER_HELLO_FLAG | SSL_CUR_SERVER_HELLO_FLAG;

                if (size < (int)sizeof(SSLv2_shello_t))
                {
                    retval |= SSL_TRUNCATED_FLAG | SSL_UNKNOWN_FLAG;
                    break;
                }

                shello = (SSLv2_shello_t*)pkt;

                if(shello->major != 2)
                {
                    retval |= SSL_BAD_VER_FLAG | SSL_UNKNOWN_FLAG;
                    break;
                }

                break;

            case SSL_V2_CKEY:
                retval |= SSL_CLIENT_KEYX_FLAG |  SSL_CUR_CLIENT_KEYX_FLAG;
                break;

            default:
                return retval | SSL_BAD_TYPE_FLAG | SSL_UNKNOWN_FLAG;
        }

        size -= (reclen + 2);
        pkt += (reclen + 2);
    }

    if (size < 0)
        retval |= SSL_TRUNCATED_FLAG;

    return retval | SSL_VER_SSLV2_FLAG;
}

uint32_t SSL_decode(const uint8_t *pkt, int size, uint32_t pkt_flags)
{
    SSL_record_t *record;
    uint16_t reclen;
    uint32_t datalen;

    if(!pkt || !size) 
        return SSL_ARG_ERROR_FLAG;

    if (size < (int)SSL_REC_PAYLOAD_OFFSET)
        return SSL_TRUNCATED_FLAG | SSL_UNKNOWN_FLAG;

    /* Determine the protocol type. */

    /* Only SSL v2 will have these bits set */
    if(pkt[0] & 0x80 || pkt[0] & 0x40)
        return SSL_decode_v2(pkt, size, pkt_flags);

    /* If this packet is only 5 bytes, it inconclusive whether its SSLv2 or TLS. 
     * If it is v2, it's definitely truncated anyway.  By decoding a 5 byte 
     * SSLv2 as TLS,the decoder will either catch a bad type, bad version, or 
     * indicate that it is truncated. */
    if(size == 5)
        return SSL_decode_v3(pkt, size, pkt_flags);

    /* At this point, 'size' has to be > 5 */

    /* If the field below contains a 2, it's either an SSLv2 client hello or 
     * it is TLS and is containing a server hello. */
    if(pkt[4] == 2)
    {
        /* This could be a TLS server hello.  Check for a TLS version string */
        if(size >= 10)
        {
            if(pkt[9] == 3)
            {
               /* Saw a TLS version, but this could also be an SSHv2 length.
                 * If it is, check if a hypothetical TLS record-data length agress 
                 * with its record length */
                datalen = THREE_BYTE_LEN( (pkt+6) );
    
                record = (SSL_record_t*)pkt;
                reclen = ntohs(record->length);
    
                /* If these lengths match, it's v3 */
                /* Otherwise, it's v2 */
                if(reclen - SSL_HS_PAYLOAD_OFFSET != datalen)
                    return SSL_decode_v2(pkt, size, pkt_flags);
            }
        }
    }
    /* Check if it's possibly a SSLv2 server-hello, in which case the version
     * is at byte 7 */
    else if(size >= 8 && pkt[7] == 2)
    {
        /* A version of '2' at byte 7 overlaps with TLS record-data length.  
         * Check if a hypothetical TLS record-data length agress with its 
         * record length */
        datalen = THREE_BYTE_LEN( (pkt+6) );

        record = (SSL_record_t*)pkt;
        reclen = ntohs(record->length);

        /* If these lengths match, it's v3 */
        /* Otherwise, it's v2 */
        if(reclen - SSL_HS_PAYLOAD_OFFSET != datalen)
            return SSL_decode_v2(pkt, size, pkt_flags);
    }

    return SSL_decode_v3(pkt, size, pkt_flags);
}

