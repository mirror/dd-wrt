/*!
 * @brief Implements output to file functions for sstp-client.
 *
 * @file sstp-log-file.c
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
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstp-common.h>
#include <sstp-log.h>
#include "sstp-log-private.h"


static void sstp_logfile_write(log_ctx_st *ctx, log_msg_st *msg, 
    log_attr_st *table[])
{
    char buf[128];
    struct iovec vec[3];
    int ret = 0;
    int cnt = 0;
    int len = 0;
    log_attr_st *attr = NULL;
 
    /* Re-open file if necessary */
    if (ctx->sock < 0)
    {
        ret = sstp_logfile_init(ctx);
        if (SSTP_OKAY != ret)
        {
            return;
        }
    }

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

    /* Add a line terminator */
    vec[cnt  ].iov_base = "\n";
    vec[cnt++].iov_len  = 1;

    /* Write the message to the file descriptor */
    ret = writev(ctx->sock, vec, cnt);
    if (ret != 0)
    {
        return;
    }

    /* Sync data to disk */
    fsync(ctx->sock);
}


static void sstp_logfile_close(log_ctx_st *ctx)
{
    if (ctx->sock >= 0)
    {
        close(ctx->sock);
        ctx->sock = -1;
    }
}


status_t sstp_logfile_init(log_ctx_st *ctx)
{
    int retval  = SSTP_FAIL;
    int flags   = O_TRUNC | O_WRONLY | O_CREAT ;
    mode_t mode = S_IRUSR | \
                  S_IWUSR | \
                  S_IRGRP | \
                  S_IROTH;

    /* Open file for writing */
    ctx->sock = open(ctx->file, flags, mode);
    if (ctx->sock <= 0)
    {
        goto done;
    }

    /* Setup callback */
    ctx->write = sstp_logfile_write;
    ctx->close = sstp_logfile_close;

    /* Success */
    retval = SSTP_OKAY;

done:

    return retval;
}

