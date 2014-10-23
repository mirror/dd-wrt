/*!
 * @brief Implements output to stdout/err functions for sstp-client.
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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstp-common.h>
#include <sstp-log.h>
#include "sstp-log-private.h"


/*! 
 * @brief Write a log message to the standard out/err file descriptor
 */
static void sstp_logstd_write(log_ctx_st *ctx, log_msg_st *msg, 
    log_attr_st *table[])
{
    char buf[128];
    struct iovec vec[3];
    int ret = 0;
    int cnt = 0;
    int len = 0;
    log_attr_st *attr = NULL;
    
    /* Get the time stamp */
    attr = table[LOG_ATTR_TIME];
    if (!attr)
    {
        return;
    }
    len += sprintf(buf + len, "%s ", attr->attr_data);

    /* Get the name and pid */
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

    /* Get the message */
    attr = table[LOG_ATTR_MESSAGE];
    if (!attr)
    {
        return;
    }

    /* Setup the iovec structure */
    vec[cnt  ].iov_base = buf;
    vec[cnt++].iov_len  = strlen(buf);
    
    /* Get the attribute length */
    vec[cnt  ].iov_base = attr->attr_data;
    vec[cnt++].iov_len  = attr->attr_len;

    /* Add the line terminator */
    vec[cnt  ].iov_base = "\n";
    vec[cnt++].iov_len  = 1;

    /* Write the message to the file descriptor */
    ret = writev(ctx->sock, vec, cnt);
    if (ret != 0)
    {
        return;
    }
}


status_t sstp_logstd_init(log_ctx_st *ctx)
{
    /* 
     * At this point the ctx->sock have been setup for us 
     */

    /* Configure callback */
    ctx->write = sstp_logstd_write;
    ctx->close = NULL;

    return SSTP_OKAY; 
}
