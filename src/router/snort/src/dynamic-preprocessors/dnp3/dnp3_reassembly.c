/*
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Dynamic preprocessor for the DNP3 protocol
 *
 */

#include <string.h>
#include <stdint.h>

#include "spp_dnp3.h"
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_packet.h"
#include "snort_bounds.h"

#include "dnp3_map.h"
#include "dnp3_reassembly.h"
#include "dnp3_roptions.h"

#ifdef DUMP_BUFFER
#include "dnp3_buffer_dump.h"
#endif

/* Minimum length of DNP3 "len" field in order to get a transport header. */
#define DNP3_MIN_TRANSPORT_LEN 6

#define DNP3_TPDU_MAX  250
#define DNP3_LPDU_MAX  292

/* CRC look-up table, for computeCRC() below */
static uint16_t crcLookUpTable[256] =
{
    0x0000, 0x365E, 0x6CBC, 0x5AE2, 0xD978, 0xEF26, 0xB5C4, 0x839A,
    0xFF89, 0xC9D7, 0x9335, 0xA56B, 0x26F1, 0x10AF, 0x4A4D, 0x7C13,
    0xB26B, 0x8435, 0xDED7, 0xE889, 0x6B13, 0x5D4D, 0x07AF, 0x31F1,
    0x4DE2, 0x7BBC, 0x215E, 0x1700, 0x949A, 0xA2C4, 0xF826, 0xCE78,
    0x29AF, 0x1FF1, 0x4513, 0x734D, 0xF0D7, 0xC689, 0x9C6B, 0xAA35,
    0xD626, 0xE078, 0xBA9A, 0x8CC4, 0x0F5E, 0x3900, 0x63E2, 0x55BC,
    0x9BC4, 0xAD9A, 0xF778, 0xC126, 0x42BC, 0x74E2, 0x2E00, 0x185E,
    0x644D, 0x5213, 0x08F1, 0x3EAF, 0xBD35, 0x8B6B, 0xD189, 0xE7D7,
    0x535E, 0x6500, 0x3FE2, 0x09BC, 0x8A26, 0xBC78, 0xE69A, 0xD0C4,
    0xACD7, 0x9A89, 0xC06B, 0xF635, 0x75AF, 0x43F1, 0x1913, 0x2F4D,
    0xE135, 0xD76B, 0x8D89, 0xBBD7, 0x384D, 0x0E13, 0x54F1, 0x62AF,
    0x1EBC, 0x28E2, 0x7200, 0x445E, 0xC7C4, 0xF19A, 0xAB78, 0x9D26,
    0x7AF1, 0x4CAF, 0x164D, 0x2013, 0xA389, 0x95D7, 0xCF35, 0xF96B,
    0x8578, 0xB326, 0xE9C4, 0xDF9A, 0x5C00, 0x6A5E, 0x30BC, 0x06E2,
    0xC89A, 0xFEC4, 0xA426, 0x9278, 0x11E2, 0x27BC, 0x7D5E, 0x4B00,
    0x3713, 0x014D, 0x5BAF, 0x6DF1, 0xEE6B, 0xD835, 0x82D7, 0xB489,
    0xA6BC, 0x90E2, 0xCA00, 0xFC5E, 0x7FC4, 0x499A, 0x1378, 0x2526,
    0x5935, 0x6F6B, 0x3589, 0x03D7, 0x804D, 0xB613, 0xECF1, 0xDAAF,
    0x14D7, 0x2289, 0x786B, 0x4E35, 0xCDAF, 0xFBF1, 0xA113, 0x974D,
    0xEB5E, 0xDD00, 0x87E2, 0xB1BC, 0x3226, 0x0478, 0x5E9A, 0x68C4,
    0x8F13, 0xB94D, 0xE3AF, 0xD5F1, 0x566B, 0x6035, 0x3AD7, 0x0C89,
    0x709A, 0x46C4, 0x1C26, 0x2A78, 0xA9E2, 0x9FBC, 0xC55E, 0xF300,
    0x3D78, 0x0B26, 0x51C4, 0x679A, 0xE400, 0xD25E, 0x88BC, 0xBEE2,
    0xC2F1, 0xF4AF, 0xAE4D, 0x9813, 0x1B89, 0x2DD7, 0x7735, 0x416B,
    0xF5E2, 0xC3BC, 0x995E, 0xAF00, 0x2C9A, 0x1AC4, 0x4026, 0x7678,
    0x0A6B, 0x3C35, 0x66D7, 0x5089, 0xD313, 0xE54D, 0xBFAF, 0x89F1,
    0x4789, 0x71D7, 0x2B35, 0x1D6B, 0x9EF1, 0xA8AF, 0xF24D, 0xC413,
    0xB800, 0x8E5E, 0xD4BC, 0xE2E2, 0x6178, 0x5726, 0x0DC4, 0x3B9A,
    0xDC4D, 0xEA13, 0xB0F1, 0x86AF, 0x0535, 0x336B, 0x6989, 0x5FD7,
    0x23C4, 0x159A, 0x4F78, 0x7926, 0xFABC, 0xCCE2, 0x9600, 0xA05E,
    0x6E26, 0x5878, 0x029A, 0x34C4, 0xB75E, 0x8100, 0xDBE2, 0xEDBC,
    0x91AF, 0xA7F1, 0xFD13, 0xCB4D, 0x48D7, 0x7E89, 0x246B, 0x1235
};

