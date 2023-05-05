/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 7/17/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_PARSER_H
#include <ctype.h>
#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "gtp_parser.h"
#include "spp_gtp.h"
#include "gtp_config.h"

#ifdef DUMP_BUFFER
#include "gtp_buffer_dump.h"
#endif


#ifdef WIN32
#pragma pack(push,gtp_hdrs,1)
#else
#pragma pack(1)
#endif

/* GTP basic Header  */
typedef struct _GTP_C_Hdr
{
    uint8_t  flag;              /* flag: version (bit 6-8), PT (5), E (3), S (2), PN (1) */
    uint8_t  type;              /* message type */
    uint16_t length;            /* length */

} GTP_C_Hdr;

typedef struct _GTP_C_Hdr_v1_2
{
    GTP_C_Hdr hdr;
    uint32_t teid;
} GTP_C_Hdr_v1_2;

typedef struct _GTP_C_Hdr_v0
{
    GTP_C_Hdr hdr;
    uint16_t sequence_num;
    uint16_t flow_lable;
    uint64_t tid;

} GTP_C_Hdr_v0;

/* GTP Information element Header  */
typedef struct _GTP_IE_Hdr
{
    uint8_t  type;
    uint16_t length;            /* length */

} GTP_IE_Hdr;


#ifdef WIN32
#pragma pack(pop,gtp_hdrs)
#else
#pragma pack()
#endif

/* This table stores all the information elements in a packet
 * To save memory, only one table for all packets, because we inspect
 * one packet at a time
 * The information in the table might from previous packet,
 * use msg_id to find out whether the information is current.
 * */
GTP_IEData gtp_ies[MAX_GTP_IE_CODE + 1];

#define GTP_HEADER_LEN_V0       (20)
#define GTP_HEADER_LEN_V1       (12)
#define GTP_HEADER_LEN_V2       (8)
#define GTP_HEADER_LEN_EPC_V2   (12)
#define GTP_LENGTH_OFFSET_V0    (GTP_HEADER_LEN_V0)
#define GTP_LENGTH_OFFSET_V1    (8)
#define GTP_LENGTH_OFFSET_V2    (4)

#define GTP_MIN_HEADER_LEN      (8)

static int gtp_processInfoElements(GTPMsg *msg, const uint8_t *, uint16_t );

/*Because different GTP versions have different format,
 * they are processed separately*/
static int gtp_parse_v0(GTPMsg *msg, const uint8_t *,uint16_t );
static int gtp_parse_v1(GTPMsg *msg, const uint8_t *, uint16_t );
static int gtp_parse_v2(GTPMsg *msg, const uint8_t *, uint16_t );

#ifdef DEBUG_MSGS
/*Display the content*/
static void convertToHex( char *output, int outputSize, const uint8_t *input, int inputSize)
{
    int i = 0;
    int length;
    int numBytesInLine = 0;
    int totalBytes = outputSize;
    char *buf_ptr = output;

    while ((i < inputSize)&&(totalBytes > 0))
    {
        length = snprintf(buf_ptr, totalBytes, "%.2x ", (uint8_t)input[i]);
        buf_ptr += length;
        totalBytes -= length;
        if (totalBytes < 0)
            break;
        numBytesInLine += length;

        if (numBytesInLine > 80)
        {
            snprintf(buf_ptr++, totalBytes, "\n");
            totalBytes--;
            numBytesInLine = 0;
        }
        i++;
    }
    return;
}
/* Display the information elements*/
static void printInfoElements(GTP_IEData *info_elements, GTPMsg *msg)
{
    int i ;

    for (i=0; i < MAX_GTP_IE_CODE + 1; i++)
    {
        char buf[STD_BUF];
        if (info_elements[i].msg_id == msg->msg_id)
        {
            convertToHex( (char *)buf, sizeof(buf),
                    msg->gtp_header + info_elements[i].shift, info_elements[i].length);
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Info type: %.3d, content: %s\n", i, buf););
        }
    }
}
#endif


/********************************************************************
 * Function: gtp_processInfoElements()
 *
 * Process information elements
 *
 * Arguments:
 *  GTPMsg *: the GTP message
 *
 *  char *
 *      Pointer to the current position in the GTP message.
 *
 *  uint8_t *
 *      Pointer to the port array mask to set bits for the ports
 *      parsed.
 *
 * Returns:
 *  GTP_Ret
 *      GTP_SUCCESS if we were able to successfully parse the
 *          port list.
 *      GTP_FAILURE if an error occured in parsing the port list.
 *
 ********************************************************************/
