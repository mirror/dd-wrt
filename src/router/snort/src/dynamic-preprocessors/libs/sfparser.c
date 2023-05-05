/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc.
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
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "sf_types.h"
#include "sfcommon.h"
#include "ctype.h"

#define SET_ERR(x,y) \
    if(errstr && \
        snprintf((char*)errstr, SFP_MIN_ERR_STR, x, y) >= \
        SFP_MIN_ERR_STR) \
    { \
        /* tok exceeded errstr.  Overwrite trailing characters for \
         * printability */ \
        strcpy(((char*)errstr) + SFP_MIN_ERR_STR-4, "..."); \
    }

#define CLR_ERR() ((char*)errstr)[0] = 0;

SFP_ret_t SFP_ports(ports_tbl_t port_tbl, char *str, SFP_errstr_t errstr) {
    char *tok;
    char *saveptr;
    char end_brace_found = 0;
    char port_found = 0;

    if(!str)
    {
        SET_ERR("%s", "Invalid pointer");
        return SFP_ERROR;
    }

    if((tok = strtok_r(str, " ", &saveptr)) == NULL)
    {
        SET_ERR("%s", "No ports specified");
        return SFP_ERROR;
    }

    /* This string had better start with a '{' and end with a '}', or else! */
    if(strcmp(tok, "{"))
    {
        SET_ERR("Malformed port list: %s. Expecting a leading '{ '", tok);
        return SFP_ERROR;
    }

    while((tok = strtok_r(NULL, " ", &saveptr)) != NULL)
    {
        char *port_end;
        long int port;

        str = NULL;

        if(end_brace_found)
        {
            SET_ERR("Last character of a port list must be '}': %s", tok);
            return SFP_ERROR;
        }

        if(!strcmp(tok, "}"))
        {
            end_brace_found = 1;
            continue;
        }
        errno = 0;
        port = strtol(tok, &port_end, 10);

        if((port_end == tok) ||
           (*port_end && *port_end != '}') ||
           (errno == ERANGE))
        {
            SET_ERR("Unable to parse: %s", tok);
            return SFP_ERROR;
        }

        if(port < 0 || port > MAXPORTS-1)
        {
            SET_ERR("Port out of range: %s", tok);
            return SFP_ERROR;
        }

        port_tbl[ PORT_INDEX(port) ] |= CONV_PORT(port);
        port_found = 1;
    }

    if(!end_brace_found)
    {
        SET_ERR("%s", "No end brace found");
        return SFP_ERROR;
    }

    if(!port_found)
    {
        SET_ERR("%s", "No ports specified");
        return SFP_ERROR;
    }

    CLR_ERR();
    return SFP_SUCCESS;
}

SFP_ret_t SFP_snprintfa(char *buf, size_t buf_size, const char *format, ...)
{
    size_t str_len;
    int ret;
    va_list ap;

    if (buf == NULL || buf_size <= 0 || format == NULL)
        return SFP_ERROR;

    /* equivalent of a strlen that stops at buf_size */
    for(str_len = 0; (str_len < buf_size) && buf[str_len]; str_len++)
        ;

    /* Note: this is from the original SnortSnprintfAppend */
    /* "since we've already checked buf and buf_size an error
     * indicates no null termination, so just start at
     * beginning of buffer" */
    if(str_len >= buf_size)
    {
        buf[0] = '\0';
        str_len = 0;
    }

    buf[buf_size - 1] = '\0';

    va_start(ap, format);

    ret = vsnprintf(buf + str_len, buf_size - str_len, format, ap);

    va_end(ap);

    if (ret < 0)
        return SFP_ERROR;

    if (buf[buf_size - 1] != '\0' || (size_t)ret >= buf_size)
    {
        /* truncation occured */
        buf[buf_size - 1] = '\0';
        return SFP_ERROR;
    }

    return SFP_SUCCESS;
}