/* Append a DNP3 Transport segment to the reassembly buffer.
   
   Returns:
    DNP3_OK:    Segment queued successfully.
    DNP3_FAIL:  Data copy failed. Segment did not fit in reassembly buffer.
*/
static int DNP3QueueSegment(dnp3_reassembly_data_t *rdata, char *buf, uint16_t buflen)
{
    if (rdata == NULL || buf == NULL)
        return DNP3_FAIL;

    /* At first I was afraid, but we checked for DNP3_MAX_TRANSPORT_LEN earlier. */
    if (buflen + rdata->buflen > DNP3_BUFFER_SIZE)
        return DNP3_FAIL;

    memcpy((rdata->buffer + rdata->buflen), buf, (size_t) buflen);

    rdata->buflen += buflen;
    return DNP3_OK;
}

/* Reset a DNP3 reassembly buffer */
static void DNP3ReassemblyReset(dnp3_reassembly_data_t *rdata)
{
    rdata->buflen = 0;
    rdata->state = DNP3_REASSEMBLY_STATE__IDLE;
    rdata->last_seq = 0;
}

/* DNP3 Transport-Layer reassembly state machine.

   Arguments:
     rdata:     DNP3 reassembly state object.
     buf:       DNP3 Transport Layer segment
     buflen:    Length of Transport Layer segment.

   Returns:
    DNP3_FAIL:     Segment was discarded.
    DNP3_OK:       Segment was queued.
*/
static int DNP3ReassembleTransport(dnp3_reassembly_data_t *rdata, char *buf, uint16_t buflen)
{
    dnp3_transport_header_t *trans_header;

    if (rdata == NULL || buf == NULL || buflen < sizeof(dnp3_transport_header_t) ||
        (buflen > DNP3_MAX_TRANSPORT_LEN))
    {
        return DNP3_FAIL;
    }

    /* Take the first byte as a transport header, cut it off of the buffer. */
    trans_header = (dnp3_transport_header_t *)buf;
    buf += sizeof(dnp3_transport_header_t);
    buflen -= sizeof(dnp3_transport_header_t);


    /* If the previously-existing state was DONE, we need to reset it back
       to IDLE. */
    if (rdata->state == DNP3_REASSEMBLY_STATE__DONE)
        DNP3ReassemblyReset(rdata);

    switch (rdata->state)
    {
        case DNP3_REASSEMBLY_STATE__IDLE:
            /* Discard any non-first segment. */
            if ( DNP3_TRANSPORT_FIR(trans_header->control) == 0 )
                return DNP3_FAIL;

            /* Reset the buffer & queue the first segment */
            DNP3ReassemblyReset(rdata);
            DNP3QueueSegment(rdata, buf, buflen);
            rdata->last_seq = DNP3_TRANSPORT_SEQ(trans_header->control);

            if ( DNP3_TRANSPORT_FIN(trans_header->control) )
                rdata->state = DNP3_REASSEMBLY_STATE__DONE;
            else
                rdata->state = DNP3_REASSEMBLY_STATE__ASSEMBLY;

            break;

        case DNP3_REASSEMBLY_STATE__ASSEMBLY:
            /* Reset if the FIR flag is set. */
            if ( DNP3_TRANSPORT_FIR(trans_header->control) )
            {
                DNP3ReassemblyReset(rdata);
                DNP3QueueSegment(rdata, buf, buflen);
                rdata->last_seq = DNP3_TRANSPORT_SEQ(trans_header->control);
               if (DNP3_TRANSPORT_FIN(trans_header->control))
                    rdata->state = DNP3_REASSEMBLY_STATE__DONE;

                /* Raise an alert so it's clear the buffer was reset.
                   Could signify device trouble. */
                _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_REASSEMBLY_BUFFER_CLEARED,
                                1, 0, 3, DNP3_REASSEMBLY_BUFFER_CLEARED_STR, 0);
            }
            else
            {
                /* Same seq but FIN is set. Discard segment, BUT finish reassembly. */
                if ((DNP3_TRANSPORT_SEQ(trans_header->control) == rdata->last_seq) &&
                    (DNP3_TRANSPORT_FIN(trans_header->control)))
                {
                    _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_DROPPED_SEGMENT,
                                    1, 0, 3, DNP3_DROPPED_SEGMENT_STR, 0);
                    rdata->state = DNP3_REASSEMBLY_STATE__DONE;
                    return DNP3_FAIL;
                }

                /* Discard any other segments without the correct sequence. */
                if (DNP3_TRANSPORT_SEQ(trans_header->control) !=
                    ((rdata->last_seq + 1) % 0x40 ))
                {
                    _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_DROPPED_SEGMENT,
                                    1, 0, 3, DNP3_DROPPED_SEGMENT_STR, 0);
                    return DNP3_FAIL;
                }

                /* Otherwise, queue it up! */
                DNP3QueueSegment(rdata, buf, buflen);
                rdata->last_seq = DNP3_TRANSPORT_SEQ(trans_header->control);

                if (DNP3_TRANSPORT_FIN(trans_header->control))
                    rdata->state = DNP3_REASSEMBLY_STATE__DONE;
                else
                    rdata->state = DNP3_REASSEMBLY_STATE__ASSEMBLY;
            }
            break;

        case DNP3_REASSEMBLY_STATE__DONE:
            break;
    }

    /* Set the Alt Decode buffer. This must be done during preprocessing
       in order to stop the Fast Pattern matcher from using raw packet data
       to evaluate the longest content in a rule. */
    if (rdata->state == DNP3_REASSEMBLY_STATE__DONE)
    {
        uint8_t *alt_buf = _dpd.altBuffer->data;
        uint16_t alt_len = sizeof(_dpd.altBuffer->data);
        int ret;

        ret = SafeMemcpy((void *)alt_buf,
                         (const void *)rdata->buffer,
                         (size_t)rdata->buflen,
                         (const void *)alt_buf,
                         (const void *)(alt_buf + alt_len));

        if (ret == SAFEMEM_SUCCESS)
            _dpd.SetAltDecode(alt_len);

    }

    return DNP3_OK;
}