static int gtp_processInfoElements(GTPMsg *msg, const uint8_t *buff, uint16_t len )
{
    uint8_t *start;
    uint8_t type;
    int32_t unprocessed_len;
    uint8_t previous_type;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Information elements: length: %d\n",
           len););

#ifdef DUMP_BUFFER
    dumpBuffer(INFO_ELEMENTS_DUMP,buff,len);
#endif

    start = (uint8_t *)buff;
    previous_type = (uint8_t) *start;
    unprocessed_len = len;

    while ( unprocessed_len > 0)
    {
        GTP_InfoElement* ie;
        uint16_t length;

        type =  *start;

        if(previous_type  >  type)
        {
            ALERT(GTP_EVENT_OUT_OF_ORDER_IE,GTP_EVENT_OUT_OF_ORDER_IE_STR);
        }

        ie = gtp_eval_config->infoElementTable[msg->version][type];

        if ( NULL == ie )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unsupported Information elements!\n"););
            gtp_stats.unknownIEs++;
            return GTP_FAILURE;
        }

        /*For fixed length, use the table*/
        if (ie->length)
        {
            length = ie->length;
        }
        else /*For variable length, use the length field*/
        {
            GTP_IE_Hdr *ieHdr;
            /*check the length before reading*/
            if (sizeof(*ieHdr) > (unsigned) unprocessed_len)
            {
                ALERT(GTP_EVENT_BAD_IE_LEN,GTP_EVENT_BAD_IE_LEN_STR);
                return GTP_FAILURE;
            }
            ieHdr = (GTP_IE_Hdr *)start;
            length = ntohs(ieHdr->length);
            /*Check the length */
            if (length > UINT16_MAX - GTP_MIN_HEADER_LEN - sizeof(*ieHdr))
            {
                ALERT(GTP_EVENT_BAD_IE_LEN,GTP_EVENT_BAD_IE_LEN_STR);
                return GTP_FAILURE;
            }

            if (msg->version == 2)
                length += 4;
            else
                length += 3;
        }

        if (length > unprocessed_len )
        {
            ALERT(GTP_EVENT_BAD_IE_LEN,GTP_EVENT_BAD_IE_LEN_STR);
            return GTP_FAILURE;

        }

        /*Combine the same information element type into one buffer*/
        if ((previous_type == type) && (msg->info_elements[type].msg_id == msg->msg_id))
        {
            msg->info_elements[type].length += length;
        }
        else
        {
            msg->info_elements[type].length = length;
            msg->info_elements[type].shift = start - msg->gtp_header;
            msg->info_elements[type].msg_id = msg->msg_id;
        }

        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "GTP information element: %s(%d), length: %d\n",
                ie->name, ie->type, length));
        start += length;
        unprocessed_len -= length;
        previous_type = type;

    }
    DEBUG_WRAP(printInfoElements(msg->info_elements, msg););
    return GTP_SUCCESS;
}

/********************************************************************
 * Function: gtp_parse_v0()
 *
 * process the GTP v0 message.
 *
 * Arguments:
 *  GTPMsg *   - gtp message
 *  char* buff - start of the gtp message buffer
 *  uint16_t   - length of the message
 *
 * Returns:
 *  GTP_FAILURE
 *  GTP_SUCCESS
 *          Bits
 *Octets  8   7   6   5   4   3   2   1
 *1       Version     PT  1   1   1   SNN
 *2       Message Type
 *3-4     Length
 *5-6     Sequence Number
 *7-8     Flow Label
 *9       SNDCP N-PDULLC Number
 *10      Spare ‘ 1 1 1 1 1 1 1 1 ‘
 *11      Spare ‘ 1 1 1 1 1 1 1 1 ‘
 *12      Spare ‘ 1 1 1 1 1 1 1 1 ‘
 *13-20   TID
 *
 ********************************************************************/

static int gtp_parse_v0(GTPMsg *msg, const uint8_t *buff, uint16_t gtp_len)
{
    GTP_C_Hdr *hdr;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "This is a GTP v0 packet.\n"););

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_v0_DUMP,buff,gtp_len);
#endif

    hdr = (GTP_C_Hdr *) buff;

    msg->header_len = GTP_HEADER_LEN_V0;

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_HEADER_DUMP,msg->gtp_header,msg->header_len);
#endif

    /*Check the length field. */
    if (gtp_len != ((unsigned int)ntohs(hdr->length) + GTP_LENGTH_OFFSET_V0))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Calculated length %d != %d in header.\n",
                gtp_len - GTP_LENGTH_OFFSET_V0, ntohs(hdr->length)););
        ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
        return GTP_FAILURE;
    }

    return GTP_SUCCESS;
}

