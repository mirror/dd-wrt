/*!
 * @brief Declarations for libsstp-api
 *
 * @file sstp-api.h
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
#ifndef __SSTP_API_H__
#define __SSTP_API_H__


/*! Extern declarations for export of functions */
#define SSTP_API                    extern

/*! The message signature */
#define SSTP_API_MSG_MAGIC          0x73737470

/*! Align to every 4 byte boundary */
#define ALIGN32(n)                  (((n) + 3) & ~3)


/*! Message Type */
typedef enum
{
    SSTP_API_MSG_UNKNOWN = 0,
    SSTP_API_MSG_AUTH    = 1,
    SSTP_API_MSG_ADDR    = 2,
    SSTP_API_MSG_ACK     = 3,

    /*
     * Add more event message types here
     */
    _SSTP_API_MSG_MAX,

} sstp_api_msg_t;

#define SSTP_API_MSG_MAX  (_SSTP_API_MSG_MAX -1)


/*! Attribute Types */
typedef enum
{
    SSTP_API_ATTR_UNKNOWN   = 0,
    SSTP_API_ATTR_MPPE_SEND = 1,
    SSTP_API_ATTR_MPPE_RECV = 2,
    SSTP_API_ATTR_GATEWAY   = 3,
    SSTP_API_ATTR_ADDR      = 4,

    /*
     * Add more attribute type here
     */
    _SSTP_API_ATTR_MAX,

} sstp_api_attr_t;

#define SSTP_API_ATTR_MAX (_SSTP_API_ATTR_MAX-1)


/*!
 * @brief The API message
 */
typedef struct
{
    /*< The signature of the message */
    uint32_t msg_magic;

    /*< The length of the entire payload */
    uint16_t msg_len;

    /*< The type of the message */
    uint16_t msg_type;

    /*< The payload */
    uint8_t  msg_data[0];

} sstp_api_msg_st;


/*!
 * @brief The API attribute
 */
typedef struct
{
    /* The attribute type */
    uint16_t attr_type;

    /*< The attribute length */
    uint16_t attr_len;

    /*< The attribute payload */
    uint8_t  attr_data[0];

} sstp_api_attr_st;


/*!
 * @brief Provide a buffer, and convert it into a message structure
 */
SSTP_API 
sstp_api_msg_st *sstp_api_msg_new(unsigned char *buf, sstp_api_msg_t type);


/*!
 * @brief Get the length of the message
 */
SSTP_API 
int sstp_api_msg_len(sstp_api_msg_st *msg);


/*!
 * @brief Get the type of the message
 */
SSTP_API 
int sstp_api_msg_type(sstp_api_msg_st *msg, sstp_api_msg_t *type);


/*!
 * @brief Append an attribute to the message
 */
SSTP_API 
void sstp_api_attr_add(sstp_api_msg_st *msg, sstp_api_attr_t type,
        unsigned int len, void *data);


/*!
 * @brief Parse the attributes out of a messagge
 */
SSTP_API 
int sstp_api_attr_parse(char *payload, int length, sstp_api_attr_st *list[], 
        int count);


#endif  /* #ifndef __SSTP_API_H__ */