/* Check for reserved application-level function codes. */
static void DNP3CheckReservedFunction(dnp3_session_data_t *session)
{
    if ( !(DNP3FuncIsDefined( (uint16_t)session->func)) )
    {
       _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_RESERVED_FUNCTION,
                        1, 0, 3, DNP3_RESERVED_FUNCTION_STR, 0);
    }
}

/* Decode a DNP3 Application-layer Fragment, fill out the relevant session data
   for rule option evaluation. */
static int DNP3ProcessApplication(dnp3_session_data_t *session)
{
    dnp3_reassembly_data_t *rdata = NULL;

    if (session == NULL)
        return DNP3_FAIL;

    /* Master and Outstation use slightly different Application-layer headers.
       Only the outstation sends Internal Indications. */
    if (session->direction == DNP3_CLIENT)
    {
        dnp3_app_request_header_t *request = NULL;
        rdata = &(session->client_rdata);

        if (rdata->buflen < sizeof(dnp3_app_request_header_t))
            return DNP3_FAIL; /* TODO: Preprocessor Alert */

        request = (dnp3_app_request_header_t *)(rdata->buffer);
#ifdef DUMP_BUFFER
        dumpBuffer(DNP3_CLINET_REQUEST_DUMP, (const uint8_t *) rdata->buffer,rdata->buflen);
#endif
        session->func = request->function;
    }
    else if (session->direction == DNP3_SERVER)
    {
        dnp3_app_response_header_t *response = NULL;
        rdata = &(session->server_rdata);

        if (rdata->buflen < sizeof(dnp3_app_response_header_t))
            return DNP3_FAIL; /* TODO: Preprocessor Alert */

        response = (dnp3_app_response_header_t *)(rdata->buffer);
#ifdef DUMP_BUFFER
        dumpBuffer(DNP3_SERVER_RESPONSE_DUMP, (const uint8_t *) rdata->buffer,rdata->buflen);
#endif
        session->func = response->function;
        session->indications = ntohs(response->indications);
    }

    DNP3CheckReservedFunction(session);

    return DNP3_OK;
}

