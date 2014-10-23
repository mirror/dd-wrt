/*!
 * @brief The packet decoding / encoding related declarations
 *
 * @file sstp-packet.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <config.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "sstp-private.h"

/*< Support of SSTP protocol v1.0 */
#define SSTP_PROTO_VER        0x10

/*< The flag signifying a control message */
#define SSTP_MSG_FLAG_CTRL    0x01


/*!
 * @brief The SSTP packet header
 */
struct sstp_pkt
{
    /*< The sstp header version */
    uint8_t version;

    /*< 7 reserved bits, 1 ctrl bit field */
    uint8_t flags;

    /*< The length of the entire packet */
    uint16_t length;

    /*< The data packet would contain data from this point on */
    char data[0];
};


/*!
 * @brief The attribute of a control message
 */
struct sstp_attr
{
    /*< Reserved field */
    uint8_t reserved;

    /*< The attribute id */
    uint8_t type;

    /*< 4 reserved bits, 12 LSB is the length */
    uint16_t length;

    /*< The data pointer */
    char data[0];
};


/*!
 * @brief The control packet
 */
typedef struct
{
    /*< The ctrl type */
    uint16_t type;

    /*< The number of attributes */
    uint16_t nattr;

    /*< The data following the ctrl header */
    char data[0];

} sstp_ctrl_st;


status_t sstp_pkt_init(sstp_buff_st *buf, sstp_msg_t type)
{
    sstp_pkt_st *pkt = NULL;
    status_t status  = SSTP_FAIL;
    short length = 0;

    /* Reset the buffer position */
    sstp_buff_reset(buf);

    /* Verify that we have at space left */
    length = sizeof(sstp_pkt_st);
    if (sstp_buff_space(buf, length))
    {
        goto done;
    }

    /* Set the version, and flags */
    pkt = (sstp_pkt_st*) &buf->data;
    pkt->version = SSTP_PROTO_VER;
    pkt->flags   = (type != SSTP_MSG_DATA)
        ? SSTP_MSG_FLAG_CTRL
        : 0;

    /* Handle Control Messages */
    if (SSTP_MSG_DATA != type)
    {
        sstp_ctrl_st *ctrl = NULL;
        
        /* Verify that we have space left */
        length += sizeof(sstp_ctrl_st);
        if (sstp_buff_space(buf, length))
        {
            goto done;
        }
        
        /* Set the default control fields */
        ctrl = (sstp_ctrl_st*) &pkt->data;
        ctrl->type   = htons(type);
        ctrl->nattr  = 0;
    }

    /* Track the position in the buffer */
    buf->len    = length;
    pkt->length = htons(length);

    /* Success! */
    status = SSTP_OKAY;

done:

    return status;
}


status_t sstp_pkt_attr(sstp_buff_st *buf, sstp_attr_t type, 
    unsigned short len, void *data)
{
    sstp_pkt_st  *pkt  = NULL;
    sstp_ctrl_st *ctrl = NULL;
    sstp_attr_st *attr = NULL;
    status_t status    = SSTP_FAIL;
    short length       = 0;

    /* Verify that we have space left */
    length = len + sizeof(sstp_attr_st);
    if (sstp_buff_space(buf, length))
    {
        goto done;
    }

    /* Attributes applies to Control Packets only */
    pkt = (sstp_pkt_st*) &buf->data[0];
    if (!(SSTP_MSG_FLAG_CTRL & pkt->flags))
    {
        goto done;
    }

    /* Update the number of attributes section */
    ctrl = (sstp_ctrl_st*) pkt->data;
    ctrl->nattr = htons(ntohs(ctrl->nattr) + 1);

    /* Append the attribute to the end of the stream */
    attr = (sstp_attr_st*) &buf->data[buf->len];
    attr->reserved = 0;
    attr->type   = type;
    attr->length = htons(length);
    memcpy(attr->data, data, len);

    /* Update the total length */
    buf->len += length;

    /* Update the packet header */
    pkt->length = htons(buf->len);

    /* Success */
    status = SSTP_OKAY;

done:

    return status;
}