/********************************************************************
 * Function: gtp_parse_v1()
 *
 * process the GTP v1 message.
 *
 * Arguments:
 *  GTPMsg *   - gtp message
 *  char* buff - start of the gtp message buffer
 *  uint16_t   - length of the message
 *
 * Returns:
 *  GTP_FAILURE
 *  GTP_SUCCESS
 *
 * Octets  8   7   6   5   4   3   2   1
 * 1       Version     PT  (*) E   S   PN
 * 2       Message Type
 * 3       Length (1st Octet)
 * 4       Length (2nd Octet)
 * 5       Tunnel Endpoint Identifier (1st Octet)
 * 6       Tunnel Endpoint Identifier (2nd Octet)
 * 7       Tunnel Endpoint Identifier (3rd Octet)
 * 8       Tunnel Endpoint Identifier (4th Octet)
 * 9       Sequence Number (1st Octet)
 * 10      Sequence Number (2nd Octet)
 * 11      N-PDU Number
 * 12      Next Extension Header Type
 ********************************************************************/
static int gtp_parse_v1(GTPMsg *msg, const uint8_t *buff, uint16_t gtp_len)
{
    uint8_t  next_hdr_type;
    GTP_C_Hdr *hdr;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "This ia a GTP v1 packet.\n"););

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_v1_DUMP,buff,gtp_len);
#endif

    hdr = (GTP_C_Hdr *) buff;

    /*Check the length based on optional fields and extension header*/
    if (hdr->flag & 0x07)
    {

        msg->header_len = GTP_HEADER_LEN_V1;
        /*Check optional fields*/
        if (gtp_len < msg->header_len)
        {
            ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
            return GTP_FAILURE;
        }

        next_hdr_type = *(buff + msg->header_len - 1);

        /*Check extension headers*/
        while (next_hdr_type)
        {
            uint16_t ext_header_len;

            /*check length before reading data, at lease 4 bytes per extension header*/
            if (gtp_len < msg->header_len + 4)
            {
                ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
                return GTP_FAILURE;
            }

            ext_header_len = *(buff + msg->header_len);

            if (!ext_header_len)
            {
                ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
                return GTP_FAILURE;
            }

            /*Extension header length is a unit of 4 octets*/
            msg->header_len += ext_header_len*4;

            /*check length before reading data*/
            if (gtp_len < msg->header_len)
            {
                ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
                return GTP_FAILURE;
            }
            next_hdr_type = *(buff + msg->header_len - 1);
        }
    }
    else
        msg->header_len = GTP_HEADER_LEN_V1;

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_HEADER_DUMP,msg->gtp_header,msg->header_len);
#endif

    /*Check the length field. */
    if (gtp_len != ((unsigned int)ntohs(hdr->length) + GTP_LENGTH_OFFSET_V1))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Calculated length %d != %d in header.\n",
                gtp_len - GTP_LENGTH_OFFSET_V1, ntohs(hdr->length)););
        ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
        return GTP_FAILURE;
    }

    return GTP_SUCCESS;
}

/********************************************************************
 * Function: gtp_parse_v2()
 *
 * process the GTP v2 message.
 *
 * Arguments:
 *  GTPMsg *   - gtp message
 *  char* buff - start of the gtp message buffer
 *  uint16_t   - length of the message
 *
 * Returns:
 *  GTP_FAILURE
 *  GTP_SUCCESS
 *
 *Octets      8   7   6   5   4   3      2      1
 *1           Version     P   T   Spare  Spare  Spare
 *2           Message Type
 *3           Message Length (1st Octet)
 *4           Message Length (2nd Octet)
 *m to k(m+3) If T flag is set to 1, then TEID shall be placed into octets 5-8.
 *            Otherwise, TEID field is not present at all.
 *n to (n+2)  Sequence Number
 *(n+3)       Spare
 ********************************************************************/
static int gtp_parse_v2(GTPMsg *msg, const uint8_t *buff, uint16_t gtp_len)
{

    GTP_C_Hdr *hdr;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "This ia a GTP v2 packet.\n"););

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_v2_DUMP,buff,gtp_len);
#endif

    hdr = (GTP_C_Hdr *) buff;

    if (hdr->flag & 0x8)
        msg->header_len = GTP_HEADER_LEN_EPC_V2;
    else
        msg->header_len = GTP_HEADER_LEN_V2;

