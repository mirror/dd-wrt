/*!
 * @brief Plugin for pppd to relay the MPPE keys to sstp-client
 *
 * @file sstp-plugin.c
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <pppd/pppd.h>
#include <sstp-api.h>

#ifndef MPPE
#define MPPE_MAX_KEY_LEN 16
extern u_char mppe_send_key[MPPE_MAX_KEY_LEN];
extern u_char mppe_recv_key[MPPE_MAX_KEY_LEN];
extern int mppe_keys_set;
#endif
#define SSTP_MAX_BUFLEN             255

static int sstp_notify_sent = 0;

/*!
 * @brief PPP daemon requires this symbol to be exported
 */
const char pppd_version [] = VERSION;

/*! The socket we send sstp-client our MPPE keys */
static char sstp_sock[SSTP_MAX_BUFLEN+1];

/*! Set of options required for this module */
static option_t sstp_option [] = 
{
    { "sstp-sock", o_string, &sstp_sock, 
      "Set the address of the socket to connect back to sstp-client",
      OPT_PRIO | OPT_PRIV | OPT_STATIC, NULL, SSTP_MAX_BUFLEN
    }
};


/*!
 * @brief Exchange the MPPE keys with sstp-client
 */
static void sstp_send_notify(unsigned char *skey, int slen, 
    unsigned char *rkey, int rlen)
{
    struct sockaddr_un addr;
    int ret  = (-1);
    int sock = (-1);
    int alen = (sizeof(addr));
    uint8_t buf[SSTP_MAX_BUFLEN+1];
    sstp_api_msg_st  *msg  = NULL;

    /* Open the socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        fatal("Could not open socket to communicate with sstp-client");
    }

    /* Setup the address */
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sstp_sock, sizeof(addr.sun_path));

    /* Connect the socket */
    ret = connect(sock, (struct sockaddr*) &addr, alen);
    if (ret < 0)
    {
        fatal("Could not connect to sstp-client (%s), %s (%d)", sstp_sock,
            strerror(errno), errno);
    }

    /* Create a new message */
    msg = sstp_api_msg_new(buf, SSTP_API_MSG_AUTH);

    /* Add the MPPE Send Key */
    sstp_api_attr_add(msg, SSTP_API_ATTR_MPPE_SEND, 
            MPPE_MAX_KEY_LEN, skey);

    /* Add the MPPE Recv Key */
    sstp_api_attr_add(msg, SSTP_API_ATTR_MPPE_RECV, 
            MPPE_MAX_KEY_LEN, rkey);

    /* Send the structure */
    ret = send(sock, msg, sstp_api_msg_len(msg), 0);
    if (ret < 0)
    {
        fatal("Could not send data to sstp-client");
    }
    
    /* Wait for the ACK to be received */
    ret = recv(sock, msg, (sizeof(*msg)), 0);
    if (ret < 0 || ret != (sizeof(*msg)))
    {
        fatal("Could not wait for ack from sstp-client");
    }

    /* We have communicated the keys */
    sstp_notify_sent = 1;

    /* Close socket */
    close(sock);
}


/*!
 * @brief Make sure we send notification, if we didn't snoop MSCHAPv2
 * 
 * @par Note:
 *  IF MPPE was enabled, the keys have been zeroed out for security
 *  reasons. 
 *
 *  You can configure PAP, CHAP-MD5 and MSCHAP with the NAP service,
 *  these are disabled by Microsoft 2008 server by default.
 */
static void sstp_ip_up(void *arg, int dummy)
{
    if (sstp_notify_sent)
        return;

    /* Auth-Type is not MSCHAPv2, reset the keys and send blank keys */
    if (!mppe_keys_set)
    {
        memset(&mppe_send_key, 0, sizeof(mppe_send_key));
        memset(&mppe_recv_key, 0, sizeof(mppe_recv_key));
    }

    /* Send the MPPE keys to the sstpc client */
    sstp_send_notify(mppe_send_key, sizeof(mppe_send_key),
            mppe_recv_key, sizeof(mppe_recv_key));
}


/*!
 * @brief Snoop the Authentication complete packet, steal MPPE keys
 */
static void sstp_snoop_send(unsigned char *buf, int len)
{
    uint16_t protocol;

    /* Skip the HDLC header */
    buf += 2;
    len -= 2;

    /* Too short of a packet */
    if (len <= 0)
        return;

    /* Stop snooping if it is not a LCP Auth Chap packet */
    protocol = (buf[0] & 0x10) ? buf[0] : (buf[0] << 8 | buf[1]);
    if (protocol != 0xC223)
        return;

    /* Skip the LCP header */
    buf += 2;
    len -= 2;

    /* Too short of a packet */
    if (len <= 0)
        return;

    /* Check if packet is a CHAP response */
    if (buf[0] != 0x02)
        return;

    /* We should send sstpc empty keys .. */
    if (!mppe_keys_set)
    {
        return;
    }

    /* ChapMS2/ChapMS sets the MPPE keys as a part of the make_response
     * call, these might not be enabled dependent on negotiated options
     * such as MPPE and compression. If they are enabled, the keys are 
     * zeroed out in ccp.c before ip-up is called.
     * 
     * Let's steal the keys here over implementing all the code to
     * calculate the MPPE keys here.
     */
    if (debug)
    {
        char key[255];
        dbglog("%s: mppe keys are set", __func__);

        /* Add the MPPE Send Key */
        slprintf(key, sizeof(key)-1, "%0.*B", MPPE_MAX_KEY_LEN,
                 mppe_send_key);
        dbglog("%s: The mppe send key: %s", __func__, key);

        /* Add the MPPE Recv Key */
        slprintf(key, sizeof(key)-1, "%0.*B", MPPE_MAX_KEY_LEN,
                 mppe_recv_key );
        dbglog("%s: The mppe recv key: %s", __func__, key);
    }

    /* Send the MPPE keys to the sstpc client */
    sstp_send_notify(mppe_send_key, sizeof(mppe_send_key),
            mppe_recv_key, sizeof(mppe_recv_key));
}


/*!
 * @brief PPP daemon requires this symbol to be exported for initialization
 */
void plugin_init(void)
{
    /* Clear memory */
    memset(&sstp_sock, 0, sizeof(sstp_sock));

    /* Allow us to intercept options */
    add_options(sstp_option);

    /* Let's snoop for CHAP authentication */
    snoop_send_hook = sstp_snoop_send;

    /* Add ip-up notifier */
    add_notifier(&ip_up_notifier, sstp_ip_up, NULL);
}


