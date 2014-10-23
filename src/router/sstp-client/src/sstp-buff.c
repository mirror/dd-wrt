/*!
 * @brief Buffer handling routines
 *
 * @file sstp-buff.c
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "sstp-private.h"


status_t sstp_http_get(sstp_buff_st *buf, int *code, int *count,
    http_header_st *array)
{
    int index  = 0;
    int ret    = 0;
    char *ptr1 = NULL;
    status_t status = SSTP_FAIL;

    /* Get the HTTP status code */
    *code = strtoul(buf->data + 9, NULL, 10);
    if (*code == -1 && errno == ERANGE)
    {
        goto done;
    }

    /* Skip the first line */
    ptr1 = strchr(buf->data, '\n');
    if (!ptr1)
    {
        goto done;
    }

    /* Iterate through the headers */
    do
    {
        ret = sscanf(ptr1+1, "%[^:]: %[^\r\n]", array[index].name, 
                array[index].value);
        if (ret != 2)
        {
            break;
        }

        ptr1 = strchr(ptr1+1, '\n');
        if (index++ > *count)
        {
            break;
        }

    } while (ptr1 && ptr1[1] != '\r' && ptr1[1] != '\n');

    /* Save the number of headers */
    *count = index;

    /* Success! */
    status = SSTP_OKAY;

done:


    return status;
}


http_header_st *sstp_http_get_header(const char *name, int count, 
    http_header_st *array)
{
    int index = 0;

    for (index = 0; index < count; index++)
    {
        if (strcasecmp(name, array[index].name))
        {
            continue;
        }

        return &array[index];
    }

    return NULL;
}


status_t sstp_buff_space(sstp_buff_st *buf, int length)
{
    if (buf->max < (buf->len + length))
    {
        return SSTP_FAIL;       
    }

    return SSTP_OKAY;
}


void sstp_buff_reset(sstp_buff_st *buf)
{
    buf->len = 0;
    buf->off = 0;
}


void *sstp_buff_data(sstp_buff_st *buf, int index)
{
    return (&buf->data[index]);
}


status_t sstp_buff_print(sstp_buff_st *buf, const char *fmt, ...)
{
    va_list list;
    int ret;
    
    va_start(list, fmt);
    ret = vsnprintf(buf->data + buf->len, buf->max - buf->len, fmt, list);
    va_end(list);

    if (ret <= 0 || ret > (buf->max - buf->len))
    {
        return SSTP_OVERFLOW;
    }

    buf->len += ret;
    return SSTP_OKAY;
}


status_t sstp_buff_create(sstp_buff_st **buf, int size)
{
    /* Allocate the memory */
    sstp_buff_st *ctx = calloc(1, sizeof(sstp_buff_st) + size);
    if (!ctx)
    {
        return SSTP_FAIL;
    }

    /* Configure the buffer */
    ctx->max = size;
    ctx->len = 0;
    ctx->off = 0;
    *buf = ctx;

    /* Success! */
    return SSTP_OKAY;
}


void sstp_buff_destroy(sstp_buff_st *buf)
{
    if (!buf)
    {
        return;
    }

    free(buf);
}
