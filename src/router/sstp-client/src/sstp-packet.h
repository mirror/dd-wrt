/*!
 * @brief The packet decoding / encoding related declarations
 *
 * @file sstp-packet.h
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

#ifndef __SSTP_PACKET_H__
#define __SSTP_PACKET_H__


/*< The protocol encapsulation, always PPP */
#define SSTP_ENCAP_PROTO_PPP    0x0001

/*< The hash protocol bit-mask: SHA1 */
#define SSTP_PROTO_HASH_SHA1      0x01

/*< The hash protocol bit-mask: SHA256 */
#define SSTP_PROTO_HASH_SHA256    0x02


/*!
 * @brief The message types per specification
 */
typedef enum
{
    SSTP_MSG_DATA             = 0x0000,
    SSTP_MSG_CONNECT_REQ      = 0x0001,
    SSTP_MSG_CONNECT_ACK      = 0x0002,
    SSTP_MSG_CONNECT_NAK      = 0x0003,
    SSTP_MSG_CONNECTED        = 0x0004,
    SSTP_MSG_ABORT            = 0x0005,
    SSTP_MSG_DISCONNECT       = 0x0006,
    SSTP_MSG_DISCONNECT_ACK   = 0x0007,
    SSTP_ECHO_REQUEST         = 0x0008,
    SSTP_ECHO_REPLY           = 0x0009,

} sstp_msg_t;


/*!
 * @brief The type of sstp message
 */
typedef enum
{
    SSTP_PKT_UNKNOWN      = 0,
    SSTP_PKT_DATA         = 1,
    SSTP_PKT_CTRL         = 2,

} sstp_pkt_t;


/*! 
 * @brief The defined attributes per specification
 */
typedef enum
{
    SSTP_ATTR_NO_ERROR        = 0x00,
    SSTP_ATTR_ENCAP_PROTO     = 0x01, 
    SSTP_ATTR_STATUS_INFO     = 0x02,
    SSTP_ATTR_CRYPTO_BIND     = 0x03,
    SSTP_ATTR_CRYPTO_BIND_REQ = 0x04,

    /*
     * Add additional attributes here
     */

    _SSTP_ATTR_MAX

} sstp_attr_t;

#define SSTP_ATTR_MAX   (_SSTP_ATTR_MAX - 1)

/*! 
 * @brief Help trace the packet
 */
#define sstp_pkt_trace(buf)     \
    if (SSTP_LOG_TRACE <= sstp_log_level()) \
    {                                       \
        sstp_pkt_dump(buf, __FILE__, __LINE__);    \
    }


/*!
 * @brief The defined status attributes per specificiation
 */
enum
{
    /*< Duplicate Attribute Received */
    SSTP_STATUS_DUPLICATE       = 0x01,
    
    /*< Unrecognized Attribute Received */
    SSTP_STATUS_UNRECOGNIZED    = 0x02,
    
    /*< Invalid attribute length */
    SSTP_STATUS_INVALID_LENGTH  = 0x03,

    /*< Value of an attribute not supported */
    SSTP_STATUS_VALUE_NOTSUP    = 0x04,

    /*< The attribute itself is not supported */
    SSTP_STATUS_ATTR_NOTSUP     = 0x09,

    /*< Expected attribute is missing */
    SSTP_STATUS_ATTR_MISSING    = 0x0a,

    /*< Invalid value of the STATUS INFO attribute */
    SSTP_STATUS_INFO_NOSUP      = 0x0b,
};


/*< Forward declare the pkt structure */
struct sstp_pkt;
typedef struct sstp_pkt sstp_pkt_st;


/*< Forward declare the attribute */
struct sstp_attr;
typedef struct sstp_attr sstp_attr_st;


/*! 
 * @brief Start writing a SSTP packet to the buffer
 */
status_t sstp_pkt_init(sstp_buff_st *buf, sstp_msg_t type);


/*!
 * @brief Append a SSTP attribute to the buffer
 */
status_t sstp_pkt_attr(sstp_buff_st *buf, sstp_attr_t type, 
        unsigned short len, void *data);


/*!
 * @brief Get a pointer to the raw data
 */
uint8_t *sstp_pkt_data(sstp_buff_st *buf);


/*!
 * @brief Return the length of the data section
 */
int sstp_pkt_data_len(sstp_buff_st *buf);


/*!
 * @brief Calculate the total length of the packet
 */
int sstp_pkt_len(sstp_buff_st *buf);


/*! 
 * @brief Find the packet and ctrl message given the buffer
 */
sstp_pkt_t sstp_pkt_type(sstp_buff_st *buf, sstp_msg_t *type);


/*!
 * @brief Update the header of this packet
 */
void sstp_pkt_update(sstp_buff_st *buf);


/*!
 * @brief Parse a attribute section
 */
status_t sstp_pkt_parse(sstp_buff_st *buff, size_t count, 
        sstp_attr_st *attrs[]);


/*!
 * @brief Return a pointer to the data in the attribute
 */
void *sstp_attr_data(sstp_attr_st *attr);


/*!
 * @brief Returns the real length of the data section
 */
int sstp_attr_len(sstp_attr_st *attr);


/*!
 * @brief Return the string representation of the status attribute
 */ 
const char *sstp_attr_status_str(int status);


void sstp_pkt_dump(sstp_buff_st *buf, const char *file, int line);

#endif /* #ifdef __SSTP_PACKET_H__ */
