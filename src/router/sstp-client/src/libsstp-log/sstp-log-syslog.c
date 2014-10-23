/*!
 * @brief Implements output to syslog for sstp-client.
 *
 * @file sstp-log-syslog.c
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
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstp-common.h>
#include <sstp-log.h>
#include "sstp-log-private.h"


/*! 
 * @brief Write a syslog message to the /dev/log socket
 *
 * @par Note:
 *  There is probably little value in doing all this work over using the syslog
 *  function, but in the future we could switch to remote syslog and use the 
 *  RFC5424 format. This is just a placeholder for now...
 */
static void sstp_syslog_write(log_ctx_st *ctx, log_msg_st *msg, 
    log_attr_st *table[])
{
    char buf[128];
    struct iovec vec[2];
    int ret = (-1);
    int cnt = ( 0);
    int len = ( 0);
    log_attr_st *attr = NULL;

    /* In case this was closed */
    if (ctx->sock < 0)
    {
        ret = sstp_syslog_init(ctx);
        if (SSTP_OKAY != ret)
        {
            return;
        }
    }

    /* Configure the log-level of the message */
    len += sprintf(buf + len, "<%d>", LOG_LOCAL0 | ((7 - msg->msg_level) & 0x07));

    /* Get the time stamp */
    attr = table[LOG_ATTR_TIME];
    if (!attr)
    {
        return;
    }
    len += sprintf(buf + len, "%s ", attr->attr_data);

    /* Get the application name */
    attr = table[LOG_ATTR_APPNAME];
    if (!attr)
    {
        return;
    }
    len += sprintf(buf + len, "%s[%d]: ", attr->attr_data, getpid());

    /* Get the line information */
    if (ctx->debug)
    {
        attr = table[LOG_ATTR_LINEINFO];
        if (!attr)
        {
            return;
        }
        len += sprintf(buf + len, "%s ", attr->attr_data);
    }

    /* Get the message attribute */
    attr = table[LOG_ATTR_MESSAGE];
    if (!attr)
    {
        return;
    }

    /* Add the syslog header */
    vec[cnt  ].iov_base = buf;
    vec[cnt++].iov_len  = strlen(buf);
    
    /* Add the message */
    vec[cnt  ].iov_base = attr->attr_data;
    vec[cnt++].iov_len  = attr->attr_len;

    /* Write the iovec structure to socket */
    ret = writev(ctx->sock, vec, cnt);
    if (ret == -1)
    {
        ctx->close(ctx);
    }
}


/*!
 * @brief Close the output module
 */
static void sstp_syslog_close(log_ctx_st *ctx)
{
    if (ctx->sock >= 0)
    {
        close(ctx->sock);
        ctx->sock = -1;
    }
}


/*!
 * @brief Create a new syslog socket preparing the log_ctx structure
 */
status_t sstp_syslog_init(log_ctx_st *ctx)
{
    struct sockaddr_un addr;
    int retval = SSTP_FAIL;
    int ret = (-1);

    /* Create a unix domain socket */
    ctx->sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (ctx->sock <= -1)
    {
        goto done;
    }
    
    /* Configure the address */
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ctx->file, sizeof(addr.sun_path)-1);

    /* Connect the socket */
    ret = connect(ctx->sock, (struct sockaddr*) &addr, sizeof(addr));
    if (ret != 0)
    {
        goto done;
    }

    /* Configure the write/close callback */
    ctx->write = sstp_syslog_write;
    ctx->close = sstp_syslog_close;

    /* Success */
    retval = SSTP_OKAY;

done:

    /* In case of failure ... */
    if (retval != SSTP_OKAY)
    {
        if (ctx->sock >= 0)
        {
            close(ctx->sock);
            ctx->sock = -1;
        }
    }

    return retval;
}