/* Check a CRC in a single block. */
/* This code is mostly lifted from the example in the DNP3 spec. */

static inline void computeCRC(unsigned char data, uint16_t *crcAccum)
{
    *crcAccum = 
        (*crcAccum >> 8) ^ crcLookUpTable[(*crcAccum ^ data) & 0xFF];
}

static int DNP3CheckCRC(unsigned char *buf, uint16_t buflen)
{
    uint16_t idx;
    uint16_t crc = 0;

    /* Compute check code for data in received block */
    for (idx = 0; idx < buflen-2; idx++)
        computeCRC(buf[idx], &crc);
    crc = ~crc; /* Invert */

    /* Check CRC at end of block */
    if (buf[idx++] == (unsigned char)crc &&
        buf[idx] == (unsigned char)(crc >> 8))
        return DNP3_OK;
    else
        return DNP3_FAIL;
}

/* Check CRCs in a Link-Layer Frame, then fill a buffer containing just the user data  */
static int DNP3CheckRemoveCRC(dnp3_config_t *config, uint8_t *pdu_start,
                              uint16_t pdu_length, char *buf, uint16_t *buflen)
{
    char *cursor;
    uint16_t bytes_left;
    uint16_t curlen = 0;

    /* Check Header CRC */
    if ((config->check_crc) &&
        (DNP3CheckCRC((unsigned char*)pdu_start, sizeof(dnp3_link_header_t)+2) == DNP3_FAIL))
    {
        _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_BAD_CRC, 1, 0, 3,
                      DNP3_BAD_CRC_STR, 0);
        return DNP3_FAIL;
    }

    cursor = (char *)pdu_start + sizeof(dnp3_link_header_t) + 2;
    bytes_left = pdu_length - sizeof(dnp3_link_header_t) - 2;

    /* Process whole 16-byte chunks (plus 2-byte CRC) */
    while ( (bytes_left > (DNP3_CHUNK_SIZE + DNP3_CRC_SIZE)) &&
        (curlen + DNP3_CHUNK_SIZE < *buflen) )
    {
        if ((config->check_crc) &&
            (DNP3CheckCRC((unsigned char*)cursor, (DNP3_CHUNK_SIZE+DNP3_CRC_SIZE)) == DNP3_FAIL))
        {
            _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_BAD_CRC, 1, 0, 3,
                          DNP3_BAD_CRC_STR, 0);
            return DNP3_FAIL;
        }

        memcpy((buf + curlen), cursor, DNP3_CHUNK_SIZE);
        curlen += DNP3_CHUNK_SIZE;
        cursor += (DNP3_CHUNK_SIZE+DNP3_CRC_SIZE);
        bytes_left -= (DNP3_CHUNK_SIZE+DNP3_CRC_SIZE);
    }
    /* Process leftover chunk, under 16 bytes */
    if ( (bytes_left > DNP3_CRC_SIZE) &&
        (curlen + bytes_left < *buflen) )
    {
        if ((config->check_crc) && (DNP3CheckCRC((unsigned char*)cursor, bytes_left) == DNP3_FAIL))
        {
            _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_BAD_CRC, 1, 0, 3,
                          DNP3_BAD_CRC_STR, 0);
            return DNP3_FAIL;
        }

        memcpy((buf + curlen), cursor, (bytes_left - DNP3_CRC_SIZE));
        curlen += (bytes_left - DNP3_CRC_SIZE);
        cursor += bytes_left;
        bytes_left = 0;
    }

    *buflen = curlen;
    return DNP3_OK;
}