uint8_t *sstp_pkt_data(sstp_buff_st *buf)
{
    sstp_pkt_st *pkt = (sstp_pkt_st*) buf->data;
    if (!(SSTP_MSG_FLAG_CTRL & pkt->flags))
    {
        return ((uint8_t*)buf->data + sizeof(sstp_pkt_st));
    }

    /*
     * Return the pointer after the attribute section?
     */
    return NULL;
}


int sstp_pkt_data_len(sstp_buff_st *buf)
{
    sstp_pkt_st *pkt = (sstp_pkt_st*) buf->data;
    if (!(SSTP_MSG_FLAG_CTRL & pkt->flags))
    {
        return (buf->len - sizeof(sstp_pkt_st));
    }

    /*
     * Return the pointer after the attribute section?
     */
    return 0;
}


int sstp_pkt_len(sstp_buff_st *buf)
{
    sstp_pkt_st *pkt = (sstp_pkt_st*) buf->data;
    return ntohs(pkt->length);
}


void sstp_pkt_update(sstp_buff_st *buf)
{
    sstp_pkt_st *pkt = (sstp_pkt_st*) &buf->data[0];
    pkt->length = htons(buf->len);
}


sstp_pkt_t sstp_pkt_type(sstp_buff_st *buf, sstp_msg_t *type)
{
    sstp_pkt_st *pkt = NULL;

    /* Can we determine the packet type? */
    if (buf->len < sizeof(sstp_pkt_st))
    {
        return SSTP_PKT_UNKNOWN;
    }

    /* Check if this is a control packet */
    pkt = (sstp_pkt_st*) &buf->data[0];
    if (SSTP_MSG_FLAG_CTRL & pkt->flags)
    {
        sstp_ctrl_st *ctrl = (sstp_ctrl_st*) &pkt->data[0];
        if (type != NULL)
        {
            *type = ntohs(ctrl->type);
        }

        return SSTP_PKT_CTRL;
    }
    
    /* Not a control packet */
    return SSTP_PKT_DATA;
}


status_t sstp_pkt_parse(sstp_buff_st *buf, size_t count,
    sstp_attr_st *attrs[])
{
    sstp_pkt_st *pkt   = NULL;
    sstp_ctrl_st *ctrl = NULL;

    short alen   = 0;
    short index  = 0;
    short length = 0;

    status_t status = SSTP_FAIL;
    
    /* Get the minimum length of the packet */
    length = sizeof(sstp_pkt_st)  +
             sizeof(sstp_ctrl_st) +
             sizeof(sstp_attr_st) + 2 ;
    if (buf->len < length)
    {
        goto done;
    }

    /* Check if it is a control packet */
    pkt    = (sstp_pkt_st*) sstp_buff_data(buf, index);
    index += sizeof(sstp_pkt_st);
    if (!(SSTP_MSG_FLAG_CTRL & pkt->flags))
    {
        goto done;
    }

    /* Get the number of attributes */
    ctrl   = (sstp_ctrl_st*) sstp_buff_data(buf, index);
    index += sizeof(sstp_ctrl_st);
    alen   = ntohs(ctrl->nattr);
    
    /* Reset the pointers */
    memset(attrs, 0, sizeof(sstp_attr_st*) * SSTP_ATTR_MAX);
    while (alen--)
    {
        sstp_attr_st *entry = (sstp_attr_st*) 
                sstp_buff_data(buf, index);

        if (SSTP_ATTR_MAX < entry->type)
        {
            goto done;
        }

        /* Setup the return value */
        attrs[entry->type] = entry;
        index = ntohs(entry->length);
    }

    /* This is where we left of reading */
    buf->off = index;

    /* Success! */
    status = SSTP_OKAY;

done:
    
    return status;
}


void *sstp_attr_data(sstp_attr_st *attr)
{
    return ((char*)attr + sizeof(sstp_attr_st));
}


int sstp_attr_len(sstp_attr_st *attr)
{
    return (ntohs(attr->length) - sizeof(sstp_attr_st));
}