#ifdef DUMP_BUFFER
    dumpBuffer(GTP_HEADER_DUMP,msg->gtp_header,msg->header_len);
#endif

    /*Check the length field. */
    if (gtp_len != ((unsigned int)ntohs(hdr->length) + GTP_LENGTH_OFFSET_V2))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Calculated length %d != %d in header.\n",
                gtp_len - GTP_LENGTH_OFFSET_V2, ntohs(hdr->length)););
        ALERT(GTP_EVENT_BAD_MSG_LEN,GTP_EVENT_BAD_MSG_LEN_STR);
        return GTP_FAILURE;
    }

    return GTP_SUCCESS;
}

/********************************************************************
 * Function: gtp_parse()
 *
 * The main entry for parser: process the gtp messages.
 *
 * Arguments:
 *  GTPMsg *   - gtp message
 *  char* buff - start of the gtp message buffer
 *  uint16_t   - length of the message
 *
 * Returns:
 *  GTP_FAILURE
 *  GTP_SUCCESS
 ********************************************************************/
int gtp_parse(GTPMsg *msg, const uint8_t *buff, uint16_t gtp_len)
{

    int status;
    GTP_C_Hdr *hdr;
    GTP_MsgType *msgType;
    GTP_C_Hdr_v1_2 *hdrv1_2;

    /*Initialize key values*/

    status = GTP_SUCCESS;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Start parsing...\n"));

    hdrv1_2 = (GTP_C_Hdr_v1_2 *) buff;

    hdr = &(hdrv1_2->hdr);

    /*Check the length*/
    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Basic header length: %d\n", GTP_MIN_HEADER_LEN));
    if (gtp_len < GTP_MIN_HEADER_LEN)
        return GTP_FAILURE;

    /*The first 3 bits are version number*/
    msg->version = (hdr->flag & 0xE0) >> 5;
    msg->msg_type = hdr->type;
    msg->gtp_header = (uint8_t *)buff;

    if (msg->version > MAX_GTP_VERSION_CODE)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unsupported GTP version: %d!\n",msg->version););
        return GTP_FAILURE;
    }
    /*Check whether this is GTP or GTP', Exit if GTP'*/
    if (!(hdr->flag & 0x10))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unsupported GTP'!\n"););
        return GTP_FAILURE;
    }

    msgType = gtp_eval_config->msgTypeTable[msg->version][msg->msg_type];

    if ( NULL == msgType )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unsupported GTP message type: %d!\n",msg->msg_type););
        gtp_stats.unknownTypes++;
        return GTP_FAILURE;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "GTP version: %d, message type: %s(%d)\n",
                msg->version, msgType->name, msg->msg_type));
    }

    gtp_stats.messages[msg->version][msg->msg_type]++;
    /* We only care about control types*/
    if ( hdr->type == 255)
        return GTP_FAILURE;

    switch (msg->version)
    {
    case 0: /*GTP v0*/

        status = gtp_parse_v0(msg, buff, gtp_len);
        break;
    case 1: /*GTP v1*/
        if ((msg->msg_type > 3) && (hdrv1_2->teid == 0)) 
        {
            ALERT(GTP_TEID_MISSING, GTP_TEID_MISSING_STR);
        }
        status = gtp_parse_v1(msg, buff, gtp_len);
        break;

    case 2:/*GTP v2 */
        if ((msg->msg_type > 3) && (hdr->flag & 0x08) && (hdrv1_2->teid == 0)) 
        {
            ALERT(GTP_TEID_MISSING, GTP_TEID_MISSING_STR);
        }
        status = gtp_parse_v2(msg, buff, gtp_len);

        break;
    default:
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unknown protocol version.\n"););
        return GTP_FAILURE;

    }

    /*Parse information elements*/
    if ((msg->header_len < gtp_len)&& (GTP_SUCCESS == status))
    {
        msg->info_elements = gtp_ies;
        buff += msg->header_len;
        status = gtp_processInfoElements(msg, buff, (uint16_t)(gtp_len - msg->header_len));
    }
    return status;
}
/********************************************************************
 * Function: gtp_cleanInfoElements()
 *
 * Clean up the shared information elements table
 *
 * Arguments:
 *       None
 *
 * Returns:
 *       None
 ********************************************************************/

void gtp_cleanInfoElements(void)
{
    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Cleaned total bytes %d, length %d.\n",
            (MAX_GTP_IE_CODE + 1) * sizeof(GTP_IEData), sizeof(gtp_ies)););
    memset(gtp_ies, 0, sizeof(gtp_ies));
}
#endif