static int DNP3CheckReservedAddrs(dnp3_link_header_t *link)
{
    int bad_addr = 0;

    if ((link->src >= DNP3_MIN_RESERVED_ADDR) && (link->src <= DNP3_MAX_RESERVED_ADDR))
        bad_addr = 1;

    else if ((link->dest >= DNP3_MIN_RESERVED_ADDR) && (link->dest <= DNP3_MAX_RESERVED_ADDR))
        bad_addr = 1;

    if (bad_addr)
    {
     _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_RESERVED_ADDRESS, 1, 0, 3,
                        DNP3_RESERVED_ADDRESS_STR, 0);
        return DNP3_FAIL;
    }

    return DNP3_OK;
}

/* Main DNP3 Reassembly function. Moved here to avoid circular dependency between
   spp_dnp3 and dnp3_reassembly. */
int DNP3FullReassembly(dnp3_config_t *config, dnp3_session_data_t *session, SFSnortPacket *packet, uint8_t *pdu_start, uint16_t pdu_length)
{
    char buf[DNP3_TPDU_MAX];
    uint16_t buflen = sizeof(buf);
    dnp3_link_header_t *link;
    dnp3_reassembly_data_t *rdata;

    if (pdu_length < (sizeof(dnp3_link_header_t) + sizeof(dnp3_transport_header_t) + 2))
        return DNP3_FAIL;

    if ( pdu_length > DNP3_LPDU_MAX )
        // this means PAF aborted - not DNP3
        return DNP3_FAIL;

    /* Step 1: Decode header and skip to data */
    link = (dnp3_link_header_t *) pdu_start;

    if (link->len < DNP3_MIN_TRANSPORT_LEN)
    {
        _dpd.alertAdd(GENERATOR_SPP_DNP3, DNP3_DROPPED_FRAME, 1, 0, 3,
                        DNP3_DROPPED_FRAME_STR, 0);
        return DNP3_FAIL;
    }

    /* Check reserved addresses */
    if ( DNP3CheckReservedAddrs(link) == DNP3_FAIL )
    {
#ifdef DUMP_BUFFER
        dumpBuffer(DNP3_RESERVED_BAD_ADDR_DUMP,packet->payload,packet->payload_size);
#endif
        return DNP3_FAIL;
    }

    /* XXX: NEED TO TRACK SEPARATE DNP3 SESSIONS OVER SINGLE TCP SESSION */

    /* Step 2: Remove CRCs */
    if ( DNP3CheckRemoveCRC(config, pdu_start, pdu_length, buf, &buflen) == DNP3_FAIL )
    {
#ifdef DUMP_BUFFER
        dumpBuffer(DNP3_BAD_CRC_DUMP,packet->payload,packet->payload_size);
#endif
        return DNP3_FAIL;
    }

    /* Step 3: Queue user data in frame for Transport-Layer reassembly */
    if (session->direction == DNP3_CLIENT)
        rdata = &(session->client_rdata);
    else
        rdata = &(session->server_rdata);

    if (DNP3ReassembleTransport(rdata, buf, buflen) == DNP3_FAIL)
        return DNP3_FAIL;

    /* Step 4: Decode Application-Layer  */
    if (rdata->state == DNP3_REASSEMBLY_STATE__DONE)
    {
        int ret = DNP3ProcessApplication(session);

        /* To support multiple PDUs in UDP, we're going to call Detect()
           on each individual PDU. The AltDecode buffer was set earlier. */
        if ((ret == DNP3_OK) && (packet->udp_header))
            _dpd.detect(packet);
        else
            return ret;
    }

    return DNP3_OK;
}