const char *sstp_attr_status_str(int status)
{
    const char *retval = NULL;

    switch (status)
    {
    case SSTP_STATUS_DUPLICATE:
        retval = "Received Duplicate Attribute";
        break;

    case SSTP_STATUS_UNRECOGNIZED:
        retval = "Unrecognized Attribute";
        break;

    case SSTP_STATUS_INVALID_LENGTH:
        retval = "Invalid Attribute Length";
        break;
    
    case SSTP_STATUS_VALUE_NOTSUP:
        retval = "Value of attribute is incorrect";
        break;

    case SSTP_STATUS_ATTR_NOTSUP:
        retval = "Attribute is invalid or not supported";
        break;

    case SSTP_STATUS_ATTR_MISSING:
        retval = "Attribute is missing";
        break;
    
    case SSTP_STATUS_INFO_NOSUP:
        retval = "Invalid info attribute";
        break;

    default:
        retval = "Unknown Status Attribute";
        break;
    }
    
    return retval;
}


void sstp_pkt_dump(sstp_buff_st *buf, const char *file, int line)
{
    sstp_pkt_st *pkt   = NULL;
    sstp_ctrl_st *ctrl = NULL;
    int type  = 0;
    int alen  = 0;
    int index = 0;
    int length= 0;
    int i = 0;

    static const char *sstp_msg_type[] =
    {
        NULL,
        "CONNECT REQUEST",
        "CONNECT ACK",
        "CONNECT NAK",
        "CONNECTED",
        "ABORT",
        "DISCONNECT",
        "DISCONNECT ACK",
        "ECHO REQUEST",
        "ECHO REPLY",
    };

    static const char *sstp_attr_type[] = 
    {
        "NO ERROR",
        "ENCAP PROTO",
        "STATUS INFO",
        "CRYPTO BIND",
        "CRYPTO BIND REQ"
    };

    char hex[255] = {};

    pkt    = (sstp_pkt_st*) sstp_buff_data(buf, index);
    index += (sizeof(sstp_pkt_st));

    /* Packet Type / Length */
    sstp_log_msg(SSTP_LOG_TRACE, file, line, "SSTP %s PKT(%d) ", 
        (SSTP_MSG_FLAG_CTRL & pkt->flags) ? "CRTL" : "DATA", 
        (ntohs(pkt->length)));

    /* Handle control packets */
    if (SSTP_MSG_FLAG_CTRL & pkt->flags)
    {
        ctrl   = (sstp_ctrl_st*) sstp_buff_data(buf, index);
        index += (sizeof(sstp_ctrl_st));
        type   = (ntohs(ctrl->type));
        alen   = (ntohs(ctrl->nattr));

        /* Control Type, num attributes */
        sstp_log_msg(SSTP_LOG_TRACE, file, line, "  TYPE(%d): %s, ATTR(%d):",
            type, sstp_msg_type[type], alen);

        while (alen--)
        {
            sstp_attr_st *attr = (sstp_attr_st*) 
                    sstp_buff_data(buf, index);

            if (SSTP_ATTR_MAX < attr->type)
            {
                return;
            }

            sstp_log_msg(SSTP_LOG_TRACE, file, line, "    %s(%d): %d",
                sstp_attr_type[attr->type], attr->type,
                ntohs(attr->length));
            index = ntohs(attr->length);
        }
    }

    /* Only if dump was specified */
    if (SSTP_LOG_DUMP > sstp_log_level())
    {
        return;
    }

    /* Dump the message */
    length = ntohs(pkt->length);
    for (index = 0; index < length; index += 16)
    {
        int offset = 0;
        for (i = 0; i < 16 && (index+i) < length; i++)
        {
            int len = sprintf(&hex[offset], "0x%02x ", (buf->data[index+i]) & 0xFF);
            if (len < 0 || len > (sizeof(hex)-offset))
            {
                return;
            }

            offset += len;
        }

        sstp_log_msg(SSTP_LOG_TRACE, file, line, "  %s", hex);
    }
}


