/*!
 * @brief Routines for handling CHAP authentication.
 *
 * @file sstp-chap.c
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

#ifndef __SSTP_CHAP_H__
#define __SSTP_CHAP_H__


#define SSTP_CHAP_SENDING   0x01
#define SSTP_CHAP_SERVER    0x02


/*!
 * @brief The data snooped from pppd
 */
typedef struct sstp_chap
{
    /* The challenge field */
    unsigned char challenge[16];

    /*! The response field */
    unsigned char response[8];

    /*! The NT Response field */
    unsigned char nt_response[24];

    /*! Any flags */
    unsigned char flags[1];

} __attribute__((packed)) sstp_chap_st;
    

/*! 
 * @brief Takes the CHAP context and generate the MPPE key
 *
 * @param ctx   The ms-chap hanshake context
 * @param password  The user's password
 * @param skey      The resulting MPEE send key
 * @param rkey      The resulting MPPE receive key
 * @param server    Are we acting as a server?
 *
 * @retval 0: success, -1: failure
 */
int sstp_chap_mppe_get(sstp_chap_st *ctx, const char *password, 
        uint8_t skey[16], uint8_t rkey[16], char server);
 
#endif
